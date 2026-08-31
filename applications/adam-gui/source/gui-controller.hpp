#pragma once

#include <adam-core.hpp>

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

        // Const functions grouped first
        inline bool                             is_commander_active()       const { return m_commander_active.load(std::memory_order_relaxed); }
        inline bool                             is_log_history_empty()      const { std::lock_guard<std::mutex> lock(m_mutex); return m_log_history.empty(); }
        inline bool                             owns_core()                 const { return m_owns_core; }
        inline int                              get_adam_mode()             const { return m_p_adam_mode ? static_cast<int>(m_p_adam_mode->get_value()) : 0; }
        inline const adam::commander&           get_commander()             const { return m_commander; }
        inline const adam::logger_sink&         get_log_sink()              const { return m_log_sink; }
        std::vector<log_entry>                  get_log_history()           const;

        // Non-const functions
        void                                    start();
        void                                    stop();
        void                                    set_adam_mode(int mode);
        void                                    start_core();
        void                                    stop_core();
        void                                    clear_log_history();
        void                                    request_redraw();
        void                                    set_redraw_callback(std::function<void()> cb);
        void                                    enqueue_commander_action(std::function<void()> action);

        inline adam::logger_sink&               log_sink()                        { return m_log_sink; }
        inline adam::commander&                 commander()                       { return m_commander; }

    private:
        void                                    update_loop();

        adam::commander                         m_commander;
        adam::logger_sink                       m_log_sink;
        adam::configuration_parameter_integer*  m_p_adam_mode = nullptr;

        size_t                                  m_max_log_history = 1000;
        bool                                    m_owns_core       = false;

        mutable std::mutex                      m_mutex;
        mutable std::mutex                      m_core_mutex;
        std::vector<log_entry>                  m_log_history;
        std::vector<std::function<void()>>      m_deferred_commander_actions; /**< Actions queued by the UI to be executed on the worker thread. */

        std::function<void()>                   m_redraw_callback;
        std::atomic<bool>                       m_running;
        std::atomic<bool>                       m_commander_active;
        std::thread                             m_worker_thread;
    };
}