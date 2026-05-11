#include "data/port/port-output-recording.hpp"


namespace adam::modules::recrep
{
    port_output_recording::port_output_recording(const string_hashed& item_name, const configuration_parameter_list& default_params) 
     :  port_output(item_name, default_params)
    {
    }

    port_output_recording::~port_output_recording() {}
}