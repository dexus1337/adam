#include "controller/controller-cmd-dispatcher.hpp"
#include "controller/controller.hpp"
#include "controller/registry.hpp"
#include "resources/language-strings.hpp"
#include "data/port/port.hpp"
#include "module/module.hpp"
#include "data/inspector.hpp"
#include "commander/messages/event.hpp"
#include "commander/messages/message-structs.hpp"
#include "data/connection.hpp"
#include "configuration/parameters/configuration-parameter-string.hpp"
#include "configuration/parameters/configuration-parameter-list.hpp"

#include <array>
#include <format>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cstring>

namespace adam
{
    controller_cmd_dispatcher::controller_cmd_dispatcher()
    {
        register_default_handlers();
    }

    controller_cmd_dispatcher::~controller_cmd_dispatcher() {}

    void controller_cmd_dispatcher::register_handler(int type, handler_fn handler) 
    {
        m_handlers[type] = std::move(handler);
    }

    void controller_cmd_dispatcher::dispatch(const command* cmds, size_t count, command_context& ctx) const
    {
        if (!cmds || count == 0)
        {
            ctx.set_single_response_status(response_status::invalid);
            return;
        }

        auto it = m_handlers.find(static_cast<int>(cmds->get_type()));
        if (it != m_handlers.end())
        {
            it->second(cmds, count, ctx);
            return;
        }
        
        ctx.set_single_response_status(response_status::unknown);
    }

    void controller_cmd_dispatcher::register_default_handlers()
    {
        register_handler(static_cast<int>(command_type::acquire_initial_data), [](const command*, size_t, command_context& ctx)
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
            
        });

        register_handler(static_cast<int>(command_type::set_language), [](const command* cmds, size_t, command_context& ctx) 
        {
            ctx.ctrl.set_language(*cmds->get_data_as<language>());
            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::language_changed, ctx.ctrl.get_language()));

            event evt(event_type::language_changed);
            *evt.data_as<language>() = ctx.ctrl.get_language();
            ctx.ctrl.broadcast_event(evt);

