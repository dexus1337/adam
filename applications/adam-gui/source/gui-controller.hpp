#pragma once

#include <adam-sdk.hpp>
#include <commander/commander.hpp>
#include <logger/logger-sink.hpp>

#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>

namespace adam::gui 
{
    struct log_entry
    {
        uint64_t timestamp;
        adam::log::level level;
        std::string text;
    };

    class gui_controller 
    {
    public:
        gui_controller();
        ~gui_controller();

        void start();
        void stop();

        bool is_commander_active() const;
        std::vector<log_entry> get_log_history() const;

    private:
        void update_loop();

        adam::commander m_commander;
        adam::logger_sink m_log_sink;
        
        mutable std::mutex m_mutex;
        std::vector<log_entry> m_log_history;
        size_t m_max_log_history = 1000;
        
        std::atomic<bool> m_running;
        std::atomic<bool> m_commander_active;
        std::thread m_worker_thread;
    };
}