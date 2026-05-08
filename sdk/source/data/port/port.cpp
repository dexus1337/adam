#include "data/port/port.hpp"


#include "data/format.hpp"
#include "data/inspector.hpp"


namespace adam 
{
    port::~port() {}

    bool port::handle_data(buffer* buffer)
    {
        m_inspectors.iterate([&](const auto& active_inspectors) 
        {
            for (const auto& data_inspector : active_inspectors) 
                data_inspector->handle_data(buffer);
        });
        
        return true;
    }

    port::port(const string_hashed& item_name, const configuration_parameter_list& default_params) 
    :   configuration_item(item_name, default_params),
        m_data_format(&data_format_transparent) 
    {

    }
}