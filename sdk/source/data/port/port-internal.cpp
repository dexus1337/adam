#include "data/port/port-internal.hpp"

#include "configuration/parameters/configuration-parameter-string.hpp"

namespace adam 
{
    port_internal::port_internal(const string_hashed& item_name) 
     :  port_in_out(item_name)
    {
        get_parameter<configuration_parameter_string>("type")->set_value(type_name);
    }

    port_internal::~port_internal() {}
}