#include "gui-controller.hpp"

#include <memory>

namespace adam::gui 
{
    static const configuration_parameter_list& get_default_parameters()
    {
        static configuration_parameter_list params = []() 
        {
            configuration_parameter_list p;
            p.add(std::make_unique<configuration_parameter_boolean>("show_log"_ct, true));
            p.add(std::make_unique<configuration_parameter_integer>("gui_mode"_ct, 0));
            p.add(std::make_unique<configuration_parameter_integer>("fps_limit"_ct, 4));
            p.add(std::make_unique<configuration_parameter_boolean>("show_performance"_ct, false));
            p.add(std::make_unique<configuration_parameter_integer>("perf_ovly_location"_ct, 1));
            p.add(std::make_unique<configuration_parameter_double>("perf_ovly_x"_ct, -1.0));
            p.add(std::make_unique<configuration_parameter_double>("perf_ovly_y"_ct, -1.0));
            p.add(std::make_unique<configuration_parameter_integer>("perf_ovly_content"_ct, 7));
            p.add(std::make_unique<configuration_parameter_string>("theme"_ct, "default-dark"_ct));
            p.add(std::make_unique<configuration_parameter_double>("log_height"_ct, 0.0));
            p.add(std::make_unique<configuration_parameter_double>("font_scale"_ct, 1.0));
            p.add(std::make_unique<configuration_parameter_integer>("window_x"_ct, -1));
            p.add(std::make_unique<configuration_parameter_integer>("window_y"_ct, -1));
            p.add(std::make_unique<configuration_parameter_integer>("window_w"_ct, 1280));
            p.add(std::make_unique<configuration_parameter_integer>("window_h"_ct, 720));
            p.add(std::make_unique<configuration_parameter_boolean>("window_maximized"_ct, false));
            p.add(std::make_unique<configuration_parameter_integer>("log_level"_ct, 0));
            p.add(std::make_unique<configuration_parameter_integer>("language"_ct, 0));
            p.add(std::make_unique<configuration_parameter_integer>("connection_sort_mode"_ct, 3));
            return p;
        }();
        return params;
    }

    gui_controller::gui_controller() 
        : configuration_item("gui.controller", get_default_parameters()),
          m_running(false), m_commander_active(false)
    {
        load("adam-gui-config.bin");
    }

    gui_controller::~gui_controller()
    {
        save("adam-gui-config.bin");
        stop();
    }

    void gui_controller::start()
    {
        if (m_running.exchange(true))
            return;

        m_worker_thread = std::thread(&gui_controller::update_loop, this);
    }

    void gui_controller::stop()
    {
        if (!m_running.exchange(false))
            return;

        if (m_worker_thread.joinable())
            m_worker_thread.join();
    }

    bool gui_controller::is_commander_active() const
    {
        return m_commander_active.load(std::memory_order_relaxed);
    }

    const std::vector<log_entry>& gui_controller::get_log_history() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_log_history;
    }
    
    void gui_controller::clear_log_history()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_log_history.clear();
    }

    void gui_controller::request_redraw()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_redraw_callback)
            m_redraw_callback();
    }

    void gui_controller::set_redraw_callback(std::function<void()> cb)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_redraw_callback = std::move(cb);
    }

    void gui_controller::enqueue_commander_action(std::function<void()> action)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_deferred_commander_actions.push_back(std::move(action));
    }

    void gui_controller::update_loop()
    {
        auto last_reconnect_attempt = std::chrono::steady_clock::now() - std::chrono::seconds(5);

        while (m_running.load(std::memory_order_relaxed))
        {
            auto now = std::chrono::steady_clock::now();

            bool commander_active = m_commander.is_active();
            bool log_sink_active = m_log_sink.is_active();

            if (!commander_active || !log_sink_active)
            {
                if (std::chrono::duration_cast<std::chrono::seconds>(now - last_reconnect_attempt).count() >= 5)
                {
                    last_reconnect_attempt = now;

                    if (!commander_active)
                    {
                        // Also destroy the log sink as when commander has no con, controller is dead
                        if (log_sink_active)
                        {
                            m_log_sink.destroy();
                            log_sink_active = false;
                        }

                        m_commander.destroy();
                        m_commander.connect();
                        commander_active = m_commander.is_active();
                    }

                    if (!log_sink_active)
                    {
                        m_log_sink.connect();
                        log_sink_active = m_log_sink.is_active();
                        
                        if (log_sink_active && m_log_sink.queue().metadata())
                        {
                            auto* p_log_level = static_cast<adam::configuration_parameter_integer*>(get_parameters().get("log_level"_ct));
                            m_log_sink.queue().metadata()->store(static_cast<adam::log::level>(p_log_level->get_value() + 1), std::memory_order_relaxed);
                            
                            // Remove any logs that might have been received while we were disconnected to avoid showing stale logs on reconnect
                            adam::log discard;
                            while (m_log_sink.queue().pop(discard, 0)) {}
                        }
                    }
                }
            }

            if (commander_active != m_commander_active.load(std::memory_order_relaxed))
            {
                request_redraw();
            }

            // Process any pending commander actions queued by the UI
            if (commander_active)
            {
                std::vector<std::function<void()>> actions_to_run;
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    actions_to_run = std::move(m_deferred_commander_actions);
                }
                for (auto& action : actions_to_run)
                {
                    action();
                }
            }
            else
            {
                // If disconnected, clear pending actions to avoid executing stale operations upon reconnect
                std::lock_guard<std::mutex> lock(m_mutex);
                m_deferred_commander_actions.clear();
            }

            m_commander_active.store(commander_active, std::memory_order_relaxed);

            adam::log incoming_log;
            bool any_logs = false;
            if (log_sink_active)
            {
                // Pop logs with a 50ms timeout to avoid tight CPU looping and allow thread exit
                while (m_running.load(std::memory_order_relaxed) && m_log_sink.queue().pop(incoming_log, 50))
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    m_log_history.push_back({ incoming_log.get_timestamp(), incoming_log.get_level(), std::string(incoming_log.get_text()) });
                    if (m_log_history.size() > m_max_log_history)
                        m_log_history.erase(m_log_history.begin());
                    any_logs = true;
                }
            }
            else
            {
                // Sleep slightly to prevent tight loop if completely disconnected
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }

            if (any_logs)
                request_redraw();
        }

        m_log_sink.destroy();
        m_commander.destroy();
    }
}