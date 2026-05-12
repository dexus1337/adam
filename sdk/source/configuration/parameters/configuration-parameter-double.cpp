#include "configuration/parameters/configuration-parameter-double.hpp"

namespace adam
{
    configuration_parameter_double::configuration_parameter_double(const string_hashed& name, double value)
        : configuration_parameter(name), m_value(value), m_value_default(value) {}
        
    configuration_parameter_double::~configuration_parameter_double() {}

    std::unique_ptr<configuration_parameter> configuration_parameter_double::clone() const
    {
        auto new_param = std::make_unique<configuration_parameter_double>(m_str_name, m_value_default);
        new_param->set_value(m_value);
        return new_param;
    }
}