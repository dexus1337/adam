#include "data/port/port-input-internal.hpp"


namespace adam 
{
    port_input_internal::port_input_internal(const string_hashed& item_name) 
     :  port_input(item_name)
    {
    }

    port_input_internal::~port_input_internal() {}
}