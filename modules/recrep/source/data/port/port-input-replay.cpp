#include "data/port/port-input-replay.hpp"


namespace adam::modules::recrep
{
    port_input_replay::port_input_replay(const string_hashed& item_name, const configuration_parameter_list& default_params) 
     :  port_input(item_name, default_params)
    {
    }

    port_input_replay::~port_input_replay() {}
}