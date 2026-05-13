#include "data/port/port.hpp"


#include "data/format.hpp"
#include "data/inspector.hpp"


namespace adam 
{
    const configuration_parameter_list& port::get_default_parameters()
    {
        static adam::configuration_parameter_list params = []() 
        {
            adam::configuration_parameter_list p;
            p.add(std::make_unique<adam::configuration_parameter_string>("type"));
            p.add(std::make_unique<adam::configuration_parameter_string>("data_format"));
            return p;
        }();
        return params;
    }

    port::~port() 
    {
        m_inspectors.iterate([&](const auto& active_inspectors) 
        {
            for (const auto& data_inspector : active_inspectors) 
                data_inspector->destroy();
        });
    }

    bool port::handle_data(buffer* buffer)
    {
        m_inspectors.iterate([&](const auto& active_inspectors) 
        {
            for (const auto& data_inspector : active_inspectors) 
                data_inspector->handle_data(buffer);
        });
        
        return true;
    }

    port::port(const string_hashed& item_name) 
    :   configuration_item(item_name, port::get_default_parameters()),
        m_data_format(&data_format_transparent) 
    {
    }
}