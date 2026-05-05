#include "configuration/parameters/configuration-parameter-reference.hpp"

namespace adam
{
    configuration_parameter_reference::configuration_parameter_reference(const string_hashed& name, const string_hashed& target)
        : configuration_parameter(name), m_target(target) {}

    configuration_parameter_reference::~configuration_parameter_reference() {}
}