            ctx.set_single_response_status(response_status::success);
        });

        register_handler(static_cast<int>(command_type::module_path_add), [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::module_path_data>();
            string_hashed path(params->path);
            uint32_t idx = params->idx;

            if (!ctx.reg.add_module_path(path, &idx))
            {
                debug_statement(ctx.ctrl.log(log::trace, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::module_path_add_failed, ctx.ctrl.get_language()), ctx.tid, path.c_str()));
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            event evt(event_type::module_path_added);
            auto* evt_data = evt.data_as<messages::module_path_data>();
            evt_data->setup(params->path, idx);
            ctx.ctrl.broadcast_event(evt);

            debug_statement(ctx.ctrl.log(log::trace, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::module_path_added, ctx.ctrl.get_language()), ctx.tid, path.c_str()));
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(static_cast<int>(command_type::module_path_remove), [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::module_path_remove_data>();

            if (!ctx.reg.remove_module_path(params->idx))
            {
                debug_statement(ctx.ctrl.log(log::trace, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::module_path_remove_failed, ctx.ctrl.get_language()), ctx.tid, params->idx));
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            event evt(event_type::module_path_removed);
            auto* evt_data = evt.data_as<messages::module_path_remove_data>();
            evt_data->idx = params->idx;
            ctx.ctrl.broadcast_event(evt);

            debug_statement(ctx.ctrl.log(log::trace, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::module_path_removed, ctx.ctrl.get_language()), ctx.tid, params->idx));
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(static_cast<int>(command_type::module_scan), [](const command*, size_t, command_context& ctx) 
        {
            debug_statement(ctx.ctrl.log(log::trace, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::module_scan_requested, ctx.ctrl.get_language()), ctx.tid));
            
            ctx.reg.modules().scan_for_modules();
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(static_cast<int>(command_type::module_load), [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::module_action_data>();
            string_hashed name(params->module_name);

            if (ctx.reg.modules().load_module(name))
                ctx.set_single_response_status(response_status::success);
            else
                ctx.set_single_response_status(response_status::failed);
        });

        register_handler(static_cast<int>(command_type::module_unload), [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::module_action_data>();
            string_hashed name(params->module_name);

            if (ctx.reg.modules().unload_module(name))
                ctx.set_single_response_status(response_status::success);
            else
                ctx.set_single_response_status(response_status::failed);
        });

        register_handler(static_cast<int>(command_type::port_create), [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<port::basic_info>();
            string_hashed name(params->name);
            
            registry::status res = ctx.reg.create_port(name, params->type, params->module);

            if (res != registry::status_success)
            {
                auto name_view = name.c_str();
                auto status_text = registry::get_status_text(res, ctx.ctrl.get_language());
                debug_statement(ctx.ctrl.log(log::trace, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_create_failed, ctx.ctrl.get_language()), ctx.tid, name_view, status_text));
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            event evt(event_type::port_created);
            auto* evt_data = evt.data_as<port::basic_info>();
            *evt_data = *params;
            ctx.ctrl.broadcast_event(evt);

            auto name_view = name.c_str();
            debug_statement(ctx.ctrl.log(log::trace, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_created, ctx.ctrl.get_language()), ctx.tid, name_view, params->type));
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(static_cast<int>(command_type::port_destroy), [](const command* cmds, size_t, command_context& ctx) 
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
                debug_statement(ctx.ctrl.log(log::trace, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_destroy_failed, ctx.ctrl.get_language()), ctx.tid, port_hash, status_text));
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            event evt(event_type::port_destroyed);
            evt.data_as<messages::port_destroy_data>()->port = params->port;
            ctx.ctrl.broadcast_event(evt);

            debug_statement(ctx.ctrl.log(log::trace, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_destroyed, ctx.ctrl.get_language()), ctx.tid, port_str));
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(static_cast<int>(command_type::connection_create), [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<connection::basic_info>();
            string_hashed name(params->name);

            connection* new_conn = nullptr;
            registry::status res = ctx.reg.create_connection(name, &new_conn);

            if (res != registry::status_success)
            {
                auto name_view = name.c_str();
                auto status_text = registry::get_status_text(res, ctx.ctrl.get_language());
                debug_statement(ctx.ctrl.log(log::trace, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_create_failed, ctx.ctrl.get_language()), ctx.tid, name_view, status_text));
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            event evt(event_type::connection_created);
            auto* evt_data = evt.data_as<connection::basic_info>();
            *evt_data = *params;
            ctx.ctrl.broadcast_event(evt);

            auto name_view = name.c_str();
            debug_statement(ctx.ctrl.log(log::trace, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_created, ctx.ctrl.get_language()), ctx.tid, name_view));
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(static_cast<int>(command_type::connection_destroy), [](const command* cmds, size_t, command_context& ctx) 
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
                debug_statement(ctx.ctrl.log(log::trace, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_destroy_failed, ctx.ctrl.get_language()), ctx.tid, conn_hash, status_text));
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            event evt(event_type::connection_destroyed);
            evt.data_as<messages::connection_destroy_data>()->connection = params->connection;
            ctx.ctrl.broadcast_event(evt);

            debug_statement(ctx.ctrl.log(log::trace, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_destroyed, ctx.ctrl.get_language()), ctx.tid, conn_str));
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(static_cast<int>(command_type::port_start), [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::port_action_data>();
            auto it = ctx.reg.ports().find(params->port);

            if (it == ctx.reg.ports().end() || !it->second->start())
            {
                uint64_t port_hash = static_cast<uint64_t>(params->port);
                debug_statement(ctx.ctrl.log(log::trace, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_start_failed, ctx.ctrl.get_language()), ctx.tid, port_hash));
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            event evt(event_type::port_started);
            evt.data_as<messages::port_action_data>()->port = params->port;
            ctx.ctrl.broadcast_event(evt);

            debug_statement(ctx.ctrl.log(log::trace, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_started, ctx.ctrl.get_language()), ctx.tid, it->second->get_name().c_str()));
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(static_cast<int>(command_type::port_stop), [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::port_action_data>();
            auto it = ctx.reg.ports().find(params->port);

            if (it == ctx.reg.ports().end() || !it->second->stop())
            {
                uint64_t port_hash = static_cast<uint64_t>(params->port);
                debug_statement(ctx.ctrl.log(log::trace, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_stop_failed, ctx.ctrl.get_language()), ctx.tid, port_hash));
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            event evt(event_type::port_stopped);
            evt.data_as<messages::port_action_data>()->port = params->port;
            ctx.ctrl.broadcast_event(evt);

            debug_statement(ctx.ctrl.log(log::trace, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::port_stopped, ctx.ctrl.get_language()), ctx.tid, it->second->get_name().c_str()));
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(static_cast<int>(command_type::connection_start), [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::connection_action_data>();
            auto it = ctx.reg.connections().find(params->connection);

            if (it == ctx.reg.connections().end() || !it->second->start())
            {
                uint64_t conn_hash = static_cast<uint64_t>(params->connection);
                debug_statement(ctx.ctrl.log(log::trace, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_start_failed, ctx.ctrl.get_language()), ctx.tid, conn_hash));
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            event evt(event_type::connection_started);
            evt.data_as<messages::connection_action_data>()->connection = params->connection;
            ctx.ctrl.broadcast_event(evt);

            debug_statement(ctx.ctrl.log(log::trace, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_started, ctx.ctrl.get_language()), ctx.tid, it->second->get_name().c_str()));
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(static_cast<int>(command_type::connection_stop), [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::connection_action_data>();
            auto it = ctx.reg.connections().find(params->connection);

            if (it == ctx.reg.connections().end() || !it->second->stop())
            {
                uint64_t conn_hash = static_cast<uint64_t>(params->connection);
                debug_statement(ctx.ctrl.log(log::trace, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_stop_failed, ctx.ctrl.get_language()), ctx.tid, conn_hash));
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            event evt(event_type::connection_stopped);
            evt.data_as<messages::connection_action_data>()->connection = params->connection;
            ctx.ctrl.broadcast_event(evt);

            debug_statement(ctx.ctrl.log(log::trace, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_stopped, ctx.ctrl.get_language()), ctx.tid, it->second->get_name().c_str()));
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(static_cast<int>(command_type::connection_rename), [](const command* cmds, size_t, command_context& ctx) 
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
                debug_statement(ctx.ctrl.log(log::trace, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_rename_failed, ctx.ctrl.get_language()), ctx.tid, conn_hash, status_text));
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            event evt(event_type::connection_renamed);
            auto* evt_data = evt.data_as<messages::connection_rename_data>();
            *evt_data = *params;
            ctx.ctrl.broadcast_event(evt);

            debug_statement(ctx.ctrl.log(log::trace, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::connection_renamed, ctx.ctrl.get_language()), ctx.tid, old_conn_str.c_str(), new_name.c_str()));
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(static_cast<int>(command_type::inspector_create), [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::inspector_create_data>();
            auto port = ctx.reg.ports().find(params->port);

            if (port == ctx.reg.ports().end())
            {
                uint64_t port_hash = static_cast<uint64_t>(params->port);
                debug_statement(ctx.ctrl.log(log::trace, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::inspector_create_failed_port_unknown, ctx.ctrl.get_language()), ctx.tid, port_hash));
                ctx.set_single_response_status(response_status::unknown);
                return;
            }

            const auto& port_name = port->second->get_name();
            auto new_inspector = std::make_shared<data_inspector>();

            if (!new_inspector->open(port_name, ctx.tid))
            {
                auto name_view = port_name.c_str();
                debug_statement(ctx.ctrl.log(log::trace, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::inspector_create_failed_open, ctx.ctrl.get_language()), ctx.tid, name_view));
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            port->second->inspectors().push_back(new_inspector);
            ctx.thread_inspectors.emplace(params->port, new_inspector);

            auto name_view = port_name.c_str();
            debug_statement(ctx.ctrl.log(log::trace, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::inspector_created, ctx.ctrl.get_language()), ctx.tid, name_view));
            ctx.set_single_response_status(response_status::success);
        });

        register_handler(static_cast<int>(command_type::inspector_destroy), [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<messages::inspector_destroy_data>();
            auto it = ctx.thread_inspectors.find(params->port);

            if (it == ctx.thread_inspectors.end())
            {
                std::string port_str = std::to_string(static_cast<uint64_t>(params->port));
                debug_statement(ctx.ctrl.log(log::trace, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::inspector_destroy_failed_not_found, ctx.ctrl.get_language()), ctx.tid, port_str));
                ctx.set_single_response_status(response_status::failed);
                return;
            }

            auto port = ctx.reg.ports().find(params->port);
            std::string port_str = port != ctx.reg.ports().end() ? std::string(port->second->get_name().c_str()) : std::to_string(static_cast<uint64_t>(params->port));

            if (port != ctx.reg.ports().end())
                port->second->inspectors().remove(it->second);

            ctx.thread_inspectors.erase(it);

            debug_statement(ctx.ctrl.log(log::trace, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::inspector_destroyed, ctx.ctrl.get_language()), ctx.tid, port_str));
            ctx.set_single_response_status(response_status::success);
        });
    }

    std::string_view controller_cmd_dispatcher::get_log_event_text(log_event event, language lang)
    {
        static const std::unordered_map<int, std::array<std::string_view, languages_count>> translations =
        {
            { 
                static_cast<int>(log_event::language_changed),
                { "Language changed to: english!",                  "Sprache geändert zu: Deutsch." }
            },
            {
                static_cast<int>(log_event::inspector_created),
                { "Thread {:d} successfully created inspector for port \"{}\".", "Thread {:d} hat erfolgreich einen Inspektor für Port \"{}\" erstellt." }
            },
            {
                static_cast<int>(log_event::inspector_destroyed),
                { "Thread {:d} successfully destroyed inspector for port \"{}\".", "Thread {:d} hat erfolgreich den Inspektor für Port \"{}\" entfernt." }
            },
            {
                static_cast<int>(log_event::inspector_create_failed_port_unknown),
                { "Thread {:d} failed to create inspector: Unknown port hash {:d}.", "Thread {:d} konnte Inspektor nicht erstellen: Unbekannter Port-Hash {:d}." }
            },
            {
                static_cast<int>(log_event::inspector_create_failed_open),
                { "Thread {:d} failed to create inspector: Could not open queue for port \"{}\".", "Thread {:d} konnte Inspektor nicht erstellen: Warteschlange für Port \"{}\" konnte nicht geöffnet werden." }
            },
            {
                static_cast<int>(log_event::inspector_destroy_failed_port_unknown),
                { "Thread {:d} failed to destroy inspector: Unknown port hash {:d}.", "Thread {:d} konnte Inspektor nicht entfernen: Unbekannter Port-Hash {:d}." }
            },
            {
                static_cast<int>(log_event::inspector_destroy_failed_not_found),
                { "Thread {:d} failed to destroy inspector: Inspector not found for port \"{}\".", "Thread {:d} konnte Inspektor nicht entfernen: Kein Inspektor für Port \"{}\" gefunden." }
            },
        {
            static_cast<int>(log_event::module_path_added),
            { "Thread {:d} successfully added module path \"{}\".", "Thread {:d} hat Modulpfad \"{}\" erfolgreich hinzugefügt." }
        },
        {
            static_cast<int>(log_event::module_path_add_failed),
            { "Thread {:d} failed to add module path \"{}\".", "Thread {:d} konnte Modulpfad \"{}\" nicht hinzufügen." }
        },
        {
            static_cast<int>(log_event::module_path_removed),
            { "Thread {:d} successfully removed module path {:d}.", "Thread {:d} hat Modulpfad {:d} erfolgreich entfernt." }
        },
        {
            static_cast<int>(log_event::module_path_remove_failed),
            { "Thread {:d} failed to remove module path {:d}.", "Thread {:d} konnte Modulpfad {:d} nicht entfernen." }
        },
        {
            static_cast<int>(log_event::module_scan_requested),
            { "Thread {:d} requested a module scan.", "Thread {:d} hat eine Modulsuche angefordert." }
        },
            {
                static_cast<int>(log_event::port_created),
                { "Thread {:d} successfully created port \"{}\" of type \"{}\".", "Thread {:d} hat Port \"{}\" vom Typ \"{}\" erfolgreich erstellt." }
            },
            {
                static_cast<int>(log_event::port_create_failed),
                { "Thread {:d} failed to create port \"{}\": {}", "Thread {:d} konnte Port \"{}\" nicht erstellen: {}" }
            },
            {
                static_cast<int>(log_event::port_destroyed),
                { "Thread {:d} successfully destroyed port \"{}\".", "Thread {:d} hat Port \"{}\" erfolgreich entfernt." }
            },
            {
                static_cast<int>(log_event::port_destroy_failed),
                { "Thread {:d} failed to destroy port {:d}: {}", "Thread {:d} konnte Port {:d} nicht entfernen: {}" }
            },
            {
                static_cast<int>(log_event::connection_created),
                { "Thread {:d} successfully created connection \"{}\".", "Thread {:d} hat Verbindung \"{}\" erfolgreich erstellt." }
            },
            {
                static_cast<int>(log_event::connection_create_failed),
                { "Thread {:d} failed to create connection \"{}\": {}", "Thread {:d} konnte Verbindung \"{}\" nicht erstellen: {}" }
            },
            {
                static_cast<int>(log_event::connection_destroyed),
                { "Thread {:d} successfully destroyed connection \"{}\".", "Thread {:d} hat Verbindung \"{}\" erfolgreich entfernt." }
            },
            {
                static_cast<int>(log_event::connection_destroy_failed),
                { "Thread {:d} failed to destroy connection {:d}: {}", "Thread {:d} konnte Verbindung {:d} nicht entfernen: {}" }
            },
            {
                static_cast<int>(log_event::port_started),
                { "Thread {:d} successfully started port \"{}\".", "Thread {:d} hat Port \"{}\" erfolgreich gestartet." }
            },
            {
                static_cast<int>(log_event::port_start_failed),
                { "Thread {:d} failed to start port {:d}.", "Thread {:d} konnte Port {:d} nicht starten." }
            },
            {
                static_cast<int>(log_event::port_stopped),
                { "Thread {:d} successfully stopped port \"{}\".", "Thread {:d} hat Port \"{}\" erfolgreich gestoppt." }
            },
            {
                static_cast<int>(log_event::port_stop_failed),
                { "Thread {:d} failed to stop port {:d}.", "Thread {:d} konnte Port {:d} nicht stoppen." }
            },
            {
                static_cast<int>(log_event::connection_started),
                { "Thread {:d} successfully started connection \"{}\".", "Thread {:d} hat Verbindung \"{}\" erfolgreich gestartet." }
            },
            {
                static_cast<int>(log_event::connection_start_failed),
                { "Thread {:d} failed to start connection {:d}.", "Thread {:d} konnte Verbindung {:d} nicht starten." }
            },
            {
                static_cast<int>(log_event::connection_stopped),
                { "Thread {:d} successfully stopped connection \"{}\".", "Thread {:d} hat Verbindung \"{}\" erfolgreich gestoppt." }
            },
            {
                static_cast<int>(log_event::connection_stop_failed),
                { "Thread {:d} failed to stop connection {:d}.", "Thread {:d} konnte Verbindung {:d} nicht stoppen." }
            },
            {
                static_cast<int>(log_event::connection_renamed),
                { "Thread {:d} successfully renamed connection \"{}\" to \"{}\".", "Thread {:d} hat Verbindung \"{}\" erfolgreich zu \"{}\" umbenannt." }
            },
            {
                static_cast<int>(log_event::connection_rename_failed),
                { "Thread {:d} failed to rename connection {:d}: {}", "Thread {:d} konnte Verbindung {:d} nicht umbenennen: {}" }
            }
        };

        auto val    = static_cast<int>(event);
        auto it     = translations.find(val);

        if (it != translations.end())
            return it->second[static_cast<int>(lang)];
        
        return language_strings::unknown_type_message("controller_cmd_dispatcher::log_event", val, lang);
    }
}