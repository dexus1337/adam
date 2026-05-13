#include "data/port/port-input-replay.hpp"

#include "configuration/parameters/configuration-parameter-string.hpp"

namespace adam::modules::recrep
{
    const configuration_parameter_list& port_input_replay::get_default_parameters()
    {
        static adam::configuration_parameter_list params = []() 
        {
            adam::configuration_parameter_list p;
            p.add(std::make_unique<adam::configuration_parameter_string>("type"_ct));
            p.add(std::make_unique<adam::configuration_parameter_string>("data_format"_ct));
            p.add(std::make_unique<adam::configuration_parameter_string>("module_name"_ct));
            return p;
        }();
        return params;
    }

    port_input_replay::port_input_replay(const string_hashed& item_name) 
     :  port_input(item_name)
    {
        get_parameter<adam::configuration_parameter_string>("type")->set_value(type_name);

        add_parameters(port_input_replay::get_default_parameters());
    }

    port_input_replay::~port_input_replay() {}
}