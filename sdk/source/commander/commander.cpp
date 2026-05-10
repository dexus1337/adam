#include "commander/commander.hpp"


#include "controller/controller.hpp"
#include "commander/command-response/command.hpp"
#include "commander/command-response/response.hpp"


namespace adam 
{
    commander::commander() : m_queue_command(string_hashed(controller::queue_command_prefix + std::to_string(os::get_current_thread_id()))) {}

    commander::~commander() 
    {
        if (is_active())
            destroy();
    }

    bool commander::connect() 
    {
        // if theres is already a queue for current thread, delete it
        if (m_queue_command.open())
            m_queue_command.destroy();
        
        if (!m_queue_command.create(1000))
            return false;

        if (controller::request_master_queue(controller::request_command) != controller::status_success)
            return false;
            
        return true;
    }

    bool commander::destroy() 
    {
        bool res = (controller::request_master_queue(controller::request_command_destroy) == controller::status_success);

        for (auto& pair : m_inspectors)
        {
            pair.second->destroy();
            delete pair.second;
        }
        m_inspectors.clear();

        m_queue_command.disable();

        res &= m_queue_command.destroy();

        return res;
    }

    response_status commander::request_inspector_create(const string_hashed& port_name, std::function<void(buffer*)> callback, data_inspector*& out_inspector)
    {
        auto port_hash = port_name.get_hash();

        if (m_inspectors.find(port_hash) != m_inspectors.end())
            return response_status::inspector_already_exists;

        data_inspector* inspector = new data_inspector();
        
        if (!inspector->create(port_name))
        {
            delete inspector;
            return response_status::inspector_creation_failed;
        }

        if (!inspector->start_inspecting(callback))
        {
            inspector->destroy();
            delete inspector;
            return response_status::inspector_start_failed;
        }

        command cmd(command_type::inspector_create);
        cmd.data_as<command::inspector_create_data>()->port = port_hash;

        response_status res = send_command(cmd);
        if (res != response_status::success)
        {
            inspector->destroy();
            delete inspector;
            return res;
        }

        m_inspectors[port_hash] = inspector;

        out_inspector = inspector;

        return res;
    }

    response_status commander::request_inspector_destroy(data_inspector* inspector)
    {
        if (!inspector)
            return response_status::inspector_not_found;

        for (auto it = m_inspectors.begin(); it != m_inspectors.end(); ++it)
        {
            if (it->second == inspector)
            {
                command cmd(command_type::inspector_destroy);
                cmd.data_as<command::inspector_destroy_data>()->port = it->first;

                response_status res = send_command(cmd);
                if (res == response_status::success)
                {
                    inspector->destroy();
                    delete inspector;
                    m_inspectors.erase(it);
                }

                return res;
            }
        }

        return response_status::inspector_not_found;
    }

    response_status commander::send_command(const command& cmd, response* resp) 
    {
        response def_resp;

        auto* presp = resp ? resp : &def_resp;

        if (!m_queue_command.post_request(cmd, *presp))
            return response_status::command_send_failed;

        return presp->get_type();
    }
}
