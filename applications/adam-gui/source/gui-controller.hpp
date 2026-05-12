#pragma once

#include <adam-sdk.hpp>

#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>

namespace adam 
{
    // Extracted helper functions from adam SDK log.cpp
    std::string get_log_time_string(uint64_t timestamp);
    void get_log_appearance(adam::log::level level, const char*& level_str, float& r, float& g, float& b);
}

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
        std::vector<log_entry> get_log_history() const;

        void set_language(adam::language lang);
        void set_log_level(int level);

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