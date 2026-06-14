#pragma once

/**
 * @file    configuration-parameter-string.hpp
 * @author  dexus1337
 * @brief   Defines a string configuration parameter, used for defining a single string configuration value.
 * @version 1.0
 * @date    05.05.2026
 */

 
#include "api/api-sdk.hpp"

#include <unordered_map>
#include <memory>

#include "configuration/parameters/configuration-parameter.hpp"
#include "types/string-hashed.hpp"
#include "types/string-hashed-ct.hpp"


namespace adam 
{
    /**
     * @class configuration_parameter_string
     * @brief Defines a string configuration parameter, used for defining a single string configuration value.
     */
    class ADAM_SDK_API configuration_parameter_string : public configuration_parameter
    {
    public:

        using presets_container = std::unordered_map<string_hashed, std::unique_ptr<configuration_parameter_string>>;

        #pragma pack(push, 1)
        struct view : configuration_parameter::view
        {
            uint16_t length;
        };
        #pragma pack(pop)

        enum value_mode
        {
            value_mode_any,
            value_mode_preset,
            value_mode_regex
        };

        /** @brief Constructs a new configuration_parameter_string object for 'any' mode. */
        configuration_parameter_string(const string_hashed& name, const string_hashed_ct& default_value = string_hashed_ct(""));

        /** @brief Constructs a new configuration_parameter_string object for 'preset' mode. */
        configuration_parameter_string(const string_hashed& name, const string_hashed_ct& default_value, presets_container presets);

        /** @brief Constructs a new configuration_parameter_string object for 'regex' mode. */
        configuration_parameter_string(const string_hashed& name, const string_hashed_ct& default_value, std::unique_ptr<configuration_parameter_string> regex_param);

        /** @brief Constructs a new configuration_parameter_string object for 'any' mode. */
        configuration_parameter_string(const string_hashed& name, const string_hashed& default_value);

        /** @brief Constructs a new configuration_parameter_string object for 'preset' mode. */
        configuration_parameter_string(const string_hashed& name, const string_hashed& default_value, presets_container presets);

        /** @brief Constructs a new configuration_parameter_string object for 'regex' mode. */
        configuration_parameter_string(const string_hashed& name, const string_hashed& default_value, std::unique_ptr<configuration_parameter_string> regex_param);

        /** @brief Constructs a new configuration_parameter_string from a string literal, enabling automatic compile-time hashing. */
        template<size_t N>
        configuration_parameter_string(const string_hashed& name, const char (&default_value)[N]) : configuration_parameter(name), m_value(string_hashed_ct(default_value)), m_value_default(default_value) {}

        /** @brief Destroys the configuration_parameter_string object and cleans up resources. */
        ~configuration_parameter_string();

        type get_type() const override { return type_string; }
 
        /** @brief Creates a deep copy of this configuration parameter. */
        std::unique_ptr<configuration_parameter> clone() const override;

        const string_hashed& get_value() const { return m_value; }
        bool set_value(const string_hashed& value);
        string_hashed& value() { return m_value; }

        const string_hashed& get_default_value() const { return m_value_default; }
        void reset_to_default() { m_value = m_value_default; }

        value_mode get_mode() const { return m_mode; }
        void set_mode(value_mode mode) { m_mode = mode; }

        const presets_container& get_presets() const { return m_presets; }
        void add_preset(std::unique_ptr<configuration_parameter_string> preset);

        const configuration_parameter_string* get_regex_parameter() const { return m_regex.get(); }
        void set_regex(std::unique_ptr<configuration_parameter_string> regex_param);

    private:

        string_hashed                                   m_value;
        string_hashed                                   m_value_default;

        value_mode                                      m_mode;
        presets_container                               m_presets;
        std::unique_ptr<configuration_parameter_string> m_regex;
    };
}