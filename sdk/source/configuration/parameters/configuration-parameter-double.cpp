#include "configuration/parameters/configuration-parameter-double.hpp"
#include <algorithm>
#include <cmath>

namespace adam
{
    configuration_parameter_double::configuration_parameter_double(const string_hashed& name, double value)
        : configuration_parameter(name), m_value(value), m_value_default(value),
          m_mode(value_mode_any), m_min_value(0.0), m_max_value(0.0) {}

    configuration_parameter_double::configuration_parameter_double(const string_hashed& name, double default_value, double min_value, double max_value)
        : configuration_parameter(name), m_value(default_value), m_value_default(default_value),
          m_mode(value_mode_range), m_min_value(min_value), m_max_value(max_value) {}

    configuration_parameter_double::configuration_parameter_double(const string_hashed& name, double default_value, presets_container presets)
        : configuration_parameter(name), m_value(default_value), m_value_default(default_value),
          m_mode(value_mode_preset), m_min_value(0.0), m_max_value(0.0), m_presets(std::move(presets)) {}
        
    configuration_parameter_double::~configuration_parameter_double() {}

    std::unique_ptr<configuration_parameter> configuration_parameter_double::clone() const
    {
        auto new_param = std::make_unique<configuration_parameter_double>(m_str_name, m_value_default);
        new_param->set_mode(m_mode);
        new_param->set_descriptions(get_descriptions());

        if (m_mode == value_mode_range)
        {
            new_param->set_range(m_min_value, m_max_value);
        }
        else if (m_mode == value_mode_preset)
        {
            for (double preset : m_presets)
            {
                new_param->add_preset(preset);
            }
        }

        new_param->set_value(m_value);
        return new_param;
    }

    bool configuration_parameter_double::set_value(double value)
    {
        if (m_mode == value_mode_preset)
        {
            auto it = std::find_if(m_presets.begin(), m_presets.end(), [value](double preset) {
                return std::abs(preset - value) < 1e-9;
            });
            if (it == m_presets.end())
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

    void configuration_parameter_double::set_range(double min_value, double max_value)
    {
        m_min_value = min_value;
        m_max_value = max_value;
    }

    void configuration_parameter_double::add_preset(double preset)
    {
        m_presets.push_back(preset);
    }
}