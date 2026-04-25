#pragma once

/**
 * @file        module.hpp
 * @author      dexus1337
 * @brief       A base class for any external module that can be loaded into the ADAM system, providing a common interface for initialization and cleanup.
 * @version     1.0
 * @date        25.04.2026
 */

 
#include "api/api.hpp"


namespace adam 
{
    class input_port;
    class output_port;
    class filter;
    class converter;
    
    /**
     * @class   module
     * @brief   A base class for any external module that can be loaded into the ADAM system, providing a common interface for initialization and cleanup.
     */
    class ADAM_SDK_API module 
    {
    public:

        /**
         * @brief Constructs a new module object.
         */
        module();

        /**
         * @brief Destroys the module object and cleans up resources.
         */
        ~module();

    };
}