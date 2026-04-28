#include "logger/logger.hpp"


#include "controller/controller.hpp"
#include "logger/log.hpp"


namespace adam 
{
    logger::logger() : m_queue_log(string_hashed(controller::queue_log_prefix + std::to_string(os::get_current_thread_id()))) {}

    logger::~logger() {}

    bool logger::connect() 
    {
        // if theres is already a queue for current thread, delete it
        if (m_queue_log.open())
        {
            m_queue_log.destroy();
            return false;
        }
        
        if (!m_queue_log.create(1000))
            return false;

        if (!controller::request_queue_log_access())
            return false;
            
        return true;
    }

    bool logger::destroy() 
    {
        return m_queue_log.destroy();
    }

    bool logger::log(const adam::log& cmd) 
    {
        return m_queue_log.push(cmd);
    }
}