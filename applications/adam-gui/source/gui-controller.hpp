#pragma once

#include <adam-sdk.hpp>

#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <functional>

namespace adam::gui 
{
    struct log_entry
    {
        uint64_t timestamp;
        adam::log::level level;
        std::string text;
    };

    class gui_controller : public adam::configuration_item
    {
    public:
        gui_controller();
        ~gui_controller();

        void start();
        void stop();

        bool is_commander_active() const;
        const std::vector<log_entry>& get_log_history() const;
        void clear_log_history();

        void request_redraw();
        void set_redraw_callback(std::function<void()> cb);

        const adam::commander&      get_commander()     const { return m_commander; }
        const adam::logger_sink&    get_log_sink()      const { return m_log_sink; }

        adam::logger_sink&  log_sink()  { return m_log_sink; }
        adam::commander&    commander() { return m_commander; }

        /** @brief Enqueues a commander action to be executed safely on the controller's worker thread. */
        void enqueue_commander_action(std::function<void()> action);

    private:
        void update_loop();

        adam::commander         m_commander;
        adam::logger_sink       m_log_sink;

        size_t                  m_max_log_history = 1000;
        
        mutable std::mutex      m_mutex;
        std::vector<log_entry>  m_log_history;
        std::vector<std::function<void()>> m_deferred_commander_actions; /**< Actions queued by the UI to be executed on the worker thread. */

        std::function<void()>   m_redraw_callback;
        std::atomic<bool>       m_running;
        std::atomic<bool>       m_commander_active;
        std::thread             m_worker_thread;
    };
}