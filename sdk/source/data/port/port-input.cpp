#include "data/port/port-input.hpp"


namespace adam 
{
    port_input::port_input(const string_hashed& item_name, const configuration_parameter_list& default_params) 
     :  port(item_name, default_params)
    {

    }

    port_input::~port_input() {}
}