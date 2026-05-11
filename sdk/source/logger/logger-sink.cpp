#include "logger/logger-sink.hpp"


#include "controller/controller.hpp"
#include "logger/log.hpp"


namespace adam 
{
    logger_sink::logger_sink() : m_queue_log_sink() {}

    logger_sink::~logger_sink() {}

    bool logger_sink::connect() 
    {
        // if theres is already a queue for current thread, delete it
        if (m_queue_log_sink.open())
            m_queue_log_sink.destroy();
        
        m_queue_log_sink.set_name(string_hashed(controller::queue_log_sink_prefix + std::to_string(os::get_current_thread_id())));

        if (!m_queue_log_sink.create(1000))
            return false;

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
