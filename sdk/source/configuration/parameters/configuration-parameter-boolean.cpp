#include "configuration/parameters/configuration-parameter-boolean.hpp"

namespace adam
{
    configuration_parameter_boolean::configuration_parameter_boolean(const string_hashed& name, bool value)
        : configuration_parameter(name), m_value(value) {}
        
    configuration_parameter_boolean::configuration_parameter_boolean(std::string_view name, bool value)
        : configuration_parameter(string_hashed(name)), m_value(value) {}

    configuration_parameter_boolean::~configuration_parameter_boolean() {}
}