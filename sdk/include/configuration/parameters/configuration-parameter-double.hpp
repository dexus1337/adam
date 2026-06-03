#pragma once

/**
 * @file    configuration-parameter-double.hpp
 * @author  dexus1337
 * @brief   Defines a decimal configuration parameter. Double for better precision.
 * @version 1.0
 * @date    05.05.2026
 */

 
#include "api/api-sdk.hpp"

#include <string_view>
#include <vector>
#include "configuration/parameters/configuration-parameter.hpp"


namespace adam 
{
    /**
     * @class   configuration_parameter_double
     * @brief   Defines a decimal configuration parameter. Double for better precision.
     */
    class ADAM_SDK_API configuration_parameter_double : public configuration_parameter
    {
    public:
    
        using presets_container = std::vector<double>;

        #pragma pack(push, 1)
        struct view : configuration_parameter::view
        {
            double value;
        };
        #pragma pack(pop)

        enum value_mode
        {
            value_mode_any,
            value_mode_range,
            value_mode_preset
        };

        /** @brief Constructs a new configuration_parameter_double object for 'any' mode. */
        configuration_parameter_double(const string_hashed& name, double value = 0.0);

        /** @brief Constructs a new configuration_parameter_double object for 'range' mode. */
        configuration_parameter_double(const string_hashed& name, double default_value, double min_value, double max_value);

        /** @brief Constructs a new configuration_parameter_double object for 'preset' mode. */
        configuration_parameter_double(const string_hashed& name, double default_value, presets_container presets);

        /** @brief Destroys the configuration_parameter_double object and cleans up resources. */
        ~configuration_parameter_double();

        type get_type() const override { return type_double; }
 
        /** @brief Creates a deep copy of this configuration parameter. */
        std::unique_ptr<configuration_parameter> clone() const override;

        double get_value() const { return m_value; }
        bool set_value(double value);
        double& value() { return m_value; }

        double get_default_value() const { return m_value_default; }
        void reset_to_default() { m_value = m_value_default; }

        value_mode get_mode() const { return m_mode; }
        void set_mode(value_mode mode) { m_mode = mode; }

        double get_min_value() const { return m_min_value; }
        double get_max_value() const { return m_max_value; }
        void set_range(double min_value, double max_value);

        const presets_container& get_presets() const { return m_presets; }
        void add_preset(double preset);

    private:

        double m_value;
        const double m_value_default;

        value_mode m_mode;
        double m_min_value;
        double m_max_value;
        presets_container m_presets;
    };
}