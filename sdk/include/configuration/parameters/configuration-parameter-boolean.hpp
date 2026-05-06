#pragma once

/**
 * @file    configuration-parameter-boolean.hpp
 * @author  dexus1337
 * @brief   Defines a an array of configuration parameters, used for defining multiple related configuration parameters in a structured way.
 * @version 1.0
 * @date    05.05.2026
 */

 
#include "api/api.hpp"

#include <string_view>
#include "configuration/parameters/configuration-parameter.hpp"


namespace adam 
{
    /**
     * @class configuration_parameter_boolean
     * @brief Defines a an array of configuration parameters, used for defining multiple related configuration parameters in a structured way.
     */
    class ADAM_SDK_API configuration_parameter_boolean : public configuration_parameter
    {
    public:
        /** @brief Constructs a new configuration_parameter_boolean object. */
        configuration_parameter_boolean(const string_hashed& name, bool value = false);
        configuration_parameter_boolean(string_hashed::view name, bool value = false);

        /** @brief Destroys the configuration_parameter_boolean object and cleans up resources. */
        ~configuration_parameter_boolean();

        type get_type() const override { return boolean; }

        /** @brief Creates a deep copy of this configuration parameter. */
        std::unique_ptr<configuration_parameter> clone() const override { return std::make_unique<configuration_parameter_boolean>(m_str_name, m_value); }

        bool get_value() const { return m_value; }
        void set_value(bool value) { m_value = value; }

    private:

        bool m_value;
    };
}