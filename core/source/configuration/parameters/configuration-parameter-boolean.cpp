#include "configuration/parameters/configuration-parameter-boolean.hpp"

namespace adam
{
    configuration_parameter_boolean::configuration_parameter_boolean(const string_hashed& name, bool value)
        : configuration_parameter(name), m_value(value), m_value_default(value) {}
        
    configuration_parameter_boolean::~configuration_parameter_boolean() {}

    std::unique_ptr<configuration_parameter> configuration_parameter_boolean::clone() const
    {
        auto new_param = std::make_unique<configuration_parameter_boolean>(m_str_name, m_value_default);
        new_param->set_value(m_value);
        return new_param;
    }

    void configuration_parameter_boolean::copy_from(const configuration_parameter* other)
    {
        if (!other) return;

        if (const auto* p = dynamic_cast<const configuration_parameter_boolean*>(other))
            set_value(p->get_value());
    }
}