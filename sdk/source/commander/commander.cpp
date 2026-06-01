#include "commander/commander.hpp"

#include <iostream>

#include "controller/controller.hpp"
#include "commander/messages/command.hpp"
#include "commander/messages/message-structs.hpp"
#include "commander/messages/response.hpp"
#include "resources/language-strings.hpp"
#include "data/port/port.hpp"
#include "module/module.hpp"
#include "data/connection.hpp"
#include "memory/buffer/buffer-manager.hpp"
#include <mutex>


namespace adam 
{
    commander::commander() 
     :  m_queue_command(),
        m_queue_event(),
        m_dispatcher(),
        m_log_outstream(std::cout.rdbuf()),
        m_lang(),
        m_inspectors(),
        m_registry_view(),
        m_module_view()
    {
        m_command_buffer.reserve(queue_command_size);
        m_response_buffer.reserve(queue_command_size);
        
        for (size_t i = 0; i < queue_command_size; i++)
        {
            m_command_buffer.emplace_back();
            m_response_buffer.emplace_back();
        }
    }

    commander::~commander() 
    {
        if (is_active())
            destroy();
    }

    bool commander::connect() 
    {
        // First cerate command queue
        if (m_queue_command.open())
            m_queue_command.destroy();

        m_queue_command.set_name(string_hashed(controller::queue_command_prefix + std::to_string(os::get_current_thread_id())));

        if (!m_queue_command.create(queue_command_size))
            return false;

        controller::status resp = controller::request_master_queue(controller::request_command);

        if (resp != controller::status_success)
        {
            adam::stream_log(log::trace, language_strings::controller_status_text(resp, get_language()), m_log_outstream);
            
            m_queue_command.destroy();
            return false;
        }

        // Then create event queue
        if (m_queue_event.open())
            m_queue_event.destroy();

        m_queue_event.set_name(string_hashed(controller::queue_event_prefix + std::to_string(os::get_current_thread_id())));

        if (!m_queue_event.create(queue_event_size))
        {
            destroy();
            return false;
        }

        resp = controller::request_master_queue(controller::request_event);

        if (resp != controller::status_success)
        {
            adam::stream_log(log::trace, language_strings::controller_status_text(resp, get_language()), m_log_outstream);
            
            destroy();
            return false;
        }

        // Now start listening for events
        m_event_thread = std::thread(&commander::run_event_loop, this);

        // Sync config etc from controller
        if (request_initial_data() != response_status::success)
        {
            destroy();
            return false;
        }

        return true;
    }

    bool commander::disable() 
    {
        m_queue_command.disable();
        m_queue_event.disable();

        return true;
    }

    bool commander::destroy() 
    {
        bool m_cmd_active = m_queue_command.is_active();
        bool m_evt_active = m_queue_event.is_active();

        disable();

        if (m_event_thread.joinable())
        {
            if (std::this_thread::get_id() == m_event_thread.get_id())
                m_event_thread.detach();
            else
                m_event_thread.join();
        }

        bool res = true;
        
        if (m_evt_active)
        {
            auto ctrl_resp = controller::request_master_queue(controller::request_event_destroy);
            res &= (ctrl_resp == controller::status_success);

            if (ctrl_resp != controller::status_success)
                adam::stream_log(log::trace, language_strings::controller_status_text(ctrl_resp, get_language()), m_log_outstream);
        }

        if (m_cmd_active)
        {
            auto ctrl_resp = controller::request_master_queue(controller::request_command_destroy);
            res &= (ctrl_resp == controller::status_success);

            if (ctrl_resp != controller::status_success)
                adam::stream_log(log::trace, language_strings::controller_status_text(ctrl_resp, get_language()), m_log_outstream);
        }

        for (auto& pair : m_inspectors)
        {
            pair.second->destroy();
            delete pair.second;
        }
        m_inspectors.clear();
        m_registry_view.clear();
        m_module_view.clear();

        res &= m_queue_event.destroy();
        res &= m_queue_command.destroy();

        return res;
    }

