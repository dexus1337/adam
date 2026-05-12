#include "commander/commander-event-dispatcher.hpp"

#include "commander/commander.hpp"

namespace adam 
{
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
            ctx.cmdr.m_lang = e.get_data_as<command::initial_data>()->lang_info;
        });
    }
}