#pragma once

/**
 * @file    configuration-parameter.hpp
 * @author  dexus1337
 * @brief   Defines a base class for any configuration parameters
 * @version 1.0
 * @date    05.05.2026
 */

 
#include "api/api.hpp"

#include <memory>
#include "string/string-hashed.hpp"

namespace adam 
{
    /**
     * @class configuration_parameter
     * @brief Defines a base class for any configuration parameters
     */
    class ADAM_SDK_API configuration_parameter 
    {
    public:

        enum type : uint8_t
        {
            invalid = 0,
            integer,
            double_,
            boolean,
            string,
            list,
            reference
        };

        virtual type get_type()         const = 0;
        const string_hashed& get_name() const { return m_str_name; }

        /** @brief Creates a deep copy of this configuration parameter. */
        virtual std::unique_ptr<configuration_parameter> clone() const = 0;

        /** @brief Destroys the configuration_parameter object and cleans up resources. */
        virtual ~configuration_parameter();

    protected:

        /** @brief Constructs a new configuration_parameter object. */
        configuration_parameter(const string_hashed& name);

        /** @brief Constructs a new configuration_parameter object. */
        configuration_parameter(string_hashed::view name);

        string_hashed m_str_name;     /**< The name of the configuration parameter, used for identification and lookup in the ADAM system. */
    };
}