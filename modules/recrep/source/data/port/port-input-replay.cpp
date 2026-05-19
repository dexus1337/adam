#include "data/port/port-input-replay.hpp"

#include "configuration/parameters/configuration-parameter-string.hpp"

namespace adam::modules::recrep
{
    const configuration_parameter_list& port_input_replay::get_default_parameters()
    {
        static adam::configuration_parameter_list params = []() 
        {
            adam::configuration_parameter_list p;
            return p;
        }();
        return params;
    }

    port_input_replay::port_input_replay(const string_hashed& item_name) 
     :  port_input(item_name)
    {
        get_parameter<adam::configuration_parameter_string>("type"_ct)->set_value(type_name());

        add_parameters(port_input_replay::get_default_parameters());
    }

    port_input_replay::~port_input_replay() {}
}