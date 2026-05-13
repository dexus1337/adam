#include "configuration/parameters/configuration-parameter-string.hpp"

namespace adam
{
    configuration_parameter_string::configuration_parameter_string(const string_hashed& name, const string_hashed_ct& default_value)
        : configuration_parameter(name), m_value(default_value), m_value_default(default_value) {}
        
    configuration_parameter_string::~configuration_parameter_string() {}

    std::unique_ptr<configuration_parameter> configuration_parameter_string::clone() const
    {
        auto new_param = std::make_unique<configuration_parameter_string>(m_str_name, m_value_default);
        new_param->set_value(m_value);
        return new_param;
    }
}