    /** @brief Requests the initial data from the controller. */
    response_status commander::request_initial_data()
    {
        command cmd(command_type::acquire_initial_data);

        response* resp = nullptr;
        
        auto res = send_command(cmd, &resp);

        if (res != response_status::success || !resp)
            return response_status::response_receive_failed;

        auto* head = resp->data_as<messages::initial_data_header>();

        m_lang = head->lang_info;

        std::lock_guard<const module_view> lg(m_module_view);
        m_module_view.available().reserve(head->mod_info.available_modules);
        m_module_view.unavailable().reserve(head->mod_info.unavailable_modules);
        m_module_view.loaded().reserve(head->mod_info.loaded_modules);

        m_module_view.paths().resize(head->mod_info.module_paths);
        size_t current_idx = 1;

        for (size_t i = 0; i < head->mod_info.module_paths; i++)
        {
            if (!resp[current_idx-1].is_extended())
                break;
            
            auto* path_info = resp[current_idx].data_as<messages::module_path_data>();
            if (path_info->idx >= m_module_view.paths().size()) {
                m_module_view.paths().resize(path_info->idx + 1);
            }
            m_module_view.paths()[path_info->idx] = &path_info->path[0];
            current_idx++;
        }

        for (size_t i = 0; i < (head->mod_info.available_modules + head->mod_info.unavailable_modules + head->mod_info.loaded_modules); i++)
        {
            if (!resp[current_idx-1].is_extended())
                break;
            
            auto* mod_info = resp[current_idx].data_as<module::basic_info>();
    
            string_hashed mod_name(&mod_info->name[0]);
            string_hashed mod_path(&mod_info->path[0]);
    
            switch (mod_info->stat)
            {
                case module::basic_info::available:
                    m_module_view.update_module_database(mod_name, mod_path, mod_info->version);
                    m_module_view.available().emplace(mod_name, std::make_pair(mod_info->version, mod_path));
                    break;
                case module::basic_info::unavailable:
                    m_module_view.unavailable().emplace(mod_name, std::make_tuple(mod_info->version, mod_path, mod_info->rsn));
                    break;
                case module::basic_info::loaded:
                    m_module_view.load_module(mod_name, mod_path, mod_info->version);
                    break;
            }
            current_idx++;
        }

        for (size_t i = 0; i < head->conn_info.ports; i++)
        {
            if (!resp[current_idx-1].is_extended())
                break;
            
            auto* port_info = resp[current_idx].data_as<port::basic_info>();
            
            auto pview = std::make_unique<port_view>();
            pview->name             = string_hashed(&port_info->name[0]);
            pview->direction        = port_info->dir;
            pview->is_active        = port_info->is_active;
            pview->is_unavailable   = port_info->is_unavailable;
            
            if (!pview->is_unavailable)
                pview->statistic_buffer = buffer_manager::get().resolve_handle(port_info->statistic_buffer_handle);
            
            m_module_view.extract_port_type_and_module(port_info->type, port_info->type_module, pview->type, pview->type_module);
            
            if (pview->type_module.get_hash() != 0)
            {
                auto mod_it = m_module_view.loaded().find(pview->type_module.get_hash());
                if (mod_it != m_module_view.loaded().end() && mod_it->second.second)
                {
                    const auto& factories = mod_it->second.second->get_port_factories();
                    auto fact_it = factories.find(pview->type);
                    if (fact_it != factories.end() && fact_it->second.parameters)
                    {
                        if (auto* factory_user_params = dynamic_cast<const configuration_parameter_list*>(fact_it->second.parameters->get("user_parameters"_ct)))
                        {
                            pview->user_params = *factory_user_params;
                        }
                    }
                }
            }

            size_t unused_off = sizeof(port::basic_info);
            size_t unused_size = response::get_max_data_length() - unused_off;
            detail::message_deserializer<response> deserializer(resp, m_response_buffer.size(), current_idx, unused_off, unused_size);
            detail::deserialize_user_parameters(port_info->user_parameters, deserializer, pview->user_params);

            m_registry_view.ports().emplace(pview->name.get_hash(), std::move(pview));
            current_idx++;
        }

        for (size_t i = 0; i < head->conn_info.connections; i++)
        {
            if (!resp[current_idx-1].is_extended())
                break;
            
            auto* conn_info = resp[current_idx].data_as<connection::basic_info>();
            
            auto conn = std::make_unique<connection_view>();
            conn->name = string_hashed(&conn_info->name[0]);

            conn->created = conn_info->created;
            conn->edited = conn_info->edited;
            conn->sorting_index = conn_info->sorting_index;
            conn->is_active = conn_info->is_active;
            conn->valid_chain = conn_info->valid_chain;
            conn->color = conn_info->color;

            // Populate connection format fields
            m_module_view.extract_datatype_and_module(conn_info->input_format,  conn_info->input_format_module,  conn->input_format,  conn->input_format_module);
            m_module_view.extract_datatype_and_module(conn_info->output_format, conn_info->output_format_module, conn->output_format, conn->output_format_module);
            
            for (size_t j = 0; j < conn_info->input_count; j++)
                conn->inputs.push_back(conn_info->inputs[j]);
                
            for (size_t j = 0; j < conn_info->processor_count; j++)
                conn->filters.push_back(conn_info->processors[j]);
                
            for (size_t j = 0; j < conn_info->output_count; j++)
                conn->outputs.push_back(conn_info->outputs[j]);
                
            m_registry_view.connections().emplace(conn->name.get_hash(), std::move(conn));
            current_idx++;
        }

        return res;
    }

