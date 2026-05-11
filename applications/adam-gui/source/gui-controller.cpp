#include "gui-controller.hpp"

namespace adam::gui 
{
    gui_controller::gui_controller() 
        : m_running(false), m_commander_active(false)
    {
    }

    gui_controller::~gui_controller()
    {
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