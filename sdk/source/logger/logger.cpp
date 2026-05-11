#include "logger/logger.hpp"


#include "controller/controller.hpp"
#include "logger/log.hpp"


namespace adam 
{
    logger::logger() : m_queue_log() {}

    logger::~logger() {}

    bool logger::connect() 
    {
        // if theres is already a queue for current thread, delete it
        if (m_queue_log.open())
            m_queue_log.destroy();
        
        m_queue_log.set_name(string_hashed(controller::queue_log_sink_prefix + std::to_string(os::get_current_thread_id())));

        if (!m_queue_log.create(1000))
            return false;

        if (controller::request_master_queue(controller::request_log) != controller::status_success)
        {
            m_queue_log.destroy();
            return false;
        }
            
        return true;
    }

    bool logger::destroy() 
    {
        m_queue_log.disable();
        
        bool res = controller::request_master_queue(controller::request_log_destroy) == controller::status_success;

        res &= m_queue_log.destroy();

        return res;
    }

    bool logger::log(const adam::log& cmd) 
    {
        return m_queue_log.push(cmd);
    }
}
