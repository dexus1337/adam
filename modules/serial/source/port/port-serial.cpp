#include "data/port/port-serial.hpp"

#include "configuration/parameters/configuration-parameter-string.hpp"

namespace adam::modules::serial
{
    const configuration_parameter_list& port_serial::get_default_parameters()
    {
        static adam::configuration_parameter_list params = []() 
        {
            adam::configuration_parameter_list p;
            p.add(std::make_unique<adam::configuration_parameter_string>("path"_ct));
            return p;
        }();
        return params;
    }

    port_serial::port_serial(const string_hashed& item_name) 
     :  port_in_out(item_name)
    {
        get_parameter<adam::configuration_parameter_string>("type"_ct)->set_value(type_name());

        add_parameters(port_serial::get_default_parameters());
    }

    port_serial::~port_serial() {}
}