#pragma once

/**
 * @file    event.hpp
 * @author  dexus1337
 * @brief   Defines an event for the controller to broadcast state changes asynchronously to commanders.
 * @version 1.0
 * @date    12.05.2026
 */

#include "api/sdk-api.hpp"

#include <cstdint>
#include <cstddef>

#include "type.hpp"

namespace adam 
{
    enum class event_type : uint8_t
    {
        invalid = 0,

        language_changed,
        port_added,
        port_removed,
        filter_updated
    };

    /**
     * @class event
     * @brief Defines an event for the controller to broadcast state changes
     */
    #pragma pack(push, 1) // align to 1 byte
    class ADAM_SDK_API event : public commander_message_type<event_type>
    {
    public:

        /** @brief Constructs a new event object.*/
        event(event_type t = event_type::invalid) : commander_message_type(t) {}

        /** @brief Destroys the event object and cleans up resources.*/
        ~event() = default;
    };
    #pragma pack(pop)
}