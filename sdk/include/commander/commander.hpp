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
#include "registry-view.hpp"
#include "module-view.hpp"

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

        /** @brief Requests to add a new module path. */
        response_status request_module_path_add(const string_hashed& path, uint32_t index = 0);

        /** @brief Requests to remove an existing module path. */
        response_status request_module_path_remove(uint32_t index);

        /** @brief Requests a module scan. */
        response_status request_module_scan();

        /** @brief Requests to load a module. */
        response_status request_module_load(const string_hashed& name);

        /** @brief Requests to unload a module. */
        response_status request_module_unload(const string_hashed& name);

        /** @brief Requests the creation of a port. */
        response_status request_port_create(const string_hashed& name, string_hashed::hash_datatype type, string_hashed::hash_datatype module = 0);

        /** @brief Requests the destruction of a port. */
        response_status request_port_destroy(const string_hashed& port_name);

        /** @brief Requests to start a port. */
        response_status request_port_start(const string_hashed& port_name);

        /** @brief Requests to stop a port. */
        response_status request_port_stop(const string_hashed& port_name);

        /** @brief Requests the creation of a connection. */
        response_status request_connection_create(const string_hashed& name);

        /** @brief Requests the destruction of a connection. */
        response_status request_connection_destroy(const string_hashed& name);

        /** @brief Requests to start a connection. */
        response_status request_connection_start(const string_hashed& name);

        /** @brief Requests to stop a connection. */
        response_status request_connection_stop(const string_hashed& name);

        /** @brief Requests to rename a connection. */
        response_status request_connection_rename(const string_hashed& old_name, const string_hashed& new_name);

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

        /** @brief Retrieves a reference to the local module view. */
        module_view& modules() { return m_module_view; }
        const module_view& get_modules() const { return m_module_view; }

        /** @brief Retrieves a reference to the local registry view. */
        registry_view& registry() { return m_registry_view; }
        const registry_view& get_registry() const { return m_registry_view; }

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

        registry_view m_registry_view; /**< Local view of the controller's registry components */
        module_view   m_module_view;   /**< Local view of the controller's modules */
    };
}