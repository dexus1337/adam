#include "data/port/port-input.hpp"


#include "data/connection.hpp"


namespace adam 
{
    port_input::port_input(const string_hashed& item_name) 
     :  port(item_name)
    {

    }

    port_input::~port_input() {}
    
    bool port_input::handle_data(buffer* buffer)
    {
        bool result = true;

        // Send data to connections
        m_connections.iterate([&](const auto& connections) 
        {
            for (const auto& conn : connections) 
                result &= conn->handle_data(buffer);
        });

        return result;
    }

}