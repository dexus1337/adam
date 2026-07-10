#include "controller/controller-cmd-dispatcher.hpp"
#include "controller/controller.hpp"
#include "controller/registry.hpp"
#include "controller/registry-configuration-manager.hpp"
#include "resources/language-strings.hpp"
#include "data/port.hpp"
#include "data/format.hpp"
#include "module/module.hpp"
#include "data/inspector.hpp"
#include "commander/messages/event.hpp"
#include "commander/messages/message-structs.hpp"
#include "commander/messages/message-serializer.hpp"
#include "data/connection.hpp"
#include "data/processor.hpp"
#include "data/port-types/port-input.hpp"
#include "data/port-types/port-output.hpp"
#include "configuration/parameters/configuration-parameter.hpp"
#include "configuration/parameters/configuration-parameter-string.hpp"
#include "configuration/parameters/configuration-parameter-list.hpp"
#include "configuration/parameters/configuration-parameter-list-sorted.hpp"
#include "memory/buffer/buffer.hpp"
#include "memory/buffer/buffer-manager.hpp"

#include <array>
#include <format>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <thread>

namespace adam
{
    namespace 
    {
        template<typename msg_type>
        void serialize_user_parameters(const configuration_parameter_list* user_param, detail::message_serializer<msg_type>& serializer) 
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
        auto* prev_ctx = ctx.ctrl.get_command_ctx();
        ctx.ctrl.set_command_ctx(&ctx);

        if (!cmds || count == 0)
        {
            ctx.set_single_response_status(response_status::invalid);
            ctx.ctrl.set_command_ctx(prev_ctx);
            return;
        }

        auto it = m_handlers.find(cmds->get_type());
        if (it != m_handlers.end())
        {
            it->second(cmds, count, ctx);
            ctx.ctrl.set_command_ctx(prev_ctx);
            return;
        }
        
        ctx.set_single_response_status(response_status::unknown);
        ctx.ctrl.set_command_ctx(prev_ctx);
    }

    void controller_cmd_dispatcher::register_default_handlers()
    {
        register_handler(command_type::acquire_initial_data, [](const command*, size_t, command_context& ctx)
        {
            ctx.set_single_response_status(response_status::success);
            auto* data = ctx.responses.front().data_as<messages::initial_data_header>();

            data->lang_info.lang                  = ctx.ctrl.get_language();
            data->lang_info.supported_languages   |= 1 << language_english;
            data->lang_info.supported_languages   |= 1 << language_german;

            std::strncpy(data->cfg_info.name, ctx.reg.configs().get_config_name().c_str(), sizeof(data->cfg_info.name) - 1);
            data->cfg_info.name[sizeof(data->cfg_info.name) - 1] = '\0';
            std::strncpy(data->cfg_info.description, ctx.reg.configs().get_config_description().c_str(), sizeof(data->cfg_info.description) - 1);
            data->cfg_info.description[sizeof(data->cfg_info.description) - 1] = '\0';

            data->mod_info.available_modules       = static_cast<uint32_t>(ctx.reg.modules().get_available_modules().size());
            data->mod_info.unavailable_modules     = static_cast<uint32_t>(ctx.reg.modules().get_unavailable_modules().size());
            data->mod_info.loaded_modules          = static_cast<uint32_t>(ctx.reg.modules().get_loaded_modules().size());

            auto configs_found = ctx.reg.configs().scan_for_configs();
            auto* config_paths_list = ctx.reg.configs().get_config_paths();
            uint32_t config_path_count = 0;
            if (config_paths_list)
            {
                config_path_count = static_cast<uint32_t>(config_paths_list->get_children().size());
            }
            data->cfg_info.paths_count = config_path_count;
            data->cfg_info.available_configs = static_cast<uint32_t>(configs_found.size());

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
                data->conn_info.processors  = static_cast<uint32_t>(ctx.reg.processors().size() + ctx.reg.get_unavailable_processors().size());
                data->conn_info.connections = static_cast<uint32_t>(ctx.reg.connections().size() + ctx.reg.get_unavailable_connections().size());

                for (const auto& [hash, prt] : ctx.reg.ports())
                {
                    ctx.responses[resp_idx-1].set_extended(true);
                    auto* port_info = ctx.responses[resp_idx].data_as<port::basic_info>();
                    
                    std::strncpy(port_info->name, prt->get_name().c_str(), sizeof(port_info->name) - 1);
                    port_info->name[sizeof(port_info->name) - 1] = '\0';
                    
                    port_info->type = prt->get_type_name().get_hash();
                    
                    port_info->type_module = dynamic_cast<configuration_parameter_string*>(prt->get_parameters().get("type_origin_module"_ct))->get_value().get_hash();
                   
                    port_info->state_buffer_handle  = prt->get_state_buffer()->get_handle();
                    port_info->dir                  = prt->get_direction();
                    port_info->is_unavailable       = false;
                    port_info->started              = prt->is_started();

                    auto* user_param = dynamic_cast<configuration_parameter_list*>(prt->get_parameters().get("user_parameters"_ct));

                    port_info->user_parameters = static_cast<uint16_t>(user_param->get_children().size());
                    
                    size_t unused_off     = sizeof(port::basic_info);
                    size_t unused_size    = response::get_max_data_length() - unused_off;

                    detail::message_serializer<response> serializer(ctx.responses, resp_idx, unused_off, unused_size);
                    serialize_user_parameters(user_param, serializer);

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
                    port_info->started = false;
                    port_info->dir = port::direction_inout;

                    resp_idx++;

                    if (resp_idx >= ctx.responses.size())
                        ctx.responses.emplace_back();
                }
            }

            // Processors
            {
                for (const auto& [hash, prc] : ctx.reg.processors())
                {
                    ctx.responses[resp_idx-1].set_extended(true);
                    auto* proc_info = ctx.responses[resp_idx].data_as<processor::basic_info>();
                    
                    const auto& type_module = prc->get_parameter<configuration_parameter_string>("type_origin_module"_ct)->get_value();

                    proc_info->setup
                    (
                        prc->get_name(), 
                        prc->get_type_name().get_hash(), 
                        type_module.get_hash(), 
                        false,
                        prc->get_state_buffer()->get_handle()
                    );
                    
                    auto* in_fmt = prc->get_input_format();
                    auto* in_mod = in_fmt->get_origin_module();

                    auto* out_fmt = prc->get_output_format();
                    auto* out_mod = out_fmt->get_origin_module();

                    proc_info->input_datatype            = in_fmt->get_name().get_hash();
                    proc_info->input_datatype_module     = in_mod->get_name().get_hash();

                    proc_info->output_datatype           = out_fmt->get_name().get_hash();
                    proc_info->output_datatype_module    = out_mod->get_name().get_hash();

                    auto* user_param = dynamic_cast<configuration_parameter_list*>(prc->get_parameters().get("user_parameters"_ct));

                    if (user_param)
                    {
                        proc_info->user_parameters = static_cast<uint16_t>(user_param->get_children().size());
                        
                        size_t unused_off     = sizeof(processor::basic_info);
                        size_t unused_size    = response::get_max_data_length() - unused_off;

                        detail::message_serializer<response> serializer(ctx.responses, resp_idx, unused_off, unused_size);
                        serialize_user_parameters(user_param, serializer);
                    }

                    resp_idx++;

                    if (resp_idx >= ctx.responses.size())
                        ctx.responses.emplace_back();
                }

                for (const auto& [hash, upi] : ctx.reg.get_unavailable_processors())
                {
                    ctx.responses[resp_idx-1].set_extended(true);
                    auto* proc_info = ctx.responses[resp_idx].data_as<processor::basic_info>();
                    
                    proc_info->setup(upi->get_name(), upi->type, upi->type_module, true);

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
                
                conn_info->started                 = conn->is_started();
                conn_info->valid_chain             = conn->is_valid_chain();
                conn_info->is_unavailable          = false;

                conn_info->input_format            = conn->get_input_format()->get_name().get_hash();
                conn_info->input_format_module     = dynamic_cast<configuration_parameter_string*>(conn->get_parameters().get("input_format_module"_ct))->get_value().get_hash();

                conn_info->output_format           = conn->get_output_format()->get_name().get_hash();
                conn_info->output_format_module    = dynamic_cast<configuration_parameter_string*>(conn->get_parameters().get("output_format_module"_ct))->get_value().get_hash();

                conn_info->input_count             = 0;
                conn_info->processor_count         = 0;
                conn_info->output_count            = 0;
                
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

                for (size_t i = 0; i < conn->unavailable_processors().size() && conn_info->processor_count < connection::basic_info::default_type_count; ++i)
                    conn_info->processors[conn_info->processor_count++] = conn->unavailable_processors()[i].get_hash();

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
                
                conn_info->started                 = false;
                conn_info->valid_chain             = false;
                conn_info->is_unavailable          = true;

                conn_info->input_format            = dynamic_cast<configuration_parameter_string*>(uconn->get_parameters().get("input_format"_ct))->get_value().get_hash();
                conn_info->input_format_module     = dynamic_cast<configuration_parameter_string*>(uconn->get_parameters().get("input_format_module"_ct))->get_value().get_hash();

                conn_info->output_format           = dynamic_cast<configuration_parameter_string*>(uconn->get_parameters().get("output_format"_ct))->get_value().get_hash();
                conn_info->output_format_module    = dynamic_cast<configuration_parameter_string*>(uconn->get_parameters().get("output_format_module"_ct))->get_value().get_hash();

                conn_info->input_count             = 0;
                conn_info->processor_count         = 0;
                conn_info->output_count            = 0;
                
                if (auto* inputs_list = dynamic_cast<configuration_parameter_list*>(uconn->get_parameters().get("inputs"_ct)))
                {
                    for (size_t i = 0; i < inputs_list->get_children().size() && conn_info->input_count < connection::basic_info::default_type_count; ++i)
                    {
                        if (auto* param = dynamic_cast<configuration_parameter_reference*>(inputs_list->get(string_hashed(std::to_string(i)))))
                            conn_info->inputs[conn_info->input_count++] = param->get_target().get_hash();
                    }
                }
                
                if (auto* procs_list = dynamic_cast<configuration_parameter_list_sorted*>(uconn->get_parameters().get("processors"_ct)))
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

            // Config paths
            if (config_paths_list)
            {
                for (const auto& [name, param] : config_paths_list->get_children())
                {
                    auto* str_param = dynamic_cast<configuration_parameter_string*>(param.get());
                    if (!str_param) continue;

                    uint32_t idx = std::strtoul(name.c_str(), nullptr, 10);

                    ctx.responses[resp_idx-1].set_extended(true);
                    auto* path_info = ctx.responses[resp_idx].data_as<messages::config_path_data>();
                    path_info->setup(str_param->get_value().c_str(), idx);
                    resp_idx++;

                    if (resp_idx >= ctx.responses.size())
                        ctx.responses.emplace_back();
                }
            }

            // Available configs
            for (const auto& cfg : configs_found)
            {
                ctx.responses[resp_idx-1].set_extended(true);
                auto* cfg_info = ctx.responses[resp_idx].data_as<messages::config_info_data>();
                cfg_info->setup(cfg.path_idx, cfg.filename.c_str(), cfg.name.c_str(), cfg.description.c_str(), cfg.created, cfg.modified, cfg.port_count, cfg.processor_count, cfg.connection_count);
                resp_idx++;

                if (resp_idx >= ctx.responses.size())
                    ctx.responses.emplace_back();
            }
        });

        register_handler(command_type::set_language, [](const command* cmds, size_t, command_context& ctx) 
        {
            ctx.ctrl.set_language(*cmds->get_data_as<language>());
            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::language_changed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid);

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
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::module_path_add_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, path.c_str());
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            event evt(event_type::module_path_added);
            auto* evt_data = evt.data_as<messages::module_path_data>();
            evt_data->setup(params->path, idx);
            ctx.ctrl.broadcast_event(evt);

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::module_path_added, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, path.c_str());
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::module_path_remove, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::module_path_remove_data>();

            if (!ctx.reg.remove_module_path(params->idx))
            {
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::module_path_remove_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, params->idx);
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            event evt(event_type::module_path_removed);
            auto* evt_data = evt.data_as<messages::module_path_remove_data>();
            evt_data->idx = params->idx;
            ctx.ctrl.broadcast_event(evt);

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::module_path_removed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, params->idx);
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::module_scan, [](const command*, size_t, command_context& ctx) 
        {
            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::module_scan_requested, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid);
            
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

