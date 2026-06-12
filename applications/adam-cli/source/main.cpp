#include <adam-sdk.hpp>

#include "terminal-manager.hpp"
#include "cli-strings.hpp"
#include "command-database.hpp"
#include "default-commands.hpp"

#include <csignal>
#include <iostream>
#include <cstdlib>
#include <atomic>
#include <string>
#include <vector>
#include <sstream>
#include <mutex>
#include <thread>
#include <algorithm>

struct app_context
{
    adam::commander cmd;
    adam::logger lg;
    adam::logger_sink lgsnk;
    adam::cli::command_database db;
};

static app_context* g_app_ctx = nullptr;
static terminal_manager* g_term_mgr = nullptr;
void teardown(app_context& ctx);

void signal_handler(int signal) 
{
    if (signal == SIGINT) 
    {
        std::cout << "\n";
        if (g_term_mgr)
        {
            delete g_term_mgr;
            g_term_mgr = nullptr;
        }
        if (g_app_ctx)
        {
            g_app_ctx->lgsnk.queue().disable();
            teardown(*g_app_ctx);
        }
        std::exit(0);
    }
}

bool setup(app_context& ctx)
{
    adam::language lang = adam::language_english;

    if (!ctx.cmd.connect())
    {
        adam::stream_log(adam::log::fatal, adam::cli::get_cli_string(adam::cli::cmd_string_id::cannot_connect_controller, lang), std::cerr);
        return false;
    }
    
    lang = ctx.cmd.get_language();
    adam::stream_log(adam::log::info, adam::cli::get_cli_string(adam::cli::cmd_string_id::connected_controller, lang), std::cout);

    if (!ctx.lg.connect())
    {
        adam::stream_log(adam::log::warning, adam::cli::get_cli_string(adam::cli::cmd_string_id::failed_connect_logger, lang), std::cout);
    }
    else
    {
        adam::stream_log(adam::log::info, adam::cli::get_cli_string(adam::cli::cmd_string_id::connected_logger, lang), std::cout);
    }
    
    if (!ctx.lgsnk.connect())
    {
        adam::stream_log(adam::log::warning, adam::cli::get_cli_string(adam::cli::cmd_string_id::failed_connect_logger_sink, lang), std::cout);
    }
    else
    {
        adam::stream_log(adam::log::info, adam::cli::get_cli_string(adam::cli::cmd_string_id::connected_logger_sink, lang), std::cout);
    }
    
    adam::cli::register_default_commands(ctx.db, ctx.lgsnk);

    return true;
}

void teardown(app_context& ctx)
{
    adam::language lang = ctx.cmd.is_active() ? ctx.cmd.get_language() : adam::language_english;

    if (!ctx.lgsnk.destroy())
        adam::stream_log(adam::log::warning, adam::cli::get_cli_string(adam::cli::cmd_string_id::failed_destroy_logger_sink, lang), std::cout);

    if (!ctx.lg.destroy())
        adam::stream_log(adam::log::warning, adam::cli::get_cli_string(adam::cli::cmd_string_id::failed_destroy_logger, lang), std::cout);
    
    if (!ctx.cmd.destroy())
        adam::stream_log(adam::log::error, adam::cli::get_cli_string(adam::cli::cmd_string_id::failed_destroy_commander, lang), std::cerr);

    adam::stream_log(adam::log::info, adam::cli::get_cli_string(adam::cli::cmd_string_id::exiting, lang), std::cout);
}

