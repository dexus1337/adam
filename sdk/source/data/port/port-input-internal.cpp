#include "data/port/port-input-internal.hpp"


namespace adam 
{
    port_input_internal::port_input_internal(const string_hashed& item_name, const configuration_parameter_list& default_params) 
     :  port_input(item_name, default_params)
    {
    }

    port_input_internal::~port_input_internal() {}
}