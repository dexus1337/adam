#include "data/port/port-input-internal.hpp"

#include "configuration/parameters/configuration-parameter-string.hpp"

namespace adam 
{
    port_input_internal::port_input_internal(const string_hashed& item_name) 
     :  port_input(item_name)
    {
        get_parameter<configuration_parameter_string>("type")->set_value(type_name);
    }

    port_input_internal::~port_input_internal() {}
}