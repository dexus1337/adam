#pragma once

/**
 * @file    controller.hpp
 * @author  dexus1337
 * @brief   The controller class for the ADAM system, responsible for managing all components and orchestrating their interactions.
 * @version 1.0
 * @date    25.04.2026
 */
 
 
#include "api/sdk-api.hpp"

#include <unordered_map>
#include <thread>
#include <sstream>
#include <functional>
#include <format>

#include "types/string-hashed.hpp"
#include "types/queue-shared-duplex.hpp"
#include "commander/messages/command.hpp"
#include "commander/messages/response.hpp"
#include "commander/messages/event.hpp"
#include "logger/log.hpp"
#include "os/os.hpp"
#include "registry.hpp"
#include "resources/language.hpp"
#include "controller/controller-module-manager.hpp"
#include "controller/controller-cmd-dispatcher.hpp"
#include "configuration/parameters/configuration-parameter-integer.hpp"


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
        friend class logger_sink;
        friend class controller_module_manager;
        friend class registry;

    public:

        /** @brief Retrieves the singleton instance of the controller. */
        static controller& get();

        // Delete copy constructor and assignment operator to prevent copying of the singleton instance
        controller(const controller&)               = delete;
        controller& operator=(const controller&)    = delete;
        controller(controller&&)                    = delete;
        controller& operator=(controller&&)         = delete;

        /** @brief Runs the command processing loop, which continuously checks for new command queue requests. */
        bool run(bool async = false);
        
        /** @brief Stops all queues and frees resources. */
        bool destroy();

        /** @brief Stops all queues and frees resources. */
        bool is_active() const { return m_master_queue.is_active(); }
        
        // COMMAND MANAGEMENT

        enum master_queue_request
        {
            request_invalid = 0,
            request_command,
            request_command_destroy,
            request_log,
            request_log_destroy,
            request_log_sink,
            request_log_sink_destroy,
            request_event,
            request_event_destroy
        };

        enum status
        {
            status_invalid = 0,
            status_success,

            status_unavailable,
            status_unauthorized,
            status_failed,

            status_queue_existing,
            status_queue_not_existing,
            status_queue_failed_create,
            status_queue_failed_destroy,

            status_unknown_master_request
        };

        // LOG MANAGEMENT

        /** @brief Outputs a log. */
        void log(const log& cr_log);

        /** @brief Outputs a log. */
        void log(log::level t, string_hashed::view txt) { this->log(adam::log(t, txt)); }

        /** @brief Outputs a formatted log. */
        template<typename... args_type>
        void log(log::level t, std::format_string<args_type...> fmt, args_type&&... args)
        {
            this->log(adam::log(t, fmt, std::forward<args_type>(args)...));
        }

        /** @brief Outputs a formatted log using a runtime format string. */
        template<typename... args_type>
        requires (sizeof...(args_type) > 0)
        void log(log::level t, std::string_view runtime_fmt, args_type&&... args)
        {
            this->log(adam::log(t, runtime_fmt, std::forward<args_type>(args)...));
        }

        /** @brief Sets the default language for logs etc. */
        void set_language(language lang) { if (m_lang_param) m_lang_param->set_value(static_cast<int64_t>(lang)); }
        language get_language() const { return m_lang_param ? m_lang_param->get_value_as<language>() : language_english; }

        /** @brief Broadcasts an event to all connected commanders. */
        void broadcast_event(const event& e);

        // MODULE MANAGEMENT

        controller_module_manager& modules() { return m_modules; }
        const controller_module_manager& get_modules() const { return m_modules; }

        controller_cmd_dispatcher& dispatcher() { return m_dispatcher; }
        const controller_cmd_dispatcher& get_dispatcher() const { return m_dispatcher; }

        registry& get_registry() { return m_registry; }
        const registry& get_registry() const { return m_registry; }

    protected:

        /** @brief Cleans up orphaned shared memory files left by previous crashes (Linux mostly). */
        static void cleanup_zombie_shared_memory();

        /** @brief Constructs a new controller object.*/
        controller();

        /** @brief Destroys the controller object and cleans up resources.*/
        ~controller();

        // COMMAND MANAGEMENT

        /** @brief Sends a request to the master queue. */
        static status request_master_queue(master_queue_request mqr);

        // MASTER QUEUE
        struct queue_master_request_data
        {
            os::thread_id           tid;
            master_queue_request    queue;
            uint32_t                code;
        };

        using master_queue = queue_shared_duplex<queue_master_request_data, status>;        /**< A queue to request a queue, sound kinda retarded but its quite literlly this */
        static constexpr const char* master_queue_name = "adam::controller_master_queue";   /**< The name of the queue which will create new "per thread" command queues */
        
        void run_master_queue();

        master_queue    m_master_queue;
        std::thread     m_master_queue_thread;

        // SLAVE QUEUES
        template< typename queue_type >
        struct queue_slave_instance_data
        {
            queue_slave_instance_data(const string_hashed& name, os::thread_id t = 0) : queue(name), tid(t) {}

            queue_type  queue;
            std::thread queue_thread;
            os::thread_id tid;
        };

        /** @brief Creates a new (slave) queue of requested template type. For queues that DONT require a worker on the controller side */
        template<typename queue_type>
        bool create_queue_slave
        (
            os::thread_id tid, 
            std::unordered_map<os::thread_id, queue_type*>& queue_list, 
            const char* prefix
        );

        /** @brief Creates a new (slave) queue of requested template type. For queues that require a worker on the controller side */
        template<typename queue_type, typename worker_fn>
        bool create_queue_slave_with_worker
        (
            os::thread_id tid, 
            std::unordered_map<os::thread_id, queue_slave_instance_data<queue_type>*>& queue_list, 
            const char* prefix, 
            worker_fn fn
        );

        /** @brief Destroys existing (slave) queue of template type. For queues that DONT require a worker on the controller side */
        template<typename queue_type>
        bool destroy_queue_slave
        (
            os::thread_id tid, 
            std::unordered_map<os::thread_id, queue_type*>& queue_list
        );

        /** @brief Destroys existing (slave) queue of template type. For queues that require a worker on the controller side */
        template<typename queue_type>
        bool destroy_queue_slave_with_worker
        (
            os::thread_id tid, 
            std::unordered_map<os::thread_id, queue_slave_instance_data<queue_type>*>& queue_list
        );

        // REQUEST/CMD QUEUES

        using queue_command         = queue_shared_duplex<command, response>;                       /**< Cmd/resp queue type */
        using queue_command_data    = queue_slave_instance_data<queue_command>;                     /**< Cmd/resp queue worker data */
        static constexpr const char* queue_command_prefix = "adam::controller_queue_command_";      /**< Prefix for the "per thread" command queue, the target threadid will be added, is unique on all supported OS (for running threads atleast) */

        void run_queue_command(queue_command_data* data);

        std::unordered_map<os::thread_id, queue_command_data*> m_queues_command;

        // LOG QUEUES

        using queue_log             = queue_shared<adam::log>;                                      /**< Log queue type */
        using queue_log_data        = queue_slave_instance_data<queue_log>;                         /**< Log queue worker data */
        static constexpr const char* queue_logger_prefix = "adam::controller_queue_log_";           /**< Prefix for the "per thread" log queue, the target threadid will be added, is unique on all supported OS (for running threads atleast) */

        void run_queue_log(queue_log_data* data);

        std::unordered_map<os::thread_id, queue_log_data*> m_queues_log;

        using queue_log_sink        = queue_shared<adam::log, std::atomic<log::level>>;             /**< Log sink queue type */
        static constexpr const char* queue_logger_sink_prefix = "adam::controller_queue_log_sink";  /**< Prefix for the "per thread" log sink queue, the target threadid will be added, is unique on all supported OS (for running threads atleast) */

        std::unordered_map<os::thread_id, queue_log_sink*> m_queues_log_sink;

        // EVENT QUEUES

        using queue_event           = queue_shared<adam::event>;                                    /**< Event queue type */
        static constexpr const char* queue_event_prefix = "adam::controller_queue_event_";          /**< Prefix for the "per thread" event queue */

        std::unordered_map<os::thread_id, queue_event*> m_queues_event;

        // LOG MANAGEMENT

        enum log_event
        {
            thread_auth_failed,
            slave_queue_created,
            slave_queue_destroyed,
            slave_queue_already_exists,
            slave_queue_failed_to_open,
            slave_queue_failed_to_insert,
            slave_queue_worker_already_exists,
            slave_queue_worker_failed_to_open,
            slave_queue_worker_failed_to_insert,
            slave_queue_does_not_exist,
            slave_queue_failed_to_destroy,
            slave_queue_worker_does_not_exist,
            slave_queue_worker_failed_to_destroy,
            controller_shutting_down
        };

        static std::string_view get_log_event_text(log_event event, language lang);

        std::ostream m_log_outstream;

        // PARAMETERS                                   
        configuration_parameter_integer* m_lang_param;          /**< Storing commonly used parameters here for faster access */

        // MODULE MANAGEMENT
        controller_module_manager       m_modules;
        
        // REGISTRY
        registry                        m_registry;             /**< The controller's registry instance, responsible for managing configuration parameters and other registered items. */

        // DISPATCHER
        controller_cmd_dispatcher       m_dispatcher;
    };
}
