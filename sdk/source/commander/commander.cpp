#include "commander/commander.hpp"


#include "controller/controller.hpp"
#include "commander/command-response/command.hpp"
#include "commander/command-response/response.hpp"


namespace adam 
{
    commander::commander() : m_queue_command() {}

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

        m_queue_command.set_name(string_hashed(controller::queue_command_prefix + std::to_string(os::get_current_thread_id())));

        if (!m_queue_command.create(1000))
            return false;

        if (controller::request_master_queue(controller::request_command) != controller::status_success)
        {
            m_queue_command.destroy();
            return false;
        }

        if (m_queue_event.open())
            m_queue_event.destroy();

        m_queue_event.set_name(string_hashed(controller::queue_event_prefix + std::to_string(os::get_current_thread_id())));

        if (!m_queue_event.create(1000))
        {
            destroy();
            return false;
        }

        if (controller::request_master_queue(controller::request_event) != controller::status_success)
        {
            destroy();
            return false;
        }

        m_event_thread = std::thread(&commander::run_event_loop, this);

        return true;
    }

    bool commander::destroy() 
    {
        m_queue_command.disable();
        m_queue_event.disable();

        if (m_event_thread.joinable())
            m_event_thread.join();

        bool res = (controller::request_master_queue(controller::request_command_destroy) == controller::status_success);
        res &= (controller::request_master_queue(controller::request_event_destroy) == controller::status_success);

        for (auto& pair : m_inspectors)
        {
            pair.second->destroy();
            delete pair.second;
        }
        m_inspectors.clear();

        res &= m_queue_command.destroy();
        res &= m_queue_event.destroy();

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

        auto it = m_inspectors.find(inspector->get_port_name());
        if (it != m_inspectors.end() && it->second == inspector)
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

        return response_status::inspector_not_found;
    }

    response_status commander::send_command(const command& cmd, response** resp)
    {
        static constexpr int response_buffer_size = 256; // should be enough for most responses and their extensions, if not it will just resize automatically, no big deal
        thread_local std::vector<response> response_buffer = []() 
        {
            std::vector<response> buf;
            buf.reserve(response_buffer_size);
            for (size_t i = 0; i < response_buffer_size; i++)
                buf.emplace_back();
            return buf;
        }();

        if (!m_queue_command.request_queue().push(cmd))
            return response_status::command_send_failed;

        size_t resp_idx = 0;

        if (!m_queue_command.response_queue().pop(response_buffer[resp_idx], 100))
            return response_status::response_receive_failed;

        while (response_buffer[resp_idx].is_extended())
        {
            resp_idx++;

            if (resp_idx >= response_buffer.size())
                response_buffer.emplace_back();
            
            if (!m_queue_command.response_queue().pop(response_buffer[resp_idx], 100))
                return response_status::response_receive_failed; // if we fail to receive an expected extended response, we consider the whole command failed, as the sender should always send all responses together
        }

        if (resp)
            *resp = response_buffer.data();

        return response_buffer.front().get_type();
    }

    void commander::run_event_loop()
    {
        while (m_queue_event.is_active())
        {
            event incoming_event;
            if (m_queue_event.pop(incoming_event, 100))
            {
                if (m_event_callback)
                    m_event_callback(incoming_event);
            }
        }
    }
}
