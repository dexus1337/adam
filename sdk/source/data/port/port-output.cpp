#include "data/port/port-output.hpp"


namespace adam 
{
    port_output::port_output(const string_hashed& item_name) 
     :  port(item_name)
    {
        m_b_threaded = false;
    }

    port_output::~port_output() {}
}