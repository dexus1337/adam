#include "data/port/port.hpp"


#include "data/format.hpp"


namespace adam 
{
    port::port(const string_hashed& item_name, const configuration_parameter_list& default_params) 
    :   configuration_item(item_name, default_params),
        m_data_format(&data_format_transparent) 
    {

    }

    port::~port() {}
}