#pragma once

/**
 * @file    configuration-parameter-reference.hpp
 * @author  dexus1337
 * @brief   Defines a an reference of another configuration parameter, used for definning relations between configuration parameters in a structured way.
 * @version 1.0
 * @date    05.05.2026
 */

 
#include "api/api.hpp"

#include <string_view>
#include "configuration/parameters/configuration-parameter.hpp"


namespace adam 
{
    /**
     * @class configuration_parameter_reference
     * @brief Defines a an reference of another configuration parameter, used for definning relations between configuration parameters in a structured way.
     */
    class ADAM_SDK_API configuration_parameter_reference : public configuration_parameter
    {
    public:
        /** @brief Constructs a new configuration_parameter_reference object. */
        configuration_parameter_reference(const string_hashed& name, const string_hashed& target = string_hashed());
        configuration_parameter_reference(string_hashed::view name, const string_hashed& target = string_hashed());

        /** @brief Destroys the configuration_parameter_reference object and cleans up resources. */
        ~configuration_parameter_reference();

        type get_type() const override { return reference; }

        const string_hashed& get_target() const { return m_target; }
        void set_target(const string_hashed& target) { m_target = target; }

    private:

        string_hashed m_target;
    };
}