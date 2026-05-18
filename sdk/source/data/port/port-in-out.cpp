#include "data/port/port-in-out.hpp"

namespace adam 
{
    port_in_out::port_in_out(const string_hashed& item_name) 
     :  port_input(item_name)
    {

    }

    port_in_out::~port_in_out() 
    {

    }
}