#pragma once

/**
 * @file    commander-event-dispatcher.hpp
 * @author  dexus1337
 * @brief   Defines the event dispatcher and context used to route commander events.
 * @version 1.0
 * @date    12.05.2026
 */

#include "api/sdk-api.hpp"
#include "commander/messages/event.hpp"

#include <unordered_map>
#include <functional>

namespace adam 
{
    class commander;

    /**
     * @struct  event_context
     * @brief   Holds the execution context required to process an event.
     */
    struct event_context
    {
        commander& cmdr; /**< A reference to the commander instance. */
    };

    /**
     * @class   commander_event_dispatcher
     * @brief   Dispatches incoming events to their registered handler functions.
     */
    class ADAM_SDK_API commander_event_dispatcher
    {
    public:
        using handler_fn = std::function<void(const event&, event_context&)>; /**< A type alias for an event handler function. */

        /** @brief Constructs a new commander_event_dispatcher object. */
        commander_event_dispatcher();
        
        /** @brief Destroys the commander_event_dispatcher object and cleans up resources. */
        ~commander_event_dispatcher();

        /** @brief Registers a new event handler for a specific event type. */
        void register_handler(int type, handler_fn handler);
        
        /** @brief Dispatches an event to the appropriate registered handler. */
        void dispatch(const event& e, event_context& ctx) const;

        /** @brief Registers all default event handlers for the commander. */
        void register_default_handlers();

    private:
        std::unordered_map<int, handler_fn> m_handlers; /**< A map of registered event handlers, indexed by their event type. */
    };
}