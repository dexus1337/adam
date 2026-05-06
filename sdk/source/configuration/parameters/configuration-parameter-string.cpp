#include "configuration/parameters/configuration-parameter-string.hpp"

namespace adam
{
    configuration_parameter_string::configuration_parameter_string(const string_hashed& name, string_hashed::view value)
        : configuration_parameter(name), m_value(value) {}
        
    configuration_parameter_string::configuration_parameter_string(string_hashed::view name, string_hashed::view value)
        : configuration_parameter(string_hashed(name)), m_value(value) {}

    configuration_parameter_string::~configuration_parameter_string() {}
}