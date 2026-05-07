#include "data/connection.hpp"

#include "data/port/port-input.hpp"
#include "data/port/port-output.hpp"
#include "data/processor.hpp"


namespace adam 
{
    connection::connection(const string_hashed& item_name, const configuration_parameter_list& default_params) 
    :   configuration_item(item_name, default_params),
        m_ports_input(),
        m_processors(),
        m_ports_output()
    {

    }

    connection::~connection() {}
    
    bool connection::handle_data(buffer* buffer)
    {
        bool result = true;

        // Chain processors: output of one = input of next
        m_processors.iterate([&](const auto& processors) 
        {
            for (auto processor : processors) 
                result &= processor->handle_data(buffer);
        });

        // Send result to output ports
        m_ports_output.iterate([&](const auto& outputs) 
        {
            for (auto output_port : outputs) 
                result &= output_port->handle_data(buffer);
        });

        return result;
    }
}