#include "configuration/parameters/configuration-parameter.hpp"


namespace adam 
{
    configuration_parameter::configuration_parameter()
        : m_str_name() {}

    configuration_parameter::configuration_parameter(const string_hashed& name)
        : m_str_name(name) {}

    configuration_parameter::configuration_parameter(string_hashed::view name)
        : m_str_name(name) {}

    configuration_parameter::~configuration_parameter() {}
}