        register_handler(command_type::config_path_add, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::config_path_data>();
            string_hashed path(params->path);
            uint32_t idx = params->idx;

            if (!ctx.reg.configs().add_config_path(path, &idx))
            {
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::config_path_add_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, path.c_str());
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            event evt(event_type::config_path_added);
            auto* evt_data = evt.data_as<messages::config_path_data>();
            evt_data->setup(path.c_str(), idx);
            ctx.ctrl.broadcast_event(evt);

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::config_path_added, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, path.c_str());
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::config_path_remove, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::config_path_remove_data>();

            if (!ctx.reg.configs().remove_config_path(params->idx))
            {
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::config_path_remove_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, params->idx);
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            event evt(event_type::config_path_removed);
            auto* evt_data = evt.data_as<messages::config_path_remove_data>();
            evt_data->idx = params->idx;
            ctx.ctrl.broadcast_event(evt);

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::config_path_removed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, params->idx);
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::config_scan, [](const command*, size_t, command_context& ctx) 
        {
            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::config_scan_requested, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid);
            
            auto configs = ctx.reg.configs().scan_for_configs();
            for (const auto& cfg : configs)
            {
                event evt(event_type::config_available);
                auto* data = evt.data_as<messages::config_info_data>();
                data->setup(cfg.path_idx, cfg.filename.c_str(), cfg.name.c_str(), cfg.description.c_str(), cfg.created, cfg.modified, cfg.port_count, cfg.processor_count, cfg.connection_count);
                ctx.ctrl.broadcast_event(evt);
            }
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::config_export, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::config_export_data>();

            if (ctx.reg.configs().save_config(params->path_idx, params->filename, params->name, params->description))
            {
                ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::config_exported, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, params->filename);
                ctx.set_single_response_status(response_status::success);
                
                event evt(event_type::config_available);
                auto* data = evt.data_as<messages::config_info_data>();
                uint32_t port_count = static_cast<uint32_t>(ctx.reg.ports().size() + ctx.reg.unavailable_ports().size());
                uint32_t processor_count = static_cast<uint32_t>(ctx.reg.processors().size() + ctx.reg.unavailable_processors().size());
                uint32_t connection_count = static_cast<uint32_t>(ctx.reg.connections().size() + ctx.reg.unavailable_connections().size());
                data->setup(params->path_idx, params->filename, params->name, params->description, ctx.reg.configs().get_config_created(), ctx.reg.configs().get_config_modified(), port_count, processor_count, connection_count);
                ctx.ctrl.broadcast_event(evt);
            }
            else
            {
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::config_export_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, params->filename);
                ctx.set_single_response_status(response_status::failed);
            }
        });

        register_handler(command_type::config_save, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::config_save_data>();

            if (ctx.reg.configs().save_config(0xFFFFFFFF, "adam-config.adamcfg", params->name, params->description))
            {
                ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::config_exported, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, "adam-config.adamcfg");
                ctx.set_single_response_status(response_status::success);
            }
            else
            {
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::config_export_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, "adam-config.adamcfg");
                ctx.set_single_response_status(response_status::failed);
            }
        });

        register_handler(command_type::config_delete, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::config_delete_data>();

            // Do not allow deleting the default adam config
            if (std::string(params->filename) == "adam-config.adamcfg")
            {
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            if (ctx.reg.configs().delete_config(params->path_idx, params->filename))
            {
                ctx.set_single_response_status(response_status::success);
            }
            else
            {
                ctx.set_single_response_status(response_status::failed);
            }
        });

        register_handler(command_type::config_import, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::config_import_data>();

            ctx.reg.stop_items();
            if (ctx.reg.configs().load_config(params->path_idx, params->filename))
            {
                // Detach resume_active_items into a background thread so we don't block the
                // command processing thread. Blocking here prevents destroy() from joining
                // this thread, which means m_registry.save("adam-config.bin") is never reached.
                std::thread([&reg = ctx.reg]() { reg.resume_active_items(); }).detach();
                ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::config_imported, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, params->filename);
                ctx.set_single_response_status(response_status::success);
            }
            else
            {
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::config_import_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, params->filename);
                ctx.set_single_response_status(response_status::failed);
            }
        });

        register_handler(command_type::connection_create, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<connection::basic_info>();
            string_hashed name(params->name);

            connection* new_conn = nullptr;
            registry::status res = ctx.reg.create_connection(name, &new_conn);

            if (res != registry::status_success || !new_conn)
            {
                auto name_view = name.c_str();
                auto status_text = registry::get_status_text(res, ctx.ctrl.get_language());
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_create_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, name_view, status_text);
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            uint64_t current_time = static_cast<uint64_t>(std::time(nullptr));

            dynamic_cast<configuration_parameter_integer*>(new_conn->get_parameters().get("date_created"_ct))->set_value(current_time);
            dynamic_cast<configuration_parameter_integer*>(new_conn->get_parameters().get("date_edited"_ct))->set_value(current_time);

            event evt(event_type::connection_created);
            auto* evt_data = evt.data_as<connection::basic_info>();
            *evt_data = *params;
            evt_data->created                   = current_time;
            evt_data->edited                    = current_time;
            evt_data->input_format              = new_conn->get_input_format()->get_name().get_hash();
            evt_data->output_format             = new_conn->get_output_format()->get_name().get_hash();
            evt_data->sorting_index             = static_cast<uint32_t>(dynamic_cast<configuration_parameter_integer*>(new_conn->get_parameters().get("sorting_index"_ct))->get_value());
            evt_data->input_format_module       = dynamic_cast<configuration_parameter_string*>(new_conn->get_parameters().get("input_format_module"_ct))->get_value().get_hash();
            evt_data->output_format_module      = dynamic_cast<configuration_parameter_string*>(new_conn->get_parameters().get("output_format_module"_ct))->get_value().get_hash();

            ctx.ctrl.broadcast_event(evt);

            auto name_view = name.c_str();
            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_created, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, name_view);
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::connection_destroy, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::connection_destroy_data>();

            std::string conn_str;
            auto it = ctx.reg.connections().find(params->connection);
            if (it != ctx.reg.connections().end())
                conn_str = it->second->get_name().c_str();
            else
            {
                auto unavail_it = ctx.reg.unavailable_connections().find(params->connection);
                if (unavail_it != ctx.reg.unavailable_connections().end())
                    conn_str = unavail_it->second->get_name().c_str();
            }

            registry::status res = ctx.reg.destroy_connection(params->connection);

            if (res != registry::status_success)
            {
                uint64_t conn_hash = static_cast<uint64_t>(params->connection);
                auto status_text = registry::get_status_text(res, ctx.ctrl.get_language());
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_destroy_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, conn_hash, status_text);
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            event evt(event_type::connection_destroyed);
            evt.data_as<messages::connection_destroy_data>()->connection = params->connection;
            ctx.ctrl.broadcast_event(evt);

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_destroyed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, conn_str);
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::connection_start, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::connection_action_data>();
            auto it = ctx.reg.connections().find(params->connection);

            if (it == ctx.reg.connections().end() || !it->second->start())
            {
                uint64_t conn_hash = static_cast<uint64_t>(params->connection);
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_start_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, conn_hash);
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            event evt(event_type::connection_started);
            evt.data_as<messages::connection_action_data>()->connection = params->connection;
            ctx.ctrl.broadcast_event(evt);

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_started, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, it->second->get_name().c_str());
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::connection_stop, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::connection_action_data>();
            auto it = ctx.reg.connections().find(params->connection);

            if (it == ctx.reg.connections().end() || !it->second->stop())
            {
                uint64_t conn_hash = static_cast<uint64_t>(params->connection);
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_stop_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, conn_hash);
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            event evt(event_type::connection_stopped);
            evt.data_as<messages::connection_action_data>()->connection = params->connection;
            ctx.ctrl.broadcast_event(evt);

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_stopped, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, it->second->get_name().c_str());
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
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_rename_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, conn_hash, status_text);
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

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_renamed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, old_conn_str.c_str(), new_name.c_str());
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
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_port_add_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, port_hash, conn_hash, status_text);
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

            auto log_evt = params->is_input ? controller_cmd_dispatcher::log_event::connection_input_port_added : controller_cmd_dispatcher::log_event::connection_output_port_added;
            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(log_evt, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, port_str.c_str(), conn_str.c_str());
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
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_port_remove_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, port_hash, conn_hash, status_text);
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

            auto log_evt = params->is_input ? controller_cmd_dispatcher::log_event::connection_input_port_removed : controller_cmd_dispatcher::log_event::connection_output_port_removed;
            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(log_evt, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, port_str.c_str(), conn_str.c_str());
            
            if (it_port != ctx.reg.ports().end())
            {
                it_port->second->get_in_connections().iterate([](const auto&){}); // Force active array update on the double-buffer to guarantee accurate size tracking
                it_port->second->get_out_connections().iterate([](const auto&){}); // Force active array update on the double-buffer to guarantee accurate size tracking
                if (it_port->second->is_started())
                {
                    bool any_other_started = false;
                    for (const auto& [hash, conn] : ctx.reg.connections())
                    {
                        if (conn->is_started())
                        {
                            if (conn->ports_input().contains(it_port->second.get()) || conn->ports_output().contains(it_port->second.get()))
                            {
                                any_other_started = true;
                                break;
                            }
                        }
                    }

                    if (!any_other_started)
                    {
                        if (it_port->second->stop())
                        {
                            event evt_stop(event_type::port_stopped);
                            evt_stop.data_as<port::status_event_info>()->port_hash = params->port;
                            ctx.ctrl.broadcast_event(evt_stop);
                            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_stopped, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, port_str.c_str());
                        }
                    }
                }
            }

            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::connection_processor_add, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::connection_processor_add_data>();

            std::string conn_str;
            auto it_conn = ctx.reg.connections().find(params->connection);
            if (it_conn != ctx.reg.connections().end())
                conn_str = it_conn->second->get_name().c_str();

            std::string processor_str;
            auto it_proc = ctx.reg.processors().find(params->processor);
            if (it_proc != ctx.reg.processors().end())
                processor_str = it_proc->second->get_name().c_str();

            registry::status res = ctx.reg.connection_add_processor(params->connection, params->processor);

            if (res != registry::status_success)
            {
                uint64_t conn_hash = static_cast<uint64_t>(params->connection);
                uint64_t processor_hash = static_cast<uint64_t>(params->processor);
                auto status_text = registry::get_status_text(res, ctx.ctrl.get_language());
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_processor_add_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, processor_hash, conn_hash, status_text);
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            uint64_t current_time = static_cast<uint64_t>(std::time(nullptr));

            event evt(event_type::connection_processor_added);
            auto* evt_data = evt.data_as<messages::connection_processor_add_data>();
            *evt_data = *params;
            evt_data->edited = current_time;
            if (it_conn != ctx.reg.connections().end())
                evt_data->valid_chain = it_conn->second->is_valid_chain();
            else
                evt_data->valid_chain = false;
            ctx.ctrl.broadcast_event(evt);

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_processor_added, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, processor_str.c_str(), conn_str.c_str());

            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::connection_processor_remove, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::connection_processor_add_data>();

            std::string conn_str;
            auto it_conn = ctx.reg.connections().find(params->connection);
            if (it_conn != ctx.reg.connections().end())
                conn_str = it_conn->second->get_name().c_str();

            std::string processor_str;
            auto it_proc = ctx.reg.processors().find(params->processor);
            if (it_proc != ctx.reg.processors().end())
                processor_str = it_proc->second->get_name().c_str();

            registry::status res = ctx.reg.connection_remove_processor(params->connection, params->processor);

            if (res != registry::status_success)
            {
                uint64_t conn_hash = static_cast<uint64_t>(params->connection);
                uint64_t processor_hash = static_cast<uint64_t>(params->processor);
                auto status_text = registry::get_status_text(res, ctx.ctrl.get_language());
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_processor_remove_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, processor_hash, conn_hash, status_text);
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            uint64_t current_time = static_cast<uint64_t>(std::time(nullptr));

            event evt(event_type::connection_processor_removed);
            auto* evt_data = evt.data_as<messages::connection_processor_add_data>();
            *evt_data = *params;
            evt_data->edited = current_time;
            if (it_conn != ctx.reg.connections().end())
                evt_data->valid_chain = it_conn->second->is_valid_chain();
            else
                evt_data->valid_chain = false;
            ctx.ctrl.broadcast_event(evt);

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_processor_removed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, processor_str.c_str(), conn_str.c_str());

            if (it_proc != ctx.reg.processors().end())
            {
                it_proc->second->connections().iterate([](const auto&){}); // Force active array update on the double-buffer to guarantee accurate size tracking
                if (it_proc->second->connections().empty())
                {
                    ctx.reg.destroy_processor(params->processor);
                    event evt_del(event_type::processor_destroyed);
                    evt_del.data_as<messages::processor_action_data>()->processor = params->processor;
                    ctx.ctrl.broadcast_event(evt_del);
                    ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::processor_destroyed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, processor_str.c_str());
                }
            }

            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::connection_processor_reorder, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::connection_processor_reorder_data>();

            std::string conn_str;
            auto it_conn = ctx.reg.connections().find(params->connection);
            if (it_conn != ctx.reg.connections().end())
                conn_str = it_conn->second->get_name().c_str();

            std::string processor_str;
            auto it_proc = ctx.reg.processors().find(params->processor);
            if (it_proc != ctx.reg.processors().end())
                processor_str = it_proc->second->get_name().c_str();

            registry::status res = ctx.reg.connection_reorder_processor(params->connection, params->processor, params->new_index);

            if (res != registry::status_success)
            {
                uint64_t conn_hash = static_cast<uint64_t>(params->connection);
                uint64_t processor_hash = static_cast<uint64_t>(params->processor);
                auto status_text = registry::get_status_text(res, ctx.ctrl.get_language());
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_processor_reorder_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, processor_hash, conn_hash, status_text);
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            uint64_t current_time = static_cast<uint64_t>(std::time(nullptr));

            event evt(event_type::connection_processor_reordered);
            auto* evt_data = evt.data_as<messages::connection_processor_reorder_data>();
            *evt_data = *params;
            evt_data->edited = current_time;
            if (it_conn != ctx.reg.connections().end())
                evt_data->valid_chain = it_conn->second->is_valid_chain();
            else
                evt_data->valid_chain = false;
            ctx.ctrl.broadcast_event(evt);

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_processor_reordered, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, processor_str.c_str(), conn_str.c_str(), params->new_index);

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
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_sorting_index_change_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, conn_hash);
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

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_sorting_index_changed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, conn_str.c_str(), params->value);
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
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_color_change_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, conn_hash);
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

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_color_changed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, conn_str.c_str(), params->value);
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::connection_set_input_data_format, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::connection_data_format_data>();

            auto conn_it = ctx.reg.connections().find(params->connection);
            connection* conn = nullptr;
            connection::unavailable_info* unavail_conn = nullptr;

            if (conn_it != ctx.reg.connections().end())
            {
                conn = conn_it->second.get();
            }
            else
            {
                auto unavail_it = ctx.reg.unavailable_connections().find(params->connection);
                if (unavail_it != ctx.reg.unavailable_connections().end())
                {
                    unavail_conn = unavail_it->second.get();
                }
            }

            if (!conn && !unavail_conn)
            {
                uint64_t conn_hash = static_cast<uint64_t>(params->connection);
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_input_data_format_change_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, conn_hash);
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            const data_format* in_fmt = ctx.reg.get_data_format(params->format, params->format_module);
            string_hashed resolved_in_module = in_fmt->get_origin_module() ? in_fmt->get_origin_module()->get_name() : "essential"_ct;

            auto set_str_param = [&](const string_hashed_ct& key, const string_hashed& value)
            {
                auto* params_list = conn ? &conn->get_parameters() : &unavail_conn->get_parameters();
                if (auto* p = dynamic_cast<configuration_parameter_string*>(params_list->get(key)))
                    p->set_value(value);
            };

            set_str_param("input_format"_ct, in_fmt->get_name());
            set_str_param("input_format_module"_ct, resolved_in_module);

            bool valid_chain = false;

            if (conn)
            {
                bool was_started = conn->is_started();
                conn->set_input_format(in_fmt);
                
                if (was_started && !conn->is_valid_chain())
                {
                    conn->stop();
                    event evt_stop(event_type::connection_stopped);
                    evt_stop.data_as<messages::connection_action_data>()->connection = params->connection;
                    ctx.ctrl.broadcast_event(evt_stop);
                    ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_stopped, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, conn->get_name().c_str());
                }
                valid_chain = conn->is_valid_chain();
            }

            event evt(event_type::connection_input_data_format_changed);
            auto* evt_data = evt.data_as<messages::connection_data_format_data>();
            evt_data->connection            = params->connection;
            evt_data->format                = in_fmt->get_name().get_hash();
            evt_data->format_module         = resolved_in_module.get_hash();
            evt_data->valid_chain           = valid_chain;
            ctx.ctrl.broadcast_event(evt);

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_input_data_format_changed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, conn ? conn->get_name().c_str() : unavail_conn->get_name().c_str());
            ctx.set_single_response_status(response_status::success);

            if (unavail_conn)
            {
                ctx.reg.retry_unavailable_connections(0);
            }
        });

        register_handler(command_type::connection_set_output_data_format, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::connection_data_format_data>();

            auto conn_it = ctx.reg.connections().find(params->connection);
            connection* conn = nullptr;
            connection::unavailable_info* unavail_conn = nullptr;

            if (conn_it != ctx.reg.connections().end())
            {
                conn = conn_it->second.get();
            }
            else
            {
                auto unavail_it = ctx.reg.unavailable_connections().find(params->connection);
                if (unavail_it != ctx.reg.unavailable_connections().end())
                {
                    unavail_conn = unavail_it->second.get();
                }
            }

            if (!conn && !unavail_conn)
            {
                uint64_t conn_hash = static_cast<uint64_t>(params->connection);
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_output_data_format_change_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, conn_hash);
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            const data_format* out_fmt = ctx.reg.get_data_format(params->format, params->format_module);
            string_hashed resolved_out_module = out_fmt->get_origin_module() ? out_fmt->get_origin_module()->get_name() : "essential"_ct;

            auto set_str_param = [&](const string_hashed_ct& key, const string_hashed& value)
            {
                auto* params_list = conn ? &conn->get_parameters() : &unavail_conn->get_parameters();
                if (auto* p = dynamic_cast<configuration_parameter_string*>(params_list->get(key)))
                    p->set_value(value);
            };

            set_str_param("output_format"_ct, out_fmt->get_name());
            set_str_param("output_format_module"_ct, resolved_out_module);

            bool valid_chain = false;

            if (conn)
            {
                bool was_started = conn->is_started();
                conn->set_output_format(out_fmt);
                
                if (was_started && !conn->is_valid_chain())
                {
                    conn->stop();
                    event evt_stop(event_type::connection_stopped);
                    evt_stop.data_as<messages::connection_action_data>()->connection = params->connection;
                    ctx.ctrl.broadcast_event(evt_stop);
                    ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_stopped, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, conn->get_name().c_str());
                }
                valid_chain = conn->is_valid_chain();
            }

            event evt(event_type::connection_output_data_format_changed);
            auto* evt_data = evt.data_as<messages::connection_data_format_data>();
            evt_data->connection            = params->connection;
            evt_data->format                = out_fmt->get_name().get_hash();
            evt_data->format_module         = resolved_out_module.get_hash();
            evt_data->valid_chain           = valid_chain;
            ctx.ctrl.broadcast_event(evt);

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_output_data_format_changed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, conn ? conn->get_name().c_str() : unavail_conn->get_name().c_str());
            ctx.set_single_response_status(response_status::success);

            if (unavail_conn)
            {
                ctx.reg.retry_unavailable_connections(0);
            }
        });

        register_handler(command_type::connection_input_inspector_create, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::connection_action_data>();
            auto conn = ctx.reg.connections().find(params->connection);

            if (conn == ctx.reg.connections().end())
            {
                std::string conn_hash_str = std::to_string(static_cast<uint64_t>(params->connection));
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_input_inspector_create_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, conn_hash_str.c_str());
                ctx.set_single_response_status(response_status::unknown);
                return;
            }

            const auto& conn_name = conn->second->get_name();
            auto new_inspector = std::make_shared<data_inspector>();

            if (!new_inspector->open(conn_name.get_hash() ^ ("input"_ct).get_hash(), ctx.tid))
            {
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_input_inspector_create_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, conn_name.c_str());
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            conn->second->inspectors_input().push_back(new_inspector);
            ctx.thread_connection_input_inspectors.emplace(params->connection, new_inspector);

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_input_inspector_created, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, conn_name.c_str());
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::connection_input_inspector_destroy, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::connection_action_data>();
            auto it = ctx.thread_connection_input_inspectors.find(params->connection);

            if (it == ctx.thread_connection_input_inspectors.end())
            {
                std::string conn_str = std::to_string(static_cast<uint64_t>(params->connection));
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_input_inspector_destroy_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, conn_str.c_str());
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            auto conn = ctx.reg.connections().find(params->connection);
            std::string conn_str = conn != ctx.reg.connections().end() ? std::string(conn->second->get_name().c_str()) : std::to_string(static_cast<uint64_t>(params->connection));

            if (conn != ctx.reg.connections().end())
                conn->second->inspectors_input().remove(it->second);

            ctx.thread_connection_input_inspectors.erase(it);

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_input_inspector_destroyed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, conn_str.c_str());
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::connection_output_inspector_create, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::connection_action_data>();
            auto conn = ctx.reg.connections().find(params->connection);

            if (conn == ctx.reg.connections().end())
            {
                std::string conn_hash_str = std::to_string(static_cast<uint64_t>(params->connection));
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_output_inspector_create_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, conn_hash_str.c_str());
                ctx.set_single_response_status(response_status::unknown);
                return;
            }

            const auto& conn_name = conn->second->get_name();
            auto new_inspector = std::make_shared<data_inspector>();

            if (!new_inspector->open(conn_name.get_hash() ^ ("output"_ct).get_hash(), ctx.tid))
            {
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_output_inspector_create_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, conn_name.c_str());
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            conn->second->inspectors_output().push_back(new_inspector);
            ctx.thread_connection_output_inspectors.emplace(params->connection, new_inspector);

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_output_inspector_created, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, conn_name.c_str());
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::connection_output_inspector_destroy, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::connection_action_data>();
            auto it = ctx.thread_connection_output_inspectors.find(params->connection);

            if (it == ctx.thread_connection_output_inspectors.end())
            {
                std::string conn_str = std::to_string(static_cast<uint64_t>(params->connection));
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_output_inspector_destroy_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, conn_str.c_str());
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            auto conn = ctx.reg.connections().find(params->connection);
            std::string conn_str = conn != ctx.reg.connections().end() ? std::string(conn->second->get_name().c_str()) : std::to_string(static_cast<uint64_t>(params->connection));

            if (conn != ctx.reg.connections().end())
                conn->second->inspectors_output().remove(it->second);

            ctx.thread_connection_output_inspectors.erase(it);

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_output_inspector_destroyed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, conn_str.c_str());
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
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_create_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, name_view, status_text);
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            port* new_port = nullptr;
            registry::status res = ctx.reg.create_port(name, params->type, params->type_module, &new_port);

            if (res != registry::status_success)
            {
                auto name_view = name.c_str();
                auto status_text = registry::get_status_text(res, ctx.ctrl.get_language());
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_create_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, name_view, status_text);
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

                evt_data->dir       = new_port->get_direction();
                evt_data->started   = new_port->is_started();
                
                auto* user_param = dynamic_cast<configuration_parameter_list*>(new_port->get_parameters().get("user_parameters"_ct));
                if (user_param)
                {
                    evt_data->user_parameters = static_cast<uint16_t>(user_param->get_children().size());
                    
                    size_t evt_idx = 0;
                    size_t unused_off = sizeof(port::basic_info);
                    size_t unused_size = event::get_max_data_length() - unused_off;

                    detail::message_serializer<event> serializer(events, evt_idx, unused_off, unused_size);
                    serialize_user_parameters(user_param, serializer);
                }
            }
            else
                *evt_data = *params;
                
            for (const auto& ev : events)
                ctx.ctrl.broadcast_event(ev);

            auto name_view = name.c_str();
            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_created, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, name_view, new_port->get_type_name().c_str());
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
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_destroy_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, port_hash, status_text);
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            event evt(event_type::port_destroyed);
            evt.data_as<messages::port_destroy_data>()->port = params->port;
            ctx.ctrl.broadcast_event(evt);

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_destroyed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, port_str.c_str());
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
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_rename_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, port_hash, status_text);
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

                renamed_it->second->get_in_connections().iterate(update_timestamp);
                renamed_it->second->get_out_connections().iterate(update_timestamp);
            }

            event evt(event_type::port_renamed);
            auto* evt_data = evt.data_as<messages::port_rename_data>();
            evt_data->port = params->port;
            std::strncpy(evt_data->new_name, params->new_name, sizeof(evt_data->new_name) - 1);
            evt_data->new_name[sizeof(evt_data->new_name) - 1] = '\0';
            evt_data->edited = current_time;
            ctx.ctrl.broadcast_event(evt);

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_renamed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, old_port_str.c_str(), new_name.c_str());
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::port_set_parameter, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::port_set_parameter_data>();
            auto it = ctx.reg.ports().find(params->port);

            if (it == ctx.reg.ports().end())
            {
                uint64_t port_hash = static_cast<uint64_t>(params->port);
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_parameter_update_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, port_hash);
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            auto* port = it->second.get();
            auto* user_params = dynamic_cast<configuration_parameter_list*>(port->get_parameters().get("user_parameters"_ct));
            if (!user_params)
            {
                uint64_t port_hash = static_cast<uint64_t>(params->port);
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_parameter_update_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, port_hash);
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            auto* param = user_params->get(params->param_view.name);
            if (!param)
            {
                uint64_t port_hash = static_cast<uint64_t>(params->port);
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_parameter_update_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, port_hash);
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
                
                ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_parameter_updated, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, port->get_name().c_str());
                ctx.set_single_response_status(response_status::success);
            }
            else
            {
                uint64_t port_hash = static_cast<uint64_t>(params->port);
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_parameter_update_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, port_hash);
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
                    it->second->mark_start_failed();

                    ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_start_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, it->second->get_name().c_str());
                }
                else
                {
                    auto hash_str = std::format("{:x}", static_cast<uint64_t>(params->port));
                    ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_start_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, hash_str.c_str());
                }
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            event evt(event_type::port_started);
            evt.data_as<port::status_event_info>()->port_hash = params->port;
            ctx.ctrl.broadcast_event(evt);

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_started, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, it->second->get_name().c_str());
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
                    ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_stop_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, it->second->get_name().c_str());
                }
                else
                {
                    auto hash_str = std::format("{:x}", static_cast<uint64_t>(params->port));
                    ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_stop_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, hash_str.c_str());
                }
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            event evt(event_type::port_stopped);
            evt.data_as<port::status_event_info>()->port_hash = params->port;
            ctx.ctrl.broadcast_event(evt);

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_stopped, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, it->second->get_name().c_str());
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::port_inject_data, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::port_inject_data>();
            auto it = ctx.reg.ports().find(params->port);

            if (it == ctx.reg.ports().end())
            {
                uint64_t port_hash = static_cast<uint64_t>(params->port);
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_data_inject_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, port_hash);
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
            it->second->handle_data(buf, params->direction);
            debug_statement(ctx.ctrl.log(log::trace, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_data_injected, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, it->second->get_name().c_str()));
            ctx.set_single_response_status(response_status::success);

            buf->release();
        });
        
        register_handler(command_type::port_inspector_create, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::port_inspector_create_data>();
            auto port = ctx.reg.ports().find(params->port);

            if (port == ctx.reg.ports().end())
            {
                uint64_t port_hash = static_cast<uint64_t>(params->port);
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::inspector_create_failed_port_unknown, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, port_hash);
                ctx.set_single_response_status(response_status::unknown);
                return;
            }

            const auto& port_name = port->second->get_name();
            auto new_inspector = std::make_shared<data_inspector>();

            if (!new_inspector->open(port_name.get_hash(), ctx.tid))
            {
                auto name_view = port_name.c_str();
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::inspector_create_failed_open, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, name_view);
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            port->second->add_inspector(new_inspector);
            ctx.thread_inspectors.emplace(params->port, new_inspector);

            auto name_view = port_name.c_str();
            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::inspector_created, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, name_view);
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::port_inspector_destroy, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::port_inspector_destroy_data>();
            auto it = ctx.thread_inspectors.find(params->port);

            if (it == ctx.thread_inspectors.end())
            {
                std::string port_str = std::to_string(static_cast<uint64_t>(params->port));
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::inspector_destroy_failed_not_found, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, port_str.c_str());
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            auto port = ctx.reg.ports().find(params->port);
            std::string port_str = port != ctx.reg.ports().end() ? std::string(port->second->get_name().c_str()) : std::to_string(static_cast<uint64_t>(params->port));

            if (port != ctx.reg.ports().end())
                port->second->remove_inspector(it->second);

            ctx.thread_inspectors.erase(it);

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::inspector_destroyed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, port_str.c_str());
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::processor_create, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<processor::basic_info>();
            string_hashed name(params->name);

            processor* new_processor = nullptr;
            registry::status res = ctx.reg.create_processor(name, params->type, params->type_module, &new_processor);

            if (res != registry::status_success)
            {
                auto name_view = name.c_str();
                auto status_text = registry::get_status_text(res, ctx.ctrl.get_language());
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::processor_create_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, name_view, status_text);
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            // Build the event from the resolved processor state, not the raw command params.
            event evt(event_type::processor_created);
            auto* evt_data = evt.data_as<processor::basic_info>();

            const auto& type_module = new_processor->get_parameter<configuration_parameter_string>("type_origin_module"_ct)->get_value();

            evt_data->setup
            (
                new_processor->get_name(), 
                new_processor->get_type_name().get_hash(), 
                type_module.get_hash(), 
                false, 
                new_processor->get_state_buffer()->get_handle()
            );

            auto* in_fmt = new_processor->get_input_format();
            auto* in_mod = in_fmt ? in_fmt->get_origin_module() : nullptr;

            auto* out_fmt = new_processor->get_output_format();
            auto* out_mod = out_fmt ? out_fmt->get_origin_module() : nullptr;

            evt_data->input_datatype            = in_fmt ? in_fmt->get_name().get_hash() : 0ull;
            evt_data->input_datatype_module     = in_mod ? in_mod->get_name().get_hash() : 0ull;

            evt_data->output_datatype           = out_fmt ? out_fmt->get_name().get_hash() : 0ull;
            evt_data->output_datatype_module    = out_mod ? out_mod->get_name().get_hash() : 0ull;

            ctx.ctrl.broadcast_event(evt);
            
            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::processor_created, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, name.c_str(), new_processor->get_type_name().c_str());
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::processor_destroy, [](const command* cmds, size_t, command_context& ctx)
        {
            auto params = cmds->get_data_as<messages::processor_destroy_data>();

            std::string processor_str;
            auto it = ctx.reg.processors().find(params->processor);
            if (it != ctx.reg.processors().end())
                processor_str = it->second->get_name().c_str();

            registry::status res = ctx.reg.destroy_processor(params->processor);
            if (res != registry::status_success)
            {
                uint64_t processor_hash = static_cast<uint64_t>(params->processor);
                auto status_text = registry::get_status_text(res, ctx.ctrl.get_language());
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::processor_destroy_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, processor_hash, status_text);
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            event evt(event_type::processor_destroyed);
            evt.data_as<messages::processor_action_data>()->processor = params->processor;
            ctx.ctrl.broadcast_event(evt);

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::processor_destroyed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, processor_str.c_str());
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(command_type::processor_set_parameter, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::processor_set_parameter_data>();
            auto it = ctx.reg.processors().find(params->processor);

            if (it == ctx.reg.processors().end())
            {
                uint64_t processor_hash = static_cast<uint64_t>(params->processor);
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::processor_parameter_update_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, processor_hash);
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            auto* processor = it->second.get();
            auto* user_params = dynamic_cast<configuration_parameter_list*>(processor->get_parameters().get("user_parameters"_ct));
            if (!user_params)
            {
                uint64_t processor_hash = static_cast<uint64_t>(params->processor);
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::processor_parameter_update_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, processor_hash);
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            auto* param = user_params->get(params->param_view.name);
            if (!param)
            {
                uint64_t processor_hash = static_cast<uint64_t>(params->processor);
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::processor_parameter_update_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, processor_hash);
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
                event evt(event_type::processor_parameter_updated);
                auto* evt_data = evt.data_as<messages::processor_parameter_updated_data>();
                evt_data->processor = params->processor;
                evt_data->param_view = params->param_view;
                std::memcpy(evt_data->data, params->data, sizeof(evt_data->data));
                ctx.ctrl.broadcast_event(evt);
                
                ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::processor_parameter_updated, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, processor->get_name().c_str());
                ctx.set_single_response_status(response_status::success);
            }
            else
            {
                uint64_t processor_hash = static_cast<uint64_t>(params->processor);
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::processor_parameter_update_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, processor_hash);
                ctx.set_single_response_status(response_status::failed);
            }
        });

        register_handler(command_type::processor_rename, [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::processor_rename_data>();
            string_hashed new_name(params->new_name);

            std::string old_processor_str;
            auto it = ctx.reg.processors().find(params->processor);
            if (it != ctx.reg.processors().end())
                old_processor_str = it->second->get_name().c_str();

            registry::status res = ctx.reg.rename_processor(params->processor, new_name);

            if (res != registry::status_success)
            {
                uint64_t processor_hash = static_cast<uint64_t>(params->processor);
                auto status_text = registry::get_status_text(res, ctx.ctrl.get_language());
                ctx.ctrl.log(log::error, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::processor_rename_failed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, processor_hash, status_text);
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            uint64_t current_time = static_cast<uint64_t>(std::time(nullptr));
            auto renamed_it = ctx.reg.processors().find(new_name.get_hash());
            if (renamed_it != ctx.reg.processors().end())
            {
                for (auto& [conn_hash, conn_ptr] : ctx.reg.connections())
                {
                    bool contains_proc = false;
                    conn_ptr->processors().iterate([&](const auto& procs) 
                    {
                        for (auto* proc : procs)
                        {
                            if (proc->get_name().get_hash() == new_name.get_hash())
                            {
                                contains_proc = true;
                                break;
                            }
                        }
                    });

                    if (contains_proc)
                    {
                        if (auto* param = dynamic_cast<configuration_parameter_integer*>(conn_ptr->get_parameters().get("date_edited"_ct)))
                            param->set_value(static_cast<int64_t>(current_time));
                    }
                }
            }

            event evt(event_type::processor_renamed);
            auto* evt_data = evt.data_as<messages::processor_rename_data>();
            evt_data->processor = params->processor;
            std::strncpy(evt_data->new_name, params->new_name, sizeof(evt_data->new_name) - 1);
            evt_data->new_name[sizeof(evt_data->new_name) - 1] = '\0';
            evt_data->edited = current_time;
            ctx.ctrl.broadcast_event(evt);

            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::processor_renamed, ctx.ctrl.get_language()), ctx.ctrl.get_client_name(ctx.tid), ctx.tid, old_processor_str.c_str(), new_name.c_str());
            ctx.set_single_response_status(response_status::success);
        });

    }

    std::string_view controller_cmd_dispatcher::get_log_event_text(log_event event, language lang)
    {
        static const std::unordered_map<log_event, std::array<std::string_view, languages_count>> translations =
        {
            { 
                log_event::language_changed,
                { "{} ({:d}) changed language to: English.", "{} ({:d}) hat die Sprache geändert zu: Deutsch." }
            },
            {
                log_event::inspector_created,
                { "{} ({:d}) successfully created inspector for port \"{}\".", "{} ({:d}) hat erfolgreich einen Inspektor für Port \"{}\" erstellt." }
            },
            {
                log_event::inspector_destroyed,
                { "{} ({:d}) successfully destroyed inspector for port \"{}\".", "{} ({:d}) hat erfolgreich den Inspektor für Port \"{}\" entfernt." }
            },
            {
                log_event::inspector_create_failed_port_unknown,
                { "{} ({:d}) failed to create inspector: Unknown port hash {:x}.", "{} ({:d}) konnte Inspektor nicht erstellen: Unbekannter Port-Hash {:x}." }
            },
            {
                log_event::inspector_create_failed_open,
                { "{} ({:d}) failed to create inspector: Could not open queue for port \"{}\".", "{} ({:d}) konnte Inspektor nicht erstellen: Warteschlange für Port \"{}\" konnte nicht geöffnet werden." }
            },
            {
                log_event::inspector_destroy_failed_port_unknown,
                { "{} ({:d}) failed to destroy inspector: Unknown port hash {:x}.", "{} ({:d}) konnte Inspektor nicht entfernen: Unbekannter Port-Hash {:x}." }
            },
            {
                log_event::inspector_destroy_failed_not_found,
                { "{} ({:d}) failed to destroy inspector: Inspector not found for port \"{}\".", "{} ({:d}) konnte Inspektor nicht entfernen: Kein Inspektor für Port \"{}\" gefunden." }
            },
            {
                log_event::module_path_added,
                { "{} ({:d}) successfully added module path \"{}\".", "{} ({:d}) hat Modulpfad \"{}\" erfolgreich hinzugefügt." }
            },
            {
                log_event::module_path_add_failed,
                { "{} ({:d}) failed to add module path \"{}\".", "{} ({:d}) konnte Modulpfad \"{}\" nicht hinzufügen." }
            },
            {
                log_event::module_path_removed,
                { "{} ({:d}) successfully removed module path {:d}.", "{} ({:d}) hat Modulpfad {:d} erfolgreich entfernt." }
            },
            {
                log_event::module_path_remove_failed,
                { "{} ({:d}) failed to remove module path {:d}.", "{} ({:d}) konnte Modulpfad {:d} nicht entfernen." }
            },
            {
                log_event::module_scan_requested,
                { "{} ({:d}) requested a module scan.", "{} ({:d}) hat eine Modulsuche angefordert." }
            },
            {
                log_event::port_created,
                { "{} ({:d}) successfully created port \"{}\" of type \"{}\".", "{} ({:d}) hat Port \"{}\" vom Typ \"{}\" erfolgreich erstellt." }
            },
            {
                log_event::port_create_failed,
                { "{} ({:d}) failed to create port \"{}\": {}", "{} ({:d}) konnte Port \"{}\" nicht erstellen: {}" }
            },
            {
                log_event::port_destroyed,
                { "{} ({:d}) successfully destroyed port \"{}\".", "{} ({:d}) hat Port \"{}\" erfolgreich entfernt." }
            },
            {
                log_event::port_destroy_failed,
                { "{} ({:d}) failed to destroy port {:d}: {}", "{} ({:d}) konnte Port {:d} nicht entfernen: {}" }
            },
            {
                log_event::port_renamed,
                { "{} ({:d}) successfully renamed port \"{}\" to \"{}\".", "{} ({:d}) hat Port \"{}\" erfolgreich zu \"{}\" umbenannt." }
            },
            {
                log_event::port_rename_failed,
                { "{} ({:d}) failed to rename port {:d}: {}", "{} ({:d}) konnte Port {:d} nicht umbenennen: {}" }
            },
            {
                log_event::port_parameter_updated,
                { "{} ({:d}) successfully updated parameter on port \"{}\".", "{} ({:d}) hat erfolgreich einen Parameter an Port \"{}\" aktualisiert." }
            },
            {
                log_event::port_parameter_update_failed,
                { "{} ({:d}) failed to update parameter on port {:d}.", "{} ({:d}) konnte Parameter an Port {:d} nicht aktualisieren." }
            },
            {
                log_event::processor_parameter_updated,
                { "{} ({:d}) successfully updated parameter on processor \"{}\".", "{} ({:d}) hat erfolgreich einen Parameter an Prozessor \"{}\" aktualisiert." }
            },
            {
                log_event::processor_parameter_update_failed,
                { "{} ({:d}) failed to update parameter on processor {:d}.", "{} ({:d}) konnte Parameter an Prozessor {:d} nicht aktualisieren." }
            },
            {
                log_event::processor_created,
                { "{} ({:d}) successfully created processor \"{}\" of type \"{}\".", "{} ({:d}) hat Prozessor \"{}\" vom Typ \"{}\" erfolgreich erstellt." }
            },
            {
                log_event::processor_create_failed,
                { "{} ({:d}) failed to create processor \"{}\": {}", "{} ({:d}) konnte Prozessor \"{}\" nicht erstellen: {}" }
            },
            {
                log_event::processor_destroyed,
                { "{} ({:d}) successfully destroyed processor \"{}\".", "{} ({:d}) hat Prozessor \"{}\" erfolgreich entfernt." }
            },
            {
                log_event::processor_destroy_failed,
                { "{} ({:d}) failed to destroy processor {:d}: {}", "{} ({:d}) konnte Prozessor {:d} nicht entfernen: {}" }
            },
            {
                log_event::processor_renamed,
                { "{} ({:d}) successfully renamed processor \"{}\" to \"{}\".", "{} ({:d}) hat Prozessor \"{}\" erfolgreich zu \"{}\" umbenannt." }
            },
            {
                log_event::processor_rename_failed,
                { "{} ({:d}) failed to rename processor {:d}: {}", "{} ({:d}) konnte Prozessor {:d} nicht umbenennen: {}" }
            },
            {
                log_event::connection_input_data_format_changed,
                { "{} ({:d}) successfully changed input data format of connection \"{}\".", "{} ({:d}) hat das Eingangsdatenformat von Verbindung \"{}\" erfolgreich geändert." }
            },
            {
                log_event::connection_input_data_format_change_failed,
                { "{} ({:d}) failed to change input data format of connection {:d}.", "{} ({:d}) konnte das Eingangsdatenformat von Verbindung {:d} nicht ändern." }
            },
            {
                log_event::connection_output_data_format_changed,
                { "{} ({:d}) successfully changed output data format of connection \"{}\".", "{} ({:d}) hat das Ausgangsdatenformat von Verbindung \"{}\" erfolgreich geändert." }
            },
            {
                log_event::connection_output_data_format_change_failed,
                { "{} ({:d}) failed to change output data format of connection {:d}.", "{} ({:d}) konnte das Ausgangsdatenformat von Verbindung {:d} nicht ändern." }
            },
            {
                log_event::connection_created,
                { "{} ({:d}) successfully created connection \"{}\".", "{} ({:d}) hat Verbindung \"{}\" erfolgreich erstellt." }
            },
            {
                log_event::connection_create_failed,
                { "{} ({:d}) failed to create connection \"{}\": {}", "{} ({:d}) konnte Verbindung \"{}\" nicht erstellen: {}" }
            },
            {
                log_event::connection_destroyed,
                { "{} ({:d}) successfully destroyed connection \"{}\".", "{} ({:d}) hat Verbindung \"{}\" erfolgreich entfernt." }
            },
            {
                log_event::connection_destroy_failed,
                { "{} ({:d}) failed to destroy connection {:d}: {}", "{} ({:d}) konnte Verbindung {:d} nicht entfernen: {}" }
            },
            {
                log_event::port_started,
                { "{} ({:d}) successfully started port \"{}\".", "{} ({:d}) hat Port \"{}\" erfolgreich gestartet." }
            },
            {
                log_event::port_start_failed,
                { "{} ({:d}) failed to start port \"{}\".", "{} ({:d}) konnte Port \"{}\" nicht starten." }
            },
            {
                log_event::port_stopped,
                { "{} ({:d}) successfully stopped port \"{}\".", "{} ({:d}) hat Port \"{}\" erfolgreich gestoppt." }
            },
            {
                log_event::port_stop_failed,
                { "{} ({:d}) failed to stop port \"{}\".", "{} ({:d}) konnte Port \"{}\" nicht stoppen." }
            },
            {
                log_event::connection_started,
                { "{} ({:d}) successfully started connection \"{}\".", "{} ({:d}) hat Verbindung \"{}\" erfolgreich gestartet." }
            },
            {
                log_event::connection_start_failed,
                { "{} ({:d}) failed to start connection {:d}.", "{} ({:d}) konnte Verbindung {:d} nicht starten." }
            },
            {
                log_event::connection_stopped,
                { "{} ({:d}) successfully stopped connection \"{}\".", "{} ({:d}) hat Verbindung \"{}\" erfolgreich gestoppt." }
            },
            {
                log_event::connection_stop_failed,
                { "{} ({:d}) failed to stop connection {:d}.", "{} ({:d}) konnte Verbindung {:d} nicht stoppen." }
            },
            {
                log_event::connection_renamed,
                { "{} ({:d}) successfully renamed connection \"{}\" to \"{}\".", "{} ({:d}) hat Verbindung \"{}\" erfolgreich zu \"{}\" umbenannt." }
            },
            {
                log_event::connection_rename_failed,
                { "{} ({:d}) failed to rename connection {:d}: {}", "{} ({:d}) konnte Verbindung {:d} nicht umbenennen: {}" }
            },
            {
                log_event::connection_input_port_added,
                { "{} ({:d}) successfully added port \"{}\" as input to connection \"{}\".", "{} ({:d}) hat Port \"{}\" erfolgreich als Eingang zur Verbindung \"{}\" hinzugefügt." }
            },
            {
                log_event::connection_output_port_added,
                { "{} ({:d}) successfully added port \"{}\" as output to connection \"{}\".", "{} ({:d}) hat Port \"{}\" erfolgreich als Ausgang zur Verbindung \"{}\" hinzugefügt." }
            },
            {
                log_event::connection_port_add_failed,
                { "{} ({:d}) failed to add port {:d} to connection {:d}: {}", "{} ({:d}) konnte Port {:d} nicht zur Verbindung {:d} hinzufügen: {}" }
            },
            {
                log_event::connection_input_port_removed,
                { "{} ({:d}) successfully removed port \"{}\" as input from connection \"{}\".", "{} ({:d}) hat Port \"{}\" erfolgreich als Eingang von Verbindung \"{}\" entfernt." }
            },
            {
                log_event::connection_output_port_removed,
                { "{} ({:d}) successfully removed port \"{}\" as output from connection \"{}\".", "{} ({:d}) hat Port \"{}\" erfolgreich als Ausgang von Verbindung \"{}\" entfernt." }
            },
            {
                log_event::connection_port_remove_failed,
                { "{} ({:d}) failed to remove port {:d} from connection {:d}: {}", "{} ({:d}) konnte Port {:d} nicht von Verbindung {:d} entfernen: {}" }
            },
            {
                log_event::connection_processor_added,
                { "{} ({:d}) successfully added processor \"{}\" to connection \"{}\".", "{} ({:d}) hat Prozessor \"{}\" erfolgreich zur Verbindung \"{}\" hinzugefügt." }
            },
            {
                log_event::connection_processor_add_failed,
                { "{} ({:d}) failed to add processor {:d} to connection {:d}: {}", "{} ({:d}) konnte Prozessor {:d} nicht zur Verbindung {:d} hinzufügen: {}" }
            },
            {
                log_event::connection_processor_removed,
                { "{} ({:d}) successfully removed processor \"{}\" from connection \"{}\".", "{} ({:d}) hat Prozessor \"{}\" erfolgreich von Verbindung \"{}\" entfernt." }
            },
            {
                log_event::connection_processor_remove_failed,
                { "{} ({:d}) failed to remove processor {:d} from connection {:d}: {}", "{} ({:d}) konnte Prozessor {:d} nicht von Verbindung {:d} entfernen: {}" }
            },
            {
                log_event::connection_processor_reordered,
                { "{} ({:d}) successfully reordered processor \"{}\" in connection \"{}\" to index {:d}.", "{} ({:d}) hat Prozessor \"{}\" in Verbindung \"{}\" erfolgreich auf Index {:d} verschoben." }
            },
            {
                log_event::connection_processor_reorder_failed,
                { "{} ({:d}) failed to reorder processor {:d} in connection {:d}: {}", "{} ({:d}) konnte Prozessor {:d} in Verbindung {:d} nicht auf den gewünschten Index verschieben: {}" }
            },
            {
                log_event::connection_input_inspector_created,
                { "{} ({:d}) successfully created input inspector for connection \"{}\".", "{} ({:d}) hat erfolgreich einen Eingangs-Inspektor für Verbindung \"{}\" erstellt." }
            },
            {
                log_event::connection_input_inspector_create_failed,
                { "{} ({:d}) failed to create input inspector for connection \"{}\".", "{} ({:d}) konnte Eingangs-Inspektor für Verbindung \"{}\" nicht erstellen." }
            },
            {
                log_event::connection_input_inspector_destroyed,
                { "{} ({:d}) successfully destroyed input inspector for connection \"{}\".", "{} ({:d}) hat erfolgreich den Eingangs-Inspektor für Verbindung \"{}\" entfernt." }
            },
            {
                log_event::connection_input_inspector_destroy_failed,
                { "{} ({:d}) failed to destroy input inspector for connection \"{}\".", "{} ({:d}) konnte Eingangs-Inspektor für Verbindung \"{}\" nicht entfernen." }
            },
            {
                log_event::connection_output_inspector_created,
                { "{} ({:d}) successfully created output inspector for connection \"{}\".", "{} ({:d}) hat erfolgreich einen Ausgangs-Inspektor für Verbindung \"{}\" erstellt." }
            },
            {
                log_event::connection_output_inspector_create_failed,
                { "{} ({:d}) failed to create output inspector for connection \"{}\".", "{} ({:d}) konnte Ausgangs-Inspektor für Verbindung \"{}\" nicht erstellen." }
            },
            {
                log_event::connection_output_inspector_destroyed,
                { "{} ({:d}) successfully destroyed output inspector for connection \"{}\".", "{} ({:d}) hat erfolgreich den Ausgangs-Inspektor für Verbindung \"{}\" entfernt." }
            },
            {
                log_event::connection_output_inspector_destroy_failed,
                { "{} ({:d}) failed to destroy output inspector for connection \"{}\".", "{} ({:d}) konnte Ausgangs-Inspektor für Verbindung \"{}\" nicht entfernen." }
            },
            {
                log_event::connection_sorting_index_changed,
                { "{} ({:d}) successfully changed sorting index of connection \"{}\" to {:d}.", "{} ({:d}) hat den Sortierindex von Verbindung \"{}\" erfolgreich auf {:d} geändert." }
            },
            {
                log_event::connection_sorting_index_change_failed,
                { "{} ({:d}) failed to change sorting index of connection {:d}.", "{} ({:d}) konnte den Sortierindex von Verbindung {:d} nicht ändern." }
            },
            {
                log_event::connection_color_changed,
                { "{} ({:d}) successfully changed color of connection \"{}\" to #{:06x}.", "{} ({:d}) hat die Farbe von Verbindung \"{}\" erfolgreich auf #{:06x} geändert." }
            },
            {
                log_event::connection_color_change_failed,
                { "{} ({:d}) failed to change color of connection {:d}.", "{} ({:d}) konnte die Farbe von Verbindung {:d} nicht ändern." }
            },
            {
                log_event::port_data_injected,
                { "{} ({:d}) successfully injected data into port \"{}\".", "{} ({:d}) hat erfolgreich Daten in Port \"{}\" injiziert." }
            },
            {
                log_event::port_data_inject_failed,
                { "{} ({:d}) failed to inject data into port \"{}\".", "{} ({:d}) konnte keine Daten in Port \"{}\" injizieren." }
            },
            {
                log_event::config_path_added,
                { "{} ({:d}) successfully added config path \"{}\".", "{} ({:d}) hat Konfigurationspfad \"{}\" erfolgreich hinzugefügt." }
            },
            {
                log_event::config_path_add_failed,
                { "{} ({:d}) failed to add config path \"{}\".", "{} ({:d}) konnte Konfigurationspfad \"{}\" nicht hinzufügen." }
            },
            {
                log_event::config_path_removed,
                { "{} ({:d}) successfully removed config path at index {:d}.", "{} ({:d}) hat Konfigurationspfad bei Index {:d} erfolgreich entfernt." }
            },
            {
                log_event::config_path_remove_failed,
                { "{} ({:d}) failed to remove config path at index {:d}.", "{} ({:d}) konnte Konfigurationspfad bei Index {:d} nicht entfernen." }
            },
            {
                log_event::config_scan_requested,
                { "{} ({:d}) requested config scan.", "{} ({:d}) hat Konfigurations-Scan angefordert." }
            },
            {
                log_event::config_exported,
                { "{} ({:d}) successfully saved configuration: \"{}\".", "{} ({:d}) hat Konfiguration erfolgreich gespeichert: \"{}\"." }
            },
            {
                log_event::config_export_failed,
                { "{} ({:d}) failed to save configuration: \"{}\".", "{} ({:d}) konnte Konfiguration nicht speichern: \"{}\"." }
            },
            {
                log_event::config_imported,
                { "{} ({:d}) successfully loaded configuration: \"{}\".", "{} ({:d}) hat Konfiguration erfolgreich geladen: \"{}\"." }
            },
            {
                log_event::config_import_failed,
                { "{} ({:d}) failed to load configuration: \"{}\".", "{} ({:d}) konnte Konfiguration nicht laden: \"{}\"." }
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