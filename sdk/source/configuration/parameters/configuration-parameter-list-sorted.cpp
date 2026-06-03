#include "configuration/parameters/configuration-parameter-list-sorted.hpp"
#include <algorithm>
#include <cstring>

namespace adam
{
    configuration_parameter_list_sorted::configuration_parameter_list_sorted()
     :  configuration_parameter_list() {}

    configuration_parameter_list_sorted::configuration_parameter_list_sorted(const string_hashed& name)
     :  configuration_parameter_list(name) {}

    configuration_parameter_list_sorted::configuration_parameter_list_sorted(const configuration_parameter_list_sorted& other)
     :  configuration_parameter_list(other),
        m_order(other.m_order) {}

    configuration_parameter_list_sorted& configuration_parameter_list_sorted::operator=(const configuration_parameter_list_sorted& other)
    {
        if (this != &other)
        {
            configuration_parameter_list::operator=(other);
            m_order = other.m_order;
        }
        return *this;
    }

    configuration_parameter_list_sorted::configuration_parameter_list_sorted(const configuration_parameter_list& other)
     :  configuration_parameter_list(other), m_order()
    {
        if (auto* sorted_other = dynamic_cast<const configuration_parameter_list_sorted*>(&other))
        {
            m_order = sorted_other->m_order;
        }
        else
        {
            for (const auto& [name, param] : get_children())
            {
                m_order.push_back(name.get_hash());
            }
        }
    }

    configuration_parameter_list_sorted& configuration_parameter_list_sorted::operator=(const configuration_parameter_list& other)
    {
        if (this != &other)
        {
            configuration_parameter_list::operator=(other);
            m_order.clear();
            if (auto* sorted_other = dynamic_cast<const configuration_parameter_list_sorted*>(&other))
            {
                m_order = sorted_other->m_order;
            }
            else
            {
                for (const auto& [name, param] : get_children())
                {
                    m_order.push_back(name.get_hash());
                }
            }
        }
        return *this;
    }

    void configuration_parameter_list_sorted::add(std::unique_ptr<configuration_parameter> param)
    {
        if (param)
        {
            string_hash h = param->get_name().get_hash();
            bool already_exists = get_children().contains(param->get_name());
            
            configuration_parameter_list::add(std::move(param));
            
            if (!already_exists)
            {
                m_order.push_back(h);
            }
        }
    }

    void configuration_parameter_list_sorted::clear()
    {
        configuration_parameter_list::clear();
        m_order.clear();
    }

    void configuration_parameter_list_sorted::remove(string_hash name)
    {
        configuration_parameter_list::remove(name);
        m_order.erase(std::remove(m_order.begin(), m_order.end(), name), m_order.end());
    }

    bool configuration_parameter_list_sorted::rename_child(string_hash old_name, const string_hashed& new_name)
    {
        string_hash new_hash = new_name.get_hash();
        bool res = configuration_parameter_list::rename_child(old_name, new_name);
        if (res)
        {
            for (auto& hash : m_order)
            {
                if (hash == old_name)
                {
                    hash = new_hash;
                    break;
                }
            }
        }
        return res;
    }
}
