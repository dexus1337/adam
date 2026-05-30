#include "data/port/port-internal.hpp"

#include "configuration/parameters/configuration-parameter-string.hpp"
#include "data/connection.hpp"

namespace adam 
{
    port_internal::port_internal(const string_hashed& item_name) 
     :  port_in_out(item_name)
    {
        get_parameter<configuration_parameter_string>("type"_ct)->set_value(type_name());
    }

    port_internal::~port_internal() {}
    
    bool port_internal::handle_data(buffer* buffer, data_direction dir)
    {
        bool result = true;

        switch (dir)
        {
            case data_direction_in:
            {
                return port_in_out::handle_data(buffer, dir);
            }
            case data_direction_out:
            {
                // Send data to connections as input
                m_in_connections.iterate([&](const auto& connections) 
                {
                    for (const auto& conn : connections) 
                        result &= conn->handle_data(buffer, this);
                });

                auto* stat_data = m_statistic_buffer->data_as<statistic_info>();

                stat_data->total_buffers_handled++;
                stat_data->total_bytes_handled += buffer->get_size();
            }
        }

        return result;
    }

}