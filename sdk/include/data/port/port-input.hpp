#pragma once

/**
 * @file    port-input.hpp
 * @author  dexus1337
 * @brief   Defines an input port for receiving data in the ADAM system.
 * @version 1.0
 * @date    25.04.2026
 */

 
#include "api/sdk-api.hpp"
#include "data/port/port.hpp"


namespace adam 
{
    /**
     * @class port_input
     * @brief An input port for receiving data in the ADAM system.
     */
    class ADAM_SDK_API port_input : public port
    {
    public:

        /** @brief Destroys the input port object and cleans up resources. */
        ~port_input();

        /** @brief Data management routine. Will forward data to connections */
        bool handle_data(buffer* buffer);

    protected:

        /** @brief Constructs a new input port object. */
        port_input(const string_hashed& item_name);

    };
}