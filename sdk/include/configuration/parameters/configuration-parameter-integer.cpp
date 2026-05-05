#include "configuration/parameters/types/configuration-parameter-integer.hpp"

namespace adam
{
    configuration_parameter_integer::configuration_parameter_integer(const string_hashed& name, int64_t value)
        : configuration_parameter(name), m_value(value) {}
        
    configuration_parameter_integer::configuration_parameter_integer(std::string_view name, int64_t value)
        : configuration_parameter(string_hashed(name)), m_value(value) {}

    configuration_parameter_integer::~configuration_parameter_integer() {}
}