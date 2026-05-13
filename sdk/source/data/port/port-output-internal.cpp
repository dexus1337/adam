#include "data/port/port-output-internal.hpp"


namespace adam 
{
    port_output_internal::port_output_internal(const string_hashed& item_name) 
     :  port_output(item_name)
    {
        get_parameter<configuration_parameter_string>("type")->set_value(type_name);
    }

    port_output_internal::~port_output_internal() {}
}