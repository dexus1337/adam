#include "data/port/port-bidirectional.hpp"

namespace adam 
{
    port_bidirectional::port_bidirectional(const string_hashed& item_name) 
     :  port_input(item_name)
    {

    }

    port_bidirectional::~port_bidirectional() 
    {

    }
}