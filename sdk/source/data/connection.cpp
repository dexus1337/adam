#include "data/connection.hpp"

#include "data/port/port.hpp"
#include "data/processor.hpp"


namespace adam 
{
    const configuration_parameter_list& connection::get_default_parameters()
    {
        static adam::configuration_parameter_list params = []() 
        {
            adam::configuration_parameter_list p;
            p.add(std::make_unique<adam::configuration_parameter_list>("inputs"_ct));
            p.add(std::make_unique<adam::configuration_parameter_list>("processors"_ct));
            p.add(std::make_unique<adam::configuration_parameter_list>("outputs"_ct));
            p.add(std::make_unique<adam::configuration_parameter_integer>("date_created"_ct));
            p.add(std::make_unique<adam::configuration_parameter_integer>("date_edited"_ct));
            p.add(std::make_unique<adam::configuration_parameter_integer>("color_code"_ct));
            p.add(std::make_unique<adam::configuration_parameter_integer>("sorting_index"_ct));
            return p;
        }();
        return params;
    }

    connection::connection(const string_hashed& item_name) 
    :   configuration_item(item_name, connection::get_default_parameters()),
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
            for (auto* processor : processors) 
                result &= processor->handle_data(buffer);
        });

        // Send result to output ports
        m_ports_output.iterate([&](const auto& outputs) 
        {
            for (auto* output_port : outputs) 
                result &= output_port->handle_data(buffer);
        });

        return result;
    }

    bool connection::start()
    {
        bool result = true;

        m_ports_input.iterate([&](const auto& inputs) 
        {
            for (auto* in : inputs) 
                result &= in->start();
        });

        return result;
    }

    bool connection::stop()
    {
        bool result = true;

        m_ports_input.iterate([&](const auto& inputs) 
        {
            for (auto* in : inputs) 
                result &= in->stop();
        });

        return result;
    }
}