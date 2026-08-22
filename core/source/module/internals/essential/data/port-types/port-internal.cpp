#include "module/internals/essential/data/port-types/port-internal.hpp"

#include "configuration/parameters/configuration-parameter-string.hpp"
#include "data/connection.hpp"
#include "data/inspector.hpp"
#include "module/internals/essential/module-essential.hpp"

namespace adam 
{
    port_internal::port_internal(const string_hashed& item_name) 
     :  port_in_out(item_name)
    {
        port::m_b_threaded = false;
        get_parameter<configuration_parameter_string>("type"_ct)->set_value(type_name());
        get_parameter<configuration_parameter_string>("type_origin_module"_ct)->set_value(internal_module_essential.get_name());
    }

    port_internal::~port_internal() {}
    
    bool port_internal::handle_data(buffer*& buf, data_direction dir)
    {
        if (!is_running()) return false;

        switch (dir)
        {
            case data_direction_in:
            {
                return port_in_out::handle_data(buf, dir);
            }
            case data_direction_out:
            {
                auto* stat_data = m_state_buffer->data_as<state_buffer_data>();

                auto cur_buff_size = buf->get_referenced_buffer() ? buf->get_referenced_buffer()->get_size() : buf->get_size();

                // Increase recieved stats
                stat_data->total_buffers_recieved++;
                stat_data->total_bytes_recieved += cur_buff_size;
                
                stat_data->total_buffers_forwarded++;
                stat_data->total_bytes_forwarded += cur_buff_size;

                m_inspectors.iterate([&](const auto& active_inspectors) 
                {
                    for (const auto& data_inspector : active_inspectors) 
                        data_inspector->handle_data(buf);
                });

                // Send data to connections as input
                m_in_connections.iterate([&](const auto& connections) 
                {
                    for (const auto& conn : connections) 
                    {
                        adam::buffer* buff_to_send = buf;
                        conn->handle_data(buff_to_send);
                    }
                });
            }
        }

        return true;
    }

}