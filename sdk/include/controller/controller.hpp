#pragma once

/**
 * @file        controller.hpp
 * @author      dexus1337
 * @brief       Defines the core ADAM controller class which manages the entire system
 * @version     1.0
 * @date        25.04.2026
 */

 
#include "api/api.hpp"

#include <unordered_map>
#include <thread>

#include "string/string-hashed.hpp"
#include "queue/queue-shared.hpp"
#include "controller/command/command.hpp"


namespace adam 
{
    class module;

    /**
     * @class controller
     * @brief The main controller class for the ADAM system, responsible for managing all components and orchestrating their interactions.
     */
    class ADAM_SDK_API controller 
    {
        friend class commander;

    public:

        /** @brief Constructs a new controller object.*/
        controller();

        /** @brief Destroys the controller object and cleans up resources.*/
        ~controller();

        /** @brief Starts the command processing loop, which continuously checks for new command queue requests. */
        bool start(bool async = false);
        
        /** @brief Stops all queues and frees resources. */
        bool destroy();
        
        // COMMAND MANAGEMENT

        // MODULE MANAGEMENT

        /** @brief Retrieves a reference to the map of all available modules. */
        const std::unordered_map<string_hashed, string_hashed>& get_available_modules() const { return m_available_modules; }

        /** @brief Retrieves a reference to the map of all loaded modules. */
        const std::unordered_map<string_hashed, const module*>& get_loaded_modules() const { return m_loaded_modules; }

        /** @brief Retrieves a pointer to a loaded module by its hashed name. */
        const module* get_loaded_module(const string_hashed& name) const;

        /** @brief Scans the specified directory for module shared libraries, loads them, and registers their modules in the system. */
        bool scan_for_modules(std::string_view directory = "");

        /** @brief Loads a module by its hashed name from the available modules and registers it in the loaded modules map. */
        bool load_module(const string_hashed& name, const module** out_module = nullptr);

    protected:

        // COMMAND MANAGEMENT

        struct command_queue_request_data
        {
            uint32_t thread_id;
            uint32_t code;
        };

        struct command_queue_process_data
        {
            command_queue_process_data(const string_hashed& name) : queue(name) {}

            queue_shared<command>   queue;
            bool                    running;
            std::thread             command_processing_thread;
        };

        static constexpr const char* command_request_queue_name = "adam::controller_command_request_queue"; /**< The name of the queue which will create new "per process" command queues */
        static constexpr const char* command_queue_prefix       = "adam::controller_command_queue_";        /**< Prefix for the "per process" command queue */

        void run_request_command_queue();

        void run_process_command_queue(command_queue_process_data* data);
        bool stop_and_remove_process_queue(uint32_t process_id);

        queue_shared<command_queue_request_data>    m_request_command_queue;
        std::thread                                 m_command_processing_thread;
        bool                                        m_request_command_queue_running;

        std::unordered_map<uint32_t, command_queue_process_data*> m_process_queues;


        // MODULE MANAGEMENT

        std::unordered_map<string_hashed, string_hashed> m_available_modules;   /**< A map of available modules in the system, indexed by their hashed string names for efficient lookup. */
        std::unordered_map<string_hashed, const module*> m_loaded_modules;      /**< A map of loaded modules in the system, indexed by their hashed string names for efficient lookup. */

        
    };
}