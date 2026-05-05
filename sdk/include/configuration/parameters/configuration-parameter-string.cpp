#include "configuration/parameters/types/configuration-parameter-string.hpp"

namespace adam
{
    configuration_parameter_string::configuration_parameter_string(const string_hashed& name, std::string_view value)
        : configuration_parameter(name), m_value(value) {}
        
    configuration_parameter_string::configuration_parameter_string(std::string_view name, std::string_view value)
        : configuration_parameter(string_hashed(name)), m_value(value) {}

    configuration_parameter_string::~configuration_parameter_string() {}
}