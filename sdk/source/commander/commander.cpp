#include "commander/commander.hpp"


#include "controller/controller.hpp"


namespace adam 
{
    commander::commander() : m_cmd_queue(string_hashed(controller::command_queue_prefix + (std::stringstream() << std::this_thread::get_id()).str())) {}

    commander::~commander() {}

    bool commander::connect() 
    {
        if ( m_cmd_queue.open() )
        {
            m_cmd_queue.destroy();
            return false;
        }
        
        if (!m_cmd_queue.create(1000))
            return false;

        return true;
    }

    bool commander::destroy() 
    {
        return m_cmd_queue.destroy();
    }

    bool commander::send_command(const command& cmd) 
    {
        return m_cmd_queue.push(cmd);
    }
}