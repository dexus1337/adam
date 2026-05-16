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

    void commander_event_dispatcher::register_handler(int type, handler_fn handler)
    {
        m_handlers[type] = std::move(handler);
    }
    
    void commander_event_dispatcher::dispatch(const event& e, event_context& ctx) const
    {
        auto it = m_handlers.find(static_cast<int>(e.get_type()));
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
        register_handler(static_cast<int>(event_type::language_changed), [](const event& e, event_context& ctx) 
        {
            ctx.cmdr.m_lang.lang = e.get_data_as<messages::initial_data_header>()->lang_info.lang;
        });

        register_handler(static_cast<int>(event_type::module_path_added), [](const event& e, event_context& ctx) 
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

        register_handler(static_cast<int>(event_type::module_path_removed), [](const event& e, event_context& ctx) 
        {
            auto* data = e.get_data_as<messages::module_path_remove_data>();
            std::lock_guard<const module_view> lg(ctx.cmdr.modules());
            auto& paths = ctx.cmdr.modules().paths();
            if (data->idx < paths.size())
            {
                paths.erase(paths.begin() + data->idx);
            }
        });

        register_handler(static_cast<int>(event_type::module_loaded), [](const event& e, event_context& ctx) 
        {
            auto* mod_info = e.get_data_as<module::basic_info>();
            string_hashed mod_name(mod_info->name);
            string_hashed mod_path(mod_info->path);

            std::lock_guard<const module_view> lg(ctx.cmdr.modules());
            ctx.cmdr.modules().load_module(mod_name, mod_path);
            ctx.cmdr.modules().available().erase(mod_name);
        });

        register_handler(static_cast<int>(event_type::module_unloaded), [](const event& e, event_context& ctx) 
        {
            auto* mod_info = e.get_data_as<module::basic_info>();
            string_hashed mod_name(mod_info->name);
            string_hashed mod_path(mod_info->path);

            std::lock_guard<const module_view> lg(ctx.cmdr.modules());
            ctx.cmdr.modules().unload_module(mod_name);
            ctx.cmdr.modules().available().emplace(mod_name, std::make_pair(mod_info->version, mod_path));
        });

        register_handler(static_cast<int>(event_type::module_available), [](const event& e, event_context& ctx) 
        {
            auto* mod_info = e.get_data_as<module::basic_info>();
            string_hashed mod_name(mod_info->name);
            string_hashed mod_path(mod_info->path);

            std::lock_guard<const module_view> lg(ctx.cmdr.modules());
            ctx.cmdr.modules().available().emplace(mod_name, std::make_pair(mod_info->version, mod_path));
        });

        register_handler(static_cast<int>(event_type::module_unavailable), [](const event& e, event_context& ctx) 
        {
            auto* mod_info = e.get_data_as<module::basic_info>();
            string_hashed mod_name(mod_info->name);
            string_hashed mod_path(mod_info->path);

            std::lock_guard<const module_view> lg(ctx.cmdr.modules());
            ctx.cmdr.modules().unavailable().emplace(mod_name, std::make_tuple(mod_info->version, mod_path, mod_info->rsn));
        });

        register_handler(static_cast<int>(event_type::module_removed), [](const event& e, event_context& ctx) 
        {
            auto* mod_info = e.get_data_as<module::basic_info>();
            string_hashed mod_name(mod_info->name);

            std::lock_guard<const module_view> lg(ctx.cmdr.modules());
            ctx.cmdr.modules().unload_module(mod_name);
            ctx.cmdr.modules().available().erase(mod_name);
            ctx.cmdr.modules().unavailable().erase(mod_name);
        });

        register_handler(static_cast<int>(event_type::shutdown), [](const event&, event_context& ctx) 
        {
            ctx.cmdr.destroy();
        });

        register_handler(static_cast<int>(event_type::port_created), [](const event& e, event_context& ctx) 
        {
            auto* info = e.get_data_as<port::basic_info>();
            auto view = std::make_unique<port_view>();
            view->name = string_hashed(info->name);
            {
                std::lock_guard<const module_view> mod_lg(ctx.cmdr.modules());
                ctx.cmdr.get_modules().extract_port_type_and_module(info->type, info->module, view->type, view->module_name);
            }
            {
                std::lock_guard<const registry_view> reg_lg(ctx.cmdr.registry());
                ctx.cmdr.registry().ports()[view->name.get_hash()] = std::move(view);
            }
        });

        register_handler(static_cast<int>(event_type::port_destroyed), [](const event& e, event_context& ctx) 
        {
            auto* data = e.get_data_as<messages::port_destroy_data>();
            std::lock_guard<const registry_view> lg(ctx.cmdr.registry());
            ctx.cmdr.registry().ports().erase(data->port);
        });

        register_handler(static_cast<int>(event_type::port_started), [](const event& e, event_context& ctx) 
        {
            auto* data = e.get_data_as<messages::port_action_data>();
            std::lock_guard<const registry_view> lg(ctx.cmdr.registry());
            auto it = ctx.cmdr.registry().ports().find(data->port);
            if (it != ctx.cmdr.registry().ports().end()) it->second->is_active = true;
        });

        register_handler(static_cast<int>(event_type::port_stopped), [](const event& e, event_context& ctx) 
        {
            auto* data = e.get_data_as<messages::port_action_data>();
            std::lock_guard<const registry_view> lg(ctx.cmdr.registry());
            auto it = ctx.cmdr.registry().ports().find(data->port);
            if (it != ctx.cmdr.registry().ports().end()) it->second->is_active = false;
        });

        register_handler(static_cast<int>(event_type::connection_created), [](const event& e, event_context& ctx) 
        {
            auto* info = e.get_data_as<connection::basic_info>();
            auto view = std::make_unique<connection_view>();
            view->name = string_hashed(info->name);
            std::lock_guard<const registry_view> lg(ctx.cmdr.registry());
            ctx.cmdr.registry().connections()[view->name.get_hash()] = std::move(view);
        });

        register_handler(static_cast<int>(event_type::connection_destroyed), [](const event& e, event_context& ctx) 
        {
            auto* data = e.get_data_as<messages::connection_destroy_data>();
            std::lock_guard<const registry_view> lg(ctx.cmdr.registry());
            ctx.cmdr.registry().connections().erase(data->connection);
        });

        register_handler(static_cast<int>(event_type::connection_started), [](const event& e, event_context& ctx) 
        {
            auto* data = e.get_data_as<messages::connection_action_data>();
            std::lock_guard<const registry_view> lg(ctx.cmdr.registry());
            auto it = ctx.cmdr.registry().connections().find(data->connection);
            if (it != ctx.cmdr.registry().connections().end()) it->second->is_active = true;
        });

        register_handler(static_cast<int>(event_type::connection_stopped), [](const event& e, event_context& ctx) 
        {
            auto* data = e.get_data_as<messages::connection_action_data>();
            std::lock_guard<const registry_view> lg(ctx.cmdr.registry());
            auto it = ctx.cmdr.registry().connections().find(data->connection);
            if (it != ctx.cmdr.registry().connections().end()) it->second->is_active = false;
        });

        register_handler(static_cast<int>(event_type::connection_renamed), [](const event& e, event_context& ctx) 
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
                ctx.cmdr.registry().connections()[new_name.get_hash()] = std::move(view);
            }
        });
    }
}