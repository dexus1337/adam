#include "configuration/parameters/configuration-parameter-string.hpp"

#include <regex>

namespace adam
{
    configuration_parameter_string::configuration_parameter_string(const string_hashed& name, const string_hashed_ct& default_value)
        : configuration_parameter(name), m_value(default_value), m_value_default(default_value), m_mode(value_mode_any) {}
        
    configuration_parameter_string::configuration_parameter_string(const string_hashed& name, const string_hashed_ct& default_value, presets_container presets)
        : configuration_parameter(name), m_value(default_value), m_value_default(default_value), m_mode(value_mode_preset), m_presets(std::move(presets)) {}

    configuration_parameter_string::configuration_parameter_string(const string_hashed& name, const string_hashed_ct& default_value, std::unique_ptr<configuration_parameter_string> regex_param)
        : configuration_parameter(name), m_value(default_value), m_value_default(default_value), m_mode(value_mode_regex), m_regex(std::move(regex_param)) {}

    configuration_parameter_string::configuration_parameter_string(const string_hashed& name, const string_hashed& default_value)
        : configuration_parameter(name), m_value(default_value), m_value_default(default_value), m_mode(value_mode_any) {}
        
    configuration_parameter_string::configuration_parameter_string(const string_hashed& name, const string_hashed& default_value, presets_container presets)
        : configuration_parameter(name), m_value(default_value), m_value_default(default_value), m_mode(value_mode_preset), m_presets(std::move(presets)) {}

    configuration_parameter_string::configuration_parameter_string(const string_hashed& name, const string_hashed& default_value, std::unique_ptr<configuration_parameter_string> regex_param)
        : configuration_parameter(name), m_value(default_value), m_value_default(default_value), m_mode(value_mode_regex), m_regex(std::move(regex_param)) {}

    configuration_parameter_string::~configuration_parameter_string() {}

    std::unique_ptr<configuration_parameter> configuration_parameter_string::clone() const
    {
        auto new_param = std::make_unique<configuration_parameter_string>(m_str_name, m_value_default);

        new_param->set_mode(m_mode);
        new_param->set_descriptions(get_descriptions());

        for (const auto& [key, preset] : m_presets)
        {
            new_param->add_preset(std::unique_ptr<configuration_parameter_string>(static_cast<configuration_parameter_string*>(preset->clone().release())));
        }

        if (m_regex)
        {
            new_param->set_regex(std::unique_ptr<configuration_parameter_string>(static_cast<configuration_parameter_string*>(m_regex->clone().release())));
        }

        new_param->set_value(m_value);
        return new_param;
    }

    bool configuration_parameter_string::set_value(const string_hashed& value)
    {
        if (m_mode == value_mode_preset)
        {
            // Fast validity checking via unordered_map O(1)
            if (m_presets.find(value) == m_presets.end()) return false;
        }
        else if (m_mode == value_mode_regex)
        {
            if (m_regex)
            {
                try 
                {
                    std::regex re(m_regex->get_value().c_str());
                    if (!std::regex_match(value.c_str(), re)) return false;
                }
                catch (const std::regex_error&) 
                {
                    return false;
                }
            }
        }
        
        m_value = value;
        return true;
    }

    void configuration_parameter_string::add_preset(std::unique_ptr<configuration_parameter_string> preset)
    {
        m_presets.emplace(preset->get_value(), std::move(preset));
    }

    void configuration_parameter_string::set_regex(std::unique_ptr<configuration_parameter_string> regex_param)
    {
        m_regex = std::move(regex_param);
    }
}