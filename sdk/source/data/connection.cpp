#include "data/connection.hpp"

#include "data/port/port-input.hpp"
#include "data/port/port-output.hpp"
#include "data/processor.hpp"


namespace adam 
{
    connection::connection(const string_hashed& item_name, const configuration_parameter_list& default_params) 
    :   configuration_item(item_name, default_params),
        m_input_ports(),
        m_processors(),
        m_output_ports()
    {

    }

    connection::~connection() {}
}