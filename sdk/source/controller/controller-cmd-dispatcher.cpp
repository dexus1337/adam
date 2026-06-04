#include "controller/controller-cmd-dispatcher.hpp"
#include "controller/controller.hpp"
#include "controller/registry.hpp"
#include "resources/language-strings.hpp"
#include "data/port/port.hpp"
#include "data/format.hpp"
#include "module/module.hpp"
#include "data/inspector.hpp"
#include "commander/messages/event.hpp"
#include "commander/messages/message-structs.hpp"
#include "data/connection.hpp"
#include "data/processor.hpp"
#include "data/port/port-input.hpp"
#include "data/port/port-output.hpp"
#include "configuration/parameters/configuration-parameter.hpp"
#include "configuration/parameters/configuration-parameter-string.hpp"
#include "configuration/parameters/configuration-parameter-list.hpp"
#include "memory/buffer/buffer.hpp"
#include "memory/buffer/buffer-manager.hpp"

#include <array>
#include <format>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <ctime>

namespace adam
{
    namespace 
    {
        template<typename msg_type>
        struct message_serializer
        {
            std::vector<msg_type>& messages;
            size_t& msg_idx;
            size_t& unused_off;
            size_t& unused_size;

            message_serializer(std::vector<msg_type>& msgs, size_t& initial_idx, size_t& initial_off, size_t& initial_size)
                : messages(msgs), msg_idx(initial_idx), unused_off(initial_off), unused_size(initial_size) 
            {
            }

            void ensure_space(size_t needed_size) 
            {
                if (unused_size < needed_size) 
                {
                    messages[msg_idx].set_extended(true);
                    msg_idx++;
                    auto type = messages[0].get_type();
                    if (msg_idx >= messages.size()) 
                        messages.emplace_back(type);
                    unused_off = 0;
                    unused_size = msg_type::get_max_data_length();
                }
            }

            void write_bytes(const void* data, size_t size) 
            {
                const uint8_t* ptr = static_cast<const uint8_t*>(data);
                size_t remaining = size;
                while (remaining > 0) 
                {
                    if (unused_size == 0) 
                    {
                        messages[msg_idx].set_extended(true);
                        msg_idx++;
                        auto type = messages[0].get_type();
                        if (msg_idx >= messages.size()) 
                            messages.emplace_back(type);
                        unused_off = 0;
                        unused_size = msg_type::get_max_data_length();
                    }
                    size_t to_write = std::min(remaining, unused_size);
                    std::memcpy(messages[msg_idx].template data_as<uint8_t>() + unused_off, ptr, to_write);
                    unused_off += to_write;
                    unused_size -= to_write;
                    ptr += to_write;
                    remaining -= to_write;
                }
            }
            
            template<typename view_type>
            view_type* allocate_view() 
            {
                ensure_space(sizeof(view_type));
                view_type* view = reinterpret_cast<view_type*>(messages[msg_idx].template data_as<uint8_t>() + unused_off);
                unused_off += sizeof(view_type);
                unused_size -= sizeof(view_type);
                return view;
            }
        };

        template<typename msg_type>
        void serialize_user_parameters(const configuration_parameter_list* user_param, message_serializer<msg_type>& serializer) 
        {
            if (!user_param) return;

            for (const auto& [name, param] : user_param->get_children()) 
            {
                switch (param->get_type()) 
                {
                    case configuration_parameter::type::type_string: 
                    {
                        auto val = dynamic_cast<configuration_parameter_string*>(param.get())->get_value();
                        auto* view = serializer.template allocate_view<configuration_parameter_string::view>();
                        view->var_type = configuration_parameter::type::type_string;
                        view->name = name.get_hash();
                        view->length = static_cast<uint16_t>(val.size());
                        serializer.write_bytes(val.c_str(), val.size());
                        break;
                    }
                    case configuration_parameter::type::type_integer: 
                    {
                        auto* view = serializer.template allocate_view<configuration_parameter_integer::view>();

                        view->var_type = configuration_parameter::type::type_integer;
                        view->name = name.get_hash();
                        view->value = dynamic_cast<configuration_parameter_integer*>(param.get())->get_value();

                        break;
                    }
                    case configuration_parameter::type::type_boolean: 
                    {
                        auto* view = serializer.template allocate_view<configuration_parameter_boolean::view>();

                        view->var_type = configuration_parameter::type::type_boolean;
                        view->name = name.get_hash();
                        view->value = dynamic_cast<configuration_parameter_boolean*>(param.get())->get_value();
                        break;
                    }
                    case configuration_parameter::type::type_double: 
                    {
                        auto* view = serializer.template allocate_view<configuration_parameter_double::view>();

                        view->var_type = configuration_parameter::type::type_double;
                        view->name = name.get_hash();
                        view->value = dynamic_cast<configuration_parameter_double*>(param.get())->get_value();
                        break;
                    }
                    default: break;
                }
            }
        }
    }

    controller_cmd_dispatcher::controller_cmd_dispatcher()
    {
        register_default_handlers();
    }

    controller_cmd_dispatcher::~controller_cmd_dispatcher() {}

    void controller_cmd_dispatcher::register_handler(command_type type, handler_fn handler) 
    {
        m_handlers[type] = std::move(handler);
    }

    void controller_cmd_dispatcher::dispatch(const command* cmds, size_t count, command_context& ctx) const
    {
        auto* prev_ctx = ctx.ctrl.get_context();
        ctx.ctrl.set_context(&ctx);

        if (!cmds || count == 0)
        {
            ctx.set_single_response_status(response_status::invalid);
            ctx.ctrl.set_context(prev_ctx);
            return;
        }

        auto it = m_handlers.find(cmds->get_type());
        if (it != m_handlers.end())
        {
            it->second(cmds, count, ctx);
            ctx.ctrl.set_context(prev_ctx);
            return;
        }
        
        ctx.set_single_response_status(response_status::unknown);
        ctx.ctrl.set_context(prev_ctx);
    }

