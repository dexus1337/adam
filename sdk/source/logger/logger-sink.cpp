#include "logger/logger-sink.hpp"


#include "controller/controller.hpp"
#include "logger/log.hpp"


namespace adam 
{
    logger_sink::logger_sink() : m_queue_log_sink(string_hashed(controller::queue_log_sink_prefix + std::to_string(os::get_current_thread_id()))) {}

    logger_sink::~logger_sink() {}

    bool logger_sink::connect() 
    {
        // if theres is already a queue for current thread, delete it
        if (m_queue_log_sink.open())
        {
            m_queue_log_sink.destroy();
            return false;
        }
        
        if (!m_queue_log_sink.create(1000))
            return false;

        if (!controller::request_master_queue(controller::request_log_sink))
            return false;
            
        return true;
    }

    bool logger_sink::destroy() 
    {
        bool res = controller::request_master_queue(controller::request_log_sink_destroy);

        m_queue_log_sink.disable();

        res &= m_queue_log_sink.destroy();

        return res;
    }
}
