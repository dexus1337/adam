#include "controller/controller-cmd-dispatcher.hpp"
#include "controller/controller.hpp"
#include "controller/registry.hpp"
#include "resources/language-strings.hpp"
#include "data/port/port.hpp"
#include "data/inspector.hpp"

#include <array>
#include <format>

namespace adam
{
    void controller_cmd_dispatcher::register_handler(int type, handler_fn handler) 
    {
        m_handlers[type] = std::move(handler);
    }

    response controller_cmd_dispatcher::dispatch(const command& cmd, command_context& ctx) const
    {
        auto it = m_handlers.find(static_cast<int>(cmd.get_type()));
        if (it != m_handlers.end())
            return it->second(cmd, ctx);
        
        return response(response::unknown);
    }

    void controller_cmd_dispatcher::register_default_handlers()
    {
        register_handler(static_cast<int>(command::set_language), [](const command& cmd, command_context& ctx) -> response 
        {
            ctx.lang = *cmd.get_data_as<language>();
            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::language_changed, ctx.lang));
            return response::success;
        });

        register_handler(static_cast<int>(command::inspector_create), [](const command& cmd, command_context& ctx) -> response 
        {
            auto params = cmd.get_data_as<command::inspector_create_data>();
            auto port = ctx.reg.ports().find(params->port);

            if (port == ctx.reg.ports().end())
            {
                uint64_t port_hash = static_cast<uint64_t>(params->port);
                debug_statement(ctx.ctrl.log(log::trace, std::vformat(controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::inspector_create_failed_port_unknown, ctx.lang), std::make_format_args(ctx.tid, port_hash))));
                return response::unknown;
            }

            const auto& port_name = port->second->get_name();
            auto new_inspector = std::make_shared<data_inspector>();

            if (!new_inspector->open(port_name, ctx.tid))
            {
                auto name_view = port_name.c_str();
                debug_statement(ctx.ctrl.log(log::trace, std::vformat(controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::inspector_create_failed_open, ctx.lang), std::make_format_args(ctx.tid, name_view))));
                return response::failed;
            }

            port->second->inspectors().push_back(new_inspector);
            ctx.thread_inspectors.emplace(params->port, new_inspector);

            auto name_view = port_name.c_str();
            debug_statement(ctx.ctrl.log(log::trace, std::vformat(controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::inspector_created, ctx.lang), std::make_format_args(ctx.tid, name_view))));
            return response::success;
        });

        register_handler(static_cast<int>(command::inspector_destroy), [](const command& cmd, command_context& ctx) -> response 
        {
            auto params = cmd.get_data_as<command::inspector_destroy_data>();
            auto port = ctx.reg.ports().find(params->port);

            if (port == ctx.reg.ports().end())
            {
                uint64_t port_hash = static_cast<uint64_t>(params->port);
                debug_statement(ctx.ctrl.log(log::trace, std::vformat(controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::inspector_destroy_failed_port_unknown, ctx.lang), std::make_format_args(ctx.tid, port_hash))));
                return response::unknown;
            }

            const auto& port_name = port->second->get_name();
            auto it = ctx.thread_inspectors.find(params->port);

            if (it == ctx.thread_inspectors.end())
            {
                auto name_view = port_name.c_str();
                debug_statement(ctx.ctrl.log(log::trace, std::vformat(controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::inspector_destroy_failed_not_found, ctx.lang), std::make_format_args(ctx.tid, name_view))));
                return response::failed;
            }

            port->second->inspectors().remove(it->second);
            ctx.thread_inspectors.erase(it);

            auto name_view = port_name.c_str();
            debug_statement(ctx.ctrl.log(log::trace, std::vformat(controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::inspector_destroyed, ctx.lang), std::make_format_args(ctx.tid, name_view))));
            return response::success;
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
            }
        };

        auto val    = static_cast<int>(event);
        auto it     = translations.find(val);

        if (it != translations.end())
            return it->second[static_cast<int>(lang)];
        
        return language_strings::unknown_type_message(_TYPEINFO "controller_cmd_dispatcher::log_event", val, lang);
    }
}