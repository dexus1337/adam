#include "data/port/port-input-replay.hpp"


namespace adam::modules::recrep
{
    port_input_replay::port_input_replay(const string_hashed& item_name) 
     :  port_input(item_name)
    {
    }

    port_input_replay::~port_input_replay() {}
}