    response_status commander::request_module_path_add(const string_hashed& path, uint32_t index)
    {
        command cmd(command_type::module_path_add);
        auto* data = cmd.data_as<messages::module_path_data>();
        data->setup(path.c_str(), index);

        return send_command(cmd);
    }

    response_status commander::request_module_path_remove(uint32_t index)
    {
        command cmd(command_type::module_path_remove);
        auto* data = cmd.data_as<messages::module_path_remove_data>();
        data->idx = index;

        return send_command(cmd);
    }

    response_status commander::request_module_scan()
    {
        command cmd(command_type::module_scan);
        return send_command(cmd);
    }

    response_status commander::request_module_load(const string_hashed& name)
    {
        command cmd(command_type::module_load);
        auto* data = cmd.data_as<messages::module_action_data>();
        data->setup(name.c_str());

        return send_command(cmd);
    }

    response_status commander::request_module_unload(const string_hashed& name)
    {
        command cmd(command_type::module_unload);
        auto* data = cmd.data_as<messages::module_action_data>();
        data->setup(name.c_str());

        return send_command(cmd);
    }

    response_status commander::request_port_create(const string_hashed& name, string_hash type, string_hash type_module)
    {
        {
            std::lock_guard<const registry_view> lg(m_registry_view);
            if (m_registry_view.ports().find(name.get_hash()) != m_registry_view.ports().end())
                return response_status::command_send_failed;
        }

        command cmd(command_type::port_create);
        cmd.data_as<port::basic_info>()->setup(name, type, type_module);

        return send_command(cmd);
    }

    response_status commander::request_port_destroy(string_hash port_hash)
    {
        command cmd(command_type::port_destroy);
        cmd.data_as<messages::port_destroy_data>()->port = port_hash;

        return send_command(cmd);
    }

    response_status commander::request_port_start(string_hash port_hash)
    {
        command cmd(command_type::port_start);
        cmd.data_as<messages::port_action_data>()->port = port_hash;
        return send_command(cmd);
    }

    response_status commander::request_port_stop(string_hash port_hash)
    {
        command cmd(command_type::port_stop);
        cmd.data_as<messages::port_action_data>()->port = port_hash;
        return send_command(cmd);
    }

