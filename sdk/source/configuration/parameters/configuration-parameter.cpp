#include "configuration/parameters/configuration-parameter.hpp"


namespace adam 
{
    configuration_parameter::configuration_parameter(const string_hashed& name)
        : m_str_name(name) {}

    configuration_parameter::configuration_parameter(std::string_view name)
        : m_str_name(name) {}

    configuration_parameter::~configuration_parameter() {}
}