#pragma once

/**
 * @file    port-in-out.hpp
 * @author  dexus1337
 * @brief   Defines a in-out port for both receiving and sending data in the ADAM system.
 * @version 1.0
 * @date    17.05.2026
 */

 
#include "api/api-sdk.hpp"
#include "data/port/port-input.hpp"


namespace adam 
{
    /**
     * @class port_in_out
     * @brief An InOut port for receiving and sending data in the ADAM system.
     *        Inherits from port_input to natively reuse the threading and queue logic,
     *        while simultaneously functioning as an output via the base port's connections.
     */
    class ADAM_SDK_API port_in_out : public port_input
    {
    public:
        static ADAM_CONSTEXPR port_direction direction = port_direction::in_out;

        /** @brief Destroys the in_out port object and cleans up resources. */
        ~port_in_out();

        /** @brief Gets the supported data flow direction capabilities of this port. */
        port_direction get_direction() const override { return direction; }

    protected:

        /** @brief Constructs a new in_out port object. */
        port_in_out(const string_hashed& item_name);

    };
}