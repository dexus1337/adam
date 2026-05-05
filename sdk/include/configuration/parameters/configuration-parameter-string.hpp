#pragma once

/**
 * @file    configuration-parameter-string.hpp
 * @author  dexus1337
 * @brief   Defines a string configuration parameter, used for defining a single string configuration value.
 * @version 1.0
 * @date    05.05.2026
 */

 
#include "api/api.hpp"

#include <string_view>
#include "configuration/parameters/configuration-parameter.hpp"


namespace adam 
{
    /**
     * @class configuration_parameter_string
     * @brief Defines a string configuration parameter, used for defining a single string configuration value.
     */
    class ADAM_SDK_API configuration_parameter_string : public configuration_parameter
    {
    public:
        /** @brief Constructs a new configuration_parameter_string object. */
        configuration_parameter_string(const string_hashed& name, std::string_view value = "");
        configuration_parameter_string(std::string_view name, std::string_view value = "");

        /** @brief Destroys the configuration_parameter_string object and cleans up resources. */
        ~configuration_parameter_string();

        type get_type() const override { return string; }

        const std::string& get_value() const { return m_value; }
        void set_value(std::string_view value) { m_value = value; }

    private:

        std::string m_value;
    };
}