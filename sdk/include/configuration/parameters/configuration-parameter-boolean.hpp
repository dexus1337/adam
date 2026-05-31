#pragma once

/**
 * @file    configuration-parameter-boolean.hpp
 * @author  dexus1337
 * @brief   Defines a boolean configuration parameter. True or False
 * @version 1.0
 * @date    05.05.2026
 */

 
#include "api/api-sdk.hpp"

#include <string_view>
#include "configuration/parameters/configuration-parameter.hpp"


namespace adam 
{
    /**
     * @class   configuration_parameter_boolean
     * @brief   Defines a boolean configuration parameter. True or False
     */
    class ADAM_SDK_API configuration_parameter_boolean : public configuration_parameter
    {
    public:
    
        #pragma pack(push, 1)
        struct view : configuration_parameter::view
        {
            bool value;
        };
        #pragma pack(pop)

        /** @brief Constructs a new configuration_parameter_boolean object. */
        configuration_parameter_boolean(const string_hashed& name, bool value = false);

        /** @brief Destroys the configuration_parameter_boolean object and cleans up resources. */
        ~configuration_parameter_boolean();

        type get_type() const override { return type_boolean; }
 
        /** @brief Creates a deep copy of this configuration parameter. */
        std::unique_ptr<configuration_parameter> clone() const override;

        bool get_value() const { return m_value; }
        void set_value(bool value) { m_value = value; }
        bool& value() { return m_value; }

        bool get_default_value() const { return m_value_default; }
        void reset_to_default() { m_value = m_value_default; }

    private:

        bool m_value;
        const bool m_value_default;
    };
}