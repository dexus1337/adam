#include "data/port/port.hpp"


#include "data/inspector.hpp"
#include "data/connection.hpp"
#include "memory/buffer/buffer-manager.hpp"
#include "configuration/parameters/configuration-parameter-list-sorted.hpp"
#include <chrono>


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
            p.add(std::make_unique<adam::configuration_parameter_boolean>("is_active"_ct));
            p.add(std::make_unique<adam::configuration_parameter_string>("type"_ct));
            p.add(std::make_unique<adam::configuration_parameter_string>("type_origin_module"_ct));
            return p;
        }();
        return params;
    }

    port::~port() 
    {
        if (m_b_threaded && m_thread.joinable())
        {
            m_is_active->set_value(false);
            m_thread.join();
        }

        m_inspectors.iterate([&](const auto& active_inspectors) 
        {
            for (const auto& data_inspector : active_inspectors) 
                data_inspector->destroy();
        });

        m_statistic_buffer->release();
    }

    void port::worker()
    {
        while (is_active())
        {
            buffer* input = nullptr;
            if (read(input))
            {
                handle_data(input, data_direction_in);
                input->release();
            }
        }
    }

    bool port::handle_data(buffer* buffer, data_direction dir)
    {
        bool result = true;

        if(!m_is_active->get_value())
            return false;

        m_inspectors.iterate([&](const auto& active_inspectors) 
        {
            for (const auto& data_inspector : active_inspectors) 
                data_inspector->handle_data(buffer);
        });

        auto* stat_data = m_statistic_buffer->data_as<statistic_info>();

        stat_data->total_buffers_handled++;
        stat_data->total_bytes_handled += buffer->get_size();
        
        switch (dir)
        {
            case data_direction_in:
            {
                // Send data to connections
                m_in_connections.iterate([&](const auto& connections) 
                {
                    for (const auto& conn : connections) 
                    {
                        if (!conn->is_valid_chain()) continue;

                        result &= conn->handle_data(buffer);
                    }
                });

                break;
            }
            case data_direction_out:
            {
                result &= write(buffer);
                break;
            }
        }

        return result;
    }

    bool port::start()
    {
        auto* stat_data = m_statistic_buffer->data_as<statistic_info>();

        stat_data->total_buffers_handled    = 0;
        stat_data->total_bytes_handled      = 0;

        m_is_active->set_value(true);

        if (m_b_threaded)
        {
            if (m_thread.joinable())
            {
                m_thread.join();
            }
            m_thread = std::thread(&port::worker, this);
        }

        return true;
    }

    bool port::stop()
    {
        m_is_active->set_value(false);

        if (m_b_threaded && m_thread.joinable())
        {
            m_thread.join();
        }

        return true;
    }

    port::port(const string_hashed& item_name) 
    :   configuration_item(item_name, port::get_default_parameters()),
        m_b_threaded(true),
        m_thread(),
        m_in_connections(),
        m_out_connections(),
        m_inspectors(),
        m_statistic_buffer(buffer_manager::get().request_buffer(statistic_info_buffer_size)),
        m_is_active(dynamic_cast<configuration_parameter_boolean*>(get_parameters().get("is_active"_ct)))
    {
    }
}