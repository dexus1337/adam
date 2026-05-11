#include "data/port/port-output-internal.hpp"


namespace adam 
{
    port_output_internal::port_output_internal(const string_hashed& item_name, const configuration_parameter_list& default_params) 
     :  port_output(item_name, default_params)
    {
    }

    port_output_internal::~port_output_internal() {}
}