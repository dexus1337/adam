#include "data/port/port-output-recording.hpp"


namespace adam::modules::recrep
{
    port_output_recording::port_output_recording(const string_hashed& item_name) 
     :  port_output(item_name)
    {
    }

    port_output_recording::~port_output_recording() {}
}