    void controller_cmd_dispatcher::register_default_handlers()
    {
        register_handler(command_type::acquire_initial_data, [](const command*, size_t, command_context& ctx)
        {
            ctx.set_single_response_status(response_status::success);
            auto* data = ctx.responses.front().data_as<messages::initial_data_header>();

            data->lang_info.lang                  = ctx.ctrl.get_language();
            data->lang_info.supported_languages   |= ( 1 << language_english );
            data->lang_info.supported_languages   |= ( 1 << language_german );

            data->mod_info.available_modules       = static_cast<uint32_t>(ctx.reg.modules().get_available_modules().size());
            data->mod_info.unavailable_modules     = static_cast<uint32_t>(ctx.reg.modules().get_unavailable_modules().size());
            data->mod_info.loaded_modules          = static_cast<uint32_t>(ctx.reg.modules().get_loaded_modules().size());

            size_t resp_idx = 1;

            // Modules
            {
                auto* module_paths_list = ctx.reg.get_module_paths();
                uint32_t path_count = 0;
                if (module_paths_list)
                {
                    for (const auto& [name, param] : module_paths_list->get_children())
                    {
                        auto* str_param = dynamic_cast<configuration_parameter_string*>(param.get());
                        if (!str_param) continue;

                        uint32_t idx = std::strtoul(name.c_str(), nullptr, 10);

                        ctx.responses[resp_idx-1].set_extended(true);
                        auto* path_info = ctx.responses[resp_idx].data_as<messages::module_path_data>();
                        path_info->setup(str_param->get_value().c_str(), idx);
                        resp_idx++;
                        path_count++;

                        if (resp_idx >= ctx.responses.size())
                            ctx.responses.emplace_back();
                    }
                }
                data->mod_info.module_paths = path_count;

                for (const auto& mod : ctx.reg.modules().get_available_modules())
                {
                    ctx.responses[resp_idx-1].set_extended(true);
                    auto* mod_info = ctx.responses[resp_idx].data_as<module::basic_info>();
                    mod_info->setup(module::basic_info::available, mod.first.c_str(), mod.second.second.c_str(), mod.second.first);
                    resp_idx++;

                    if (resp_idx >= ctx.responses.size())
                        ctx.responses.emplace_back();
                }

                for (const auto& mod : ctx.reg.modules().get_unavailable_modules())
                {
                    ctx.responses[resp_idx-1].set_extended(true);
                    auto* mod_info = ctx.responses[resp_idx].data_as<module::basic_info>();
                    mod_info->setup(module::basic_info::unavailable, mod.first.c_str(), std::get<1>(mod.second).c_str(), std::get<0>(mod.second), std::get<2>(mod.second));
                    resp_idx++;

                    if (resp_idx >= ctx.responses.size())
                        ctx.responses.emplace_back();
                }

                for (const auto& mod : ctx.reg.modules().get_loaded_modules())
                {
                    ctx.responses[resp_idx-1].set_extended(true);
                    auto* mod_info = ctx.responses[resp_idx].data_as<module::basic_info>();
                    mod_info->setup(module::basic_info::loaded, mod.first.c_str(), mod.second->get_filepath().c_str(), mod.second->get_version());
                    resp_idx++;

                    if (resp_idx >= ctx.responses.size())
                        ctx.responses.emplace_back();
                }
            }

            // Ports
            {
                data->conn_info.ports       = static_cast<uint32_t>(ctx.reg.ports().size() + ctx.reg.get_unavailable_ports().size());
                data->conn_info.processors  = static_cast<uint32_t>(ctx.reg.filters().size() + ctx.reg.converters().size());
                data->conn_info.connections = static_cast<uint32_t>(ctx.reg.connections().size());

                for (const auto& [hash, prt] : ctx.reg.ports())
                {
                    ctx.responses[resp_idx-1].set_extended(true);
                    auto* port_info = ctx.responses[resp_idx].data_as<port::basic_info>();
                    
                    std::strncpy(port_info->name, prt->get_name().c_str(), sizeof(port_info->name) - 1);
                    port_info->name[sizeof(port_info->name) - 1] = '\0';
                    
                    port_info->type = prt->get_type_name().get_hash();
                    
                    if (auto* mod_param = dynamic_cast<configuration_parameter_string*>(prt->get_parameters().get("type_origin_module"_ct)))
                        port_info->type_module = mod_param->get_value().empty() ? 0 : mod_param->get_value().get_hash();
                    else
                        port_info->type_module = 0;

                    port_info->statistic_buffer_handle  = prt->get_state_buffer()->get_handle();
                    port_info->dir                      = prt->get_direction();
                    port_info->is_unavailable           = false;
                    port_info->is_active                = prt->is_active();

                    auto* user_param = dynamic_cast<configuration_parameter_list*>(prt->get_parameters().get("user_parameters"_ct));

                    if (user_param)
                    {
                        port_info->user_parameters = static_cast<uint16_t>(user_param->get_children().size());
                        
                        size_t unused_off     = sizeof(port::basic_info);
                        size_t unused_size    = response::get_max_data_length() - unused_off;

                        message_serializer<response> serializer(ctx.responses, resp_idx, unused_off, unused_size);
                        serialize_user_parameters(user_param, serializer);
                    }

                    resp_idx++;

                    if (resp_idx >= ctx.responses.size())
                        ctx.responses.emplace_back();
                }

                for (const auto& [hash, upi] : ctx.reg.get_unavailable_ports())
                {
                    ctx.responses[resp_idx-1].set_extended(true);
                    auto* port_info = ctx.responses[resp_idx].data_as<port::basic_info>();
                    
                    port_info->setup(upi->get_name(), upi->type, upi->type_module, true);
                    port_info->is_unavailable = true;
                    port_info->is_active = false;
                    port_info->dir = port::direction_inout;

                    resp_idx++;

                    if (resp_idx >= ctx.responses.size())
                        ctx.responses.emplace_back();
                }
            }

            // Connections
            for (const auto& [hash, conn] : ctx.reg.connections())
            {
                ctx.responses[resp_idx-1].set_extended(true);
                auto* conn_info = ctx.responses[resp_idx].data_as<connection::basic_info>();
                conn_info->setup(conn->get_name());
                
                if (auto* param = dynamic_cast<configuration_parameter_integer*>(conn->get_parameters().get("date_created"_ct)))
                    conn_info->created = static_cast<uint64_t>(param->get_value());
                if (auto* param = dynamic_cast<configuration_parameter_integer*>(conn->get_parameters().get("date_edited"_ct)))
                    conn_info->edited = static_cast<uint64_t>(param->get_value());
                if (auto* param = dynamic_cast<configuration_parameter_integer*>(conn->get_parameters().get("sorting_index"_ct)))
                    conn_info->sorting_index = static_cast<uint32_t>(param->get_value());
                if (auto* param = dynamic_cast<configuration_parameter_integer*>(conn->get_parameters().get("color_code"_ct)))
                    conn_info->color = static_cast<uint32_t>(param->get_value());
                
                conn_info->is_active = conn->is_active();
                conn_info->valid_chain = conn->is_valid_chain();
                conn_info->is_unavailable = false;

                // Include connection format data
                if (conn->get_input_format())
                    conn_info->input_format = conn->get_input_format()->get_name().get_hash();
                else
                    conn_info->input_format = ("transparent"_ct).get_hash();
                
                if (auto* param = dynamic_cast<configuration_parameter_string*>(conn->get_parameters().get("input_format_module"_ct)))
                    conn_info->input_format_module = param->get_value().get_hash();
                else
                    conn_info->input_format_module = 0;

                if (conn->get_output_format())
                    conn_info->output_format = conn->get_output_format()->get_name().get_hash();
                else
                    conn_info->output_format = ("transparent"_ct).get_hash();
                
                if (auto* param = dynamic_cast<configuration_parameter_string*>(conn->get_parameters().get("output_format_module"_ct)))
                    conn_info->output_format_module = param->get_value().get_hash();
                else
                    conn_info->output_format_module = 0;

                conn_info->input_count = 0;
                conn_info->processor_count = 0;
                conn_info->output_count = 0;
                
                conn->ports_input().iterate([&](const auto& inputs) 
                {
                    for (size_t i = 0; i < inputs.size() && conn_info->input_count < connection::basic_info::default_type_count; ++i)
                        conn_info->inputs[conn_info->input_count++] = inputs[i]->get_name().get_hash();
                });
                
                for (size_t i = 0; i < conn->unavailable_inputs().size() && conn_info->input_count < connection::basic_info::default_type_count; ++i)
                    conn_info->inputs[conn_info->input_count++] = conn->unavailable_inputs()[i].get_hash();

                conn->processors().iterate([&](const auto& procs) 
                {
                    conn_info->processor_count = static_cast<uint16_t>(std::min(procs.size(), static_cast<size_t>(connection::basic_info::default_type_count)));
                    for (size_t i = 0; i < conn_info->processor_count; ++i)
                        conn_info->processors[i] = procs[i]->get_name().get_hash();
                });

                conn->ports_output().iterate([&](const auto& outputs) 
                {
                    for (size_t i = 0; i < outputs.size() && conn_info->output_count < connection::basic_info::default_type_count; ++i)
                        conn_info->outputs[conn_info->output_count++] = outputs[i]->get_name().get_hash();
                });

                for (size_t i = 0; i < conn->unavailable_outputs().size() && conn_info->output_count < connection::basic_info::default_type_count; ++i)
                    conn_info->outputs[conn_info->output_count++] = conn->unavailable_outputs()[i].get_hash();

                resp_idx++;

                if (resp_idx >= ctx.responses.size())
                    ctx.responses.emplace_back();
            }
            
            for (const auto& [hash, uconn] : ctx.reg.get_unavailable_connections())
            {
                ctx.responses[resp_idx-1].set_extended(true);
                auto* conn_info = ctx.responses[resp_idx].data_as<connection::basic_info>();
                conn_info->setup(uconn->get_name());
                
                if (auto* param = dynamic_cast<configuration_parameter_integer*>(uconn->get_parameters().get("date_created"_ct)))
                    conn_info->created = static_cast<uint64_t>(param->get_value());
                if (auto* param = dynamic_cast<configuration_parameter_integer*>(uconn->get_parameters().get("date_edited"_ct)))
                    conn_info->edited = static_cast<uint64_t>(param->get_value());
                if (auto* param = dynamic_cast<configuration_parameter_integer*>(uconn->get_parameters().get("sorting_index"_ct)))
                    conn_info->sorting_index = static_cast<uint32_t>(param->get_value());
                if (auto* param = dynamic_cast<configuration_parameter_integer*>(uconn->get_parameters().get("color_code"_ct)))
                    conn_info->color = static_cast<uint32_t>(param->get_value());
                
                conn_info->is_active = false;
                conn_info->valid_chain = false;
                conn_info->is_unavailable = true;

                if (auto* param = dynamic_cast<configuration_parameter_string*>(uconn->get_parameters().get("input_format"_ct)))
                    conn_info->input_format = param->get_value().get_hash();
                else
                    conn_info->input_format = ("transparent"_ct).get_hash();
                if (auto* param = dynamic_cast<configuration_parameter_string*>(uconn->get_parameters().get("input_format_module"_ct)))
                    conn_info->input_format_module = param->get_value().get_hash();
                else
                    conn_info->input_format_module = 0;

                if (auto* param = dynamic_cast<configuration_parameter_string*>(uconn->get_parameters().get("output_format"_ct)))
                    conn_info->output_format = param->get_value().get_hash();
                else
                    conn_info->output_format = ("transparent"_ct).get_hash();
                if (auto* param = dynamic_cast<configuration_parameter_string*>(uconn->get_parameters().get("output_format_module"_ct)))
                    conn_info->output_format_module = param->get_value().get_hash();
                else
                    conn_info->output_format_module = 0;

                conn_info->input_count = 0;
                conn_info->processor_count = 0;
                conn_info->output_count = 0;
                
                if (auto* inputs_list = dynamic_cast<configuration_parameter_list*>(uconn->get_parameters().get("inputs"_ct)))
                {
                    for (size_t i = 0; i < inputs_list->get_children().size() && conn_info->input_count < connection::basic_info::default_type_count; ++i)
                    {
                        if (auto* param = dynamic_cast<configuration_parameter_reference*>(inputs_list->get(string_hashed(std::to_string(i)))))
                            conn_info->inputs[conn_info->input_count++] = param->get_target().get_hash();
                    }
                }
                
                if (auto* procs_list = dynamic_cast<configuration_parameter_list*>(uconn->get_parameters().get("processors"_ct)))
                {
                    for (size_t i = 0; i < procs_list->get_children().size() && conn_info->processor_count < connection::basic_info::default_type_count; ++i)
                    {
                        if (auto* param = dynamic_cast<configuration_parameter_reference*>(procs_list->get(string_hashed(std::to_string(i)))))
                            conn_info->processors[conn_info->processor_count++] = param->get_target().get_hash();
                    }
                }

                if (auto* outputs_list = dynamic_cast<configuration_parameter_list*>(uconn->get_parameters().get("outputs"_ct)))
                {
                    for (size_t i = 0; i < outputs_list->get_children().size() && conn_info->output_count < connection::basic_info::default_type_count; ++i)
                    {
                        if (auto* param = dynamic_cast<configuration_parameter_reference*>(outputs_list->get(string_hashed(std::to_string(i)))))
                            conn_info->outputs[conn_info->output_count++] = param->get_target().get_hash();
                    }
                }

                resp_idx++;

                if (resp_idx >= ctx.responses.size())
                    ctx.responses.emplace_back();
            }
        });

        register_handler(command_type::set_language, [](const command* cmds, size_t, command_context& ctx) 
        {
            ctx.ctrl.set_language(*cmds->get_data_as<language>());
            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::language_changed, ctx.ctrl.get_language()));

            event evt(event_type::language_changed);
            *evt.data_as<language>() = ctx.ctrl.get_language();
            ctx.ctrl.broadcast_event(evt);

            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::module_path_add, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::module_path_data>();
            string_hashed path(params->path);
            uint32_t idx = params->idx;

            if (!ctx.reg.add_module_path(path, &idx))
            {
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::module_path_add_failed, ctx.ctrl.get_language()), ctx.tid, path.c_str());
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            event evt(event_type::module_path_added);
            auto* evt_data = evt.data_as<messages::module_path_data>();
            evt_data->setup(params->path, idx);
            ctx.ctrl.broadcast_event(evt);

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::module_path_added, ctx.ctrl.get_language()), ctx.tid, path.c_str());
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::module_path_remove, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::module_path_remove_data>();

            if (!ctx.reg.remove_module_path(params->idx))
            {
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::module_path_remove_failed, ctx.ctrl.get_language()), ctx.tid, params->idx);
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            event evt(event_type::module_path_removed);
            auto* evt_data = evt.data_as<messages::module_path_remove_data>();
            evt_data->idx = params->idx;
            ctx.ctrl.broadcast_event(evt);

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::module_path_removed, ctx.ctrl.get_language()), ctx.tid, params->idx);
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::module_scan, [](const command*, size_t, command_context& ctx) 
        {
            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::module_scan_requested, ctx.ctrl.get_language()), ctx.tid);
            
            ctx.reg.modules().scan_for_modules();
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::module_load, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::module_action_data>();
            string_hashed name(params->module_name);

            if (ctx.reg.modules().load_module(name))
                ctx.set_single_response_status(response_status::success);
            else
                ctx.set_single_response_status(response_status::failed);
        });

        register_handler(command_type::module_unload, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::module_action_data>();
            string_hashed name(params->module_name);

            if (ctx.reg.modules().unload_module(name))
                ctx.set_single_response_status(response_status::success);
            else
                ctx.set_single_response_status(response_status::failed);
        });

        register_handler(command_type::connection_create, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<connection::basic_info>();
            string_hashed name(params->name);

            connection* new_conn = nullptr;
            registry::status res = ctx.reg.create_connection(name, &new_conn);

            if (res != registry::status_success)
            {
                auto name_view = name.c_str();
                auto status_text = registry::get_status_text(res, ctx.ctrl.get_language());
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_create_failed, ctx.ctrl.get_language()), ctx.tid, name_view, status_text);
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            uint64_t current_time = static_cast<uint64_t>(std::time(nullptr));
            if (new_conn)
            {
                if (auto* param = dynamic_cast<configuration_parameter_integer*>(new_conn->get_parameters().get("date_created"_ct)))
                    param->set_value(current_time);
                if (auto* param = dynamic_cast<configuration_parameter_integer*>(new_conn->get_parameters().get("date_edited"_ct)))
                    param->set_value(current_time);
            }

            event evt(event_type::connection_created);
            auto* evt_data = evt.data_as<connection::basic_info>();
            *evt_data = *params;
            evt_data->created = current_time;
            evt_data->edited = current_time;
            ctx.ctrl.broadcast_event(evt);

            auto name_view = name.c_str();
            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_created, ctx.ctrl.get_language()), ctx.tid, name_view);
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::connection_destroy, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::connection_destroy_data>();

            std::string conn_str;
            auto it = ctx.reg.connections().find(params->connection);
            if (it != ctx.reg.connections().end())
                conn_str = it->second->get_name().c_str();

            registry::status res = ctx.reg.destroy_connection(params->connection);

            if (res != registry::status_success)
            {
                uint64_t conn_hash = static_cast<uint64_t>(params->connection);
                auto status_text = registry::get_status_text(res, ctx.ctrl.get_language());
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_destroy_failed, ctx.ctrl.get_language()), ctx.tid, conn_hash, status_text);
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            event evt(event_type::connection_destroyed);
            evt.data_as<messages::connection_destroy_data>()->connection = params->connection;
            ctx.ctrl.broadcast_event(evt);

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_destroyed, ctx.ctrl.get_language()), ctx.tid, conn_str);
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::connection_start, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::connection_action_data>();
            auto it = ctx.reg.connections().find(params->connection);

            if (it == ctx.reg.connections().end() || !it->second->start())
            {
                uint64_t conn_hash = static_cast<uint64_t>(params->connection);
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_start_failed, ctx.ctrl.get_language()), ctx.tid, conn_hash);
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            event evt(event_type::connection_started);
            evt.data_as<messages::connection_action_data>()->connection = params->connection;
            ctx.ctrl.broadcast_event(evt);

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_started, ctx.ctrl.get_language()), ctx.tid, it->second->get_name().c_str());
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::connection_stop, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::connection_action_data>();
            auto it = ctx.reg.connections().find(params->connection);

            if (it == ctx.reg.connections().end() || !it->second->stop())
            {
                uint64_t conn_hash = static_cast<uint64_t>(params->connection);
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_stop_failed, ctx.ctrl.get_language()), ctx.tid, conn_hash);
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            event evt(event_type::connection_stopped);
            evt.data_as<messages::connection_action_data>()->connection = params->connection;
            ctx.ctrl.broadcast_event(evt);

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_stopped, ctx.ctrl.get_language()), ctx.tid, it->second->get_name().c_str());
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::connection_rename, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::connection_rename_data>();
            string_hashed new_name(params->new_name);

            std::string old_conn_str;
            auto it = ctx.reg.connections().find(params->connection);
            if (it != ctx.reg.connections().end())
                old_conn_str = it->second->get_name().c_str();

            registry::status res = ctx.reg.rename_connection(params->connection, new_name);

            if (res != registry::status_success)
            {
                uint64_t conn_hash = static_cast<uint64_t>(params->connection);
                auto status_text = registry::get_status_text(res, ctx.ctrl.get_language());
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_rename_failed, ctx.ctrl.get_language()), ctx.tid, conn_hash, status_text);
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            uint64_t current_time = static_cast<uint64_t>(std::time(nullptr));
            auto it_renamed = ctx.reg.connections().find(new_name.get_hash());
            if (it_renamed != ctx.reg.connections().end())
            {
                if (auto* param = dynamic_cast<configuration_parameter_integer*>(it_renamed->second->get_parameters().get("date_edited"_ct)))
                    param->set_value(static_cast<int64_t>(current_time));
            }

            event evt(event_type::connection_renamed);
            auto* evt_data = evt.data_as<messages::connection_rename_data>();
            *evt_data = *params;
            evt_data->edited = current_time;
            ctx.ctrl.broadcast_event(evt);

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_renamed, ctx.ctrl.get_language()), ctx.tid, old_conn_str.c_str(), new_name.c_str());
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::connection_port_add, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::connection_port_add_data>();

            std::string conn_str;
            auto it_conn = ctx.reg.connections().find(params->connection);
            if (it_conn != ctx.reg.connections().end())
                conn_str = it_conn->second->get_name().c_str();

            std::string port_str;
            auto it_port = ctx.reg.ports().find(params->port);
            if (it_port != ctx.reg.ports().end())
                port_str = it_port->second->get_name().c_str();

            registry::status res = ctx.reg.connection_add_port(params->connection, params->port, params->is_input);

            if (res != registry::status_success)
            {
                uint64_t conn_hash = static_cast<uint64_t>(params->connection);
                uint64_t port_hash = static_cast<uint64_t>(params->port);
                auto status_text = registry::get_status_text(res, ctx.ctrl.get_language());
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_port_add_failed, ctx.ctrl.get_language()), ctx.tid, port_hash, conn_hash, status_text);
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            uint64_t current_time = static_cast<uint64_t>(std::time(nullptr));
            if (it_conn != ctx.reg.connections().end())
            {
                if (auto* param = dynamic_cast<configuration_parameter_integer*>(it_conn->second->get_parameters().get("date_edited"_ct)))
                    param->set_value(static_cast<int64_t>(current_time));
            }

            event evt(event_type::connection_port_added);
            auto* evt_data = evt.data_as<messages::connection_port_add_data>();
            *evt_data = *params;
            evt_data->edited = current_time;
            if (it_conn != ctx.reg.connections().end())
                evt_data->valid_chain = it_conn->second->is_valid_chain();
            else
                evt_data->valid_chain = false;
            ctx.ctrl.broadcast_event(evt);

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_port_added, ctx.ctrl.get_language()), ctx.tid, port_str.c_str(), conn_str.c_str());
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::connection_port_remove, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::connection_port_add_data>();

            std::string conn_str;
            auto it_conn = ctx.reg.connections().find(params->connection);
            if (it_conn != ctx.reg.connections().end())
                conn_str = it_conn->second->get_name().c_str();

            std::string port_str;
            auto it_port = ctx.reg.ports().find(params->port);
            if (it_port != ctx.reg.ports().end())
                port_str = it_port->second->get_name().c_str();

            registry::status res = ctx.reg.connection_remove_port(params->connection, params->port, params->is_input);

            if (res != registry::status_success)
            {
                uint64_t conn_hash = static_cast<uint64_t>(params->connection);
                uint64_t port_hash = static_cast<uint64_t>(params->port);
                auto status_text = registry::get_status_text(res, ctx.ctrl.get_language());
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_port_remove_failed, ctx.ctrl.get_language()), ctx.tid, port_hash, conn_hash, status_text);
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            uint64_t current_time = static_cast<uint64_t>(std::time(nullptr));

            event evt(event_type::connection_port_removed);
            auto* evt_data = evt.data_as<messages::connection_port_add_data>();
            *evt_data = *params;
            evt_data->edited = current_time;
            if (it_conn != ctx.reg.connections().end())
                evt_data->valid_chain = it_conn->second->is_valid_chain();
            else
                evt_data->valid_chain = false;
            ctx.ctrl.broadcast_event(evt);

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_port_removed, ctx.ctrl.get_language()), ctx.tid, port_str.c_str(), conn_str.c_str());
            
            if (it_port != ctx.reg.ports().end())
            {
                it_port->second->in_connections().iterate([](const auto&){}); // Force active array update on the double-buffer to guarantee accurate size tracking
                it_port->second->out_connections().iterate([](const auto&){}); // Force active array update on the double-buffer to guarantee accurate size tracking
                if (it_port->second->in_connections().empty() && it_port->second->out_connections().empty())
                {
                    ctx.reg.destroy_port(params->port);
                    event evt_del(event_type::port_destroyed);
                    evt_del.data_as<messages::port_destroy_data>()->port = params->port;
                    ctx.ctrl.broadcast_event(evt_del);
                    ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_destroyed, ctx.ctrl.get_language()), ctx.tid, port_str.c_str());
                }
            }

            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::connection_sorting_index_change, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::connection_property_change_data>();

            std::string conn_str;
            auto it = ctx.reg.connections().find(params->connection);
            if (it == ctx.reg.connections().end())
            {
                uint64_t conn_hash = static_cast<uint64_t>(params->connection);
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_sorting_index_change_failed, ctx.ctrl.get_language()), ctx.tid, conn_hash);
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            conn_str = it->second->get_name().c_str();

            if (auto* param = dynamic_cast<configuration_parameter_integer*>(it->second->get_parameters().get("sorting_index"_ct)))
                param->set_value(params->value);

            uint64_t current_time = static_cast<uint64_t>(std::time(nullptr));
            if (auto* param = dynamic_cast<configuration_parameter_integer*>(it->second->get_parameters().get("date_edited"_ct)))
                param->set_value(static_cast<int64_t>(current_time));

            event evt(event_type::connection_sorting_index_changed);
            auto* evt_data = evt.data_as<messages::connection_property_change_data>();
            *evt_data = *params;
            evt_data->edited = current_time;
            ctx.ctrl.broadcast_event(evt);

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_sorting_index_changed, ctx.ctrl.get_language()), ctx.tid, conn_str.c_str(), params->value);
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::connection_color_change, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::connection_property_change_data>();

            std::string conn_str;
            auto it = ctx.reg.connections().find(params->connection);
            if (it == ctx.reg.connections().end())
            {
                uint64_t conn_hash = static_cast<uint64_t>(params->connection);
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_color_change_failed, ctx.ctrl.get_language()), ctx.tid, conn_hash);
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            conn_str = it->second->get_name().c_str();

            if (auto* param = dynamic_cast<configuration_parameter_integer*>(it->second->get_parameters().get("color_code"_ct)))
                param->set_value(params->value);

            uint64_t current_time = static_cast<uint64_t>(std::time(nullptr));
            if (auto* param = dynamic_cast<configuration_parameter_integer*>(it->second->get_parameters().get("date_edited"_ct)))
                param->set_value(static_cast<int64_t>(current_time));

            event evt(event_type::connection_color_changed);
            auto* evt_data = evt.data_as<messages::connection_property_change_data>();
            *evt_data = *params;
            evt_data->edited = current_time;
            ctx.ctrl.broadcast_event(evt);

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_color_changed, ctx.ctrl.get_language()), ctx.tid, conn_str.c_str(), params->value);
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::connection_set_input_data_format, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::connection_data_format_data>();

            auto conn_it = ctx.reg.connections().find(params->connection);
            if (conn_it == ctx.reg.connections().end())
            {
                uint64_t conn_hash = static_cast<uint64_t>(params->connection);
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_input_data_format_change_failed, ctx.ctrl.get_language()), ctx.tid, conn_hash);
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            connection* conn = conn_it->second.get();

            const data_format* in_fmt  = &data_format_transparent;
            string_hashed resolved_in_module;

            auto resolve_format = [&](string_hash fmt_hash, string_hash mod_hash, const data_format*& out_format, string_hashed& out_module)
            {
                if (fmt_hash == 0 || fmt_hash == ("transparent"_ct).get_hash())
                    return; // stays transparent

                if (mod_hash != 0)
                {
                    auto mod_it = ctx.reg.modules().get_loaded_modules().find(mod_hash);
                    if (mod_it != ctx.reg.modules().get_loaded_modules().end())
                    {
                        auto fmt_it = mod_it->second->get_data_formats().find(fmt_hash);
                        if (fmt_it != mod_it->second->get_data_formats().end())
                        {
                            out_format = fmt_it->second;
                            out_module = mod_it->first;
                        }
                    }
                }
                else
                {
                    for (const auto& [mod_name, mod] : ctx.reg.modules().get_loaded_modules())
                    {
                        auto fmt_it = mod->get_data_formats().find(fmt_hash);
                        if (fmt_it != mod->get_data_formats().end())
                        {
                            out_format = fmt_it->second;
                            out_module = mod_name;
                            break;
                        }
                    }
                }
            };

            resolve_format(params->format, params->format_module, in_fmt, resolved_in_module);

            bool was_active = conn->is_active();
            conn->set_input_format(in_fmt);
            
            if (was_active && !conn->is_valid_chain())
            {
                conn->stop();
                event evt_stop(event_type::connection_stopped);
                evt_stop.data_as<messages::connection_action_data>()->connection = params->connection;
                ctx.ctrl.broadcast_event(evt_stop);
                ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_stopped, ctx.ctrl.get_language()), ctx.tid, conn->get_name().c_str());
            }

            auto set_str_param = [&](const string_hashed_ct& key, const string_hashed& value)
            {
                if (auto* p = dynamic_cast<configuration_parameter_string*>(conn->get_parameters().get(key)))
                    p->set_value(value);
            };

            set_str_param("input_format"_ct,        in_fmt  == &data_format_transparent ? "transparent"_ct : in_fmt->get_name());
            set_str_param("input_format_module"_ct, resolved_in_module);

            event evt(event_type::connection_input_data_format_changed);
            auto* evt_data = evt.data_as<messages::connection_data_format_data>();
            evt_data->connection            = params->connection;
            evt_data->format                = in_fmt->get_name().get_hash();
            evt_data->format_module         = resolved_in_module.get_hash();
            evt_data->valid_chain           = conn->is_valid_chain();
            ctx.ctrl.broadcast_event(evt);

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_input_data_format_changed, ctx.ctrl.get_language()), ctx.tid, conn->get_name().c_str());
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::connection_set_output_data_format, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::connection_data_format_data>();

            auto conn_it = ctx.reg.connections().find(params->connection);
            if (conn_it == ctx.reg.connections().end())
            {
                uint64_t conn_hash = static_cast<uint64_t>(params->connection);
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_output_data_format_change_failed, ctx.ctrl.get_language()), ctx.tid, conn_hash);
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            connection* conn = conn_it->second.get();

            const data_format* out_fmt = &data_format_transparent;
            string_hashed resolved_out_module;

            auto resolve_format = [&](string_hash fmt_hash, string_hash mod_hash, const data_format*& out_format, string_hashed& out_module)
            {
                if (fmt_hash == 0 || fmt_hash == ("transparent"_ct).get_hash())
                    return; // stays transparent

                if (mod_hash != 0)
                {
                    auto mod_it = ctx.reg.modules().get_loaded_modules().find(mod_hash);
                    if (mod_it != ctx.reg.modules().get_loaded_modules().end())
                    {
                        auto fmt_it = mod_it->second->get_data_formats().find(fmt_hash);
                        if (fmt_it != mod_it->second->get_data_formats().end())
                        {
                            out_format = fmt_it->second;
                            out_module = mod_it->first;
                        }
                    }
                }
                else
                {
                    for (const auto& [mod_name, mod] : ctx.reg.modules().get_loaded_modules())
                    {
                        auto fmt_it = mod->get_data_formats().find(fmt_hash);
                        if (fmt_it != mod->get_data_formats().end())
                        {
                            out_format = fmt_it->second;
                            out_module = mod_name;
                            break;
                        }
                    }
                }
            };

            resolve_format(params->format, params->format_module, out_fmt, resolved_out_module);

            bool was_active = conn->is_active();
            conn->set_output_format(out_fmt);
            
            if (was_active && !conn->is_valid_chain())
            {
                conn->stop();
                event evt_stop(event_type::connection_stopped);
                evt_stop.data_as<messages::connection_action_data>()->connection = params->connection;
                ctx.ctrl.broadcast_event(evt_stop);
                ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_stopped, ctx.ctrl.get_language()), ctx.tid, conn->get_name().c_str());
            }

            auto set_str_param = [&](const string_hashed_ct& key, const string_hashed& value)
            {
                if (auto* p = dynamic_cast<configuration_parameter_string*>(conn->get_parameters().get(key)))
                    p->set_value(value);
            };

            set_str_param("output_format"_ct,        out_fmt == &data_format_transparent ? "transparent"_ct : out_fmt->get_name());
            set_str_param("output_format_module"_ct, resolved_out_module);

            event evt(event_type::connection_output_data_format_changed);
            auto* evt_data = evt.data_as<messages::connection_data_format_data>();
            evt_data->connection            = params->connection;
            evt_data->format                = out_fmt->get_name().get_hash();
            evt_data->format_module         = resolved_out_module.get_hash();
            evt_data->valid_chain           = conn->is_valid_chain();
            ctx.ctrl.broadcast_event(evt);

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_output_data_format_changed, ctx.ctrl.get_language()), ctx.tid, conn->get_name().c_str());
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::port_create, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<port::basic_info>();
            string_hashed name(params->name);
            
            if (ctx.reg.ports().find(name.get_hash()) != ctx.reg.ports().end() || 
                ctx.reg.get_unavailable_ports().find(name.get_hash()) != ctx.reg.get_unavailable_ports().end())
            {
                auto name_view = name.c_str();
                auto status_text = registry::get_status_text(registry::status_error_port_already_exists, ctx.ctrl.get_language());
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_create_failed, ctx.ctrl.get_language()), ctx.tid, name_view, status_text);
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            port* new_port = nullptr;
            registry::status res = ctx.reg.create_port(name, params->type, params->type_module, &new_port);

            if (res != registry::status_success)
            {
                auto name_view = name.c_str();
                auto status_text = registry::get_status_text(res, ctx.ctrl.get_language());
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_create_failed, ctx.ctrl.get_language()), ctx.tid, name_view, status_text);
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            std::vector<event> events;
            events.emplace_back(event_type::port_created);
            auto* evt_data = events[0].data_as<port::basic_info>();
            if (new_port)
            {
                evt_data->setup(new_port->get_name(), new_port->get_type_name().get_hash(), 0, false, new_port->get_state_buffer()->get_handle());

                if (auto* mod_param = dynamic_cast<configuration_parameter_string*>(new_port->get_parameters().get("type_origin_module"_ct)))
                    evt_data->type_module = mod_param->get_value().empty() ? 0 : mod_param->get_value().get_hash();

                evt_data->dir                       = new_port->get_direction();
                evt_data->is_active                 = new_port->is_active();
                
                auto* user_param = dynamic_cast<configuration_parameter_list*>(new_port->get_parameters().get("user_parameters"_ct));
                if (user_param)
                {
                    evt_data->user_parameters = static_cast<uint16_t>(user_param->get_children().size());
                    
                    size_t evt_idx = 0;
                    size_t unused_off = sizeof(port::basic_info);
                    size_t unused_size = event::get_max_data_length() - unused_off;

                    message_serializer<event> serializer(events, evt_idx, unused_off, unused_size);
                    serialize_user_parameters(user_param, serializer);
                }
            }
            else
                *evt_data = *params;
                
            for (const auto& ev : events)
                ctx.ctrl.broadcast_event(ev);

            auto name_view = name.c_str();
            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_created, ctx.ctrl.get_language()), ctx.tid, name_view, new_port->get_type_name().c_str());
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::port_destroy, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::port_destroy_data>();

            // Get port string if it exists for success logging
            std::string port_str;
            auto it = ctx.reg.ports().find(params->port);
            if (it != ctx.reg.ports().end())
                port_str = it->second->get_name().c_str();

            registry::status res = ctx.reg.destroy_port(params->port);

            if (res != registry::status_success)
            {
                uint64_t port_hash = static_cast<uint64_t>(params->port);
                auto status_text = registry::get_status_text(res, ctx.ctrl.get_language());
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_destroy_failed, ctx.ctrl.get_language()), ctx.tid, port_hash, status_text);
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            event evt(event_type::port_destroyed);
            evt.data_as<messages::port_destroy_data>()->port = params->port;
            ctx.ctrl.broadcast_event(evt);

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_destroyed, ctx.ctrl.get_language()), ctx.tid, port_str.c_str());
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::port_rename, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::port_rename_data>();
            string_hashed new_name(params->new_name);

            std::string old_port_str;
            auto it = ctx.reg.ports().find(params->port);
            if (it != ctx.reg.ports().end())
                old_port_str = it->second->get_name().c_str();

            registry::status res = ctx.reg.rename_port(params->port, new_name);

            if (res != registry::status_success)
            {
                uint64_t port_hash = static_cast<uint64_t>(params->port);
                auto status_text = registry::get_status_text(res, ctx.ctrl.get_language());
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_rename_failed, ctx.ctrl.get_language()), ctx.tid, port_hash, status_text);
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            uint64_t current_time = static_cast<uint64_t>(std::time(nullptr));
            auto renamed_it = ctx.reg.ports().find(new_name.get_hash());
            if (renamed_it != ctx.reg.ports().end())
            {
                auto update_timestamp = [&](const auto& conns)
                {
                    for (auto* conn : conns)
                    {
                        if (auto* param = dynamic_cast<configuration_parameter_integer*>(conn->get_parameters().get("date_edited"_ct)))
                            param->set_value(static_cast<int64_t>(current_time));
                    }
                };

                renamed_it->second->in_connections().iterate(update_timestamp);
                renamed_it->second->out_connections().iterate(update_timestamp);
            }

            event evt(event_type::port_renamed);
            auto* evt_data = evt.data_as<messages::port_rename_data>();
            evt_data->port = params->port;
            std::strncpy(evt_data->new_name, params->new_name, sizeof(evt_data->new_name) - 1);
            evt_data->new_name[sizeof(evt_data->new_name) - 1] = '\0';
            evt_data->edited = current_time;
            ctx.ctrl.broadcast_event(evt);

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_renamed, ctx.ctrl.get_language()), ctx.tid, old_port_str.c_str(), new_name.c_str());
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::port_set_parameter, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::port_set_parameter_data>();
            auto it = ctx.reg.ports().find(params->port);

            if (it == ctx.reg.ports().end())
            {
                uint64_t port_hash = static_cast<uint64_t>(params->port);
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_parameter_update_failed, ctx.ctrl.get_language()), ctx.tid, port_hash);
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            auto* port = it->second.get();
            auto* user_params = dynamic_cast<configuration_parameter_list*>(port->get_parameters().get("user_parameters"_ct));
            if (!user_params)
            {
                uint64_t port_hash = static_cast<uint64_t>(params->port);
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_parameter_update_failed, ctx.ctrl.get_language()), ctx.tid, port_hash);
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            auto* param = user_params->get(params->param_view.name);
            if (!param)
            {
                uint64_t port_hash = static_cast<uint64_t>(params->port);
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_parameter_update_failed, ctx.ctrl.get_language()), ctx.tid, port_hash);
                ctx.set_single_response_status(response_status::failed);
                return;
            }
            
            bool changed = false;

            if (param->get_type() == params->param_view.var_type)
            {
                switch (param->get_type())
                {
                    case configuration_parameter::type_integer:
                    {
                        int64_t val;
                        std::memcpy(&val, params->data, sizeof(int64_t));
                        if (static_cast<configuration_parameter_integer*>(param)->set_value(val)) changed = true;
                        break;
                    }
                    case configuration_parameter::type_double:
                    {
                        double val;
                        std::memcpy(&val, params->data, sizeof(double));
                        static_cast<configuration_parameter_double*>(param)->set_value(val);
                        changed = true;
                        break;
                    }
                    case configuration_parameter::type_boolean:
                    {
                        bool val;
                        std::memcpy(&val, params->data, sizeof(bool));
                        static_cast<configuration_parameter_boolean*>(param)->set_value(val);
                        changed = true;
                        break;
                    }
                    case configuration_parameter::type_string:
                    {
                        uint16_t len;
                        std::memcpy(&len, params->data, sizeof(uint16_t));
                        std::string val(len, '\0');
                        if (len > 0)
                            std::memcpy(&val[0], params->data + sizeof(uint16_t), len);
                        if (static_cast<configuration_parameter_string*>(param)->set_value(string_hashed(val))) changed = true;
                        break;
                    }
                    default:
                        break;
                }
            }

            if (changed)
            {
                event evt(event_type::port_parameter_updated);
                auto* evt_data = evt.data_as<messages::port_parameter_updated_data>();
                evt_data->port = params->port;
                evt_data->param_view = params->param_view;
                std::memcpy(evt_data->data, params->data, sizeof(evt_data->data));
                ctx.ctrl.broadcast_event(evt);
                
                ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_parameter_updated, ctx.ctrl.get_language()), ctx.tid, port->get_name().c_str());
                ctx.set_single_response_status(response_status::success);
            }
            else
            {
                uint64_t port_hash = static_cast<uint64_t>(params->port);
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_parameter_update_failed, ctx.ctrl.get_language()), ctx.tid, port_hash);
                ctx.set_single_response_status(response_status::failed);
            }
        });

        register_handler(command_type::port_start, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::port_action_data>();
            auto it = ctx.reg.ports().find(params->port);

            if (it == ctx.reg.ports().end() || !it->second->start())
            {
                if (it != ctx.reg.ports().end())
                {
                    ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_start_failed, ctx.ctrl.get_language()), ctx.tid, it->second->get_name().c_str());
                }
                else
                {
                    auto hash_str = std::format("0x{:X}", static_cast<uint64_t>(params->port));
                    ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_start_failed, ctx.ctrl.get_language()), ctx.tid, hash_str.c_str());
                }
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            event evt(event_type::port_started);
            evt.data_as<port::status_event_info>()->port_hash = params->port;
            ctx.ctrl.broadcast_event(evt);

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_started, ctx.ctrl.get_language()), ctx.tid, it->second->get_name().c_str());
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::port_stop, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::port_action_data>();
            auto it = ctx.reg.ports().find(params->port);

            if (it == ctx.reg.ports().end() || !it->second->stop())
            {
                if (it != ctx.reg.ports().end())
                {
                    ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_stop_failed, ctx.ctrl.get_language()), ctx.tid, it->second->get_name().c_str());
                }
                else
                {
                    auto hash_str = std::format("0x{:X}", static_cast<uint64_t>(params->port));
                    ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_stop_failed, ctx.ctrl.get_language()), ctx.tid, hash_str.c_str());
                }
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            event evt(event_type::port_stopped);
            evt.data_as<port::status_event_info>()->port_hash = params->port;
            ctx.ctrl.broadcast_event(evt);

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_stopped, ctx.ctrl.get_language()), ctx.tid, it->second->get_name().c_str());
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::port_inject_data, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::port_inject_data>();
            auto it = ctx.reg.ports().find(params->port);

            if (it == ctx.reg.ports().end())
            {
                uint64_t port_hash = static_cast<uint64_t>(params->port);
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_data_inject_failed, ctx.ctrl.get_language()), ctx.tid, port_hash);
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            if (!params->size)
            {
                ctx.set_single_response_status(response_status::invalid_argument);
                return;
            }

            buffer* buf = buffer_manager::get().request_buffer(params->total_size);
            uint32_t off = 0;
            buf->set_timestamp();

            while (cmds)
            {
                std::memcpy(buf->data_as<uint8_t>() + off, params->data, params->size);

                off += params->size;

                if (off >= params->total_size)
                    break;

                if (!cmds->is_extended())
                    break;

                cmds++;
                params = cmds->get_data_as<messages::port_inject_data>();
            }

            buf->set_size(off);

            if (it->second->handle_data(buf, params->direction))
            {
                debug_statement(ctx.ctrl.log(log::trace, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_data_injected, ctx.ctrl.get_language()), ctx.tid, it->second->get_name().c_str()));
                ctx.set_single_response_status(response_status::success);
            }
            else
            {
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_data_inject_failed, ctx.ctrl.get_language()), ctx.tid, it->second->get_name().c_str());
                ctx.set_single_response_status(response_status::failed);
            }

            buf->release();
        });
        
        register_handler(command_type::inspector_create, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::inspector_create_data>();
            auto port = ctx.reg.ports().find(params->port);

            if (port == ctx.reg.ports().end())
            {
                uint64_t port_hash = static_cast<uint64_t>(params->port);
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::inspector_create_failed_port_unknown, ctx.ctrl.get_language()), ctx.tid, port_hash);
                ctx.set_single_response_status(response_status::unknown);
                return;
            }

            const auto& port_name = port->second->get_name();
            auto new_inspector = std::make_shared<data_inspector>();

            if (!new_inspector->open(port_name.get_hash(), ctx.tid))
            {
                auto name_view = port_name.c_str();
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::inspector_create_failed_open, ctx.ctrl.get_language()), ctx.tid, name_view);
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            port->second->inspectors().push_back(new_inspector);
            ctx.thread_inspectors.emplace(params->port, new_inspector);

            auto name_view = port_name.c_str();
            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::inspector_created, ctx.ctrl.get_language()), ctx.tid, name_view);
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::inspector_destroy, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::inspector_destroy_data>();
            auto it = ctx.thread_inspectors.find(params->port);

            if (it == ctx.thread_inspectors.end())
            {
                std::string port_str = std::to_string(static_cast<uint64_t>(params->port));
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::inspector_destroy_failed_not_found, ctx.ctrl.get_language()), ctx.tid, port_str.c_str());
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            auto port = ctx.reg.ports().find(params->port);
            std::string port_str = port != ctx.reg.ports().end() ? std::string(port->second->get_name().c_str()) : std::to_string(static_cast<uint64_t>(params->port));

            if (port != ctx.reg.ports().end())
                port->second->inspectors().remove(it->second);

            ctx.thread_inspectors.erase(it);

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::inspector_destroyed, ctx.ctrl.get_language()), ctx.tid, port_str.c_str());
            ctx.set_single_response_status(response_status::success);
        });

    }

    std::string_view controller_cmd_dispatcher::get_log_event_text(log_event event, language lang)
    {
        static const std::unordered_map<log_event, std::array<std::string_view, languages_count>> translations =
        {
            { 
                log_event::language_changed,
                { "Language changed to: English.", "Sprache geändert zu: Deutsch." }
            },
            {
                log_event::inspector_created,
                { "Thread {:d} successfully created inspector for port \"{}\".", "Thread {:d} hat erfolgreich einen Inspektor für Port \"{}\" erstellt." }
            },
            {
                log_event::inspector_destroyed,
                { "Thread {:d} successfully destroyed inspector for port \"{}\".", "Thread {:d} hat erfolgreich den Inspektor für Port \"{}\" entfernt." }
            },
            {
                log_event::inspector_create_failed_port_unknown,
                { "Thread {:d} failed to create inspector: Unknown port hash {:d}.", "Thread {:d} konnte Inspektor nicht erstellen: Unbekannter Port-Hash {:d}." }
            },
            {
                log_event::inspector_create_failed_open,
                { "Thread {:d} failed to create inspector: Could not open queue for port \"{}\".", "Thread {:d} konnte Inspektor nicht erstellen: Warteschlange für Port \"{}\" konnte nicht geöffnet werden." }
            },
            {
                log_event::inspector_destroy_failed_port_unknown,
                { "Thread {:d} failed to destroy inspector: Unknown port hash {:d}.", "Thread {:d} konnte Inspektor nicht entfernen: Unbekannter Port-Hash {:d}." }
            },
            {
                log_event::inspector_destroy_failed_not_found,
                { "Thread {:d} failed to destroy inspector: Inspector not found for port \"{}\".", "Thread {:d} konnte Inspektor nicht entfernen: Kein Inspektor für Port \"{}\" gefunden." }
            },
            {
                log_event::module_path_added,
                { "Thread {:d} successfully added module path \"{}\".", "Thread {:d} hat Modulpfad \"{}\" erfolgreich hinzugefügt." }
            },
            {
                log_event::module_path_add_failed,
                { "Thread {:d} failed to add module path \"{}\".", "Thread {:d} konnte Modulpfad \"{}\" nicht hinzufügen." }
            },
            {
                log_event::module_path_removed,
                { "Thread {:d} successfully removed module path {:d}.", "Thread {:d} hat Modulpfad {:d} erfolgreich entfernt." }
            },
            {
                log_event::module_path_remove_failed,
                { "Thread {:d} failed to remove module path {:d}.", "Thread {:d} konnte Modulpfad {:d} nicht entfernen." }
            },
            {
                log_event::module_scan_requested,
                { "Thread {:d} requested a module scan.", "Thread {:d} hat eine Modulsuche angefordert." }
            },
            {
                log_event::port_created,
                { "Thread {:d} successfully created port \"{}\" of type \"{}\".", "Thread {:d} hat Port \"{}\" vom Typ \"{}\" erfolgreich erstellt." }
            },
            {
                log_event::port_create_failed,
                { "Thread {:d} failed to create port \"{}\": {}", "Thread {:d} konnte Port \"{}\" nicht erstellen: {}" }
            },
            {
                log_event::port_destroyed,
                { "Thread {:d} successfully destroyed port \"{}\".", "Thread {:d} hat Port \"{}\" erfolgreich entfernt." }
            },
            {
                log_event::port_destroy_failed,
                { "Thread {:d} failed to destroy port {:d}: {}", "Thread {:d} konnte Port {:d} nicht entfernen: {}" }
            },
            {
                log_event::port_renamed,
                { "Thread {:d} successfully renamed port \"{}\" to \"{}\".", "Thread {:d} hat Port \"{}\" erfolgreich zu \"{}\" umbenannt." }
            },
            {
                log_event::port_rename_failed,
                { "Thread {:d} failed to rename port {:d}: {}", "Thread {:d} konnte Port {:d} nicht umbenennen: {}" }
            },
            {
                log_event::port_parameter_updated,
                { "Thread {:d} successfully updated parameter on port \"{}\".", "Thread {:d} hat erfolgreich einen Parameter an Port \"{}\" aktualisiert." }
            },
            {
                log_event::port_parameter_update_failed,
                { "Thread {:d} failed to update parameter on port {:d}.", "Thread {:d} konnte Parameter an Port {:d} nicht aktualisieren." }
            },
            {
                log_event::connection_input_data_format_changed,
                { "Thread {:d} successfully changed input data format of connection \"{}\".", "Thread {:d} hat das Eingangsdatenformat von Verbindung \"{}\" erfolgreich geändert." }
            },
            {
                log_event::connection_input_data_format_change_failed,
                { "Thread {:d} failed to change input data format of connection {:d}.", "Thread {:d} konnte das Eingangsdatenformat von Verbindung {:d} nicht ändern." }
            },
            {
                log_event::connection_output_data_format_changed,
                { "Thread {:d} successfully changed output data format of connection \"{}\".", "Thread {:d} hat das Ausgangsdatenformat von Verbindung \"{}\" erfolgreich geändert." }
            },
            {
                log_event::connection_output_data_format_change_failed,
                { "Thread {:d} failed to change output data format of connection {:d}.", "Thread {:d} konnte das Ausgangsdatenformat von Verbindung {:d} nicht ändern." }
            },
            {
                log_event::connection_created,
                { "Thread {:d} successfully created connection \"{}\".", "Thread {:d} hat Verbindung \"{}\" erfolgreich erstellt." }
            },
            {
                log_event::connection_create_failed,
                { "Thread {:d} failed to create connection \"{}\": {}", "Thread {:d} konnte Verbindung \"{}\" nicht erstellen: {}" }
            },
            {
                log_event::connection_destroyed,
                { "Thread {:d} successfully destroyed connection \"{}\".", "Thread {:d} hat Verbindung \"{}\" erfolgreich entfernt." }
            },
            {
                log_event::connection_destroy_failed,
                { "Thread {:d} failed to destroy connection {:d}: {}", "Thread {:d} konnte Verbindung {:d} nicht entfernen: {}" }
            },
            {
                log_event::port_started,
                { "Thread {:d} successfully started port \"{}\".", "Thread {:d} hat Port \"{}\" erfolgreich gestartet." }
            },
            {
                log_event::port_start_failed,
                { "Thread {:d} failed to start port \"{}\".", "Thread {:d} konnte Port \"{}\" nicht starten." }
            },
            {
                log_event::port_stopped,
                { "Thread {:d} successfully stopped port \"{}\".", "Thread {:d} hat Port \"{}\" erfolgreich gestoppt." }
            },
            {
                log_event::port_stop_failed,
                { "Thread {:d} failed to stop port \"{}\".", "Thread {:d} konnte Port \"{}\" nicht stoppen." }
            },
            {
                log_event::connection_started,
                { "Thread {:d} successfully started connection \"{}\".", "Thread {:d} hat Verbindung \"{}\" erfolgreich gestartet." }
            },
            {
                log_event::connection_start_failed,
                { "Thread {:d} failed to start connection {:d}.", "Thread {:d} konnte Verbindung {:d} nicht starten." }
            },
            {
                log_event::connection_stopped,
                { "Thread {:d} successfully stopped connection \"{}\".", "Thread {:d} hat Verbindung \"{}\" erfolgreich gestoppt." }
            },
            {
                log_event::connection_stop_failed,
                { "Thread {:d} failed to stop connection {:d}.", "Thread {:d} konnte Verbindung {:d} nicht stoppen." }
            },
            {
                log_event::connection_renamed,
                { "Thread {:d} successfully renamed connection \"{}\" to \"{}\".", "Thread {:d} hat Verbindung \"{}\" erfolgreich zu \"{}\" umbenannt." }
            },
            {
                log_event::connection_rename_failed,
                { "Thread {:d} failed to rename connection {:d}: {}", "Thread {:d} konnte Verbindung {:d} nicht umbenennen: {}" }
            },
            {
                log_event::connection_port_added,
                { "Thread {:d} successfully added port \"{}\" to connection \"{}\".", "Thread {:d} hat Port \"{}\" erfolgreich zur Verbindung \"{}\" hinzugefügt." }
            },
            {
                log_event::connection_port_add_failed,
                { "Thread {:d} failed to add port {:d} to connection {:d}: {}", "Thread {:d} konnte Port {:d} nicht zur Verbindung {:d} hinzufügen: {}" }
            },
            {
                log_event::connection_port_removed,
                { "Thread {:d} successfully removed port \"{}\" from connection \"{}\".", "Thread {:d} hat Port \"{}\" erfolgreich von Verbindung \"{}\" entfernt." }
            },
            {
                log_event::connection_port_remove_failed,
                { "Thread {:d} failed to remove port {:d} from connection {:d}: {}", "Thread {:d} konnte Port {:d} nicht von Verbindung {:d} entfernen: {}" }
            },
            {
                log_event::connection_sorting_index_changed,
                { "Thread {:d} successfully changed sorting index of connection \"{}\" to {:d}.", "Thread {:d} hat den Sortierindex von Verbindung \"{}\" erfolgreich auf {:d} geändert." }
            },
            {
                log_event::connection_sorting_index_change_failed,
                { "Thread {:d} failed to change sorting index of connection {:d}.", "Thread {:d} konnte den Sortierindex von Verbindung {:d} nicht ändern." }
            },
            {
                log_event::connection_color_changed,
                { "Thread {:d} successfully changed color of connection \"{}\" to #{:06x}.", "Thread {:d} hat die Farbe von Verbindung \"{}\" erfolgreich auf #{:06x} geändert." }
            },
            {
                log_event::connection_color_change_failed,
                { "Thread {:d} failed to change color of connection {:d}.", "Thread {:d} konnte die Farbe von Verbindung {:d} nicht ändern." }
            },
            {
                log_event::port_data_injected,
                { "Thread {:d} successfully injected data into port \"{}\".", "Thread {:d} hat erfolgreich Daten in Port \"{}\" injiziert." }
            },
            {
                log_event::port_data_inject_failed,
                { "Thread {:d} failed to inject data into port \"{}\".", "Thread {:d} konnte keine Daten in Port \"{}\" injizieren." }
            }
        };

        auto it = translations.find(event);

        if (it != translations.end())
        {
            size_t lang_idx = static_cast<size_t>(lang);
            if (lang_idx < languages_count)
                return it->second[lang_idx];
            return it->second[0];
        }
        
        return language_strings::unknown_type_message("controller_cmd_dispatcher::log_event", event, lang);
    }
}