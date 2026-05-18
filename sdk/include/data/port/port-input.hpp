#pragma once

/**
 * @file    port-input.hpp
 * @author  dexus1337
 * @brief   Defines an input port for receiving data in the ADAM system.
 * @version 1.0
 * @date    25.04.2026
 */

 
#include "api/api-sdk.hpp"
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
        static constexpr port_direction direction = port_direction::input;

        /** @brief Destroys the input port object and cleans up resources. */
        ~port_input();

        /** @brief Gets the supported data flow direction capabilities of this port. */
        port_direction get_direction() const override { return direction; }

        /** @brief Data management routine. Will forward data to connections */
        bool handle_data(buffer* buffer);

    protected:

        /** @brief Constructs a new input port object. */
        port_input(const string_hashed& item_name);

    };
}