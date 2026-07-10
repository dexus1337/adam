#include "commander/commander-event-dispatcher.hpp"

#include "commander/commander.hpp"
#include "commander/messages/command.hpp"
#include "commander/messages/message-structs.hpp"
#include "commander/config-view.hpp"
#include "data/port.hpp"
#include "module/module.hpp"
#include "data/connection.hpp"
#include "memory/buffer/buffer-manager.hpp"
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
    
    void commander_event_dispatcher::dispatch(const event* events, size_t count, event_context& ctx) const
    {
        if (count == 0 || !events) return;
        auto it = m_handlers.find(events[0].get_type());
        if (it != m_handlers.end())
        {
            if (it->second)
            {
                it->second(events, count, ctx);
            }
        }
    }

    void commander_event_dispatcher::register_default_handlers()
    {
        register_handler(event_type::language_changed, [](const event* events, size_t, event_context& ctx) 
        {
            ctx.cmdr.m_lang.lang = events[0].get_data_as<messages::initial_data_header>()->lang_info.lang;
        });

        register_handler(event_type::module_path_added, [](const event* events, size_t, event_context& ctx) 
        {
            auto* data = events[0].get_data_as<messages::module_path_data>();
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

        register_handler(event_type::module_path_removed, [](const event* events, size_t, event_context& ctx) 
        {
            auto* data = events[0].get_data_as<messages::module_path_remove_data>();
            std::lock_guard<const module_view> lg(ctx.cmdr.modules());
            auto& paths = ctx.cmdr.modules().paths();
            if (data->idx < paths.size())
            {
                paths.erase(paths.begin() + data->idx);
            }
        });

        register_handler(event_type::config_path_added, [](const event* events, size_t, event_context& ctx) 
        {
            auto* data = events[0].get_data_as<messages::config_path_data>();
            std::lock_guard<const config_view> lg(ctx.cmdr.configs());
            auto& paths = ctx.cmdr.configs().paths();
            if (data->idx >= paths.size())
            {
                paths.push_back(data->path);
            }
            else
            {
                paths.insert(paths.begin() + data->idx, data->path);
            }
        });

        register_handler(event_type::config_path_removed, [](const event* events, size_t, event_context& ctx) 
        {
            auto* data = events[0].get_data_as<messages::config_path_remove_data>();
            std::lock_guard<const config_view> lg(ctx.cmdr.configs());
            auto& paths = ctx.cmdr.configs().paths();
            if (data->idx < paths.size())
            {
                paths.erase(paths.begin() + data->idx);
            }
        });

        register_handler(event_type::config_available, [](const event* events, size_t, event_context& ctx) 
        {
            auto* data = events[0].get_data_as<messages::config_info_data>();
            std::lock_guard<const config_view> lg(ctx.cmdr.configs());
            
            std::string path;
            const auto& paths = ctx.cmdr.configs().paths();
            if (data->path_idx < paths.size())
            {
                path = paths[data->path_idx].c_str();
                if (!path.empty() && path.back() != '/' && path.back() != '\\')
                {
                    path += "/";
                }
                path += data->filename;
            }
            else
            {
                path = data->filename;
            }

            config_info info;
            info.path_idx = data->path_idx;
            info.filename = data->filename;
            info.name = string_hashed(&data->name[0]);
            info.description = string_hashed(&data->description[0]);
            info.created = data->created;
            info.modified = data->modified;
            info.port_count = data->port_count;
            info.processor_count = data->processor_count;
            info.connection_count = data->connection_count;

            ctx.cmdr.configs().available()[string_hashed(path)] = info;
        });

        register_handler(event_type::module_loaded, [](const event* events, size_t, event_context& ctx) 
        {
            auto* mod_info = events[0].get_data_as<module::basic_info>();
            string_hashed mod_name(mod_info->name);
            string_hashed mod_path(mod_info->path);

            std::lock_guard<const module_view> lg(ctx.cmdr.modules());
            ctx.cmdr.modules().load_module(mod_name, mod_path, mod_info->version);
            ctx.cmdr.modules().available().erase(mod_name);
        });

        register_handler(event_type::module_unloaded, [](const event* events, size_t, event_context& ctx) 
        {
            auto* mod_info = events[0].get_data_as<module::basic_info>();
            string_hashed mod_name(mod_info->name);
            string_hashed mod_path(mod_info->path);

            std::lock_guard<const module_view> lg(ctx.cmdr.modules());
            ctx.cmdr.modules().unload_module(mod_name);
            ctx.cmdr.modules().available().emplace(mod_name, std::make_pair(mod_info->version, mod_path));
        });

        register_handler(event_type::module_available, [](const event* events, size_t, event_context& ctx) 
        {
            auto* mod_info = events[0].get_data_as<module::basic_info>();
            string_hashed mod_name(mod_info->name);
            string_hashed mod_path(mod_info->path);

            std::lock_guard<const module_view> lg(ctx.cmdr.modules());
            ctx.cmdr.modules().update_module_database(mod_name, mod_path, mod_info->version);
            ctx.cmdr.modules().available().emplace(mod_name, std::make_pair(mod_info->version, mod_path));
        });

        register_handler(event_type::module_unavailable, [](const event* events, size_t, event_context& ctx) 
        {
            auto* mod_info = events[0].get_data_as<module::basic_info>();
            string_hashed mod_name(mod_info->name);
            string_hashed mod_path(mod_info->path);

            std::lock_guard<const module_view> lg(ctx.cmdr.modules());
            ctx.cmdr.modules().unavailable().emplace(mod_name, std::make_tuple(mod_info->version, mod_path, mod_info->rsn));
        });

        register_handler(event_type::module_removed, [](const event* events, size_t, event_context& ctx) 
        {
            auto* mod_info = events[0].get_data_as<module::basic_info>();
            string_hashed mod_name(mod_info->name);

            std::lock_guard<const module_view> lg(ctx.cmdr.modules());
            ctx.cmdr.modules().unload_module(mod_name);
            ctx.cmdr.modules().available().erase(mod_name);
            ctx.cmdr.modules().unavailable().erase(mod_name);
        });

        register_handler(event_type::shutdown, [](const event*, size_t, event_context& ctx) 
        {
            ctx.cmdr.destroy();
        });

        register_handler(event_type::port_created, [](const event* events, size_t count, event_context& ctx) 
        {
            auto* info = events[0].get_data_as<port::basic_info>();
            auto view = std::make_unique<port_view>();
            view->name = string_hashed(info->name);
            view->direction = info->dir;
            view->is_unavailable = info->is_unavailable;
            view->started = info->started;
            if (!view->is_unavailable)
                view->statistic_buffer = buffer_manager::get().resolve_handle(info->state_buffer_handle);
            {
                std::lock_guard<const module_view> mod_lg(ctx.cmdr.modules());
                ctx.cmdr.get_modules().extract_port_type_and_module(info->type, info->type_module, view->type, view->type_module);
                
                const auto* mod = ctx.cmdr.get_modules().get_module(view->type_module.get_hash());
                if (mod)
                {
                    const auto& factories = mod->get_port_factories();
                    auto fact_it = factories.find(view->type);
                    if (fact_it != factories.end() && fact_it->second.parameters)
                    {
                        if (auto* factory_user_params = dynamic_cast<const configuration_parameter_list*>(fact_it->second.parameters->get("user_parameters"_ct)))
                        {
                            view->user_params = *factory_user_params;
                        }
                    }
                }
            }
            
            size_t evt_idx = 0;
            size_t unused_off = sizeof(port::basic_info);
            size_t unused_size = event::get_max_data_length() - unused_off;
            detail::message_deserializer<event> deserializer(events, count, evt_idx, unused_off, unused_size);
            detail::deserialize_user_parameters(info->user_parameters, deserializer, view->user_params);

            {
                std::lock_guard<const registry_view> reg_lg(ctx.cmdr.registry());
                ctx.cmdr.registry().ports()[view->name.get_hash()] = std::move(view);
            }
        });

        register_handler(event_type::port_available, [](const event* events, size_t, event_context& ctx) 
        {
            auto* info = events[0].get_data_as<port::basic_info>();
            
            bool found = false;
            {
                std::lock_guard<const registry_view> reg_lg(ctx.cmdr.registry());
                auto it = ctx.cmdr.registry().ports().find(string_hashed(info->name).get_hash());
                if (it != ctx.cmdr.registry().ports().end())
                {
                    it->second->is_unavailable = false;
                    it->second->direction = info->dir;
                    it->second->started = info->started;
                    if (it->second->statistic_buffer)
                    {
                        it->second->statistic_buffer->release();
                    }
                    it->second->statistic_buffer = buffer_manager::get().resolve_handle(info->state_buffer_handle);
                    found = true;
                }
            }
            
            if (!found)
            {
                auto view = std::make_unique<port_view>();
                view->name = string_hashed(info->name);
                view->direction = info->dir;
                view->is_unavailable = false;
                view->started = info->started;
                view->statistic_buffer = buffer_manager::get().resolve_handle(info->state_buffer_handle);
                
                {
                    std::lock_guard<const module_view> mod_lg(ctx.cmdr.modules());
                    ctx.cmdr.get_modules().extract_port_type_and_module(info->type, info->type_module, view->type, view->type_module);
                    
                    const auto* mod = ctx.cmdr.get_modules().get_module(view->type_module.get_hash());
                    if (mod)
                    {
                        const auto& factories = mod->get_port_factories();
                        auto fact_it = factories.find(view->type);
                        if (fact_it != factories.end() && fact_it->second.parameters)
                        {
                            if (auto* factory_user_params = dynamic_cast<const configuration_parameter_list*>(fact_it->second.parameters->get("user_parameters"_ct)))
                            {
                                view->user_params = *factory_user_params;
                            }
                        }
                    }
                }
                
                std::lock_guard<const registry_view> reg_lg(ctx.cmdr.registry());
                ctx.cmdr.registry().ports()[view->name.get_hash()] = std::move(view);
            }
        });

        register_handler(event_type::port_unavailable, [](const event* events, size_t, event_context& ctx) 
        {
            auto* data = events[0].get_data_as<messages::port_action_data>();
            std::lock_guard<const registry_view> lg(ctx.cmdr.registry());
            auto it = ctx.cmdr.registry().ports().find(data->port);
            if (it != ctx.cmdr.registry().ports().end())
            {
                it->second->is_unavailable = true;
                if (it->second->statistic_buffer)
                {
                    it->second->statistic_buffer->release();
                    it->second->statistic_buffer = nullptr;
                }
            }
        });

        register_handler(event_type::port_destroyed, [](const event* events, size_t, event_context& ctx) 
        {
            auto* data = events[0].get_data_as<messages::port_destroy_data>();
            std::lock_guard<const registry_view> lg(ctx.cmdr.registry());
            ctx.cmdr.registry().ports().erase(data->port);

            for (auto& [conn_hash, conn] : ctx.cmdr.registry().connections())
            {
                conn->inputs.erase(std::remove(conn->inputs.begin(), conn->inputs.end(), data->port), conn->inputs.end());
                conn->outputs.erase(std::remove(conn->outputs.begin(), conn->outputs.end(), data->port), conn->outputs.end());
            }
        });

        register_handler(event_type::port_started, [](const event* events, size_t, event_context& ctx) 
        {
            auto* data = events[0].get_data_as<messages::port_action_data>();
            std::lock_guard<const registry_view> lg(ctx.cmdr.registry());
            auto it = ctx.cmdr.registry().ports().find(data->port);
            if (it != ctx.cmdr.registry().ports().end()) it->second->started = true;
        });

        register_handler(event_type::port_stopped, [](const event* events, size_t, event_context& ctx) 
        {
            auto* data = events[0].get_data_as<messages::port_action_data>();
            std::lock_guard<const registry_view> lg(ctx.cmdr.registry());
            auto it = ctx.cmdr.registry().ports().find(data->port);
            if (it != ctx.cmdr.registry().ports().end()) it->second->started = false;
        });

        register_handler(event_type::port_renamed, [](const event* events, size_t, event_context& ctx) 
        {
            auto* data = events[0].get_data_as<messages::port_rename_data>();
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

        register_handler(event_type::port_parameter_updated, [](const event* events, size_t, event_context& ctx) 
        {
            auto* data = events[0].get_data_as<messages::port_parameter_updated_data>();
            std::lock_guard<const registry_view> lg(ctx.cmdr.registry());
            auto it = ctx.cmdr.registry().ports().find(data->port);
            if (it != ctx.cmdr.registry().ports().end())
            {
                auto* param = it->second->user_params.get(data->param_view.name);
                if (param && param->get_type() == data->param_view.var_type)
                {
                    switch (param->get_type())
                    {
                        case configuration_parameter::type_integer:
                        {
                            int64_t val;
                            std::memcpy(&val, data->data, sizeof(int64_t));
                            static_cast<configuration_parameter_integer*>(param)->set_value(val);
                            break;
                        }
                        case configuration_parameter::type_double:
                        {
                            double val;
                            std::memcpy(&val, data->data, sizeof(double));
                            static_cast<configuration_parameter_double*>(param)->set_value(val);
                            break;
                        }
                        case configuration_parameter::type_boolean:
                        {
                            bool val;
                            std::memcpy(&val, data->data, sizeof(bool));
                            static_cast<configuration_parameter_boolean*>(param)->set_value(val);
                            break;
                        }
                        case configuration_parameter::type_string:
                        {
                            uint16_t len;
                            std::memcpy(&len, data->data, sizeof(uint16_t));
                            std::string val(len, '\0');
                            if (len > 0)
                                std::memcpy(&val[0], data->data + sizeof(uint16_t), len);
                            static_cast<configuration_parameter_string*>(param)->set_value(string_hashed(val));
                            break;
                        }
                        default:
                            break;
                    }
                }
            }
        });

        register_handler(event_type::processor_created, [](const event* events, size_t count, event_context& ctx) 
        {
            auto* info = events[0].get_data_as<processor::basic_info>();
            auto view = std::make_unique<processor_view>();
            view->name = string_hashed(info->name);
            view->is_unavailable = info->is_unavailable;
            view->is_filter = (info->input_datatype == info->output_datatype);
            if (!view->is_unavailable)
                view->state_buffer = buffer_manager::get().resolve_handle(info->state_buffer_handle);

            {
                std::lock_guard<const module_view> mod_lg(ctx.cmdr.modules());
                ctx.cmdr.get_modules().extract_processor_type_and_module(info->type, info->type_module, view->type, view->module_name);
                string_hashed dummy_mod;
                ctx.cmdr.get_modules().extract_datatype_and_module(info->input_datatype, info->input_datatype_module, view->input_datatype, dummy_mod);
                ctx.cmdr.get_modules().extract_datatype_and_module(info->output_datatype, info->output_datatype_module, view->output_datatype, dummy_mod);
                
                const auto* mod = ctx.cmdr.get_modules().get_module(view->module_name.get_hash());
                if (mod)
                {
                    const auto& factories = mod->get_processor_factories();
                    auto fact_it = factories.find(view->type);
                    if (fact_it != factories.end() && fact_it->second.parameters)
                    {
                        if (auto* factory_user_params = dynamic_cast<const configuration_parameter_list*>(fact_it->second.parameters->get("user_parameters"_ct)))
                        {
                            view->user_params = *factory_user_params;
                        }
                    }
                }
            }
            
            size_t evt_idx = 0;
            size_t unused_off = sizeof(processor::basic_info);
            size_t unused_size = event::get_max_data_length() - unused_off;
            detail::message_deserializer<event> deserializer(events, count, evt_idx, unused_off, unused_size);
            detail::deserialize_user_parameters(info->user_parameters, deserializer, view->user_params);

            {
                std::lock_guard<const registry_view> reg_lg(ctx.cmdr.registry());
                ctx.cmdr.registry().processors()[view->name.get_hash()] = std::move(view);
            }
        });

        register_handler(event_type::processor_available, [](const event* events, size_t, event_context& ctx) 
        {
            auto* info = events[0].get_data_as<processor::basic_info>();
            
            bool found = false;
            {
                std::lock_guard<const registry_view> reg_lg(ctx.cmdr.registry());
                auto it = ctx.cmdr.registry().processors().find(string_hashed(info->name).get_hash());
                if (it != ctx.cmdr.registry().processors().end())
                {
                    it->second->is_unavailable = false;
                    if (it->second->state_buffer)
                    {
                        it->second->state_buffer->release();
                    }
                    it->second->state_buffer = buffer_manager::get().resolve_handle(info->state_buffer_handle);
                    found = true;
                }
            }
            
            if (!found)
            {
                auto view = std::make_unique<processor_view>();
                view->name = string_hashed(info->name);
                view->is_unavailable = false;
                view->is_filter = (info->input_datatype == info->output_datatype);
                view->state_buffer = buffer_manager::get().resolve_handle(info->state_buffer_handle);
                
                {
                    std::lock_guard<const module_view> mod_lg(ctx.cmdr.modules());
                    ctx.cmdr.get_modules().extract_processor_type_and_module(info->type, info->type_module, view->type, view->module_name);
                    string_hashed dummy_mod;
                    ctx.cmdr.get_modules().extract_datatype_and_module(info->input_datatype, info->input_datatype_module, view->input_datatype, dummy_mod);
                    ctx.cmdr.get_modules().extract_datatype_and_module(info->output_datatype, info->output_datatype_module, view->output_datatype, dummy_mod);
                    
                    const auto* mod = ctx.cmdr.get_modules().get_module(view->module_name.get_hash());
                    if (mod)
                    {
                        const auto& factories = mod->get_processor_factories();
                        auto fact_it = factories.find(view->type);
                        if (fact_it != factories.end() && fact_it->second.parameters)
                        {
                            if (auto* factory_user_params = dynamic_cast<const configuration_parameter_list*>(fact_it->second.parameters->get("user_parameters"_ct)))
                            {
                                view->user_params = *factory_user_params;
                            }
                        }
                    }
                }
                
                std::lock_guard<const registry_view> reg_lg(ctx.cmdr.registry());
                ctx.cmdr.registry().processors()[view->name.get_hash()] = std::move(view);
            }
        });

        register_handler(event_type::processor_unavailable, [](const event* events, size_t, event_context& ctx) 
        {
            auto* data = events[0].get_data_as<messages::processor_action_data>();
            std::lock_guard<const registry_view> lg(ctx.cmdr.registry());
            auto it = ctx.cmdr.registry().processors().find(data->processor);
            if (it != ctx.cmdr.registry().processors().end())
            {
                it->second->is_unavailable = true;
                if (it->second->state_buffer)
                {
                    it->second->state_buffer->release();
                    it->second->state_buffer = nullptr;
                }
            }
        });

        register_handler(event_type::processor_destroyed, [](const event* events, size_t, event_context& ctx) 
        {
            auto* data = events[0].get_data_as<messages::processor_action_data>();
            std::lock_guard<const registry_view> lg(ctx.cmdr.registry());
            ctx.cmdr.registry().processors().erase(data->processor);

            for (auto& [conn_hash, conn] : ctx.cmdr.registry().connections())
            {
                conn->processors.erase(std::remove(conn->processors.begin(), conn->processors.end(), data->processor), conn->processors.end());
            }
        });

        register_handler(event_type::processor_renamed, [](const event* events, size_t, event_context& ctx) 
        {
            auto* data = events[0].get_data_as<messages::processor_rename_data>();
            std::lock_guard<const registry_view> lg(ctx.cmdr.registry());
            auto it = ctx.cmdr.registry().processors().find(data->processor);
            if (it != ctx.cmdr.registry().processors().end())
            {
                auto view = std::move(it->second);
                ctx.cmdr.registry().processors().erase(it);
                
                string_hashed new_name(data->new_name);
                view->name = new_name;
                string_hash new_hash = new_name.get_hash();
                ctx.cmdr.registry().processors()[new_hash] = std::move(view);
                
                for (auto& [conn_hash, conn] : ctx.cmdr.registry().connections())
                {
                    for (auto& pid : conn->processors)
                    {
                        if (pid == data->processor)
                        {
                            pid = new_hash;
                            conn->edited = data->edited;
                        }
                    }
                }
            }
        });

        register_handler(event_type::processor_parameter_updated, [](const event* events, size_t, event_context& ctx) 
        {
            auto* data = events[0].get_data_as<messages::processor_parameter_updated_data>();
            std::lock_guard<const registry_view> lg(ctx.cmdr.registry());
            auto it = ctx.cmdr.registry().processors().find(data->processor);
            if (it != ctx.cmdr.registry().processors().end())
            {
                auto* param = it->second->user_params.get(data->param_view.name);
                if (param && param->get_type() == data->param_view.var_type)
                {
                    switch (param->get_type())
                    {
                        case configuration_parameter::type_integer:
                        {
                            int64_t val;
                            std::memcpy(&val, data->data, sizeof(int64_t));
                            static_cast<configuration_parameter_integer*>(param)->set_value(val);
                            break;
                        }
                        case configuration_parameter::type_double:
                        {
                            double val;
                            std::memcpy(&val, data->data, sizeof(double));
                            static_cast<configuration_parameter_double*>(param)->set_value(val);
                            break;
                        }
                        case configuration_parameter::type_boolean:
                        {
                            bool val;
                            std::memcpy(&val, data->data, sizeof(bool));
                            static_cast<configuration_parameter_boolean*>(param)->set_value(val);
                            break;
                        }
                        case configuration_parameter::type_string:
                        {
                            uint16_t len;
                            std::memcpy(&len, data->data, sizeof(uint16_t));
                            std::string val(len, '\0');
                            if (len > 0)
                                std::memcpy(&val[0], data->data + sizeof(uint16_t), len);
                            static_cast<configuration_parameter_string*>(param)->set_value(string_hashed(val));
                            break;
                        }
                        default:
                            break;
                    }
                }
            }
        });

        register_handler(event_type::connection_input_data_format_changed, [](const event* events, size_t, event_context& ctx) 
        {
            // Update the connection view with new format information
            auto* data = events[0].get_data_as<messages::connection_data_format_data>();
            std::lock_guard<const registry_view> reg_lg(ctx.cmdr.registry());
            auto it = ctx.cmdr.registry().connections().find(data->connection);
            if (it != ctx.cmdr.registry().connections().end())
            {
                std::lock_guard<const module_view> mod_lg(ctx.cmdr.modules());
                ctx.cmdr.get_modules().extract_datatype_and_module(data->format, data->format_module, it->second->input_format, it->second->input_format_module);
                it->second->valid_chain = data->valid_chain;
            }
        });

        register_handler(event_type::connection_output_data_format_changed, [](const event* events, size_t, event_context& ctx) 
        {
            // Update the connection view with new format information
            auto* data = events[0].get_data_as<messages::connection_data_format_data>();
            std::lock_guard<const registry_view> reg_lg(ctx.cmdr.registry());
            auto it = ctx.cmdr.registry().connections().find(data->connection);
            if (it != ctx.cmdr.registry().connections().end())
            {
                std::lock_guard<const module_view> mod_lg(ctx.cmdr.modules());
                ctx.cmdr.get_modules().extract_datatype_and_module(data->format, data->format_module, it->second->output_format, it->second->output_format_module);
                it->second->valid_chain = data->valid_chain;
            }
        });

        register_handler(event_type::connection_created, [](const event* events, size_t, event_context& ctx) 
        {
            auto* info = events[0].get_data_as<connection::basic_info>();
            auto view = std::make_unique<connection_view>();
            view->name = string_hashed(info->name);
            view->created = info->created;
            view->edited = info->edited;
            view->sorting_index = info->sorting_index;
            view->color = info->color;
            view->started = info->started;
            view->is_unavailable = info->is_unavailable;
            view->valid_chain = info->valid_chain;
            {
                // Populate format strings from name hashes
                std::lock_guard<const module_view> mod_lg(ctx.cmdr.modules());
                ctx.cmdr.get_modules().extract_datatype_and_module(info->input_format,  info->input_format_module,  view->input_format,  view->input_format_module);
                ctx.cmdr.get_modules().extract_datatype_and_module(info->output_format, info->output_format_module, view->output_format, view->output_format_module);
            }
            std::lock_guard<const registry_view> lg(ctx.cmdr.registry());
            ctx.cmdr.registry().connections()[view->name.get_hash()] = std::move(view);
        });

        register_handler(event_type::connection_destroyed, [](const event* events, size_t, event_context& ctx) 
        {
            auto* data = events[0].get_data_as<messages::connection_destroy_data>();
            std::lock_guard<const registry_view> lg(ctx.cmdr.registry());
            ctx.cmdr.registry().connections().erase(data->connection);
        });

        register_handler(event_type::connection_started, [](const event* events, size_t, event_context& ctx) 
        {
            auto* data = events[0].get_data_as<messages::connection_action_data>();
            std::lock_guard<const registry_view> lg(ctx.cmdr.registry());
            auto it = ctx.cmdr.registry().connections().find(data->connection);
            if (it != ctx.cmdr.registry().connections().end()) it->second->started = true;
        });

        register_handler(event_type::connection_stopped, [](const event* events, size_t, event_context& ctx) 
        {
            auto* data = events[0].get_data_as<messages::connection_action_data>();
            std::lock_guard<const registry_view> lg(ctx.cmdr.registry());
            auto it = ctx.cmdr.registry().connections().find(data->connection);
            if (it != ctx.cmdr.registry().connections().end()) it->second->started = false;
        });

        register_handler(event_type::connection_renamed, [](const event* events, size_t, event_context& ctx) 
        {
            auto* data = events[0].get_data_as<messages::connection_rename_data>();
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

        register_handler(event_type::connection_port_added, [](const event* events, size_t, event_context& ctx) 
        {
            auto* data = events[0].get_data_as<messages::connection_port_add_data>();
            std::lock_guard<const registry_view> lg(ctx.cmdr.registry());
            auto it = ctx.cmdr.registry().connections().find(data->connection);
            if (it != ctx.cmdr.registry().connections().end())
            {
                if (data->is_input)
                    it->second->inputs.push_back(data->port);
                else
                    it->second->outputs.push_back(data->port);
                it->second->edited = data->edited;
                it->second->valid_chain = data->valid_chain;
            }
        });

        register_handler(event_type::connection_port_removed, [](const event* events, size_t, event_context& ctx) 
        {
            auto* data = events[0].get_data_as<messages::connection_port_add_data>();
            std::lock_guard<const registry_view> lg(ctx.cmdr.registry());
            auto it = ctx.cmdr.registry().connections().find(data->connection);
            if (it != ctx.cmdr.registry().connections().end())
            {
                if (data->is_input)
                {
                    it->second->inputs.erase(std::remove(it->second->inputs.begin(), it->second->inputs.end(), data->port), it->second->inputs.end());
                }
                else
                {
                    it->second->outputs.erase(std::remove(it->second->outputs.begin(), it->second->outputs.end(), data->port), it->second->outputs.end());
                }
                it->second->edited = data->edited;
                it->second->valid_chain = data->valid_chain;
            }
        });

        register_handler(event_type::connection_processor_added, [](const event* events, size_t, event_context& ctx) 
        {
            auto* data = events[0].get_data_as<messages::connection_processor_add_data>();
            std::lock_guard<const registry_view> lg(ctx.cmdr.registry());
            auto it = ctx.cmdr.registry().connections().find(data->connection);
            if (it != ctx.cmdr.registry().connections().end())
            {
                it->second->processors.push_back(data->processor);
                it->second->edited = data->edited;
                it->second->valid_chain = data->valid_chain;
            }
        });

        register_handler(event_type::connection_processor_removed, [](const event* events, size_t, event_context& ctx) 
        {
            auto* data = events[0].get_data_as<messages::connection_processor_add_data>();
            std::lock_guard<const registry_view> lg(ctx.cmdr.registry());
            auto it = ctx.cmdr.registry().connections().find(data->connection);
            if (it != ctx.cmdr.registry().connections().end())
            {
                it->second->processors.erase(std::remove(it->second->processors.begin(), it->second->processors.end(), data->processor), it->second->processors.end());
                it->second->edited = data->edited;
                it->second->valid_chain = data->valid_chain;
            }
        });

        register_handler(event_type::connection_processor_reordered, [](const event* events, size_t, event_context& ctx) 
        {
            auto* data = events[0].get_data_as<messages::connection_processor_reorder_data>();
            std::lock_guard<const registry_view> lg(ctx.cmdr.registry());
            auto it = ctx.cmdr.registry().connections().find(data->connection);
            if (it != ctx.cmdr.registry().connections().end())
            {
                auto& procs = it->second->processors;
                auto pit = std::find(procs.begin(), procs.end(), data->processor);
                if (pit != procs.end())
                {
                    procs.erase(pit);
                }
                uint32_t new_idx = data->new_index;
                if (new_idx > procs.size())
                    new_idx = static_cast<uint32_t>(procs.size());
                procs.insert(procs.begin() + new_idx, data->processor);

                it->second->edited = data->edited;
                it->second->valid_chain = data->valid_chain;
            }
        });

        register_handler(event_type::connection_sorting_index_changed, [](const event* events, size_t, event_context& ctx) 
        {
            auto* data = events[0].get_data_as<messages::connection_property_change_data>();
            std::lock_guard<const registry_view> lg(ctx.cmdr.registry());
            auto it = ctx.cmdr.registry().connections().find(data->connection);
            if (it != ctx.cmdr.registry().connections().end())
            {
                it->second->sorting_index = data->value;
                it->second->edited = data->edited;
            }
        });

        register_handler(event_type::connection_color_changed, [](const event* events, size_t, event_context& ctx) 
        {
            auto* data = events[0].get_data_as<messages::connection_property_change_data>();
            std::lock_guard<const registry_view> lg(ctx.cmdr.registry());
            auto it = ctx.cmdr.registry().connections().find(data->connection);
            if (it != ctx.cmdr.registry().connections().end())
            {
                it->second->color = data->value;
                it->second->edited = data->edited;
            }
        });

        register_handler(event_type::connection_available, [](const event* events, size_t, event_context& ctx) 
        {
            auto* data = events[0].get_data_as<connection::basic_info>();
            string_hashed name(data->name);

            std::lock_guard<const registry_view> reg_lg(ctx.cmdr.registry());
            
            auto it = ctx.cmdr.registry().connections().find(name.get_hash());
            if (it != ctx.cmdr.registry().connections().end())
            {
                it->second->is_unavailable = data->is_unavailable;
                it->second->valid_chain = data->valid_chain;
                it->second->started = data->started;

                std::lock_guard<const module_view> mod_lg(ctx.cmdr.modules());
                ctx.cmdr.get_modules().extract_datatype_and_module(data->input_format, data->input_format_module, it->second->input_format, it->second->input_format_module);
                ctx.cmdr.get_modules().extract_datatype_and_module(data->output_format, data->output_format_module, it->second->output_format, it->second->output_format_module);

                it->second->inputs.clear();
                for (size_t i = 0; i < data->input_count; ++i)
                    it->second->inputs.push_back(data->inputs[i]);

                it->second->processors.clear();
                for (size_t i = 0; i < data->processor_count; ++i)
                    it->second->processors.push_back(data->processors[i]);

                it->second->outputs.clear();
                for (size_t i = 0; i < data->output_count; ++i)
                    it->second->outputs.push_back(data->outputs[i]);
            }
            else
            {
                auto new_conn = std::make_unique<connection_view>();
                new_conn->name = name;
                new_conn->created = data->created;
                new_conn->edited = data->edited;
                new_conn->sorting_index = data->sorting_index;
                new_conn->color = data->color;
                
                new_conn->started = data->started;
                new_conn->valid_chain = data->valid_chain;
                new_conn->is_unavailable = data->is_unavailable;

                std::lock_guard<const module_view> mod_lg(ctx.cmdr.modules());
                ctx.cmdr.get_modules().extract_datatype_and_module(data->input_format, data->input_format_module, new_conn->input_format, new_conn->input_format_module);
                ctx.cmdr.get_modules().extract_datatype_and_module(data->output_format, data->output_format_module, new_conn->output_format, new_conn->output_format_module);

                for (size_t i = 0; i < data->input_count; ++i)
                    new_conn->inputs.push_back(data->inputs[i]);

                for (size_t i = 0; i < data->processor_count; ++i)
                    new_conn->processors.push_back(data->processors[i]);

                for (size_t i = 0; i < data->output_count; ++i)
                    new_conn->outputs.push_back(data->outputs[i]);

                ctx.cmdr.registry().connections().emplace(name.get_hash(), std::move(new_conn));
            }
        });

        register_handler(event_type::connection_unavailable, [](const event* events, size_t, event_context& ctx) 
        {
            auto* data = events[0].get_data_as<messages::connection_action_data>();

            std::lock_guard<const registry_view> lg(ctx.cmdr.registry());
            auto it = ctx.cmdr.registry().connections().find(data->connection);
            if (it != ctx.cmdr.registry().connections().end())
            {
                it->second->is_unavailable = true;
                it->second->valid_chain = false;
                it->second->started = false;
            }
        });
    }
}