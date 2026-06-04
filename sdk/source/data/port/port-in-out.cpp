#include "data/port/port-in-out.hpp"

namespace adam 
{
    port_in_out::port_in_out(const string_hashed& item_name, size_t state_buffer_size) 
     :  port_input(item_name, state_buffer_size)
    {

    }

    port_in_out::~port_in_out() 
    {

    }
}