int main() 
{
    std::signal(SIGINT, signal_handler);

    app_context ctx;
    g_app_ctx = &ctx;
    if (!setup(ctx))
        return 1;

    g_term_mgr = new terminal_manager();
    std::mutex console_mutex;
    std::string current_input;
    size_t cursor_pos = 0;

    std::thread logger_sink_watcher([&]() 
    {
        while (ctx.lgsnk.queue().is_active())
        {
            adam::log cur_log;

            if (!ctx.lgsnk.queue().pop(cur_log, 100))
                continue;

            std::lock_guard<std::mutex> lock(console_mutex);
            std::cout << "\r\033[2K"; // ANSI clear line
            adam::stream_log(cur_log, std::cout);
            std::cout << "> " << current_input;
            if (cursor_pos < current_input.length())
                std::cout << "\033[" << (current_input.length() - cursor_pos) << "D";
            std::cout << std::flush;
        }
    });

    std::cout << "> " << std::flush;

    std::vector<std::string> command_history;
    size_t history_index = 0;
    
    auto refresh_line = [&]() 
    {
        std::cout << "\r\033[2K> " << current_input;
        if (cursor_pos < current_input.length())
            std::cout << "\033[" << (current_input.length() - cursor_pos) << "D";
        std::cout << std::flush;
    };

    while (true)
    {
        int c = terminal_manager::read_char_raw();
        if (c == -2) continue;

        std::string input;

        if (c == EOF || c == 3 || c == 4) // EOF, Ctrl+C or Ctrl+D
        {
            std::cout << "\n";
            break;
        }
        else if (c == '\r' || c == '\n')
        {
            std::lock_guard<std::mutex> lock(console_mutex);
            input = current_input;
            current_input.clear();
            cursor_pos = 0;
            std::cout << "\n";
            
            if (input.empty())
            {
                std::cout << "> " << std::flush;
                continue;
            }
            
            if (command_history.empty() || command_history.back() != input)
                command_history.push_back(input);
            history_index = command_history.size();
        }
        else if (c == '\t') // Tab for auto-complete
        {
            std::lock_guard<std::mutex> lock(console_mutex);

            auto handle_matches = [&](const std::vector<std::string>& possible_matches, std::string prefix, std::string base_input)
            {
                std::vector<std::string> matches;
                for (const auto& name : possible_matches)
                {
                    if (name.compare(0, prefix.length(), prefix) == 0)
                        matches.push_back(name);
                }

                if (matches.size() == 1)
                {
                    std::string match = matches[0];
                    if (match.find(' ') != std::string::npos && match.front() != '"')
                        match = "\"" + match + "\"";
                    
                    current_input = base_input + match + " ";
                    cursor_pos = current_input.length();
                    std::cout << "\r\033[2K> " << current_input << std::flush;
                }
                else if (matches.size() > 1)
                {
                    std::sort(matches.begin(), matches.end());
                    std::cout << "\n";
                    for (const auto& match : matches)
                    {
                        std::string m = match;
                        if (m.find(' ') != std::string::npos && m.front() != '"') m = "\"" + m + "\"";
                        std::cout << m << "  ";
                    }
                    std::cout << "\n";

                    std::string common_prefix = matches[0];
                    for (size_t i = 1; i < matches.size(); ++i)
                    {
                        size_t j = 0;
                        while (j < common_prefix.size() && j < matches[i].size() && common_prefix[j] == matches[i][j])
                            j++;
                        common_prefix = common_prefix.substr(0, j);
                    }
                    if (common_prefix.size() > prefix.size())
                    {
                        if (common_prefix.find(' ') != std::string::npos && common_prefix.front() != '"')
                            current_input = base_input + "\"" + common_prefix;
                        else
                            current_input = base_input + common_prefix;
                        cursor_pos = current_input.length();
                    }
                    refresh_line();
                }
            };

            std::vector<std::string> tokens;
            std::string current_token;
            bool in_quotes = false;
            for (char ch : current_input)
            {
                if (ch == '"') in_quotes = !in_quotes;
                else if (std::isspace(ch) && !in_quotes)
                {
                    if (!current_token.empty())
                    {
                        tokens.push_back(current_token);
                        current_token.clear();
                    }
                }
                else current_token += ch;
            }
            if (!current_token.empty() || in_quotes)
                tokens.push_back(current_token);
            
            bool ends_with_space = !current_input.empty() && std::isspace(current_input.back());
            
            if (tokens.empty() || (tokens.size() == 1 && !ends_with_space))
            {
                std::vector<std::string> cmd_names;
                for (const auto& [name, info] : ctx.db.get_commands()) cmd_names.push_back(name);
                handle_matches(cmd_names, tokens.empty() ? "" : tokens[0], "");
            }
            else
            {
                std::string base;
                size_t limit = ends_with_space ? tokens.size() : tokens.size() - 1;
                for (size_t i = 0; i < limit; ++i)
                {
                    if (tokens[i].find(' ') != std::string::npos) base += "\"" + tokens[i] + "\" ";
                    else base += tokens[i] + " ";
                }

                if ((tokens.size() == 1 && ends_with_space) || (tokens.size() == 2 && !ends_with_space))
                {
                    std::string cmd = tokens[0];
                    std::string arg = (tokens.size() == 2) ? tokens[1] : "";
                    std::vector<std::string> candidates;
                    
                    if (cmd.find("port_") == 0 && cmd != "port_create" && cmd != "port_list")
                    {
                        for (const auto& [hash, p] : ctx.cmd.get_registry().get_ports())
                            candidates.push_back(std::string(p->name.c_str()));
                    }
                    else if (cmd.find("conn_") == 0 && cmd != "conn_create" && cmd != "conn_list")
                    {
                        for (const auto& [hash, p] : ctx.cmd.get_registry().get_connections())
                            candidates.push_back(std::string(p->name.c_str()));
                    }
                    else if (cmd.find("proc_") == 0 && cmd != "proc_create" && cmd != "proc_list")
                    {
                        for (const auto& [hash, p] : ctx.cmd.get_registry().get_processors())
                            candidates.push_back(std::string(p->name.c_str()));
                    }
                    
                    if (!candidates.empty()) handle_matches(candidates, arg, base);
                }
                else if (tokens[0] == "port_set_param" && ((tokens.size() == 2 && ends_with_space) || (tokens.size() == 3 && !ends_with_space)))
                {
                    std::string port_name = tokens[1];
                    std::string arg = (tokens.size() == 3) ? tokens[2] : "";
                    auto it = ctx.cmd.get_registry().get_ports().find(adam::string_hashed(port_name.c_str()).get_hash());
                    if (it != ctx.cmd.get_registry().get_ports().end())
                    {
                        std::vector<std::string> candidates;
                        for (const auto& [hash, p] : it->second->user_params.get_children())
                            candidates.push_back(std::string(p->get_name().c_str()));
                        if (!candidates.empty()) handle_matches(candidates, arg, base);
                    }
                }
                else if (tokens[0] == "port_set_param" && ((tokens.size() == 3 && ends_with_space) || (tokens.size() == 4 && !ends_with_space)))
                {
                    std::string port_name = tokens[1];
                    std::string param_name = tokens[2];
                    std::string arg = (tokens.size() == 4) ? tokens[3] : "";
                    auto it = ctx.cmd.get_registry().get_ports().find(adam::string_hashed(port_name.c_str()).get_hash());
                    if (it != ctx.cmd.get_registry().get_ports().end())
                    {
                        auto* param = it->second->user_params.get(adam::string_hashed(param_name.c_str()).get_hash());
                        if (param)
                        {
                            std::vector<std::string> candidates;
                            if (param->get_type() == adam::configuration_parameter::type_boolean)
                            {
                                candidates = {"true", "false"};
                            }
                            else if (param->get_type() == adam::configuration_parameter::type_string)
                            {
                                auto* p = static_cast<adam::configuration_parameter_string*>(param);
                                if (p->get_mode() == adam::configuration_parameter_string::value_mode_preset)
                                {
                                    for (const auto& [phash, pval] : p->get_presets())
                                        candidates.push_back(std::string(pval->get_value().c_str()));
                                }
                            }
                            if (!candidates.empty()) handle_matches(candidates, arg, base);
                        }
                    }
                }
                else if (tokens[0] == "proc_set_param" && ((tokens.size() == 2 && ends_with_space) || (tokens.size() == 3 && !ends_with_space)))
                {
                    std::string proc_name = tokens[1];
                    std::string arg = (tokens.size() == 3) ? tokens[2] : "";
                    auto it = ctx.cmd.get_registry().get_processors().find(adam::string_hashed(proc_name.c_str()).get_hash());
                    if (it != ctx.cmd.get_registry().get_processors().end())
                    {
                        std::vector<std::string> candidates;
                        for (const auto& [hash, p] : it->second->user_params.get_children())
                            candidates.push_back(std::string(p->get_name().c_str()));
                        if (!candidates.empty()) handle_matches(candidates, arg, base);
                    }
                }
                else if (tokens[0] == "proc_set_param" && ((tokens.size() == 3 && ends_with_space) || (tokens.size() == 4 && !ends_with_space)))
                {
                    std::string proc_name = tokens[1];
                    std::string param_name = tokens[2];
                    std::string arg = (tokens.size() == 4) ? tokens[3] : "";
                    auto it = ctx.cmd.get_registry().get_processors().find(adam::string_hashed(proc_name.c_str()).get_hash());
                    if (it != ctx.cmd.get_registry().get_processors().end())
                    {
                        auto* param = it->second->user_params.get(adam::string_hashed(param_name.c_str()).get_hash());
                        if (param)
                        {
                            std::vector<std::string> candidates;
                            if (param->get_type() == adam::configuration_parameter::type_boolean)
                            {
                                candidates = {"true", "false"};
                            }
                            else if (param->get_type() == adam::configuration_parameter::type_string)
                            {
                                auto* p = static_cast<adam::configuration_parameter_string*>(param);
                                if (p->get_mode() == adam::configuration_parameter_string::value_mode_preset)
                                {
                                    for (const auto& [phash, pval] : p->get_presets())
                                        candidates.push_back(std::string(pval->get_value().c_str()));
                                }
                            }
                            if (!candidates.empty()) handle_matches(candidates, arg, base);
                        }
                    }
                }
                else if ((tokens[0] == "conn_add_proc" || tokens[0] == "conn_rm_proc" || tokens[0] == "conn_reorder_proc") && ((tokens.size() == 2 && ends_with_space) || (tokens.size() == 3 && !ends_with_space)))
                {
                    std::string arg = (tokens.size() == 3) ? tokens[2] : "";
                    std::vector<std::string> candidates;
                    for (const auto& [hash, p] : ctx.cmd.get_registry().get_processors())
                        candidates.push_back(std::string(p->name.c_str()));
                    if (!candidates.empty()) handle_matches(candidates, arg, base);
                }
            }
            continue;
        }
        else if (c == terminal_manager::key_up) // Up arrow
        {
            std::lock_guard<std::mutex> lock(console_mutex);
            if (!command_history.empty() && history_index > 0)
            {
                history_index--;
                current_input = command_history[history_index];
                cursor_pos = current_input.length();
                refresh_line();
            }
            continue;
        }
        else if (c == terminal_manager::key_down) // Down arrow
        {
            std::lock_guard<std::mutex> lock(console_mutex);
            if (history_index < command_history.size())
            {
                history_index++;
                if (history_index == command_history.size())
                    current_input = "";
                else
                    current_input = command_history[history_index];
                cursor_pos = current_input.length();
                refresh_line();
            }
            continue;
        }
        else if (c == terminal_manager::key_left) // Left arrow
        {
            std::lock_guard<std::mutex> lock(console_mutex);
            if (cursor_pos > 0)
            {
                cursor_pos--;
                refresh_line();
            }
            continue;
        }
        else if (c == terminal_manager::key_right) // Right arrow
        {
            std::lock_guard<std::mutex> lock(console_mutex);
            if (cursor_pos < current_input.length())
            {
                cursor_pos++;
                refresh_line();
            }
            continue;
        }
        else if (c == '\b' || c == 127) // Backspace
        {
            std::lock_guard<std::mutex> lock(console_mutex);
            if (cursor_pos > 0)
            {
                current_input.erase(cursor_pos - 1, 1);
                cursor_pos--;
                refresh_line();
            }
            continue;
        }
        else if (c >= 32 && c <= 126) // Printable characters
        {
            std::lock_guard<std::mutex> lock(console_mutex);
            current_input.insert(cursor_pos, 1, static_cast<char>(c));
            cursor_pos++;
            refresh_line();
            continue;
        }
        else
        {
            continue;
        }

        std::vector<std::string> tokens;
        std::string current_token;
        bool in_quotes = false;
        for (char ch : input)
        {
            if (ch == '"') in_quotes = !in_quotes;
            else if (std::isspace(ch) && !in_quotes)
            {
                if (!current_token.empty())
                {
                    tokens.push_back(current_token);
                    current_token.clear();
                }
            }
            else current_token += ch;
        }
        if (!current_token.empty()) tokens.push_back(current_token);

        if (tokens.empty()) continue;
        
        std::string command_name = tokens[0];
        if (command_name == "exit" || command_name == "quit")
            break;

        std::vector<std::string> params;
        for (size_t i = 1; i < tokens.size(); ++i) params.push_back(tokens[i]);

        if (!ctx.db.execute_command(command_name, params, ctx.cmd, console_mutex))
        {
            std::lock_guard<std::mutex> lock(console_mutex);
            adam::stream_log(adam::log::warning, adam::cli::get_cli_string(adam::cli::cmd_string_id::unknown_command, ctx.cmd.get_language()), std::cout);
        }
        
        {
            std::lock_guard<std::mutex> lock(console_mutex);
            std::cout << "> " << std::flush;
        }
    }

    ctx.lgsnk.queue().disable();
    logger_sink_watcher.join();

    if (g_term_mgr)
    {
        delete g_term_mgr;
        g_term_mgr = nullptr;
    }

    teardown(ctx);

    return 0;
}
