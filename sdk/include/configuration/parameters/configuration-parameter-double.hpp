#pragma once

/**
 * @file    configuration-parameter-double.hpp
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
     * @class configuration_parameter_double
     * @brief Defines a an array of configuration parameters, used for defining multiple related configuration parameters in a structured way.
     */
    class ADAM_SDK_API configuration_parameter_double : public configuration_parameter
    {
    public:
        /** @brief Constructs a new configuration_parameter_double object. */
        configuration_parameter_double(const string_hashed& name, double value = 0.0);
        configuration_parameter_double(string_hashed::view name, double value = 0.0);

        /** @brief Destroys the configuration_parameter_double object and cleans up resources. */
        ~configuration_parameter_double();

        type get_type() const override { return double_; }

        /** @brief Creates a deep copy of this configuration parameter. */
        std::unique_ptr<configuration_parameter> clone() const override { return std::make_unique<configuration_parameter_double>(m_str_name, m_value); }

        double get_value() const { return m_value; }
        void set_value(double value) { m_value = value; }

    private:

        double m_value;
    };
}