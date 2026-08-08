#pragma once

/**
 * @file    cop-controller.hpp
 * @author  dexus1337
 * @brief   Application state controller for adam-cop
 * @version 1.0
 * @date    05.08.2026
 */

#include <adam-sdk.hpp>
#include "data/waypoint.hpp"

#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <functional>

namespace adam::cop
{
    struct log_entry
    {
        uint64_t timestamp;
        adam::log::level level;
        std::string text;
    };

    class cop_controller : public adam::configuration_item
    {
    public:
        cop_controller();
        ~cop_controller();

        void start();
        void stop();

        bool is_commander_active() const;
        std::vector<log_entry> get_log_history() const;
        bool is_log_history_empty() const;
        void clear_log_history();

        void request_redraw();
        void set_redraw_callback(std::function<void()> cb);

        const adam::commander&   get_commander() const { return m_commander; }
        const adam::logger_sink& get_log_sink()  const { return m_log_sink; }

        adam::logger_sink& log_sink()  { return m_log_sink; }
        adam::commander&   commander() { return m_commander; }

        void enqueue_commander_action(std::function<void()> action);

        void add_waypoint(std::unique_ptr<waypoint> wp);
        void remove_waypoint(adam::string_hash item_hash);
        void clear_waypoints();
        const std::vector<std::unique_ptr<waypoint>>& get_waypoints() const { return m_waypoints; }
        std::vector<std::unique_ptr<waypoint>>& waypoints() { return m_waypoints; }

        void save_config();

    private:
        void load_waypoints_from_config();
        void sync_waypoints_to_config();
        void update_loop();

        adam::commander        m_commander;
        adam::logger_sink      m_log_sink;

        size_t                 m_max_log_history = 1000;

        mutable std::mutex     m_mutex;
        std::vector<log_entry> m_log_history;
        std::vector<std::function<void()>> m_deferred_commander_actions;

        std::function<void()>  m_redraw_callback;
        std::atomic<bool>      m_running;
        std::atomic<bool>      m_commander_active;
        std::thread            m_worker_thread;
        std::vector<std::unique_ptr<waypoint>> m_waypoints;
        uint32_t               m_next_wp_id = 1;
    };
}
