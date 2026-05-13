#include "commander/commander-event-dispatcher.hpp"

#include "commander/commander.hpp"
#include "commander/messages/command.hpp"

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

            ctx.cmdr.m_loaded_modules_cache.emplace(mod_name, nullptr);
            ctx.cmdr.m_available_modules_cache.erase(mod_name);
        });

        register_handler(static_cast<int>(event_type::module_unloaded), [](const event& e, event_context& ctx) 
        {
            auto* mod_info = e.get_data_as<module::basic_info>();
            string_hashed mod_name(mod_info->name);
            string_hashed mod_path(mod_info->path);

            ctx.cmdr.m_loaded_modules_cache.erase(mod_name);
            ctx.cmdr.m_available_modules_cache.emplace(mod_name, std::make_pair(mod_info->version, mod_path));
        });

        register_handler(static_cast<int>(event_type::module_available), [](const event& e, event_context& ctx) 
        {
            auto* mod_info = e.get_data_as<module::basic_info>();
            string_hashed mod_name(mod_info->name);
            string_hashed mod_path(mod_info->path);

            ctx.cmdr.m_available_modules_cache.emplace(mod_name, std::make_pair(mod_info->version, mod_path));
        });

        register_handler(static_cast<int>(event_type::module_unavailable), [](const event& e, event_context& ctx) 
        {
            auto* mod_info = e.get_data_as<module::basic_info>();
            string_hashed mod_name(mod_info->name);
            string_hashed mod_path(mod_info->path);

            ctx.cmdr.m_unavailable_modules_cache.emplace(mod_name, std::make_tuple(mod_info->version, mod_path, mod_info->rsn));
        });

        register_handler(static_cast<int>(event_type::shutdown), [](const event&, event_context& ctx) 
        {
            ctx.cmdr.destroy();
        });
    }
}