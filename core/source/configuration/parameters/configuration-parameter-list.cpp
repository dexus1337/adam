#include "configuration/parameters/configuration-parameter-list.hpp"

namespace adam
{
    configuration_parameter_list::configuration_parameter_list()
     :  configuration_parameter(), m_children() {}

    configuration_parameter_list::configuration_parameter_list(const string_hashed& name)
     :  configuration_parameter(name), m_children() {}

    configuration_parameter_list::configuration_parameter_list(const configuration_parameter_list& other)
     :  configuration_parameter(other.get_name())
    {
        for (const auto& [name, param] : other.m_children)
        {
            m_children.emplace(name, param ? param->clone() : nullptr);
        }
    }

    configuration_parameter_list& configuration_parameter_list::operator=(const configuration_parameter_list& other)
    {
        if (this != &other) 
        {
            // configuration_parameter::operator=(other); // Uncomment if the base class has state to copy
            m_children.clear();
            for (const auto& [name, param] : other.m_children)
            {
                m_children.emplace(name, param ? param->clone() : nullptr);
            }
        }
        return *this;
    }

    configuration_parameter_list::~configuration_parameter_list() {}

    void configuration_parameter_list::copy_values_from(const configuration_parameter* other)
    {
        if (!other)
            return;

        const auto* src_list = dynamic_cast<const configuration_parameter_list*>(other);
        if (!src_list)
            return;

        for (auto& [name, param] : m_children)
        {
            if (!param)
                continue;

            const auto* src_param = src_list->get(name.get_hash());
            if (!src_param)
                continue;

            param->copy_values_from(src_param);
        }
    }

    void configuration_parameter_list::copy_from(const configuration_parameter* other)
    {
        if (!other)
        {
            return;
        }

        const auto* src_list = dynamic_cast<const configuration_parameter_list*>(other);
        if (!src_list)
        {
            return;
        }

        clear();

        for (const auto& [name, param] : src_list->get_children())
        {
            if (param)
            {
                add(param->clone());
            }
        }
    }

    void configuration_parameter_list::add(std::unique_ptr<configuration_parameter> param)
    {
        if (param && !m_children.contains(param->get_name()))
            m_children.emplace(param->get_name(), std::move(param));
    }

    void configuration_parameter_list::clear()
    {
        m_children.clear();
    }

    void configuration_parameter_list::remove(string_hash name) 
    { 
        if (auto it = m_children.find(name); it != m_children.end())
            m_children.erase(it); 
    }

    configuration_parameter* configuration_parameter_list::get(string_hash name) const
    {
        auto it = m_children.find(name);
        if (it != m_children.end())
            return it->second.get();
            
        return nullptr;
    }

    bool configuration_parameter_list::rename_child(string_hash old_name, const string_hashed& new_name)
    {
        auto it = m_children.find(old_name);
        if (it == m_children.end()) return false;
        
        auto param = std::move(it->second);
        m_children.erase(it);
        param->set_name(new_name);
        m_children.emplace(new_name, std::move(param));
        return true;
    }
}