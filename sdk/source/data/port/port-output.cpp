#include "data/port/port-output.hpp"


namespace adam 
{
    port_output::port_output(const string_hashed& item_name, uint32_t state_buffer_size) 
     :  port(item_name, state_buffer_size)
    {
        m_b_threaded = false;
    }

    port_output::~port_output() {}
}