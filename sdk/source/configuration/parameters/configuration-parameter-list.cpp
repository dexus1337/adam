#include "configuration/parameters/configuration-parameter-list.hpp"

namespace adam
{
    configuration_parameter_list::configuration_parameter_list(const string_hashed& name)
        : configuration_parameter(name) {}

    configuration_parameter_list::~configuration_parameter_list() {}

    void configuration_parameter_list::add(std::unique_ptr<configuration_parameter> param)
    {
        if (param)
            m_children.emplace(param->get_name(), std::move(param));
    }

    configuration_parameter* configuration_parameter_list::get(const string_hashed& name) const
    {
        auto it = m_children.find(name);
        if (it != m_children.end())
            return it->second.get();
            
        return nullptr;
    }
}