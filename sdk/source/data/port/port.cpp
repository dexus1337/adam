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
            {
                data_inspector->destroy();
            }
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

        stat_data->reset_statistics();
    }

    bool port::handle_data(buffer*& buf, data_direction dir)
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

        auto cur_buff_size = buf->get_referenced_buffer() ? buf->get_referenced_buffer()->get_size() : buf->get_size();

        stat_data->total_buffers_recieved++;
        stat_data->total_bytes_recieved += cur_buff_size;

        auto return_with_statistics = [&](bool success) -> bool 
        { 
            if (success) 
            {
                stat_data->total_buffers_forwarded++;
                stat_data->total_bytes_forwarded += cur_buff_size;
            }
            else 
            {
                stat_data->total_buffers_discarded++;
                stat_data->total_bytes_discarded += cur_buff_size;
            }
            return success; 
        };
        
        switch (dir)
        {
            case data_direction_in:
            {
                // Populate map for active formats, if anything changed
                bool formats_updated = m_formats.is_dirty();
                auto& active_formats = m_formats.get_active();
                formats_updated = formats_updated || m_parse_cache.empty() != active_formats.empty();
                if (formats_updated) 
                {
                    m_parse_cache.clear();
                    m_parse_cache.reserve(active_formats.size());
                    for (auto& [hash, format] : active_formats)
                    {
                        #if defined(ADAM_PORT_USE_VECTOR_PARSE_CACHE)
                        m_parse_cache.emplace_back(hash, nullptr);
                        #else
                        m_parse_cache.emplace(hash, nullptr);
                        #endif
                    }
                }

                // Parse data for each datatype
                for (auto& [hash, format] : active_formats)
                {
                    if (auto* parser = format->get_parser()) 
                    {
                        #if defined(ADAM_PORT_USE_VECTOR_PARSE_CACHE)
                        auto it = std::find_if(m_parse_cache.begin(), m_parse_cache.end(), [hash](const auto& entry) { return entry.first == hash; });
                        #else
                        auto it = m_parse_cache.find(hash);
                        #endif
                        if (it == m_parse_cache.end()) 
                            return return_with_statistics(false);
                        
                        if (!parser->parse(buf, it->second))
                            return return_with_statistics(false);
                    }
                }

                // Send data to connections
                m_in_connections.iterate([&](const auto& connections)
                {
                    for (const auto& conn : connections)
                    {
                        adam::buffer* buff_to_send = buf;
                        
                        if (conn->get_input_format()->get_parser() != nullptr)
                        {
                            const string_hash format_hash = conn->get_input_format()->get_name().get_hash();
                            
                            #if defined(ADAM_PORT_USE_VECTOR_PARSE_CACHE)
                            auto it = std::find_if(m_parse_cache.cbegin(), m_parse_cache.cend(), [format_hash](const auto& entry) { return entry.first == format_hash; });
                            if (it != m_parse_cache.cend()) buff_to_send = it->second;
                            #else
                            auto it = m_parse_cache.find(format_hash);
                            if (it != m_parse_cache.end()) buff_to_send = it->second;
                            #endif
                        }

                        result &= conn->handle_data(buff_to_send);
                    }
                });

                // Release internal format data
                for (auto& [h, b] : m_parse_cache)
                {
                    if (b) 
                    {
                        b->release(); 
                        b = nullptr;
                    }
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

        return return_with_statistics(result);
    }

    bool port::start()
    {
        reset_state_buffer();

        m_started->set_value(true);

        if (m_b_threaded)
        {
            set_state(state_started);

            m_thread = std::thread(&port::worker, this);
        }
        else
        {
            set_state(state_running);
        }

        return true;
    }

    bool port::stop()
    {
        m_started->set_value(false);

        if (m_b_threaded && m_thread.joinable())
        {
            if (m_thread.get_id() == std::this_thread::get_id())
                m_thread.detach();
            else
                m_thread.join();
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