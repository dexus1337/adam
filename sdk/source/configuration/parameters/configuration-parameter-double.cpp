#include "configuration/parameters/configuration-parameter-double.hpp"

namespace adam
{
    configuration_parameter_double::configuration_parameter_double(const string_hashed& name, double value)
        : configuration_parameter(name), m_value(value) {}
        
    configuration_parameter_double::configuration_parameter_double(std::string_view name, double value)
        : configuration_parameter(string_hashed(name)), m_value(value) {}

    configuration_parameter_double::~configuration_parameter_double() {}
}