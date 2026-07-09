#include "data/port.hpp"
#include "os/os.hpp"


#include "data/connection.hpp"
#include "data/inspector.hpp"
#include "data/parser.hpp"
#include "memory/buffer/buffer.hpp"
#include "memory/buffer/buffer-manager.hpp"
#include "configuration/parameters/configuration-parameter-list-sorted.hpp"


namespace adam 
{
    const configuration_parameter_list& port::get_default_parameters()
    {
        static adam::configuration_parameter_list_sorted user_params = []() 
        {
            adam::configuration_parameter_list_sorted p;
            return p;
        }();

        static adam::configuration_parameter_list params = []() 
        {
            adam::configuration_parameter_list p;
            auto up = std::make_unique<adam::configuration_parameter_list_sorted>(user_params);
            up->set_name("user_parameters"_ct);
            p.add(std::move(up));
            p.add(std::make_unique<adam::configuration_parameter_boolean>("started"_ct));
            p.add(std::make_unique<adam::configuration_parameter_string>("type"_ct));
            p.add(std::make_unique<adam::configuration_parameter_string>("type_origin_module"_ct));
            return p;
        }();
        
        return params;
    }

    port::~port() 
    {
        stop();
        
        m_inspectors.iterate([&](const auto& active_inspectors) 
        {
            for (const auto& data_inspector : active_inspectors) 
                data_inspector->destroy();
        });

        m_state_buffer->release();
    }

    void port::worker()
    {
        set_state(state_running);
        
        while (is_running())
        {
            buffer* input = nullptr;
            if (read(input))
            {
                handle_data(input, data_direction_in);
                input->release();
            }
        }
    }

    void port::reset_state_buffer()
    {
        auto* stat_data = get_state_buffer_data();

        stat_data->total_buffers_handled    = 0;
        stat_data->total_bytes_handled      = 0;
        stat_data->total_buffers_discarded  = 0;
        stat_data->total_bytes_discarded    = 0;
    }

    bool port::handle_data(buffer* buf, data_direction dir)
    {
        bool result = true;

        if(!m_started->get_value())
            return false;

        m_inspectors.iterate([&](const auto& active_inspectors) 
        {
            for (const auto& data_inspector : active_inspectors) 
                data_inspector->handle_data(buf);
        });

        auto* stat_data = m_state_buffer->data_as<state_buffer_data>();

        stat_data->total_buffers_handled++;
        stat_data->total_bytes_handled += buf->get_size();
        
        switch (dir)
        {
            case data_direction_in:
            {
                // Populate map for active formats, if anything changed
                bool formats_updated = m_formats.is_dirty() || m_parse_cache.empty() != active_formats.empty();
                auto& active_formats = m_formats.get_active();
                if (formats_updated)
                {
                    m_parse_cache.clear();
                    for (auto& [hash, format] : active_formats)
                    {
                        m_parse_cache[hash] = nullptr;
                    }
                }

                // Parse data for each datatype
                for (auto& [hash, format] : active_formats)
                {
                    if (format->get_parser())
                        format->get_parser()->parse(buf, m_parse_cache[hash]);
                }

                // Send data to connections
                m_in_connections.iterate([&](const auto& connections) 
                {
                    for (const auto& conn : connections) 
                    {
                        if (!conn->is_valid_chain()) continue;

                        adam::buffer* internal_data = nullptr;
                        
                        auto it = m_parse_cache.find(conn->get_input_format()->get_name().get_hash());
                        if (it != m_parse_cache.end())
                            internal_data = it->second;

                        result &= conn->handle_data(internal_data ? internal_data : buf);
                    }
                });

                // Release internal format data
                for (auto& [hash, format] : active_formats)
                {
                    if (format->get_parser() && m_parse_cache[hash])
                        m_parse_cache[hash]->release();
                }

                break;
            }
            case data_direction_out:
            {
                if (m_use_spinlock_for_write)
                {
                    spinlock::guard lock(m_spinlock);
                    result &= write(buf);
                }
                else
                {
                    result &= write(buf);
                }
                break;
            }
        }

        return result;
    }

    bool port::start()
    {
        reset_state_buffer();

        m_started->set_value(true);

        if (m_b_threaded)
        {
            if (m_thread.joinable())
            {
                m_thread.join();
            }
            m_thread = std::thread(&port::worker, this);
        }

        set_state(state_started);
        return true;
    }

    bool port::stop()
    {
        m_started->set_value(false);

        if (m_b_threaded && m_thread.joinable())
        {
            if (m_thread.get_id() == std::this_thread::get_id())
            {
                m_thread.detach();
            }
            else
            {
                m_thread.join();
            }
        }

        set_state(state_stopped);
        return true;
    }

    void port::mark_start_failed()
    {
        set_state(state_error);
        m_started->set_value(false);
    }

    port::port(const string_hashed& item_name, uint32_t state_buffer_size) 
    :   registry_item(item_name, port::get_default_parameters()),
        m_b_threaded(true),
        m_thread(),
        m_in_connections(),
        m_out_connections(),
        m_inspectors(),
        m_started(dynamic_cast<configuration_parameter_boolean*>(get_parameters().get("started"_ct))),
        m_use_spinlock_for_write(false)
    {
        m_state_buffer = buffer_manager::get().request_buffer(state_buffer_size);
        
        reset_state_buffer();
    }

    void port::add_inspector(std::shared_ptr<data_inspector> inspector)
    {
        m_inspectors.push_back(inspector);
    }

    void port::remove_inspector(std::shared_ptr<data_inspector> inspector)
    {
        m_inspectors.remove(inspector);
    }

    void port::add_as_connection_input(connection* conn)
    {
        m_in_connections.push_back(conn);
        rebuild_formats_database();
    }

    void port::remove_as_connection_input(connection* conn)
    {
        m_in_connections.remove(conn);
        rebuild_formats_database();
    }

    void port::add_as_connection_output(connection* conn)
    {
        m_out_connections.push_back(conn);
    }

    void port::remove_as_connection_output(connection* conn)
    {
        m_out_connections.remove(conn);
    }

    void port::rebuild_formats_database()
    {
        std::unordered_map<string_hash, const data_format*> new_formats;

        m_in_connections.iterate([&](const auto& conns)
        {
            for (auto* conn : conns)
            {
                if (!conn->is_valid_chain()) 
                    continue;

                const data_format* fmt = conn->get_input_format();
                if (fmt)
                {
                    new_formats.emplace(fmt->get_name().get_hash(), fmt);
                }
            }
        });

        m_formats.update(new_formats);
    }
}