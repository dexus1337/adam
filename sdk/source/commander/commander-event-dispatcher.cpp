#include "commander/commander-event-dispatcher.hpp"

#include "commander/commander.hpp"
#include "commander/messages/command.hpp"
#include "commander/messages/message-structs.hpp"
#include "data/port/port.hpp"
#include "module/module.hpp"
#include "data/connection.hpp"
#include <mutex>
#include <algorithm>

namespace adam 
{
    commander_event_dispatcher::commander_event_dispatcher()
    {
        register_default_handlers();
    }

    commander_event_dispatcher::~commander_event_dispatcher() {}

    void commander_event_dispatcher::register_handler(event_type type, handler_fn handler)
    {
        m_handlers[type] = std::move(handler);
    }
    
    void commander_event_dispatcher::dispatch(const event& e, event_context& ctx) const
    {
        auto it = m_handlers.find(e.get_type());
        if (it != m_handlers.end())
        {
            if (it->second)
            {
                it->second(e, ctx);
            }
        }
    }

    void commander_event_dispatcher::register_default_handlers()
    {
        register_handler(event_type::language_changed, [](const event& e, event_context& ctx) 
        {
            ctx.cmdr.m_lang.lang = e.get_data_as<messages::initial_data_header>()->lang_info.lang;
        });

        register_handler(event_type::module_path_added, [](const event& e, event_context& ctx) 
        {
            auto* data = e.get_data_as<messages::module_path_data>();
            std::lock_guard<const module_view> lg(ctx.cmdr.modules());
            auto& paths = ctx.cmdr.modules().paths();
            if (data->idx >= paths.size())
            {
                paths.push_back(data->path);
            }
            else
            {
                paths.insert(paths.begin() + data->idx, data->path);
            }
        });

        register_handler(event_type::module_path_removed, [](const event& e, event_context& ctx) 
        {
            auto* data = e.get_data_as<messages::module_path_remove_data>();
            std::lock_guard<const module_view> lg(ctx.cmdr.modules());
            auto& paths = ctx.cmdr.modules().paths();
            if (data->idx < paths.size())
            {
                paths.erase(paths.begin() + data->idx);
            }
        });

        register_handler(event_type::module_loaded, [](const event& e, event_context& ctx) 
        {
            auto* mod_info = e.get_data_as<module::basic_info>();
            string_hashed mod_name(mod_info->name);
            string_hashed mod_path(mod_info->path);

            std::lock_guard<const module_view> lg(ctx.cmdr.modules());
            ctx.cmdr.modules().load_module(mod_name, mod_path, mod_info->version);
            ctx.cmdr.modules().available().erase(mod_name);
        });

        register_handler(event_type::module_unloaded, [](const event& e, event_context& ctx) 
        {
            auto* mod_info = e.get_data_as<module::basic_info>();
            string_hashed mod_name(mod_info->name);
            string_hashed mod_path(mod_info->path);

            std::lock_guard<const module_view> lg(ctx.cmdr.modules());
            ctx.cmdr.modules().unload_module(mod_name);
            ctx.cmdr.modules().available().emplace(mod_name, std::make_pair(mod_info->version, mod_path));
        });

        register_handler(event_type::module_available, [](const event& e, event_context& ctx) 
        {
            auto* mod_info = e.get_data_as<module::basic_info>();
            string_hashed mod_name(mod_info->name);
            string_hashed mod_path(mod_info->path);

            std::lock_guard<const module_view> lg(ctx.cmdr.modules());
            ctx.cmdr.modules().update_module_database(mod_name, mod_path, mod_info->version);
            ctx.cmdr.modules().available().emplace(mod_name, std::make_pair(mod_info->version, mod_path));
        });

        register_handler(event_type::module_unavailable, [](const event& e, event_context& ctx) 
        {
            auto* mod_info = e.get_data_as<module::basic_info>();
            string_hashed mod_name(mod_info->name);
            string_hashed mod_path(mod_info->path);

            std::lock_guard<const module_view> lg(ctx.cmdr.modules());
            ctx.cmdr.modules().unavailable().emplace(mod_name, std::make_tuple(mod_info->version, mod_path, mod_info->rsn));
        });

        register_handler(event_type::module_removed, [](const event& e, event_context& ctx) 
        {
            auto* mod_info = e.get_data_as<module::basic_info>();
            string_hashed mod_name(mod_info->name);

            std::lock_guard<const module_view> lg(ctx.cmdr.modules());
            ctx.cmdr.modules().unload_module(mod_name);
            ctx.cmdr.modules().available().erase(mod_name);
            ctx.cmdr.modules().unavailable().erase(mod_name);
        });

        register_handler(event_type::shutdown, [](const event&, event_context& ctx) 
        {
            ctx.cmdr.destroy();
        });

        register_handler(event_type::port_created, [](const event& e, event_context& ctx) 
        {
            auto* info = e.get_data_as<port::basic_info>();
            auto view = std::make_unique<port_view>();
            view->name = string_hashed(info->name);
            view->direction = info->direction;
            {
                std::lock_guard<const module_view> mod_lg(ctx.cmdr.modules());
                ctx.cmdr.get_modules().extract_port_type_and_module(info->type, info->type_module, view->type, view->type_module);
                ctx.cmdr.get_modules().extract_datatype_and_module(info->format, info->format_module, view->datatype, view->datatype_module);
            }
            {
                std::lock_guard<const registry_view> reg_lg(ctx.cmdr.registry());
                ctx.cmdr.registry().ports()[view->name.get_hash()] = std::move(view);
            }
        });

        register_handler(event_type::port_available, [](const event& e, event_context& ctx) 
        {
            auto* info = e.get_data_as<port::basic_info>();
            
            bool found = false;
            {
                std::lock_guard<const registry_view> reg_lg(ctx.cmdr.registry());
                auto it = ctx.cmdr.registry().ports().find(string_hashed(info->name).get_hash());
                if (it != ctx.cmdr.registry().ports().end())
                {
                    it->second->is_unavailable = false;
                    it->second->direction = info->direction;
                    found = true;
                }
            }
            
            if (!found)
            {
                auto view = std::make_unique<port_view>();
                view->name = string_hashed(info->name);
                view->direction = info->direction;
                view->is_unavailable = false;
                
                {
                    std::lock_guard<const module_view> mod_lg(ctx.cmdr.modules());
                    ctx.cmdr.get_modules().extract_port_type_and_module(info->type, info->type_module, view->type, view->type_module);
                    ctx.cmdr.get_modules().extract_datatype_and_module(info->format, info->format_module, view->datatype, view->datatype_module);
                }
                
                std::lock_guard<const registry_view> reg_lg(ctx.cmdr.registry());
                ctx.cmdr.registry().ports()[view->name.get_hash()] = std::move(view);
            }
        });

        register_handler(event_type::port_unavailable, [](const event& e, event_context& ctx) 
        {
            auto* data = e.get_data_as<messages::port_action_data>();
            std::lock_guard<const registry_view> lg(ctx.cmdr.registry());
            auto it = ctx.cmdr.registry().ports().find(data->port);
            if (it != ctx.cmdr.registry().ports().end())
            {
                it->second->is_unavailable = true;
            }
        });

        register_handler(event_type::port_destroyed, [](const event& e, event_context& ctx) 
        {
            auto* data = e.get_data_as<messages::port_destroy_data>();
            std::lock_guard<const registry_view> lg(ctx.cmdr.registry());
            ctx.cmdr.registry().ports().erase(data->port);
        });

        register_handler(event_type::port_started, [](const event& e, event_context& ctx) 
        {
            auto* data = e.get_data_as<messages::port_action_data>();
            std::lock_guard<const registry_view> lg(ctx.cmdr.registry());
            auto it = ctx.cmdr.registry().ports().find(data->port);
            if (it != ctx.cmdr.registry().ports().end()) it->second->is_active = true;
        });

        register_handler(event_type::port_stopped, [](const event& e, event_context& ctx) 
        {
            auto* data = e.get_data_as<messages::port_action_data>();
            std::lock_guard<const registry_view> lg(ctx.cmdr.registry());
            auto it = ctx.cmdr.registry().ports().find(data->port);
            if (it != ctx.cmdr.registry().ports().end()) it->second->is_active = false;
        });

        register_handler(event_type::port_renamed, [](const event& e, event_context& ctx) 
        {
            auto* data = e.get_data_as<messages::port_rename_data>();
            std::lock_guard<const registry_view> lg(ctx.cmdr.registry());
            auto it = ctx.cmdr.registry().ports().find(data->port);
            if (it != ctx.cmdr.registry().ports().end())
            {
                auto view = std::move(it->second);
                ctx.cmdr.registry().ports().erase(it);
                
                string_hashed new_name(data->new_name);
                view->name = new_name;
                string_hash new_hash = new_name.get_hash();
                ctx.cmdr.registry().ports()[new_hash] = std::move(view);
                
                for (auto& [conn_hash, conn] : ctx.cmdr.registry().connections())
                {
                    for (auto& pid : conn->inputs)
                    {
                        if (pid == data->port)
                        {
                            pid = new_hash;
                            conn->edited = data->edited;
                        }
                    }

                    for (auto& pid : conn->outputs)
                    {
                        if (pid == data->port)
                        {
                            pid = new_hash;
                            conn->edited = data->edited;
                        }
                    }
                }
            }
        });

        register_handler(event_type::connection_created, [](const event& e, event_context& ctx) 
        {
            auto* info = e.get_data_as<connection::basic_info>();
            auto view = std::make_unique<connection_view>();
            view->name = string_hashed(info->name);
            view->created = info->created;
            view->edited = info->edited;
            view->sorting_index = info->sorting_index;
            view->color = info->color;
            std::lock_guard<const registry_view> lg(ctx.cmdr.registry());
            ctx.cmdr.registry().connections()[view->name.get_hash()] = std::move(view);
        });

        register_handler(event_type::connection_destroyed, [](const event& e, event_context& ctx) 
        {
            auto* data = e.get_data_as<messages::connection_destroy_data>();
            std::lock_guard<const registry_view> lg(ctx.cmdr.registry());
            ctx.cmdr.registry().connections().erase(data->connection);
        });

        register_handler(event_type::connection_started, [](const event& e, event_context& ctx) 
        {
            auto* data = e.get_data_as<messages::connection_action_data>();
            std::lock_guard<const registry_view> lg(ctx.cmdr.registry());
            auto it = ctx.cmdr.registry().connections().find(data->connection);
            if (it != ctx.cmdr.registry().connections().end()) it->second->is_active = true;
        });

        register_handler(event_type::connection_stopped, [](const event& e, event_context& ctx) 
        {
            auto* data = e.get_data_as<messages::connection_action_data>();
            std::lock_guard<const registry_view> lg(ctx.cmdr.registry());
            auto it = ctx.cmdr.registry().connections().find(data->connection);
            if (it != ctx.cmdr.registry().connections().end()) it->second->is_active = false;
        });

        register_handler(event_type::connection_renamed, [](const event& e, event_context& ctx) 
        {
            auto* data = e.get_data_as<messages::connection_rename_data>();
            std::lock_guard<const registry_view> lg(ctx.cmdr.registry());
            auto it = ctx.cmdr.registry().connections().find(data->connection);
            if (it != ctx.cmdr.registry().connections().end())
            {
                auto view = std::move(it->second);
                ctx.cmdr.registry().connections().erase(it);
                
                string_hashed new_name(data->new_name);
                view->name = new_name;
                view->edited = data->edited;
                ctx.cmdr.registry().connections()[new_name.get_hash()] = std::move(view);
            }
        });

        register_handler(event_type::connection_port_added, [](const event& e, event_context& ctx) 
        {
            auto* data = e.get_data_as<messages::connection_port_add_data>();
            std::lock_guard<const registry_view> lg(ctx.cmdr.registry());
            auto it = ctx.cmdr.registry().connections().find(data->connection);
            if (it != ctx.cmdr.registry().connections().end())
            {
                if (data->is_input)
                    it->second->inputs.push_back(data->port);
                else
                    it->second->outputs.push_back(data->port);
                it->second->edited = data->edited;
            }
        });

        register_handler(event_type::connection_sorting_index_changed, [](const event& e, event_context& ctx) 
        {
            auto* data = e.get_data_as<messages::connection_property_change_data>();
            std::lock_guard<const registry_view> lg(ctx.cmdr.registry());
            auto it = ctx.cmdr.registry().connections().find(data->connection);
            if (it != ctx.cmdr.registry().connections().end())
            {
                it->second->sorting_index = data->value;
                it->second->edited = data->edited;
            }
        });

        register_handler(event_type::connection_color_changed, [](const event& e, event_context& ctx) 
        {
            auto* data = e.get_data_as<messages::connection_property_change_data>();
            std::lock_guard<const registry_view> lg(ctx.cmdr.registry());
            auto it = ctx.cmdr.registry().connections().find(data->connection);
            if (it != ctx.cmdr.registry().connections().end())
            {
                it->second->color = data->value;
                it->second->edited = data->edited;
            }
        });
    }
}