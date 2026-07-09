#include "default-commands.hpp"
#include "cli-strings.hpp"

#include <iostream>
#include <iomanip>
#include <algorithm>

namespace adam::cli
{
    void register_default_commands(command_database& db, adam::logger_sink& lgsnk)
    {
        command_database* p_db = &db;
        adam::logger_sink* p_lgsnk = &lgsnk;

        db.register_command("help", cmd_string_id::desc_help, [p_db](const std::vector<std::string>&, adam::commander& c, std::mutex& console_mutex) 
        {
            std::lock_guard<std::mutex> lock(console_mutex);
            std::cout << "\r\033[2K\n" << get_cli_string(cmd_string_id::available_commands, c.get_language()) << "\n";
            
            std::vector<std::pair<std::string, std::string>> cmds;
            for (const auto& [name, info] : p_db->get_commands())
            {
                cmds.push_back({name, get_cli_string(info.desc_id, c.get_language())});
            }
            std::sort(cmds.begin(), cmds.end());

            for (const auto& [name, desc] : cmds)
            {
                std::cout << "  " << std::left << std::setw(15) << name << " - " << desc << "\n";
            }
            std::cout << std::endl;
        });

        db.register_command("setlang", cmd_string_id::desc_setlang, [](const std::vector<std::string>& params, adam::commander& c, std::mutex& console_mutex) 
        {
            if (params.size() == 1 && params[0] == "de")
            {
                c.request_language_change(adam::language_german);
            }
            else if (params.size() == 1 && params[0] == "en")
            {
                c.request_language_change(adam::language_english);
            }
            else
            {
                std::lock_guard<std::mutex> lock(console_mutex);
                adam::stream_log(adam::log::warning, adam::cli::get_cli_string(adam::cli::cmd_string_id::usage_setlang, c.get_language()), std::cout);
            }
        });

        db.register_command("loglvl", cmd_string_id::desc_loglvl, [p_lgsnk](const std::vector<std::string>& params, adam::commander& c, std::mutex& console_mutex) 
        {
            if (params.size() == 1)
            {
                const std::string& val = params[0];
                adam::log::level new_level = adam::log::level::info;
                bool valid = true;

                if (val == "0" || val == "trace") new_level = adam::log::level::trace;
                else if (val == "1" || val == "info") new_level = adam::log::level::info;
                else if (val == "2" || val == "warn" || val == "warning") new_level = adam::log::level::warning;
                else if (val == "3" || val == "error") new_level = adam::log::level::error;
                else valid = false;

                if (valid)
                {
                    if (p_lgsnk->queue().metadata())
                    {
                        p_lgsnk->queue().metadata()->store(new_level, std::memory_order_relaxed);
                        std::lock_guard<std::mutex> lock(console_mutex);
                        adam::stream_log(adam::log::info, adam::cli::get_cli_string(adam::cli::cmd_string_id::log_level_updated, c.get_language()), std::cout);
                    }
                    return;
                }
            }
            
            std::lock_guard<std::mutex> lock(console_mutex);
            adam::stream_log(adam::log::warning, adam::cli::get_cli_string(adam::cli::cmd_string_id::usage_loglvl, c.get_language()), std::cout);
        });
        
        // MODULES
        db.register_command("mod_scan", cmd_string_id::desc_mod_scan, [](const std::vector<std::string>&, adam::commander& c, std::mutex&) 
        {
            c.request_module_scan();
        });

        db.register_command("mod_load", cmd_string_id::desc_mod_load, [](const std::vector<std::string>& params, adam::commander& c, std::mutex& console_mutex) 
        {
            if (params.size() == 1) c.request_module_load(adam::string_hashed(params[0].c_str()));
            else { std::lock_guard<std::mutex> lock(console_mutex); adam::stream_log(adam::log::warning, get_cli_string(cmd_string_id::usage_mod_load, c.get_language()), std::cout); }
        });

        db.register_command("mod_unload", cmd_string_id::desc_mod_unload, [](const std::vector<std::string>& params, adam::commander& c, std::mutex& console_mutex) 
        {
            if (params.size() == 1) c.request_module_unload(adam::string_hashed(params[0].c_str()));
            else { std::lock_guard<std::mutex> lock(console_mutex); adam::stream_log(adam::log::warning, get_cli_string(cmd_string_id::usage_mod_unload, c.get_language()), std::cout); }
        });

        db.register_command("mod_list", cmd_string_id::desc_mod_list, [](const std::vector<std::string>&, adam::commander& c, std::mutex& console_mutex) 
        {
            std::lock_guard<std::mutex> lock(console_mutex);
            std::cout << "\r\033[2K\nLoaded Modules:\n";
            for (const auto& [name, mod] : c.get_modules().get_loaded()) std::cout << "  " << name.c_str() << " (" << mod.second->get_filepath().c_str() << ")\n";
            std::cout << "Available Modules:\n";
            for (const auto& [name, mod] : c.get_modules().get_available()) std::cout << "  " << name.c_str() << " (" << mod.second.c_str() << ")\n";
            std::cout << "Unavailable Modules:\n";
            for (const auto& [name, mod] : c.get_modules().get_unavailable()) std::cout << "  " << name.c_str() << " (" << std::get<1>(mod).c_str() << ")\n";
            std::cout << std::endl;
        });

        db.register_command("mod_path_add", cmd_string_id::desc_mod_path_add, [](const std::vector<std::string>& params, adam::commander& c, std::mutex& console_mutex) 
        {
            if (params.size() == 1) c.request_module_path_add(adam::string_hashed(params[0].c_str()));
            else { std::lock_guard<std::mutex> lock(console_mutex); adam::stream_log(adam::log::warning, get_cli_string(cmd_string_id::usage_mod_path_add, c.get_language()), std::cout); }
        });

        db.register_command("mod_path_list", cmd_string_id::desc_mod_path_list, [](const std::vector<std::string>&, adam::commander& c, std::mutex& console_mutex) 
        {
            auto paths = c.get_modules().get_paths();
            std::lock_guard<std::mutex> lock(console_mutex);
            if (paths.empty())
            {
                std::cout << "No module paths found.\n";
                return;
            }
            std::cout << "Module Paths:\n";
            for (size_t i = 0; i < paths.size(); ++i)
                std::cout << "  [" << i << "] " << paths[i].c_str() << "\n";
            std::cout << std::endl;
        });

        db.register_command("mod_path_rm", cmd_string_id::desc_mod_path_rm, [](const std::vector<std::string>& params, adam::commander& c, std::mutex& console_mutex) 
        {
            if (params.size() == 1) 
            {
                try { c.request_module_path_remove(std::stoul(params[0])); }
                catch (...) { std::lock_guard<std::mutex> lock(console_mutex); adam::stream_log(adam::log::warning, get_cli_string(cmd_string_id::usage_mod_path_rm, c.get_language()), std::cout); }
            }
            else { std::lock_guard<std::mutex> lock(console_mutex); adam::stream_log(adam::log::warning, get_cli_string(cmd_string_id::usage_mod_path_rm, c.get_language()), std::cout); }
        });

        // PORTS
        db.register_command("port_create", cmd_string_id::desc_port_create, [](const std::vector<std::string>& params, adam::commander& c, std::mutex& console_mutex) 
        {
            if (params.size() >= 2 && params.size() <= 3) 
            {
                adam::string_hash mod_hash = (params.size() == 3) ? adam::string_hashed(params[2].c_str()).get_hash() : 0;
                c.request_port_create(adam::string_hashed(params[0].c_str()), adam::string_hashed(params[1].c_str()).get_hash(), mod_hash);
            } 
            else { std::lock_guard<std::mutex> lock(console_mutex); adam::stream_log(adam::log::warning, get_cli_string(cmd_string_id::usage_port_create, c.get_language()), std::cout); }
        });

        db.register_command("port_destroy", cmd_string_id::desc_port_destroy, [](const std::vector<std::string>& params, adam::commander& c, std::mutex& console_mutex) 
        {
            if (params.size() == 1) c.request_port_destroy(adam::string_hashed(params[0].c_str()).get_hash());
            else { std::lock_guard<std::mutex> lock(console_mutex); adam::stream_log(adam::log::warning, get_cli_string(cmd_string_id::usage_port_destroy, c.get_language()), std::cout); }
        });

        db.register_command("port_start", cmd_string_id::desc_port_start, [](const std::vector<std::string>& params, adam::commander& c, std::mutex& console_mutex) 
        {
            if (params.size() == 1) c.request_port_start(adam::string_hashed(params[0].c_str()).get_hash());
            else { std::lock_guard<std::mutex> lock(console_mutex); adam::stream_log(adam::log::warning, get_cli_string(cmd_string_id::usage_port_start, c.get_language()), std::cout); }
        });

        db.register_command("port_stop", cmd_string_id::desc_port_stop, [](const std::vector<std::string>& params, adam::commander& c, std::mutex& console_mutex) 
        {
            if (params.size() == 1) c.request_port_stop(adam::string_hashed(params[0].c_str()).get_hash());
            else { std::lock_guard<std::mutex> lock(console_mutex); adam::stream_log(adam::log::warning, get_cli_string(cmd_string_id::usage_port_stop, c.get_language()), std::cout); }
        });

        db.register_command("port_list", cmd_string_id::desc_port_list, [](const std::vector<std::string>&, adam::commander& c, std::mutex& console_mutex) 
        {
            std::lock_guard<std::mutex> lock(console_mutex);
            std::cout << "\r\033[2K\nPorts:\n";
            bool first = true;
            for (const auto& [hash, p] : c.get_registry().get_ports()) 
            {
                if (!first) std::cout << "\n";
                first = false;
                std::cout << "  " << p->name.c_str() << " (Type: " << p->type.c_str() << ", Started: " << (p->started ? "Yes" : "No") << ")\n";
                std::vector<std::string> used_in;
                std::vector<std::string> used_out;
                for (const auto& [chash, conn] : c.get_registry().get_connections())
                {
                    if (std::find(conn->inputs.begin(), conn->inputs.end(), hash) != conn->inputs.end()) used_in.push_back(conn->name.c_str());
                    if (std::find(conn->outputs.begin(), conn->outputs.end(), hash) != conn->outputs.end()) used_out.push_back(conn->name.c_str());
                }
                if (!used_in.empty() || !used_out.empty())
                {
                    std::cout << "    Connections:\n";
                    for (const auto& cn : used_in) std::cout << "      " << cn << " [\033[94mInput\033[0m]\n";
                    for (const auto& cn : used_out) std::cout << "      " << cn << " [\033[91mOutput\033[0m]\n";
                }
            }
            std::cout << std::endl;
        });

        db.register_command("port_rename", cmd_string_id::desc_port_rename, [](const std::vector<std::string>& params, adam::commander& c, std::mutex& console_mutex) 
        {
            if (params.size() == 2) c.request_port_rename(adam::string_hashed(params[0].c_str()).get_hash(), adam::string_hashed(params[1].c_str()));
            else { std::lock_guard<std::mutex> lock(console_mutex); adam::stream_log(adam::log::warning, get_cli_string(cmd_string_id::usage_port_rename, c.get_language()), std::cout); }
        });

        db.register_command("port_params", cmd_string_id::desc_port_params, [](const std::vector<std::string>& params, adam::commander& c, std::mutex& console_mutex) 
        {
            if (params.size() == 1) 
            {
                auto it = c.get_registry().get_ports().find(adam::string_hashed(params[0].c_str()).get_hash());
                if (it != c.get_registry().get_ports().end())
                {
                    std::lock_guard<std::mutex> lock(console_mutex);
                    std::cout << "\r\033[2K\nParameters for port '" << it->second->name.c_str() << "':\n";
                    if (it->second->user_params.get_children().empty()) std::cout << "  <none>\n";
                    for (const auto& [hash, param] : it->second->user_params.get_children())
                    {
                        std::cout << "  " << param->get_name().c_str() << " (";
                        if (param->get_type() == adam::configuration_parameter::type_integer) 
                        {
                            auto* p = static_cast<adam::configuration_parameter_integer*>(param.get());
                            std::cout << "int): " << p->get_value();
                            if (p->get_mode() == adam::configuration_parameter_integer::value_mode_range)
                                std::cout << " [" << p->get_min_value() << " - " << p->get_max_value() << "]";
                        }
                        else if (param->get_type() == adam::configuration_parameter::type_double) 
                        {
                            auto* p = static_cast<adam::configuration_parameter_double*>(param.get());
                            std::cout << "double): " << p->get_value();
                            if (p->get_mode() == adam::configuration_parameter_double::value_mode_range)
                                std::cout << " [" << p->get_min_value() << " - " << p->get_max_value() << "]";
                        }
                        else if (param->get_type() == adam::configuration_parameter::type_string) 
                        {
                            auto* p = static_cast<adam::configuration_parameter_string*>(param.get());
                            std::cout << "string): " << p->get_value().c_str();
                        }
                        else if (param->get_type() == adam::configuration_parameter::type_boolean) 
                        {
                            auto* p = static_cast<adam::configuration_parameter_boolean*>(param.get());
                            std::cout << "bool): " << (p->get_value() ? "true" : "false");
                        }
                        std::cout << "\n";
                    }
                    std::cout << std::endl;
                }
                else 
                {
                    std::lock_guard<std::mutex> lock(console_mutex); 
                    adam::stream_log(adam::log::error, "Port not found.", std::cout);
                }
            }
            else { std::lock_guard<std::mutex> lock(console_mutex); adam::stream_log(adam::log::warning, get_cli_string(cmd_string_id::usage_port_params, c.get_language()), std::cout); }
        });

        db.register_command("port_set_param", cmd_string_id::desc_port_set_param, [](const std::vector<std::string>& params, adam::commander& c, std::mutex& console_mutex) 
        {
            if (params.size() >= 2) 
            {
                auto port_hash = adam::string_hashed(params[0].c_str()).get_hash();
                auto it = c.get_registry().get_ports().find(port_hash);

                if (it != c.get_registry().get_ports().end())
                {
                    auto param_name = adam::string_hashed(params[1].c_str());
                    
                    std::string val_str = "";
                    if (params.size() >= 3)
                    {
                        val_str = params[2];
                        for (size_t i = 3; i < params.size(); ++i) val_str += " " + params[i];
                    }
                    
                    auto* param = it->second->user_params.get(param_name.get_hash());
                    if (param)
                    {
                        bool valid = false;
                        try 
                        {
                            if (param->get_type() == adam::configuration_parameter::type_integer) 
                            {
                                int64_t v = std::stoll(val_str);
                                if (static_cast<adam::configuration_parameter_integer*>(param)->set_value(v))
                                {
                                    valid = true;
                                    c.request_port_parameter_set(port_hash, param_name, v);
                                }
                            }
                            else if (param->get_type() == adam::configuration_parameter::type_double) 
                            {
                                double v = std::stod(val_str);
                                if (static_cast<adam::configuration_parameter_double*>(param)->set_value(v))
                                {
                                    valid = true;
                                    c.request_port_parameter_set(port_hash, param_name, v);
                                }
                            }
                            else if (param->get_type() == adam::configuration_parameter::type_string) 
                            {
                                adam::string_hashed v(val_str.c_str());
                                if (static_cast<adam::configuration_parameter_string*>(param)->set_value(v))
                                {
                                    valid = true;
                                    c.request_port_parameter_set(port_hash, param_name, v);
                                }
                            }
                            else if (param->get_type() == adam::configuration_parameter::type_boolean) 
                            {
                                bool v = (val_str == "true" || val_str == "1");
                                static_cast<adam::configuration_parameter_boolean*>(param)->set_value(v);
                                valid = true;
                                c.request_port_parameter_set(port_hash, param_name, v);
                            }
                        } catch (...) {}
                        
                        if (!valid) { std::lock_guard<std::mutex> lock(console_mutex); adam::stream_log(adam::log::error, get_cli_string(cmd_string_id::err_param_invalid, c.get_language()), std::cout); }
                    }
                    else { std::lock_guard<std::mutex> lock(console_mutex); adam::stream_log(adam::log::error, get_cli_string(cmd_string_id::err_param_not_found, c.get_language()), std::cout); }
                }
                else 
                {
                    std::lock_guard<std::mutex> lock(console_mutex); 
                    adam::stream_log(adam::log::error, "Port not found.", std::cout);
                }
            }
            else { std::lock_guard<std::mutex> lock(console_mutex); adam::stream_log(adam::log::warning, get_cli_string(cmd_string_id::usage_port_set_param, c.get_language()), std::cout); }
        });

        db.register_command("port_inject", cmd_string_id::desc_port_inject, [](const std::vector<std::string>& params, adam::commander& c, std::mutex& console_mutex) 
        {
            if (params.size() >= 3) 
            {
                auto it = c.get_registry().get_ports().find(adam::string_hashed(params[0].c_str()).get_hash());

                if (it != c.get_registry().get_ports().end())
                {
                    bool is_input = (params[2] == "in");
                    
                    std::string hex_str = params[1];
                    for (size_t i = 3; i < params.size(); ++i) hex_str += " " + params[i];
                    
                    auto parse_hex_bytes = [](const std::string& input) -> std::pair<bool, std::vector<uint8_t>>
                    {
                        std::vector<uint8_t> bytes;
                        bool high_nibble = true;
                        uint8_t current_byte = 0;
                        for (char ch : input)
                        {
                            if (std::isspace(ch)) continue;
                            uint8_t val = 0;
                            if (ch >= '0' && ch <= '9') val = ch - '0';
                            else if (ch >= 'a' && ch <= 'f') val = ch - 'a' + 10;
                            else if (ch >= 'A' && ch <= 'F') val = ch - 'A' + 10;
                            else return {false, {}}; 
                            
                            if (high_nibble) { current_byte = val << 4; high_nibble = false; }
                            else { current_byte |= val; bytes.push_back(current_byte); high_nibble = true; }
                        }
                        if (!high_nibble) return {false, {}};
                        return {true, bytes};
                    };

                    auto res = parse_hex_bytes(hex_str);
                    if (res.first && !res.second.empty())
                    {
                        c.request_port_inject_data(it->first, res.second.data(), res.second.size(), is_input ? adam::data_direction_in : adam::data_direction_out);
                    }
                    else { std::lock_guard<std::mutex> lock(console_mutex); adam::stream_log(adam::log::error, get_cli_string(cmd_string_id::err_param_invalid, c.get_language()), std::cout); }
                }
                else 
                {
                    std::lock_guard<std::mutex> lock(console_mutex); 
                    adam::stream_log(adam::log::error, "Port not found.", std::cout);
                }
            }
            else { std::lock_guard<std::mutex> lock(console_mutex); adam::stream_log(adam::log::warning, get_cli_string(cmd_string_id::usage_port_inject, c.get_language()), std::cout); }
        });

        // CONNECTIONS
        db.register_command("conn_create", cmd_string_id::desc_conn_create, [](const std::vector<std::string>& params, adam::commander& c, std::mutex& console_mutex) 
        {
            if (params.size() == 1) c.request_connection_create(adam::string_hashed(params[0].c_str()));
            else { std::lock_guard<std::mutex> lock(console_mutex); adam::stream_log(adam::log::warning, get_cli_string(cmd_string_id::usage_conn_create, c.get_language()), std::cout); }
        });

        db.register_command("conn_destroy", cmd_string_id::desc_conn_destroy, [](const std::vector<std::string>& params, adam::commander& c, std::mutex& console_mutex) 
        {
            if (params.size() == 1) c.request_connection_destroy(adam::string_hashed(params[0].c_str()).get_hash());
            else { std::lock_guard<std::mutex> lock(console_mutex); adam::stream_log(adam::log::warning, get_cli_string(cmd_string_id::usage_conn_destroy, c.get_language()), std::cout); }
        });

        db.register_command("conn_start", cmd_string_id::desc_conn_start, [](const std::vector<std::string>& params, adam::commander& c, std::mutex& console_mutex) 
        {
            if (params.size() == 1) c.request_connection_start(adam::string_hashed(params[0].c_str()).get_hash());
            else { std::lock_guard<std::mutex> lock(console_mutex); adam::stream_log(adam::log::warning, get_cli_string(cmd_string_id::usage_conn_start, c.get_language()), std::cout); }
        });

        db.register_command("conn_stop", cmd_string_id::desc_conn_stop, [](const std::vector<std::string>& params, adam::commander& c, std::mutex& console_mutex) 
        {
            if (params.size() == 1) c.request_connection_stop(adam::string_hashed(params[0].c_str()).get_hash());
            else { std::lock_guard<std::mutex> lock(console_mutex); adam::stream_log(adam::log::warning, get_cli_string(cmd_string_id::usage_conn_stop, c.get_language()), std::cout); }
        });

        db.register_command("conn_add_port", cmd_string_id::desc_conn_add_port, [](const std::vector<std::string>& params, adam::commander& c, std::mutex& console_mutex) 
        {
            if (params.size() == 3) 
            {
                bool is_input = (params[2] == "in");
                c.request_connection_port_add(adam::string_hashed(params[0].c_str()).get_hash(), adam::string_hashed(params[1].c_str()).get_hash(), is_input);
            } 
            else { std::lock_guard<std::mutex> lock(console_mutex); adam::stream_log(adam::log::warning, get_cli_string(cmd_string_id::usage_conn_add_port, c.get_language()), std::cout); }
        });

        db.register_command("conn_rm_port", cmd_string_id::desc_conn_rm_port, [](const std::vector<std::string>& params, adam::commander& c, std::mutex& console_mutex) 
        {
            if (params.size() == 3) 
            {
                bool is_input = (params[2] == "in");
                c.request_connection_port_remove(adam::string_hashed(params[0].c_str()).get_hash(), adam::string_hashed(params[1].c_str()).get_hash(), is_input);
            } 
            else { std::lock_guard<std::mutex> lock(console_mutex); adam::stream_log(adam::log::warning, get_cli_string(cmd_string_id::usage_conn_rm_port, c.get_language()), std::cout); }
        });

        db.register_command("conn_list", cmd_string_id::desc_conn_list, [](const std::vector<std::string>&, adam::commander& c, std::mutex& console_mutex) 
        {
            std::lock_guard<std::mutex> lock(console_mutex);
            std::cout << "\r\033[2K\nConnections:\n";
            bool first = true;
            for (const auto& [hash, conn] : c.get_registry().get_connections()) 
            {
                if (!first) std::cout << "\n";
                first = false;
                std::cout << "  " << conn->name.c_str() << " (Started: " << (conn->started ? "Yes" : "No") << ")\n";
                std::cout << "    \033[94mInputs:\033[0m\n";
                if (conn->inputs.empty()) std::cout << "      <none>\n";
                for (auto ph : conn->inputs)
                {
                    auto it = c.get_registry().get_ports().find(ph);
                    if (it != c.get_registry().get_ports().end()) std::cout << "      \033[94m" << it->second->name.c_str() << "\033[0m\n";
                    else std::cout << "      \033[94m<unknown:" << ph << ">\033[0m\n";
                }
                std::cout << "    \033[96mProcessors:\033[0m\n";
                if (conn->processors.empty()) std::cout << "      <none>\n";
                for (auto prh : conn->processors)
                {
                    auto it = c.get_registry().get_processors().find(prh);
                    if (it != c.get_registry().get_processors().end())
                    {
                        std::cout << "      \033[96m" << it->second->name.c_str() << "\033[0m";
                        if (it->second->is_filter) std::cout << " [\033[92mFilter\033[0m]\n";
                        else std::cout << " [\033[95mConverter\033[0m]\n";
                    }
                    else std::cout << "      \033[96m<unknown:" << prh << ">\033[0m\n";
                }
                std::cout << "    \033[91mOutputs:\033[0m\n";
                if (conn->outputs.empty()) std::cout << "      <none>\n";
                for (auto ph : conn->outputs)
                {
                    auto it = c.get_registry().get_ports().find(ph);
                    if (it != c.get_registry().get_ports().end()) std::cout << "      \033[91m" << it->second->name.c_str() << "\033[0m\n";
                    else std::cout << "      \033[91m<unknown:" << ph << ">\033[0m\n";
                }
            }
            std::cout << std::endl;
        });

        db.register_command("conn_rename", cmd_string_id::desc_conn_rename, [](const std::vector<std::string>& params, adam::commander& c, std::mutex& console_mutex) 
        {
            if (params.size() == 2) c.request_connection_rename(adam::string_hashed(params[0].c_str()).get_hash(), adam::string_hashed(params[1].c_str()));
            else { std::lock_guard<std::mutex> lock(console_mutex); adam::stream_log(adam::log::warning, get_cli_string(cmd_string_id::usage_conn_rename, c.get_language()), std::cout); }
        });

        db.register_command("conn_set_fmt_in", cmd_string_id::desc_conn_set_fmt_in, [](const std::vector<std::string>& params, adam::commander& c, std::mutex& console_mutex) 
        {
            if (params.size() >= 2 && params.size() <= 3) 
            {
                adam::string_hash mod_hash = (params.size() == 3) ? adam::string_hashed(params[2].c_str()).get_hash() : 0;
                c.request_connection_set_input_data_format(adam::string_hashed(params[0].c_str()).get_hash(), adam::string_hashed(params[1].c_str()).get_hash(), mod_hash);
            }
            else { std::lock_guard<std::mutex> lock(console_mutex); adam::stream_log(adam::log::warning, get_cli_string(cmd_string_id::usage_conn_set_fmt_in, c.get_language()), std::cout); }
        });

        db.register_command("conn_set_fmt_out", cmd_string_id::desc_conn_set_fmt_out, [](const std::vector<std::string>& params, adam::commander& c, std::mutex& console_mutex) 
        {
            if (params.size() >= 2 && params.size() <= 3) 
            {
                adam::string_hash mod_hash = (params.size() == 3) ? adam::string_hashed(params[2].c_str()).get_hash() : 0;
                c.request_connection_set_output_data_format(adam::string_hashed(params[0].c_str()).get_hash(), adam::string_hashed(params[1].c_str()).get_hash(), mod_hash);
            }
            else { std::lock_guard<std::mutex> lock(console_mutex); adam::stream_log(adam::log::warning, get_cli_string(cmd_string_id::usage_conn_set_fmt_out, c.get_language()), std::cout); }
        });

        // PROCESSORS
        db.register_command("proc_create", cmd_string_id::desc_proc_create, [](const std::vector<std::string>& params, adam::commander& c, std::mutex& console_mutex) 
        {
            if (params.size() >= 2 && params.size() <= 4) 
            {
                adam::string_hashed name(params[0].c_str());
                adam::string_hash type = adam::string_hashed(params[1].c_str()).get_hash();
                adam::string_hash mod_hash = 0;
                bool is_filter = false;

                if (params.size() == 3)
                {
                    if (params[2] == "filter") is_filter = true;
                    else if (params[2] == "converter") is_filter = false;
                    else mod_hash = adam::string_hashed(params[2].c_str()).get_hash();
                }
                else if (params.size() == 4)
                {
                    mod_hash = adam::string_hashed(params[2].c_str()).get_hash();
                    if (params[3] == "filter") is_filter = true;
                }

                c.request_processor_create(name, type, mod_hash);
            } 
            else { std::lock_guard<std::mutex> lock(console_mutex); adam::stream_log(adam::log::warning, get_cli_string(cmd_string_id::usage_proc_create, c.get_language()), std::cout); }
        });

        db.register_command("proc_destroy", cmd_string_id::desc_proc_destroy, [](const std::vector<std::string>& params, adam::commander& c, std::mutex& console_mutex) 
        {
            if (params.size() == 1) c.request_processor_destroy(adam::string_hashed(params[0].c_str()).get_hash());
            else { std::lock_guard<std::mutex> lock(console_mutex); adam::stream_log(adam::log::warning, get_cli_string(cmd_string_id::usage_proc_destroy, c.get_language()), std::cout); }
        });

        db.register_command("proc_rename", cmd_string_id::desc_proc_rename, [](const std::vector<std::string>& params, adam::commander& c, std::mutex& console_mutex) 
        {
            if (params.size() == 2) c.request_processor_rename(adam::string_hashed(params[0].c_str()).get_hash(), adam::string_hashed(params[1].c_str()));
            else { std::lock_guard<std::mutex> lock(console_mutex); adam::stream_log(adam::log::warning, get_cli_string(cmd_string_id::usage_proc_rename, c.get_language()), std::cout); }
        });

        db.register_command("proc_params", cmd_string_id::desc_proc_params, [](const std::vector<std::string>& params, adam::commander& c, std::mutex& console_mutex) 
        {
            if (params.size() == 1) 
            {
                auto it = c.get_registry().get_processors().find(adam::string_hashed(params[0].c_str()).get_hash());
                if (it != c.get_registry().get_processors().end())
                {
                    std::lock_guard<std::mutex> lock(console_mutex);
                    std::cout << "\r\033[2K\nParameters for processor '" << it->second->name.c_str() << "':\n";
                    if (it->second->user_params.get_children().empty()) std::cout << "  <none>\n";
                    for (const auto& [hash, param] : it->second->user_params.get_children())
                    {
                        std::cout << "  " << param->get_name().c_str() << " (";
                        if (param->get_type() == adam::configuration_parameter::type_integer) 
                        {
                            auto* p = static_cast<adam::configuration_parameter_integer*>(param.get());
                            std::cout << "int): " << p->get_value();
                            if (p->get_mode() == adam::configuration_parameter_integer::value_mode_range)
                                std::cout << " [" << p->get_min_value() << " - " << p->get_max_value() << "]";
                        }
                        else if (param->get_type() == adam::configuration_parameter::type_double) 
                        {
                            auto* p = static_cast<adam::configuration_parameter_double*>(param.get());
                            std::cout << "double): " << p->get_value();
                            if (p->get_mode() == adam::configuration_parameter_double::value_mode_range)
                                std::cout << " [" << p->get_min_value() << " - " << p->get_max_value() << "]";
                        }
                        else if (param->get_type() == adam::configuration_parameter::type_string) 
                        {
                            auto* p = static_cast<adam::configuration_parameter_string*>(param.get());
                            std::cout << "string): " << p->get_value().c_str();
                        }
                        else if (param->get_type() == adam::configuration_parameter::type_boolean) 
                        {
                            auto* p = static_cast<adam::configuration_parameter_boolean*>(param.get());
                            std::cout << "bool): " << (p->get_value() ? "true" : "false");
                        }
                        std::cout << "\n";
                    }
                    std::cout << std::endl;
                }
                else 
                {
                    std::lock_guard<std::mutex> lock(console_mutex); 
                    adam::stream_log(adam::log::error, "Processor not found.", std::cout);
                }
            }
            else { std::lock_guard<std::mutex> lock(console_mutex); adam::stream_log(adam::log::warning, get_cli_string(cmd_string_id::usage_proc_params, c.get_language()), std::cout); }
        });

        db.register_command("proc_set_param", cmd_string_id::desc_proc_set_param, [](const std::vector<std::string>& params, adam::commander& c, std::mutex& console_mutex) 
        {
            if (params.size() >= 2) 
            {
                auto proc_hash = adam::string_hashed(params[0].c_str()).get_hash();
                auto it = c.get_registry().get_processors().find(proc_hash);

                if (it != c.get_registry().get_processors().end())
                {
                    auto param_name = adam::string_hashed(params[1].c_str());
                    
                    std::string val_str = "";
                    if (params.size() >= 3)
                    {
                        val_str = params[2];
                        for (size_t i = 3; i < params.size(); ++i) val_str += " " + params[i];
                    }
                    
                    auto* param = it->second->user_params.get(param_name.get_hash());
                    if (param)
                    {
                        bool valid = false;
                        try 
                        {
                            if (param->get_type() == adam::configuration_parameter::type_integer) 
                            {
                                int64_t v = std::stoll(val_str);
                                if (static_cast<adam::configuration_parameter_integer*>(param)->set_value(v))
                                {
                                    valid = true;
                                    c.request_processor_parameter_set(proc_hash, param_name, v);
                                }
                            }
                            else if (param->get_type() == adam::configuration_parameter::type_double) 
                            {
                                double v = std::stod(val_str);
                                if (static_cast<adam::configuration_parameter_double*>(param)->set_value(v))
                                {
                                    valid = true;
                                    c.request_processor_parameter_set(proc_hash, param_name, v);
                                }
                            }
                            else if (param->get_type() == adam::configuration_parameter::type_string) 
                            {
                                adam::string_hashed v(val_str.c_str());
                                if (static_cast<adam::configuration_parameter_string*>(param)->set_value(v))
                                {
                                    valid = true;
                                    c.request_processor_parameter_set(proc_hash, param_name, v);
                                }
                            }
                            else if (param->get_type() == adam::configuration_parameter::type_boolean) 
                            {
                                bool v = (val_str == "true" || val_str == "1");
                                static_cast<adam::configuration_parameter_boolean*>(param)->set_value(v);
                                valid = true;
                                c.request_processor_parameter_set(proc_hash, param_name, v);
                            }
                        } catch (...) {}
                        
                        if (!valid) { std::lock_guard<std::mutex> lock(console_mutex); adam::stream_log(adam::log::error, get_cli_string(cmd_string_id::err_param_invalid, c.get_language()), std::cout); }
                    }
                    else { std::lock_guard<std::mutex> lock(console_mutex); adam::stream_log(adam::log::error, get_cli_string(cmd_string_id::err_param_not_found, c.get_language()), std::cout); }
                }
                else 
                {
                    std::lock_guard<std::mutex> lock(console_mutex); 
                    adam::stream_log(adam::log::error, "Processor not found.", std::cout);
                }
            }
            else { std::lock_guard<std::mutex> lock(console_mutex); adam::stream_log(adam::log::warning, get_cli_string(cmd_string_id::usage_proc_set_param, c.get_language()), std::cout); }
        });

        db.register_command("proc_list", cmd_string_id::desc_proc_list, [](const std::vector<std::string>&, adam::commander& c, std::mutex& console_mutex) 
        {
            std::lock_guard<std::mutex> lock(console_mutex);
            std::cout << "\r\033[2K\nProcessors:\n";
            bool first = true;
            for (const auto& [hash, p] : c.get_registry().get_processors()) 
            {
                if (!first) std::cout << "\n";
                first = false;
                
                std::cout << "  " << p->name.c_str() << " (Type: " << p->type.c_str();
                if (p->module_name.get_hash() != 0) std::cout << ", Module: " << p->module_name.c_str();
                std::cout << ", ";
                if (p->is_filter) std::cout << "\033[92mFilter\033[0m";
                else std::cout << "\033[95mConverter\033[0m";
                std::cout << ")\n";

                std::vector<std::string> used_in;
                for (const auto& [chash, conn] : c.get_registry().get_connections())
                {
                    if (std::find(conn->processors.begin(), conn->processors.end(), hash) != conn->processors.end()) 
                        used_in.push_back(conn->name.c_str());
                }
                if (!used_in.empty())
                {
                    std::cout << "    Connections:\n";
                    for (const auto& cn : used_in) std::cout << "      " << cn << "\n";
                }
            }
            std::cout << std::endl;
        });

        db.register_command("conn_add_proc", cmd_string_id::desc_conn_add_proc, [](const std::vector<std::string>& params, adam::commander& c, std::mutex& console_mutex) 
        {
            if (params.size() == 2) c.request_connection_processor_add(adam::string_hashed(params[0].c_str()).get_hash(), adam::string_hashed(params[1].c_str()).get_hash());
            else { std::lock_guard<std::mutex> lock(console_mutex); adam::stream_log(adam::log::warning, get_cli_string(cmd_string_id::usage_conn_add_proc, c.get_language()), std::cout); }
        });

        db.register_command("conn_rm_proc", cmd_string_id::desc_conn_rm_proc, [](const std::vector<std::string>& params, adam::commander& c, std::mutex& console_mutex) 
        {
            if (params.size() == 2) c.request_connection_processor_remove(adam::string_hashed(params[0].c_str()).get_hash(), adam::string_hashed(params[1].c_str()).get_hash());
            else { std::lock_guard<std::mutex> lock(console_mutex); adam::stream_log(adam::log::warning, get_cli_string(cmd_string_id::usage_conn_rm_proc, c.get_language()), std::cout); }
        });

        db.register_command("conn_reorder_proc", cmd_string_id::desc_conn_reorder_proc, [](const std::vector<std::string>& params, adam::commander& c, std::mutex& console_mutex) 
        {
            if (params.size() == 3) 
            {
                try { c.request_connection_processor_reorder(adam::string_hashed(params[0].c_str()).get_hash(), adam::string_hashed(params[1].c_str()).get_hash(), std::stoul(params[2])); }
                catch (...) { std::lock_guard<std::mutex> lock(console_mutex); adam::stream_log(adam::log::warning, get_cli_string(cmd_string_id::usage_conn_reorder_proc, c.get_language()), std::cout); }
            }
            else { std::lock_guard<std::mutex> lock(console_mutex); adam::stream_log(adam::log::warning, get_cli_string(cmd_string_id::usage_conn_reorder_proc, c.get_language()), std::cout); }
        });

        db.register_command("cfg_scan", cmd_string_id::desc_cfg_scan, [](const std::vector<std::string>&, adam::commander& c, std::mutex& console_mutex) 
        {
            adam::response_status status = c.request_config_scan();
            std::lock_guard<std::mutex> lock(console_mutex);
            if (status == adam::response_status::success)
                std::cout << "Configuration scan successful.\n";
            else
                adam::stream_log(adam::log::error, "Configuration scan failed.", std::cout);
        });

        db.register_command("cfg_path_add", cmd_string_id::desc_cfg_path_add, [](const std::vector<std::string>& params, adam::commander& c, std::mutex& console_mutex) 
        {
            if (params.size() == 1) c.request_config_path_add(adam::string_hashed(params[0].c_str()));
            else { std::lock_guard<std::mutex> lock(console_mutex); adam::stream_log(adam::log::warning, get_cli_string(cmd_string_id::usage_cfg_path_add, c.get_language()), std::cout); }
        });

        db.register_command("cfg_path_rm", cmd_string_id::desc_cfg_path_rm, [](const std::vector<std::string>& params, adam::commander& c, std::mutex& console_mutex) 
        {
            if (params.size() == 1) 
            {
                try { c.request_config_path_remove(std::stoul(params[0])); }
                catch (...) { std::lock_guard<std::mutex> lock(console_mutex); adam::stream_log(adam::log::warning, get_cli_string(cmd_string_id::usage_cfg_path_rm, c.get_language()), std::cout); }
            }
            else { std::lock_guard<std::mutex> lock(console_mutex); adam::stream_log(adam::log::warning, get_cli_string(cmd_string_id::usage_cfg_path_rm, c.get_language()), std::cout); }
        });

        db.register_command("cfg_path_list", cmd_string_id::desc_cfg_path_list, [](const std::vector<std::string>&, adam::commander& c, std::mutex& console_mutex) 
        {
            auto paths = c.configs().get_paths();
            std::lock_guard<std::mutex> lock(console_mutex);
            if (paths.empty())
            {
                std::cout << "No configuration paths found.\n";
                return;
            }
            std::cout << "Configuration Paths:\n";
            for (size_t i = 0; i < paths.size(); ++i)
                std::cout << "  [" << i << "] " << paths[i].c_str() << "\n";
            std::cout << std::endl;
        });

        db.register_command("cfg_list", cmd_string_id::desc_cfg_list, [](const std::vector<std::string>&, adam::commander& c, std::mutex& console_mutex) 
        {
            auto configs = c.configs().get_available();
            std::lock_guard<std::mutex> lock(console_mutex);
            if (configs.empty())
            {
                std::cout << "No configurations found.\n";
                return;
            }
            std::cout << std::left << std::setw(9) << "PathIdx" 
                      << std::setw(25) << "Filename" 
                      << std::setw(20) << "Name" 
                      << std::setw(8) << "Ports" 
                      << std::setw(8) << "Procs" 
                      << std::setw(8) << "Conns" 
                      << "Description\n";
            std::cout << std::string(100, '-') << "\n";
            for (const auto& [hash, cfg] : configs)
            {
                std::cout << std::left << std::setw(9) << cfg.path_idx 
                          << std::setw(25) << cfg.filename 
                          << std::setw(20) << cfg.name.c_str() 
                          << std::setw(8) << cfg.port_count 
                          << std::setw(8) << cfg.processor_count 
                          << std::setw(8) << cfg.connection_count 
                          << cfg.description.c_str() << "\n";
            }
        });

        db.register_command("cfg_save", cmd_string_id::desc_cfg_save, [](const std::vector<std::string>& params, adam::commander& c, std::mutex& console_mutex) 
        {
            std::string name = params.size() > 0 ? params[0] : c.configs().get_name();
            std::string desc = params.size() > 1 ? params[1] : c.configs().get_description();
            
            if (name.empty())
            {
                std::lock_guard<std::mutex> lock(console_mutex);
                adam::stream_log(adam::log::warning, adam::cli::get_cli_string(adam::cli::cmd_string_id::usage_cfg_save, c.get_language()), std::cout);
                return;
            }
            
            adam::response_status status = c.request_config_save(name, desc);
            std::lock_guard<std::mutex> lock(console_mutex);
            if (status == adam::response_status::success)
                std::cout << "Configuration saved successfully.\n";
            else
                adam::stream_log(adam::log::error, "Failed to save configuration.", std::cout);
        });

        db.register_command("cfg_load", cmd_string_id::desc_cfg_load, [](const std::vector<std::string>& params, adam::commander& c, std::mutex& console_mutex) 
        {
            if (params.size() < 2)
            {
                std::lock_guard<std::mutex> lock(console_mutex);
                adam::stream_log(adam::log::warning, adam::cli::get_cli_string(adam::cli::cmd_string_id::usage_cfg_load, c.get_language()), std::cout);
                return;
            }
            uint32_t path_idx = 0;
            try { path_idx = std::stoul(params[0]); } catch (...) { 
                std::lock_guard<std::mutex> lock(console_mutex);
                adam::stream_log(adam::log::error, "Invalid path index.", std::cout);
                return; 
            }
            adam::response_status status = c.request_config_import(path_idx, string_hashed(params[1].c_str()));
            std::lock_guard<std::mutex> lock(console_mutex);
            if (status == adam::response_status::success)
                std::cout << "Configuration loaded successfully.\n";
            else
                adam::stream_log(adam::log::error, "Failed to load configuration.", std::cout);
        });

        db.register_command("cfg_export", cmd_string_id::desc_cfg_export, [](const std::vector<std::string>& params, adam::commander& c, std::mutex& console_mutex) 
        {
            if (params.size() < 3)
            {
                std::lock_guard<std::mutex> lock(console_mutex);
                adam::stream_log(adam::log::warning, adam::cli::get_cli_string(adam::cli::cmd_string_id::usage_cfg_export, c.get_language()), std::cout);
                return;
            }
            uint32_t path_idx = 0;
            try { path_idx = std::stoul(params[0]); } catch (...) { 
                std::lock_guard<std::mutex> lock(console_mutex);
                adam::stream_log(adam::log::error, "Invalid path index.", std::cout);
                return; 
            }
            std::string desc = params.size() > 3 ? params[3] : "";
            adam::response_status status = c.request_config_export(path_idx, string_hashed(params[1].c_str()), params[2], desc);
            std::lock_guard<std::mutex> lock(console_mutex);
            if (status == adam::response_status::success)
                std::cout << "Configuration exported successfully.\n";
            else
                adam::stream_log(adam::log::error, "Failed to export configuration.", std::cout);
        });

        db.register_command("cfg_delete", cmd_string_id::desc_cfg_delete, [](const std::vector<std::string>& params, adam::commander& c, std::mutex& console_mutex) 
        {
            if (params.size() < 2)
            {
                std::lock_guard<std::mutex> lock(console_mutex);
                adam::stream_log(adam::log::warning, adam::cli::get_cli_string(adam::cli::cmd_string_id::usage_cfg_delete, c.get_language()), std::cout);
                return;
            }
            uint32_t path_idx = 0;
            try { path_idx = std::stoul(params[0]); } catch (...) { 
                std::lock_guard<std::mutex> lock(console_mutex);
                adam::stream_log(adam::log::error, "Invalid path index.", std::cout);
                return; 
            }
            adam::response_status status = c.request_config_delete(path_idx, string_hashed(params[1].c_str()));
            std::lock_guard<std::mutex> lock(console_mutex);
            if (status == adam::response_status::success)
                std::cout << "Configuration deleted successfully.\n";
            else
                adam::stream_log(adam::log::error, "Failed to delete configuration.", std::cout);
        });
        
        auto clear_func = [](const std::vector<std::string>&, adam::commander&, std::mutex& console_mutex) 
        {
            std::lock_guard<std::mutex> lock(console_mutex);
            std::cout << "\033[2J\033[H";
        };
        db.register_command("clean", cmd_string_id::desc_clean, clear_func);
        db.register_command("clear", cmd_string_id::desc_clear, clear_func);

        // Register exit and quit so they appear in the help listing (actual logic is handled natively in the loop)
        db.register_command("exit", cmd_string_id::desc_exit, [](const std::vector<std::string>&, adam::commander&, std::mutex&){});
        db.register_command("quit", cmd_string_id::desc_quit, [](const std::vector<std::string>&, adam::commander&, std::mutex&){});
    }
}