    response_status commander::request_port_rename(string_hash old_hash, const string_hashed& new_name)
    {
        if (old_hash == new_name.get_hash())
            return response_status::success;

        {
            std::lock_guard<const registry_view> lg(m_registry_view);
            if (m_registry_view.ports().find(new_name.get_hash()) != m_registry_view.ports().end())
                return response_status::command_send_failed;
        }

        command cmd(command_type::port_rename);
        auto* data = cmd.data_as<messages::port_rename_data>();
        data->port = old_hash;
        std::strncpy(data->new_name, new_name.c_str(), sizeof(data->new_name) - 1);
        data->new_name[sizeof(data->new_name) - 1] = '\0';
        return send_command(cmd);
    }

    response_status commander::request_port_parameter_set(string_hash port_hash, const string_hashed& param_name, int64_t value)
    {
        command cmd(command_type::port_set_parameter);
        auto* data = cmd.data_as<messages::port_set_parameter_data>();
        data->port = port_hash;
        data->param_view.var_type = configuration_parameter::type_integer;
        data->param_view.name = param_name.get_hash();
        std::memcpy(data->data, &value, sizeof(int64_t));
        return send_command(cmd);
    }

    response_status commander::request_port_parameter_set(string_hash port_hash, const string_hashed& param_name, double value)
    {
        command cmd(command_type::port_set_parameter);
        auto* data = cmd.data_as<messages::port_set_parameter_data>();
        data->port = port_hash;
        data->param_view.var_type = configuration_parameter::type_double;
        data->param_view.name = param_name.get_hash();
        std::memcpy(data->data, &value, sizeof(double));
        return send_command(cmd);
    }

    response_status commander::request_port_parameter_set(string_hash port_hash, const string_hashed& param_name, bool value)
    {
        command cmd(command_type::port_set_parameter);
        auto* data = cmd.data_as<messages::port_set_parameter_data>();
        data->port = port_hash;
        data->param_view.var_type = configuration_parameter::type_boolean;
        data->param_view.name = param_name.get_hash();
        std::memcpy(data->data, &value, sizeof(bool));
        return send_command(cmd);
    }

    response_status commander::request_port_parameter_set(string_hash port_hash, const string_hashed& param_name, const string_hashed& value)
    {
        command cmd(command_type::port_set_parameter);
        auto* data = cmd.data_as<messages::port_set_parameter_data>();
        data->port = port_hash;
        data->param_view.var_type = configuration_parameter::type_string;
        data->param_view.name = param_name.get_hash();
        
        uint16_t len = static_cast<uint16_t>(value.size());
        size_t max_len = sizeof(data->data) - sizeof(uint16_t);
        if (len > max_len) len = static_cast<uint16_t>(max_len);
        
        std::memcpy(data->data, &len, sizeof(uint16_t));
        if (len > 0)
            std::memcpy(data->data + sizeof(uint16_t), value.c_str(), len);
            
        return send_command(cmd);
    }

    response_status commander::request_port_parameter_set(string_hash port_hash, const string_hashed& param_name, const string_hashed_ct& value)
    {
        command cmd(command_type::port_set_parameter);
        auto* data = cmd.data_as<messages::port_set_parameter_data>();
        data->port = port_hash;
        data->param_view.var_type = configuration_parameter::type_string;
        data->param_view.name = param_name.get_hash();
        
        uint16_t len = static_cast<uint16_t>(value.get_length());
        size_t max_len = sizeof(data->data) - sizeof(uint16_t);
        if (len > max_len) len = static_cast<uint16_t>(max_len);
        
        std::memcpy(data->data, &len, sizeof(uint16_t));
        if (len > 0)
            std::memcpy(data->data + sizeof(uint16_t), value.c_str(), len);
            
        return send_command(cmd);
    }

    response_status commander::request_connection_set_input_data_format(string_hash conn_hash, string_hash format, string_hash format_module)
    {
        command cmd(command_type::connection_set_input_data_format);
        auto* data = cmd.data_as<messages::connection_data_format_data>();
        data->connection = conn_hash;
        data->format = format;
        data->format_module = format_module;
        data->valid_chain = false;
        return send_command(cmd);
    }

