#include "configuration/parameters/configuration-parameter-integer.hpp"

namespace adam
{
    configuration_parameter_integer::configuration_parameter_integer(const string_hashed& name, int64_t value)
        : configuration_parameter(name), m_value(value), m_value_default(value), 
          m_mode(value_mode_any), m_min_value(0), m_max_value(0) {}

    configuration_parameter_integer::configuration_parameter_integer(const string_hashed& name, int64_t default_value, int64_t min_value, int64_t max_value)
        : configuration_parameter(name), m_value(default_value), m_value_default(default_value), 
          m_mode(value_mode_range), m_min_value(min_value), m_max_value(max_value) {}

    configuration_parameter_integer::configuration_parameter_integer(const string_hashed& name, int64_t default_value, presets_container presets)
        : configuration_parameter(name), m_value(default_value), m_value_default(default_value), 
          m_mode(value_mode_preset), m_min_value(0), m_max_value(0), m_presets(std::move(presets)) {}
        
    configuration_parameter_integer::~configuration_parameter_integer() {}

    std::unique_ptr<configuration_parameter> configuration_parameter_integer::clone() const
    {
        auto new_param = std::make_unique<configuration_parameter_integer>(m_str_name, m_value_default);
        new_param->set_mode(m_mode);
        new_param->set_descriptions(get_descriptions());

        if (m_mode == value_mode_range)
        {
            new_param->set_range(m_min_value, m_max_value);
        }
        else if (m_mode == value_mode_preset)
        {
            for (int64_t preset : m_presets)
            {
                new_param->add_preset(preset);
            }
        }

        new_param->set_value(m_value);
        return new_param;
    }

    bool configuration_parameter_integer::set_value(int64_t value)
    {
        if (m_mode == value_mode_preset)
        {
            // O(1) preset validation lookup
            if (m_presets.find(value) == m_presets.end())
            {
                return false;
            }
        }
        else if (m_mode == value_mode_range)
        {
            if (value < m_min_value || value > m_max_value)
            {
                return false;
            }
        }

        m_value = value;
        return true;
    }

    void configuration_parameter_integer::set_range(int64_t min_value, int64_t max_value)
    {
        m_min_value = min_value;
        m_max_value = max_value;
    }

    void configuration_parameter_integer::add_preset(int64_t preset)
    {
        m_presets.insert(preset);
    }
}