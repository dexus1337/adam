#include "commander/commander-event-dispatcher.hpp"

#include "commander/commander.hpp"
#include "commander/messages/command.hpp"
#include "data/port/port.hpp"
#include "module/module.hpp"
#include "data/connection.hpp"
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
            ctx.cmdr.m_lang.lang = e.get_data_as<command::initial_data_header>()->lang_info.lang;
        });

        register_handler(static_cast<int>(event_type::module_loaded), [](const event& e, event_context& ctx) 
        {
            auto* mod_info = e.get_data_as<module::basic_info>();
            string_hashed mod_name(mod_info->name);

            ctx.cmdr.modules().loaded().emplace(mod_name, nullptr);
            ctx.cmdr.modules().available().erase(mod_name);
        });

        register_handler(static_cast<int>(event_type::module_unloaded), [](const event& e, event_context& ctx) 
        {
            auto* mod_info = e.get_data_as<module::basic_info>();
            string_hashed mod_name(mod_info->name);
            string_hashed mod_path(mod_info->path);

            ctx.cmdr.modules().loaded().erase(mod_name);
            ctx.cmdr.modules().available().emplace(mod_name, std::make_pair(mod_info->version, mod_path));
        });

        register_handler(static_cast<int>(event_type::module_available), [](const event& e, event_context& ctx) 
        {
            auto* mod_info = e.get_data_as<module::basic_info>();
            string_hashed mod_name(mod_info->name);
            string_hashed mod_path(mod_info->path);

            ctx.cmdr.modules().available().emplace(mod_name, std::make_pair(mod_info->version, mod_path));
        });

        register_handler(static_cast<int>(event_type::module_unavailable), [](const event& e, event_context& ctx) 
        {
            auto* mod_info = e.get_data_as<module::basic_info>();
            string_hashed mod_name(mod_info->name);
            string_hashed mod_path(mod_info->path);

            ctx.cmdr.modules().unavailable().emplace(mod_name, std::make_tuple(mod_info->version, mod_path, mod_info->rsn));
        });

        register_handler(static_cast<int>(event_type::module_removed), [](const event& e, event_context& ctx) 
        {
            auto* mod_info = e.get_data_as<module::basic_info>();
            string_hashed mod_name(mod_info->name);

            ctx.cmdr.modules().loaded().erase(mod_name);
            ctx.cmdr.modules().available().erase(mod_name);
            ctx.cmdr.modules().unavailable().erase(mod_name);
        });

        register_handler(static_cast<int>(event_type::module_path_added), [](const event& e, event_context& ctx) 
        {
            auto* data = e.get_data_as<command::module_path_data>();
            ctx.cmdr.modules().paths().push_back(data->path);
        });

        register_handler(static_cast<int>(event_type::module_path_removed), [](const event& e, event_context& ctx) 
        {
            auto* data = e.get_data_as<command::module_path_data>();
            auto& paths = ctx.cmdr.modules().paths();
            paths.erase(std::remove(paths.begin(), paths.end(), std::string(data->path)), paths.end());
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
            view->type = string_hashed(info->type);
            view->module_name = string_hashed(info->module_name);
            ctx.cmdr.registry().ports()[view->name.get_hash()] = std::move(view);
        });

        register_handler(static_cast<int>(event_type::port_destroyed), [](const event& e, event_context& ctx) 
        {
            auto* data = e.get_data_as<command::port_destroy_data>();
            ctx.cmdr.registry().ports().erase(data->port);
        });

        register_handler(static_cast<int>(event_type::connection_created), [](const event& e, event_context& ctx) 
        {
            auto* info = e.get_data_as<connection::basic_info>();
            auto view = std::make_unique<connection_view>();
            view->name = string_hashed(info->name);
            ctx.cmdr.registry().connections()[view->name.get_hash()] = std::move(view);
        });

        register_handler(static_cast<int>(event_type::connection_destroyed), [](const event& e, event_context& ctx) 
        {
            auto* data = e.get_data_as<command::connection_destroy_data>();
            ctx.cmdr.registry().connections().erase(data->connection);
        });
    }
}