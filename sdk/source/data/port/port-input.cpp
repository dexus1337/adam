#include "data/port/port-input.hpp"


#include "data/connection.hpp"


namespace adam 
{
    port_input::port_input(const string_hashed& item_name, const configuration_parameter_list& default_params) 
     :  port(item_name, default_params)
    {

    }

    port_input::~port_input() {}
    
    bool port_input::handle_data(buffer* buffer)
    {
        bool result = true;

        // Send data to connections
        m_connections.iterate([&](const auto& connections) 
        {
            for (auto conn : connections) 
                result &= conn->handle_data(buffer);
        });

        return result;
    }

}