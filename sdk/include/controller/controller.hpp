#pragma once

/**
 * @file    controller.hpp
 * @author  dexus1337
 * @brief   The controller class for the ADAM system, responsible for managing all components and orchestrating their interactions.
 * @version 1.0
 * @date    25.04.2026
 */
 
 
#include "api/api.hpp"

#include <unordered_map>
#include <thread>
#include <sstream>

#include "string/string-hashed.hpp"
#include "queue/queue-shared-duplex.hpp"
#include "commander/command.hpp"
#include "controller/response/response.hpp"
#include "logger/log.hpp"
#include "os/os.hpp"


namespace adam 
{
    class module;

    /**
     * @class   controller
     * @brief   The controller class for the ADAM system, responsible for managing all components and orchestrating their interactions.
     *          
     *          It uses multiple (duplex) qeues in shared memory. These queue are thread-unique and can
     *          be requested using the master queue (also returned).
     * 
     *          queue_command:          duplex queue    -> Retrieves commands, gives responses
     *          queue_log               one-way queue   -> Retrieves logs, outputs to all registered queue_log_sink
     *          queue_log_sink:         one-way queue   -> Forwards all recieved logs
     */
    class ADAM_SDK_API controller 
    {
        friend class commander;
        friend class logger;

    public:

        /** @brief Constructs a new controller object.*/
        controller();

        /** @brief Destroys the controller object and cleans up resources.*/
        ~controller();

        /** @brief Runs the command processing loop, which continuously checks for new command queue requests. */
        bool run(bool async = false);
        
        /** @brief Stops all queues and frees resources. */
        bool destroy();
        
        // COMMAND MANAGEMENT

        enum master_queue_request
        {
            request_invalid = 0,
            request_command,
            request_log,
            request_log_sink
        };

        enum master_queue_response
        {
            response_invalid = 0,
            response_success,
            response_existing,
            response_unavailable,
            response_unauthorized
        };

        // LOG MANAGEMENT

        /** @brief Outputs a log. */
        void log(const log& cr_log);

        /** @brief Outputs a log. */
        void log(std::string_view txt, log::level t) { this->log(adam::log(txt, t)); }

        // MODULE MANAGEMENT

        using map_available_modules = std::unordered_map<string_hashed, std::pair<uint32_t, string_hashed>>;
        using map_loaded_modules    = std::unordered_map<string_hashed, const module*>;

        /** @brief Retrieves a reference to the map of all available modules. */
        const map_available_modules& get_available_modules() const { return m_available_modules; }

        /** @brief Retrieves a reference to the map of all loaded modules. */
        const map_loaded_modules& get_loaded_modules() const { return m_loaded_modules; }

        /** @brief Retrieves a pointer to a loaded module by its hashed name. */
        const module* get_loaded_module(const string_hashed& name) const;

        /** @brief Scans the specified directory for module shared libraries, loads them, and registers their modules in the system. */
        bool scan_for_modules(std::string_view directory = "");

        /** @brief Loads a module by its hashed name from the available modules and registers it in the loaded modules map. */
        bool load_module(const string_hashed& name, const module** out_module = nullptr);

    protected:

        // COMMAND MANAGEMENT

        /** @brief Uses the master queue in order to request the controller to listen on the unique thread command queue. */
        static bool request_queue_command_access();

        /** @brief Uses the master queue in order to request the controller to listen on the unique thread command queue. */
        static bool request_log_queue_access();


        // MASTER QUEUE
        struct master_queue_request_data
        {
            os::thread_id           tid;
            master_queue_request    queue;
            uint64_t                code;
        };

        using master_queue = queue_shared_duplex<master_queue_request_data, master_queue_response>; /**< A queue to request a queue, sound kinda retarded but its quite literlly this */
        static constexpr const char* master_queue_name = "adam::controller_master_queue";           /**< The name of the queue which will create new "per process" command queues */
        
        void run_master_queue();

        master_queue    m_master_queue;
        std::thread     m_master_queue_thread;
        bool            m_master_queue_running;

        // REQUEST/CMD QUEUES

        using queue_command = queue_shared_duplex<command, response>;                               /**< cmd/resp queue type */
        static constexpr const char* queue_command_prefix = "adam::controller_queue_command_";      /**< Prefix for the "per process" command queue, the target threadid will be added, is unique on all supported OS (for running threads atleast) */

        struct queue_command_process_data
        {
            queue_command_process_data(const string_hashed& name) : queue(name) {}

            queue_command   queue;
            bool            running;
            std::thread     queue_thread;
        };

        void run_process_queue_command(queue_command_process_data* data);
        bool destroy_process_queue_command(os::thread_id tid);

        std::unordered_map<os::thread_id, queue_command_process_data*> m_process_queues_command;

        // LOG QUEUES

        using queue_log = queue_shared<adam::log>;                                                  /**< Log queue type */
        static constexpr const char* queue_log_prefix = "adam::controller_queue_log_";              /**< Prefix for the "per process" log queue, the target threadid will be added, is unique on all supported OS (for running threads atleast) */

        struct queue_log_process_data
        {
            queue_log_process_data(const string_hashed& name) : queue(name) {}

            queue_log   queue;
            bool        running;
            std::thread queue_thread;
        };

        void run_process_queue_log(queue_log_process_data* data);
        bool destroy_process_queue_log(os::thread_id tid);

        std::unordered_map<os::thread_id, queue_log_process_data*> m_process_queues_log;


        // MODULE MANAGEMENT

        map_available_modules   m_available_modules;   /**< A map of available modules in the system, indexed by their hashed string names for efficient lookup. */
        map_loaded_modules      m_loaded_modules;      /**< A map of loaded modules in the system, indexed by their hashed string names for efficient lookup. */

        
    };
}