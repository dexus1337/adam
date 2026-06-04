#include "data/port/port-input.hpp"


namespace adam 
{
    port_input::port_input(const string_hashed& item_name, size_t state_buffer_size) 
     :  port(item_name, state_buffer_size)
    {

    }

    port_input::~port_input() {}

}