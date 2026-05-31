#pragma once

/**
 * @file    configuration-parameter-integer.hpp
 * @author  dexus1337
 * @brief   Defines a integer configuration parameter. Signed 64bit.
 * @version 1.0
 * @date    05.05.2026
 */

 
#include "api/api-sdk.hpp"

#include <string_view>
#include <unordered_set>
#include "configuration/parameters/configuration-parameter.hpp"


namespace adam 
{
    /**
     * @class configuration_parameter_integer
     * @brief Defines a integer configuration parameter. Signed 64bit.
     */
    class ADAM_SDK_API configuration_parameter_integer : public configuration_parameter
    {
    public:
    
        using presets_container = std::unordered_set<int64_t>;

        #pragma pack(push, 1)
        struct view : configuration_parameter::view
        {
            int64_t value;
        };
        #pragma pack(pop)

        enum value_mode
        {
            value_mode_any,
            value_mode_range,
            value_mode_preset
        };

        /** @brief Constructs a new configuration_parameter_integer object for 'any' mode. */
        configuration_parameter_integer(const string_hashed& name, int64_t value = 0);

        /** @brief Constructs a new configuration_parameter_integer object for 'range' mode. */
        configuration_parameter_integer(const string_hashed& name, int64_t default_value, int64_t min_value, int64_t max_value);

        /** @brief Constructs a new configuration_parameter_integer object for 'preset' mode. */
        configuration_parameter_integer(const string_hashed& name, int64_t default_value, presets_container presets);

        /** @brief Destroys the configuration_parameter_integer object and cleans up resources. */
        ~configuration_parameter_integer();

        type get_type() const override { return type_integer; }
 
        /** @brief Creates a deep copy of this configuration parameter. */
        std::unique_ptr<configuration_parameter> clone() const override;

        int64_t get_value() const { return m_value; }
        bool set_value(int64_t value);
        int64_t& value() { return m_value; }

        int64_t get_default_value() const { return m_value_default; }
        void reset_to_default() { m_value = m_value_default; }

        value_mode get_mode() const { return m_mode; }
        void set_mode(value_mode mode) { m_mode = mode; }

        int64_t get_min_value() const { return m_min_value; }
        int64_t get_max_value() const { return m_max_value; }
        void set_range(int64_t min_value, int64_t max_value);

        const presets_container& get_presets() const { return m_presets; }
        void add_preset(int64_t preset);

        template<typename integer_type>
        integer_type get_value_as() const { return static_cast<integer_type>(m_value); }

    private:

        int64_t m_value;
        const int64_t m_value_default;
        
        value_mode m_mode;
        int64_t m_min_value;
        int64_t m_max_value;
        presets_container m_presets;
    };
}