#include "configuration/parameters/configuration-parameter-integer.hpp"

namespace adam
{
    configuration_parameter_integer::configuration_parameter_integer(const string_hashed& name, int64_t value)
        : configuration_parameter(name), m_value(value) {}
        
    configuration_parameter_integer::~configuration_parameter_integer() {}
}