#pragma once

/**
 * @file    commander.hpp
 * @author  dexus1337
 * @brief   Defines the ADAM commander which allows to send commands to the commander from external processes
 * @version 1.0
 * @date    25.04.2026
 */

 
#include "api/api-sdk.hpp"

#include "controller/controller.hpp"
#include "data/inspector.hpp"
#include "commander-event-dispatcher.hpp"
#include "registry-view.hpp"
#include "module-view.hpp"
#include "os/os.hpp"

#include <unordered_map>
#include <atomic>
#include <thread>
#include <functional>
#include <vector>


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

        static ADAM_CONSTEXPR uint32_t queue_command_size   = 0x1000;
        static ADAM_CONSTEXPR uint32_t queue_event_size     = 0x1000;

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
        response_status request_port_create(const string_hashed& name, string_hash type, string_hash type_module);

        /** @brief Requests the destruction of a port. */
        response_status request_port_destroy(string_hash port_hash);

        /** @brief Requests to start a port. */
        response_status request_port_start(string_hash port_hash);

        /** @brief Requests to stop a port. */
        response_status request_port_stop(string_hash port_hash);
        
        /** @brief Requests to rename a port. */
        response_status request_port_rename(string_hash old_hash, const string_hashed& new_name);

        /** @brief Requests to set a parameter of a port. */
        response_status request_port_parameter_set(string_hash port_hash, const string_hashed& param_name, int64_t value);
        response_status request_port_parameter_set(string_hash port_hash, const string_hashed& param_name, double value);
        response_status request_port_parameter_set(string_hash port_hash, const string_hashed& param_name, bool value);
        response_status request_port_parameter_set(string_hash port_hash, const string_hashed& param_name, const string_hashed& value);
        response_status request_port_parameter_set(string_hash port_hash, const string_hashed& param_name, const string_hashed_ct& value);

        /** @brief Requests to change the input data format of a connection. */
        response_status request_connection_set_input_data_format(string_hash conn_hash, string_hash format, string_hash format_module);

        /** @brief Requests to change the output data format of a connection. */
        response_status request_connection_set_output_data_format(string_hash conn_hash, string_hash format, string_hash format_module);

        /** @brief Requests the creation of a connection. */
        response_status request_connection_create(const string_hashed& name);

        /** @brief Requests the destruction of a connection. */
        response_status request_connection_destroy(string_hash hash);

        /** @brief Requests to start a connection. */
        response_status request_connection_start(string_hash hash);

        /** @brief Requests to stop a connection. */
        response_status request_connection_stop(string_hash hash);

        /** @brief Requests to rename a connection. */
        response_status request_connection_rename(string_hash old_hash, const string_hashed& new_name);

        /** @brief Requests to attach a port to a connection. */
        response_status request_connection_port_add(string_hash conn_hash, string_hash port_hash, bool is_input);

        /** @brief Requests to remove a port from a connection. */
        response_status request_connection_port_remove(string_hash conn_hash, string_hash port_hash, bool is_input);

        /** @brief Requests the creation of a processor. */
        response_status request_processor_create(const string_hashed& name, string_hash type, string_hash type_module, bool is_filter = false);

        /** @brief Requests the destruction of a processor. */
        response_status request_processor_destroy(string_hash processor_hash);

        /** @brief Requests to rename a processor. */
        response_status request_processor_rename(string_hash old_hash, const string_hashed& new_name);

        /** @brief Requests to attach a processor to a connection. */
        response_status request_connection_processor_add(string_hash conn_hash, string_hash processor_hash);

        /** @brief Requests to remove a processor from a connection. */
        response_status request_connection_processor_remove(string_hash conn_hash, string_hash processor_hash);

        /** @brief Requests to reorder a processor in a connection. */
        response_status request_connection_processor_reorder(string_hash conn_hash, string_hash processor_hash, uint32_t new_index);

        /** @brief Requests to set a parameter of a processor. */
        response_status request_processor_parameter_set(string_hash processor_hash, const string_hashed& param_name, int64_t value);
        response_status request_processor_parameter_set(string_hash processor_hash, const string_hashed& param_name, double value);
        response_status request_processor_parameter_set(string_hash processor_hash, const string_hashed& param_name, bool value);
        response_status request_processor_parameter_set(string_hash processor_hash, const string_hashed& param_name, const string_hashed& value);
        response_status request_processor_parameter_set(string_hash processor_hash, const string_hashed& param_name, const string_hashed_ct& value);

        /** @brief Requests to change the sorting index of a connection. */
        response_status request_connection_sorting_index_change(string_hash hash, uint32_t sorting_index);

        /** @brief Requests to change the color of a connection. */
        response_status request_connection_color_change(string_hash hash, uint32_t color);

        /** @brief Requests the creation of a data inspector on a specific port. */
        response_status request_inspector_create(string_hash port_hash, std::function<void(buffer*)> callback, data_inspector*& out_inspector);

        /** @brief Requests the destruction of a data inspector on a specific port. */
        response_status request_inspector_destroy(data_inspector* inspector);

        /** @brief Requests the creation of a data inspector on a connection input. */
        response_status request_connection_input_inspector_create(string_hash conn_hash, std::function<void(buffer*)> callback, data_inspector*& out_inspector);

        /** @brief Requests the destruction of a data inspector on a connection input. */
        response_status request_connection_input_inspector_destroy(data_inspector* inspector);

        /** @brief Requests the creation of a data inspector on a connection output. */
        response_status request_connection_output_inspector_create(string_hash conn_hash, std::function<void(buffer*)> callback, data_inspector*& out_inspector);

        /** @brief Requests the destruction of a data inspector on a connection output. */
        response_status request_connection_output_inspector_destroy(data_inspector* inspector);

        /** @brief Requests to inject data into a port. */
        response_status request_port_inject_data(string_hash port_hash, const void* data, size_t size, data_direction dir);

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

        std::unordered_map<string_hash, data_inspector*>& inspectors() { return m_inspectors; }
        const std::unordered_map<string_hash, data_inspector*>& get_inspectors() const { return m_inspectors; }

        std::unordered_map<string_hash, data_inspector*>& connection_input_inspectors() { return m_connection_input_inspectors; }
        const std::unordered_map<string_hash, data_inspector*>& get_connection_input_inspectors() const { return m_connection_input_inspectors; }

        std::unordered_map<string_hash, data_inspector*>& connection_output_inspectors() { return m_connection_output_inspectors; }
        const std::unordered_map<string_hash, data_inspector*>& get_connection_output_inspectors() const { return m_connection_output_inspectors; }

    protected:

        /** @brief Sends a command. Can have multiple (extended) responses. */
        response_status send_command(const command& cmd, response** resp = nullptr);

        /** @brief Sends multiple commands. Can have multiple (extended) responses. */
        response_status send_command(const command* cmds, size_t count, response** resp = nullptr);

        void run_event_loop();

        controller::queue_command   m_queue_command;
        controller::queue_event     m_queue_event;
        
        std::thread                 m_event_thread;
        commander_event_dispatcher  m_dispatcher;

        std::ostream                m_log_outstream;

        language_info               m_lang;

        std::unordered_map<string_hash, data_inspector*> m_inspectors;
        std::unordered_map<string_hash, data_inspector*> m_connection_input_inspectors;
        std::unordered_map<string_hash, data_inspector*> m_connection_output_inspectors;

        registry_view m_registry_view; /**< Local view of the controller's registry components */
        module_view   m_module_view;   /**< Local view of the controller's modules */
        
        std::vector<command>  m_command_buffer;  /**< Reusable buffer for building commands */
        std::vector<response> m_response_buffer; /**< Reusable buffer for receiving responses */
    };
}