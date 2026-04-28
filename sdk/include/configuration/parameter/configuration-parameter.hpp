#pragma once

/**
 * @file    configuration-parameter.hpp
 * @author  dexus1337
 * @brief   Defines a base class for any configuration parameters
 * @version 1.0
 * @date    25.04.2026
 */

 
#include "api/api.hpp"


namespace adam 
{
    /**
     * @class configuration_parameter
     * @brief Defines a base class for any configuration parameters
     */
    class ADAM_SDK_API configuration_parameter 
    {
    public:

        /**
         * @brief Constructs a new configuration_parameter object.
         */
        configuration_parameter();

        /**
         * @brief Destroys the configuration_parameter object and cleans up resources.
         */
        ~configuration_parameter();

    };
}