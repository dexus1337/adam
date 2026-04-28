#pragma once

/**
 * @file    port-output.hpp
 * @author  dexus1337
 * @brief   Defines an output port for sending data in the ADAM system.
 * @version 1.0
 * @date    25.04.2026
 */

 
#include "api/api.hpp"
#include "port/port.hpp"


namespace adam 
{
    /**
     * @class port_output
     * @brief An output port for sending data in the ADAM system.
     */
    class ADAM_SDK_API port_output : public port
    {
    public:

        /**
         * @brief Constructs a new output port object.
         */
        port_output();

        /**
         * @brief Destroys the output port object and cleans up resources.
         */
        ~port_output();

    };
}