#pragma once

/**
 * @file        memory-manager.hpp
 * @author      dexus1337
 * @brief       A singleton class responsible for managing memory buffers
 * @version     1.0
 * @date        25.04.2026
 */

 
#include "api/api.hpp"


namespace adam 
{
    /**
     * @class   memory_manager
     * @brief   A singleton class responsible for managing memory buffers
     */
    class ADAM_SDK_API memory_manager 
    {
    public:

        /**
         * @brief Constructs a new memory_manager object.
         */
        memory_manager();

        /**
         * @brief Destroys the memory_manager object and cleans up resources.
         */
        ~memory_manager();
    };
}