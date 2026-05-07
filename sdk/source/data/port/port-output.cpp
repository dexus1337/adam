#include "data/port/port-output.hpp"


namespace adam 
{
    port_output::port_output(const string_hashed& item_name, const configuration_parameter_list& default_params) 
     :  port(item_name, default_params)
    {

    }

    port_output::~port_output() {}
}