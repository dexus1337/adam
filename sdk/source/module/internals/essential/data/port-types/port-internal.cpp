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
    
    bool port_internal::handle_data(buffer* buf, data_direction dir)
    {
        bool result = true;

        if (get_state_buffer_data()->cur_state != state_running)
            return false;

        switch (dir)
        {
            case data_direction_in:
            {
                return port_in_out::handle_data(buf, dir);
            }
            case data_direction_out:
            {
                m_inspectors.iterate([&](const auto& active_inspectors) 
                {
                    for (const auto& data_inspector : active_inspectors) 
                        data_inspector->handle_data(buf);
                });

                // Send data to connections as input
                m_in_connections.iterate([&](const auto& connections) 
                {
                    for (const auto& conn : connections) 
                        result &= conn->handle_data(buf);
                });

                auto* stat_data = m_state_buffer->data_as<state_buffer_data>();

                stat_data->total_buffers_handled++;
                stat_data->total_bytes_handled += buf->get_size();
            }
        }

        return result;
    }

}