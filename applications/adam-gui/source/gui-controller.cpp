#include "gui-controller.hpp"

#include <memory>

namespace adam::gui 
{
    static adam::configuration_parameter_list create_gui_defaults()
    {
        adam::configuration_parameter_list params;
        params.add(std::make_unique<adam::configuration_parameter_boolean>("show_log", true));
        params.add(std::make_unique<adam::configuration_parameter_boolean>("show_performance", false));
        params.add(std::make_unique<adam::configuration_parameter_integer>("perf_ovly_location", 1));
        params.add(std::make_unique<adam::configuration_parameter_double>("perf_ovly_x", -1.0));
        params.add(std::make_unique<adam::configuration_parameter_double>("perf_ovly_y", -1.0));
        params.add(std::make_unique<adam::configuration_parameter_integer>("perf_ovly_content", 7));
        params.add(std::make_unique<adam::configuration_parameter_boolean>("dark_theme", true));
        params.add(std::make_unique<adam::configuration_parameter_double>("log_height", 250.0));
        params.add(std::make_unique<adam::configuration_parameter_double>("font_scale", 1.0));
        params.add(std::make_unique<adam::configuration_parameter_integer>("window_x", -1));
        params.add(std::make_unique<adam::configuration_parameter_integer>("window_y", -1));
        params.add(std::make_unique<adam::configuration_parameter_integer>("window_w", 1280));
        params.add(std::make_unique<adam::configuration_parameter_integer>("window_h", 720));
        params.add(std::make_unique<adam::configuration_parameter_boolean>("window_maximized", false));
        params.add(std::make_unique<adam::configuration_parameter_integer>("log_level", 0));
        return params;
    }

    gui_controller::gui_controller() 
        : adam::configuration_item("gui.controller", create_gui_defaults()),
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

    std::vector<log_entry> gui_controller::get_log_history() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_log_history;
    }
    
    void gui_controller::clear_log_history()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_log_history.clear();
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
                        m_commander.connect();
                        commander_active = m_commander.is_active();
                    }

                    if (!log_sink_active)
                    {
                        m_log_sink.connect();
                        log_sink_active = m_log_sink.is_active();
                        
                        if (log_sink_active && m_log_sink.queue().metadata())
                        {
                            auto* p_log_level = static_cast<adam::configuration_parameter_integer*>(get_parameters().get("log_level"));
                            m_log_sink.queue().metadata()->store(static_cast<adam::log::level>(p_log_level->get_value() + 1), std::memory_order_relaxed);
                            
                            adam::log discard;
                            while (m_log_sink.queue().pop(discard, 0)) {}
                        }
                    }
                }
            }

            m_commander_active.store(commander_active, std::memory_order_relaxed);

            adam::log incoming_log;
            if (log_sink_active)
            {
                // Pop logs with a 50ms timeout to avoid tight CPU looping and allow thread exit
                while (m_running.load(std::memory_order_relaxed) && m_log_sink.queue().pop(incoming_log, 50))
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    m_log_history.push_back({ incoming_log.get_timestamp(), incoming_log.get_level(), std::string(incoming_log.get_text()) });
                    if (m_log_history.size() > m_max_log_history)
                        m_log_history.erase(m_log_history.begin());
                }
            }
            else
            {
                // Sleep slightly to prevent tight loop if completely disconnected
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }

        m_log_sink.destroy();
        m_commander.destroy();
    }
}