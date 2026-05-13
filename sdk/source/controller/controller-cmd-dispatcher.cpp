#include "controller/controller-cmd-dispatcher.hpp"
#include "controller/controller.hpp"
#include "controller/registry.hpp"
#include "resources/language-strings.hpp"
#include "data/port/port.hpp"
#include "data/inspector.hpp"
#include "commander/messages/event.hpp"

#include <array>
#include <format>
#include <vector>

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
        register_handler(static_cast<int>(command_type::receive_initial_data), [](const command*, size_t, command_context& ctx)
        {
            ctx.set_single_response_status(response_status::success);
            auto* data = ctx.responses.front().data_as<command::initial_data::header>();

            data->lang_info.lang                  = ctx.ctrl.get_language();
            data->lang_info.supported_languages   |= ( 1 << language_english );
            data->lang_info.supported_languages   |= ( 1 << language_german );

            data->mod_info.available_modules       = static_cast<uint32_t>(ctx.ctrl.modules().get_available_modules().size());
            data->mod_info.unavailable_modules     = static_cast<uint32_t>(ctx.ctrl.modules().get_unavailable_modules().size());
            data->mod_info.loaded_modules          = static_cast<uint32_t>(ctx.ctrl.modules().get_loaded_modules().size());

            size_t resp_idx = 1;

            for (const auto& mod : ctx.ctrl.modules().get_available_modules())
            {
                ctx.responses[resp_idx-1].set_extended(true);
                auto* mod_info = ctx.responses[resp_idx].data_as<command::initial_data::module_info>();
                mod_info->setup(command::initial_data::module_info::available, mod.first.c_str(), mod.second.second.c_str(), mod.second.first);
                resp_idx++;

                if (resp_idx >= ctx.responses.size())
                    ctx.responses.emplace_back();
            }

            for (const auto& mod : ctx.ctrl.modules().get_unavailable_modules())
            {
                ctx.responses[resp_idx-1].set_extended(true);
                auto* mod_info = ctx.responses[resp_idx].data_as<command::initial_data::module_info>();
                mod_info->setup(command::initial_data::module_info::unavailable, mod.first.c_str(), mod.second.second.c_str(), mod.second.first);
                resp_idx++;

                if (resp_idx >= ctx.responses.size())
                    ctx.responses.emplace_back();
            }

            for (const auto& mod : ctx.ctrl.modules().get_loaded_modules())
            {
                ctx.responses[resp_idx-1].set_extended(true);
                auto* mod_info = ctx.responses[resp_idx].data_as<command::initial_data::module_info>();
                mod_info->setup(command::initial_data::module_info::loaded, mod.first.c_str(), mod.second->get_filepath().c_str(), mod.second->get_version());
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

        register_handler(static_cast<int>(command_type::inspector_create), [](const command* cmds, size_t, command_context& ctx) 
        {
            auto params = cmds->get_data_as<command::inspector_create_data>();
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
            auto params = cmds->get_data_as<command::inspector_destroy_data>();
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
            }
        };

        auto val    = static_cast<int>(event);
        auto it     = translations.find(val);

        if (it != translations.end())
            return it->second[static_cast<int>(lang)];
        
        return language_strings::unknown_type_message("controller_cmd_dispatcher::log_event", val, lang);
    }
}