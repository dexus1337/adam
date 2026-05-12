#include "configuration/parameters/configuration-parameter-integer.hpp"

namespace adam
{
    configuration_parameter_integer::configuration_parameter_integer(const string_hashed& name, int64_t value)
        : configuration_parameter(name), m_value(value), m_value_default(value) {}
        
    configuration_parameter_integer::~configuration_parameter_integer() {}

    std::unique_ptr<configuration_parameter> configuration_parameter_integer::clone() const
    {
        auto new_param = std::make_unique<configuration_parameter_integer>(m_str_name, m_value_default);
        new_param->set_value(m_value);
        return new_param;
    }
}