    response_status commander::request_connection_set_output_data_format(string_hash conn_hash, string_hash format, string_hash format_module)
    {
        command cmd(command_type::connection_set_output_data_format);
        auto* data = cmd.data_as<messages::connection_data_format_data>();
        data->connection = conn_hash;
        data->format = format;
        data->format_module = format_module;
        data->valid_chain = false;
        return send_command(cmd);
    }

    response_status commander::request_connection_create(const string_hashed& name)
    {
        {
            std::lock_guard<const registry_view> lg(m_registry_view);
            if (m_registry_view.connections().find(name.get_hash()) != m_registry_view.connections().end())
                return response_status::command_send_failed;
        }

        command cmd(command_type::connection_create);
        cmd.data_as<connection::basic_info>()->setup(name);

        return send_command(cmd);
    }

    response_status commander::request_connection_destroy(string_hash hash)
    {
        command cmd(command_type::connection_destroy);
        cmd.data_as<messages::connection_destroy_data>()->connection = hash;

        return send_command(cmd);
    }

    response_status commander::request_connection_start(string_hash hash)
    {
        command cmd(command_type::connection_start);
        cmd.data_as<messages::connection_action_data>()->connection = hash;
        return send_command(cmd);
    }

    response_status commander::request_connection_stop(string_hash hash)
    {
        command cmd(command_type::connection_stop);
        cmd.data_as<messages::connection_action_data>()->connection = hash;
        return send_command(cmd);
    }

    response_status commander::request_connection_rename(string_hash old_hash, const string_hashed& new_name)
    {
        if (old_hash == new_name.get_hash())
            return response_status::success;

        {
            std::lock_guard<const registry_view> lg(m_registry_view);
            if (m_registry_view.connections().find(new_name.get_hash()) != m_registry_view.connections().end())
                return response_status::command_send_failed;
        }

        command cmd(command_type::connection_rename);
        auto* data = cmd.data_as<messages::connection_rename_data>();
        data->connection = old_hash;
        std::strncpy(data->new_name, new_name.c_str(), sizeof(data->new_name) - 1);
        data->new_name[sizeof(data->new_name) - 1] = '\0';
        return send_command(cmd);
    }

    response_status commander::request_connection_port_add(string_hash conn_hash, string_hash port_hash, bool is_input)
    {
        command cmd(command_type::connection_port_add);
        auto* data = cmd.data_as<messages::connection_port_add_data>();
        data->connection = conn_hash;
        data->port = port_hash;
        data->is_input = is_input;
        return send_command(cmd);
    }

    response_status commander::request_connection_port_remove(string_hash conn_hash, string_hash port_hash, bool is_input)
    {
        command cmd(command_type::connection_port_remove);
        auto* data = cmd.data_as<messages::connection_port_add_data>();
        data->connection = conn_hash;
        data->port = port_hash;
        data->is_input = is_input;
        return send_command(cmd);
    }

    response_status commander::request_connection_sorting_index_change(string_hash hash, uint32_t sorting_index)
    {
        command cmd(command_type::connection_sorting_index_change);
        auto* data = cmd.data_as<messages::connection_property_change_data>();
        data->connection = hash;
        data->value = sorting_index;
        return send_command(cmd);
    }

    response_status commander::request_connection_color_change(string_hash hash, uint32_t color)
    {
        command cmd(command_type::connection_color_change);
        auto* data = cmd.data_as<messages::connection_property_change_data>();
        data->connection = hash;
        data->value = color;
        return send_command(cmd);
    }

