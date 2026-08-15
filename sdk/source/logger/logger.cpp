#include "logger/logger.hpp"


#include "controller/controller.hpp"
#include "logger/log.hpp"


namespace adam 
{
    logger::logger() : m_queue_log() {}

    logger::~logger() 
    {
        if (is_active()) destroy();
    }

    bool logger::connect() 
    {
        m_queue_log.set_name(string_hashed(controller::queue_logger_prefix + std::to_string(os::get_current_thread_id())));

        // If there is already an orphaned queue for the current thread, destroy it first
        if (m_queue_log.open()) m_queue_log.destroy();

        if (!m_queue_log.create(1000)) return false;

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
