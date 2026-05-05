#include "configuration/parameters/types/configuration-parameter-reference.hpp"

namespace adam
{
    configuration_parameter_reference::configuration_parameter_reference(const string_hashed& name, const string_hashed& target)
        : configuration_parameter(name), m_target(target) {}
        
    configuration_parameter_reference::configuration_parameter_reference(std::string_view name, const string_hashed& target)
        : configuration_parameter(string_hashed(name)), m_target(target) {}

    configuration_parameter_reference::~configuration_parameter_reference() {}
}