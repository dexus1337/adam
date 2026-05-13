#pragma once

/**
 * @file    commander.hpp
 * @author  dexus1337
 * @brief   Defines the ADAM commander which allows to send commands to the commander from external processes
 * @version 1.0
 * @date    25.04.2026
 */

 
#include "api/sdk-api.hpp"

#include "controller/controller.hpp"
#include "data/inspector.hpp"
#include "commander-event-dispatcher.hpp"

#include <unordered_map>
#include <atomic>
#include <thread>
#include <functional>


namespace adam 
{
    class command;
    class response;
    class event;

    /**
     * @class commander
     * @brief Defines the ADAM commander which allows to send commands to the controller from external processes
     */
    class ADAM_SDK_API commander
    {
        friend class commander_event_dispatcher;

    public:

        /** @brief Constructs a new commander object.*/
        commander();

        /** @brief Destroys the commander object and cleans up resources.*/
        ~commander();

        bool is_active() const { return m_queue_command.is_active() && m_queue_event.is_active(); }

        /** @brief Establishes a connection to the main commander. */
        bool connect();

        /** @brief Disable all functionality. */
        bool disable();

        /** @brief Disconnect and free resources. */
        bool destroy();

        /** @brief Requests the initial data from the controller. */
        response_status request_initial_data();

        /** @brief Requests the creation of a port. */
        response_status request_port_create(const string_hashed& name, const string_hashed& type, const string_hashed& module_name = string_hashed());

        /** @brief Requests the destruction of a port. */
        response_status request_port_destroy(const string_hashed& port_name);

        /** @brief Requests the creation of a connection. */
        response_status request_connection_create(const string_hashed& name);

        /** @brief Requests the destruction of a connection. */
        response_status request_connection_destroy(const string_hashed& name);

        /** @brief Requests the creation of a data inspector on a specific port. */
        response_status request_inspector_create(const string_hashed& port_name, std::function<void(buffer*)> callback, data_inspector*& out_inspector);

        /** @brief Requests the destruction of a data inspector on a specific port. */
        response_status request_inspector_destroy(data_inspector* inspector);

        /** @brief Requests a language change. */
        response_status request_language_change(language lang);

        /** @brief Gets the current language. */
        language get_language() const { return m_lang.lang; }

        /** @brief Gets the available languages. */
        uint32_t get_available_languages() const { return m_lang.supported_languages; }

        /** @brief Retrieves a reference to the cache of available modules. */
        const controller_module_manager::map_available_modules& get_available_modules() const { return m_available_modules_cache; }

        /** @brief Retrieves a reference to the cache of unavailable modules. */
        const controller_module_manager::map_unavailable_modules& get_unavailable_modules() const { return m_unavailable_modules_cache; }

        /** @brief Retrieves a reference to the cache of loaded modules. */
        const controller_module_manager::map_loaded_modules& get_loaded_modules() const { return m_loaded_modules_cache; }

        commander_event_dispatcher& dispatcher() { return m_dispatcher; }
        const commander_event_dispatcher& get_dispatcher() const { return m_dispatcher; }

    protected:

        /** @brief Sends a command. Can have multiple (extended) responses. */
        response_status send_command(const command& cmd, response** resp = nullptr);

        void run_event_loop();

        controller::queue_command   m_queue_command;
        controller::queue_event     m_queue_event;
        
        std::thread                 m_event_thread;
        commander_event_dispatcher  m_dispatcher;

        std::ostream                m_log_outstream;

        language_info               m_lang;

        std::unordered_map<string_hashed::hash_datatype, data_inspector*> m_inspectors;

        controller_module_manager::map_available_modules    m_available_modules_cache;      /**< A cache of available modules, used for quick lookup when receiving initial data. */
        controller_module_manager::map_unavailable_modules  m_unavailable_modules_cache;    /**< A cache of unavailable modules, used for quick lookup when receiving initial data. */
        controller_module_manager::map_loaded_modules       m_loaded_modules_cache;         /**< A cache of loaded modules, used for quick lookup when receiving initial data. */
    };
}