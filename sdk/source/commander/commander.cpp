#include "commander/commander.hpp"


#include "controller/controller.hpp"
#include "commander/command.hpp"
#include "controller/response.hpp"


namespace adam 
{
    commander::commander() : m_queue_command(string_hashed(controller::queue_command_prefix + std::to_string(os::get_current_thread_id()))) {}

    commander::~commander() {}

    bool commander::connect() 
    {
        // if theres is already a queue for current thread, delete it
        if (m_queue_command.open())
        {
            m_queue_command.destroy();
            return false;
        }
        
        if (!m_queue_command.create(1000))
            return false;

        if (!controller::request_master_queue(controller::request_command))
            return false;
            
        return true;
    }

    bool commander::destroy() 
    {
        bool res = controller::request_master_queue(controller::request_command_destroy);

        m_queue_command.disable();

        res &= m_queue_command.destroy();

        return res;
    }

    bool commander::send_command(const command& cmd, response* resp) 
    {
        response def_resp;

        auto* presp = resp ? resp : &def_resp;

        return m_queue_command.post_request(cmd, *presp);
    }
}
