#include "logger/logger-sink.hpp"


#include "controller/controller.hpp"
#include "logger/log.hpp"


namespace adam 
{
    logger_sink::logger_sink() : m_queue_log_sink() {}

    logger_sink::~logger_sink() 
    {
        if (is_active()) destroy();
    }

    bool logger_sink::connect() 
    {
        m_queue_log_sink.set_name(string_hashed(controller::queue_logger_sink_prefix + std::to_string(os::get_current_thread_id())));

        // If there is already an orphaned queue for the current thread, destroy it first
        if (m_queue_log_sink.open()) m_queue_log_sink.destroy();

        if (!m_queue_log_sink.create(1000)) return false;

        if (controller::request_master_queue(controller::request_log_sink) != controller::status_success)
        {
            m_queue_log_sink.destroy();
            return false;
        }
            
        return true;
    }

    bool logger_sink::destroy() 
    {
        m_queue_log_sink.disable();

        bool res = controller::request_master_queue(controller::request_log_sink_destroy) == controller::status_success;

        res &= m_queue_log_sink.destroy();

        return res;
    }
}
