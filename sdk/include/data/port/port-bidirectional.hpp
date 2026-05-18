#pragma once

/**
 * @file    port-bidirectional.hpp
 * @author  dexus1337
 * @brief   Defines a bidirectional port for both receiving and sending data in the ADAM system.
 * @version 1.0
 * @date    17.05.2026
 */

 
#include "api/api-sdk.hpp"
#include "data/port/port-input.hpp"


namespace adam 
{
    /**
     * @class port_bidirectional
     * @brief A bidirectional port for receiving and sending data in the ADAM system.
     *        Inherits from port_input to natively reuse the threading and queue logic,
     *        while simultaneously functioning as an output via the base port's connections.
     */
    class ADAM_SDK_API port_bidirectional : public port_input
    {
    public:
        static constexpr port_direction direction = port_direction::bidirectional;

        /** @brief Destroys the bidirectional port object and cleans up resources. */
        ~port_bidirectional();

        /** @brief Gets the supported data flow direction capabilities of this port. */
        port_direction get_direction() const override { return direction; }

    protected:

        /** @brief Constructs a new bidirectional port object. */
        port_bidirectional(const string_hashed& item_name);

    };
}