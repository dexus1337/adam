#pragma once

/**
 * @file    configuration-parameter-integer.hpp
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
     * @class configuration_parameter_integer
     * @brief Defines a an array of configuration parameters, used for defining multiple related configuration parameters in a structured way.
     */
    class ADAM_SDK_API configuration_parameter_integer : public configuration_parameter
    {
    public:
        /** @brief Constructs a new configuration_parameter_integer object. */
        configuration_parameter_integer(const string_hashed& name, int64_t value = 0);
        configuration_parameter_integer(string_hashed::view name, int64_t value = 0);

        /** @brief Destroys the configuration_parameter_integer object and cleans up resources. */
        ~configuration_parameter_integer();

        type get_type() const override { return integer; }

        /** @brief Creates a deep copy of this configuration parameter. */
        std::unique_ptr<configuration_parameter> clone() const override { return std::make_unique<configuration_parameter_integer>(m_str_name, m_value); }

        int64_t get_value() const { return m_value; }
        void set_value(int64_t value) { m_value = value; }

    private:

        int64_t m_value;
    };
}