    response_status commander::request_inspector_create(string_hash port_hash, std::function<void(buffer*)> callback, data_inspector*& out_inspector)
    {
        if (m_inspectors.find(port_hash) != m_inspectors.end())
            return response_status::inspector_already_exists;

        data_inspector* inspector = new data_inspector();
        
        if (!inspector->create(port_hash))
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
        cmd.data_as<messages::inspector_create_data>()->port = port_hash;

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

        auto it = m_inspectors.find(inspector->get_port_hash());
        if (it != m_inspectors.end() && it->second == inspector)
        {
            command cmd(command_type::inspector_destroy);
            cmd.data_as<messages::inspector_destroy_data>()->port = it->first;

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

    response_status commander::request_port_inject_data(string_hash port_hash, const void* data, size_t size, data_direction dir)
    {
        const uint8_t* ptr = static_cast<const uint8_t*>(data);
        size_t remaining = size;
        
        size_t cmd_idx = 0;

        do
        {
            if (cmd_idx >= m_command_buffer.size())
            {
                m_command_buffer.emplace_back();
            }
            
            command& cmd = m_command_buffer[cmd_idx];
            cmd = command(command_type::port_inject_data);
            
            auto* cmd_data = cmd.data_as<messages::port_inject_data>();
            cmd_data->port = port_hash;
            cmd_data->total_size = static_cast<uint32_t>(size);
            cmd_data->direction = dir;
            
            size_t copy_size = remaining;
            if (copy_size > sizeof(cmd_data->data))
            {
                copy_size = sizeof(cmd_data->data);
            }
                
            cmd_data->size = static_cast<uint32_t>(copy_size);
            
            if (copy_size > 0 && ptr)
            {
                std::memcpy(cmd_data->data, ptr, copy_size);
                ptr += copy_size;
            }
                
            remaining -= copy_size;
            
            if (remaining > 0)
            {
                cmd.set_extended(true);
            }
                
            cmd_idx++;
        } while (remaining > 0);

        return send_command(m_command_buffer.data(), cmd_idx);
    }

    response_status commander::request_language_change(language lang)
    {
        command cmd(command_type::set_language);
        *cmd.data_as<language>() = lang;

        return send_command(cmd);
    }

    response_status commander::send_command(const command& cmd, response** resp)
    {
        return send_command(&cmd, 1, resp);
    }

    response_status commander::send_command(const command* cmds, size_t count, response** resp)
    {
        if (!cmds || count == 0)
        {
            return response_status::command_send_failed;
        }

        for (size_t i = 0; i < count; i++)
        {
            if (!m_queue_command.request_queue().push(cmds[i]))
            {
                return response_status::command_send_failed;
            }
        }

        size_t resp_idx = 0;

        if (!m_queue_command.response_queue().pop(m_response_buffer[resp_idx], 100))
        {
            return response_status::response_receive_failed;
        }

        while (m_response_buffer[resp_idx].is_extended())
        {
            resp_idx++;

            if (resp_idx >= m_response_buffer.size())
            {
                m_response_buffer.emplace_back();
            }
            
            if (!m_queue_command.response_queue().pop(m_response_buffer[resp_idx], 100))
            {
                return response_status::response_receive_failed; // if we fail to receive an expected extended response, we consider the whole command failed, as the sender should always send all responses together
            }
        }

        if (resp)
        {
            *resp = m_response_buffer.data();
        }

        return m_response_buffer.front().get_type();
    }

    void commander::run_event_loop()
    {
        event_context ctx { *this };

        std::vector<event> event_buffer;
        event_buffer.reserve(queue_event_size);
        
        for (size_t i = 0; i < queue_event_size; i++)
        {
            event_buffer.emplace_back();
        }

        while (m_queue_event.is_active())
        {
            size_t evt_idx = 0;

            if (!m_queue_event.pop(event_buffer[evt_idx], 100))
                continue;

            while (event_buffer[evt_idx].is_extended())
            {
                evt_idx++;
                if (evt_idx >= event_buffer.size()) // Shouldn't happen, but just in case
                    event_buffer.emplace_back();
                
                if (!m_queue_event.pop(event_buffer[evt_idx], 100))
                {
                    evt_idx--; // Backtrack the index since this fetch failed
                    break;
                }
            }

            m_dispatcher.dispatch(event_buffer.data(), evt_idx + 1, ctx);
        }
    }
}
