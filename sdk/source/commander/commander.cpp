#include "commander/commander.hpp"


#include "controller/controller.hpp"
#include "commander/command.hpp"
#include "controller/response.hpp"


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

    response::type commander::request_inspector_create(const string_hashed& port_name, std::function<void(buffer*)> callback, data_inspector*& out_inspector)
    {
        auto port_hash = port_name.get_hash();

        if (m_inspectors.find(port_hash) != m_inspectors.end())
            return response::inspector_already_exists;

        data_inspector* inspector = new data_inspector();
        
        if (!inspector->create(port_name))
        {
            delete inspector;
            return response::inspector_creation_failed;
        }

        if (!inspector->start_inspecting(callback))
        {
            inspector->destroy();
            delete inspector;
            return response::inspector_start_failed;
        }

        command cmd(command::inspector_create);
        cmd.get_data_as<command::inspector_create_data>()->port = port_hash;

        response resp;
        if (!send_command(cmd, &resp) || resp.get_type() != response::success)
        {
            inspector->destroy();
            delete inspector;
            return response::command_send_failed;
        }

        m_inspectors[port_hash] = inspector;

        out_inspector = inspector;

        return resp.get_type();
    }

    response::type commander::request_inspector_destroy(data_inspector* inspector)
    {
        if (!inspector)
            return response::inspector_not_found;

        for (auto it = m_inspectors.begin(); it != m_inspectors.end(); ++it)
        {
            if (it->second == inspector)
            {
                command cmd(command::inspector_destroy);
                cmd.get_data_as<command::inspector_destroy_data>()->port = it->first;

                response resp;
                bool success = send_command(cmd, &resp);

                if (success)
                {
                    inspector->destroy();
                    delete inspector;
                    m_inspectors.erase(it);
                    return resp.get_type();
                }

                return response::command_send_failed;
            }
        }

        return response::inspector_not_found;
    }

    bool commander::send_command(const command& cmd, response* resp) 
    {
        response def_resp;

        auto* presp = resp ? resp : &def_resp;

        return m_queue_command.post_request(cmd, *presp);
    }
}
