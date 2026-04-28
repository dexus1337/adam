#pragma once

/**
 * @file    configuration-item.hpp
 * @author  dexus1337
 * @brief   Defines a base class for any configuration objects
 * @version 1.0
 * @date    25.04.2026
 */

 
#include "api/api.hpp"


namespace adam 
{
    /**
     * @class configuration_item
     * @brief Defines a base class for any configuration objects
     */
    class ADAM_SDK_API configuration_item 
    {
    public:

        /**
         * @brief Constructs a new configuration_item object.
         */
        configuration_item();

        /**
         * @brief Destroys the configuration_item object and cleans up resources.
         */
        ~configuration_item();

    };
}