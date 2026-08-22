#include "configuration/parameters/configuration-parameter-reference.hpp"

namespace adam
{
    configuration_parameter_reference::configuration_parameter_reference(const string_hashed& name, const string_hashed& target)
        : configuration_parameter(name), m_target(target), m_target_default(target) {}
        
    configuration_parameter_reference::~configuration_parameter_reference() {}

    std::unique_ptr<configuration_parameter> configuration_parameter_reference::clone() const
    {
        auto new_param = std::make_unique<configuration_parameter_reference>(m_str_name, m_target_default);
        new_param->set_target(m_target);
        return new_param;
    }

    void configuration_parameter_reference::copy_values_from(const configuration_parameter* other)
    {
        if (!other) return;

        if (const auto* p = dynamic_cast<const configuration_parameter_reference*>(other))
            set_target(p->get_target());
    }
}