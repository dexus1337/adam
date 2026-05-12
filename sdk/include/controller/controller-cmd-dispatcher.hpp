#pragma once

/**
 * @file    controller-cmd-dispatcher.hpp
 * @author  dexus1337
 * @brief   Defines the command dispatcher and context used to route controller commands.
 * @version 1.0
 * @date    09.05.2026
 */


#include "api/sdk-api.hpp"
#include "commander/messages/command.hpp"
#include "commander/messages/response.hpp"
#include "resources/language.hpp"
#include "os/os.hpp"
#include "types/string-hashed.hpp"

#include <unordered_map>
#include <memory>
#include <functional>

namespace adam 
{
    class registry;
    class data_inspector;
    class controller;

    /**
     * @struct  command_context
     * @brief   Holds the execution context required to process a command.
     */
    struct command_context
    {
        os::thread_id tid;  /**< The ID of the thread that dispatched the command. */
        registry& reg;      /**< A reference to the controller's registry. */
        controller& ctrl;   /**< A reference to the main controller instance. */
        std::unordered_map<string_hashed::hash_datatype, std::shared_ptr<data_inspector>> thread_inspectors; /**< A map of data inspectors managed by this thread context. */
    };

    /**
     * @class   controller_cmd_dispatcher
     * @brief   Dispatches incoming commands to their registered handler functions.
     */
    class ADAM_SDK_API controller_cmd_dispatcher
    {
    public:
        enum log_event
        {
            language_changed,
            inspector_created,
            inspector_destroyed,
            inspector_create_failed_port_unknown,
            inspector_create_failed_open,
            inspector_destroy_failed_port_unknown,
            inspector_destroy_failed_not_found
        };

        static std::string_view get_log_event_text(log_event event, language lang);

        using handler_fn = std::function<response(const command*, size_t, command_context&)>; /**< A type alias for a command handler function. */

        /** @brief Registers a new command handler for a specific command type. */
        void register_handler(int type, handler_fn handler);
        
        /** @brief Dispatches a command to the appropriate registered handler. */
        response dispatch(const command* cmds, size_t count, command_context& ctx) const;

        /** @brief Registers all default command handlers for the controller. */
        void register_default_handlers();

    private:
        std::unordered_map<int, handler_fn> m_handlers; /**< A map of registered command handlers, indexed by their command type. */
    };
}