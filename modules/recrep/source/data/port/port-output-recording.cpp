#include "data/port/port-output-recording.hpp"

#include "configuration/parameters/configuration-parameter-string.hpp"

namespace adam::modules::recrep
{
    port_output_recording::port_output_recording(const string_hashed& item_name) 
     :  port_output(item_name)
    {
        get_parameter<adam::configuration_parameter_string>("type")->set_value(type_name);
    }

    port_output_recording::~port_output_recording() {}
}