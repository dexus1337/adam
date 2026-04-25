#pragma once

/**
 * @file        port-input.hpp
 * @author      dexus1337
 * @brief       Defines an input port for receiving data in the ADAM system.
 * @version     1.0
 * @date        25.04.2026
 */

 
#include "api/api.hpp"
#include "port/port.hpp"


namespace adam 
{
    /**
     * @class port_input
     * @brief An input port for receiving data in the ADAM system.
     */
    class ADAM_SDK_API port_input : public port
    {
    public:

        /**
         * @brief Constructs a new input port object.
         */
        port_input();

        /**
         * @brief Destroys the input port object and cleans up resources.
         */
        ~port_input();

    };
}