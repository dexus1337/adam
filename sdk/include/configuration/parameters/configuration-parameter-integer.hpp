#pragma once

/**
 * @file    configuration-parameter-integer.hpp
 * @author  dexus1337
 * @brief   Defines a an array of configuration parameters, used for defining multiple related configuration parameters in a structured way.
 * @version 1.0
 * @date    05.05.2026
 */

 
#include "api/sdk-api.hpp"

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

        /** @brief Destroys the configuration_parameter_integer object and cleans up resources. */
        ~configuration_parameter_integer();

        type get_type() const override { return integer; }
 
        /** @brief Creates a deep copy of this configuration parameter. */
        std::unique_ptr<configuration_parameter> clone() const override;

        int64_t get_value() const { return m_value; }
        void set_value(int64_t value) { m_value = value; }
        int64_t& value() { return m_value; }

        int64_t get_default_value() const { return m_value_default; }
        void reset_to_default() { m_value = m_value_default; }

        template<typename integer_type>
        integer_type get_value_as() const { return static_cast<integer_type>(m_value); }

    private:

        int64_t m_value;
        const int64_t m_value_default;
    };
}