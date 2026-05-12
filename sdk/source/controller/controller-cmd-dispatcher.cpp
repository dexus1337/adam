#include "controller/controller-cmd-dispatcher.hpp"
#include "controller/controller.hpp"
#include "controller/registry.hpp"
#include "resources/language-strings.hpp"
#include "data/port/port.hpp"
#include "data/inspector.hpp"
#include "commander/messages/event.hpp"

#include <array>
#include <format>

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

    response controller_cmd_dispatcher::dispatch(const command* cmds, size_t count, command_context& ctx) const
    {
        if (!cmds || count == 0)
            return response(response_status::invalid);

        auto it = m_handlers.find(static_cast<int>(cmds[0].get_type()));
        if (it != m_handlers.end())
            return it->second(cmds, count, ctx);
        
        return response(response_status::unknown);
    }

    void controller_cmd_dispatcher::register_default_handlers()
    {
        register_handler(static_cast<int>(command_type::receive_initial_data), [](const command*, size_t, command_context& ctx) -> response
        {
            response res(response_status::success);
            auto* data = res.data_as<command::initial_data>();

            data->lang_info.lang                  = ctx.ctrl.get_language();
            data->lang_info.supported_languages   |= ( 1 << language_english );
            data->lang_info.supported_languages   |= ( 1 << language_german );

            return res;
        });

        register_handler(static_cast<int>(command_type::set_language), [](const command* cmds, size_t, command_context& ctx) -> response 
        {
            ctx.ctrl.set_language(*cmds[0].get_data_as<language>());
            ctx.ctrl.log(log::info, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::language_changed, ctx.ctrl.get_language()));

            event evt(event_type::language_changed);
            *evt.data_as<language>() = ctx.ctrl.get_language();
            ctx.ctrl.broadcast_event(evt);

            return response_status::success;
        });

        register_handler(static_cast<int>(command_type::inspector_create), [](const command* cmds, size_t, command_context& ctx) -> response 
        {
            auto params = cmds[0].get_data_as<command::inspector_create_data>();
            auto port = ctx.reg.ports().find(params->port);

            if (port == ctx.reg.ports().end())
            {
                uint64_t port_hash = static_cast<uint64_t>(params->port);
                debug_statement(ctx.ctrl.log(log::trace, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::inspector_create_failed_port_unknown, ctx.ctrl.get_language()), ctx.tid, port_hash));
                return response_status::unknown;
            }

            const auto& port_name = port->second->get_name();
            auto new_inspector = std::make_shared<data_inspector>();

            if (!new_inspector->open(port_name, ctx.tid))
            {
                auto name_view = port_name.c_str();
                debug_statement(ctx.ctrl.log(log::trace, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::inspector_create_failed_open, ctx.ctrl.get_language()), ctx.tid, name_view));
                return response_status::failed;
            }

            port->second->inspectors().push_back(new_inspector);
            ctx.thread_inspectors.emplace(params->port, new_inspector);

            auto name_view = port_name.c_str();
            debug_statement(ctx.ctrl.log(log::trace, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::inspector_created, ctx.ctrl.get_language()), ctx.tid, name_view));
            return response_status::success;
        });

        register_handler(static_cast<int>(command_type::inspector_destroy), [](const command* cmds, size_t, command_context& ctx) -> response 
        {
            auto params = cmds[0].get_data_as<command::inspector_destroy_data>();
            auto port = ctx.reg.ports().find(params->port);

            if (port == ctx.reg.ports().end())
            {
                uint64_t port_hash = static_cast<uint64_t>(params->port);
                debug_statement(ctx.ctrl.log(log::trace, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::inspector_destroy_failed_port_unknown, ctx.ctrl.get_language()), ctx.tid, port_hash));
                return response_status::unknown;
            }

            const auto& port_name = port->second->get_name();
            auto it = ctx.thread_inspectors.find(params->port);

            if (it == ctx.thread_inspectors.end())
            {
                auto name_view = port_name.c_str();
                debug_statement(ctx.ctrl.log(log::trace, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::inspector_destroy_failed_not_found, ctx.ctrl.get_language()), ctx.tid, name_view));
                return response_status::failed;
            }

            port->second->inspectors().remove(it->second);
            ctx.thread_inspectors.erase(it);

            auto name_view = port_name.c_str();
            debug_statement(ctx.ctrl.log(log::trace, controller_cmd_dispatcher::get_log_event_text(controller_cmd_dispatcher::log_event::inspector_destroyed, ctx.ctrl.get_language()), ctx.tid, name_view));
            return response_status::success;
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
        
        return language_strings::unknown_type_message("controller_cmd_dispatcher::log_event", val, lang);
    }
}