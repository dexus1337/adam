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
            if (current_input.find(' ') == std::string::npos)
            {
                std::vector<std::string> matches;
                for (const auto& [name, info] : ctx.db.get_commands())
                {
                    if (name.compare(0, current_input.length(), current_input) == 0)
                    {
                        matches.push_back(name);
                    }
                }

                if (matches.size() == 1)
                {
                    current_input = matches[0] + " ";
                    cursor_pos = current_input.length();
                    std::cout << "\r\033[2K> " << current_input << std::flush;
                }
                else if (matches.size() > 1)
                {
                    std::sort(matches.begin(), matches.end());
                    std::cout << "\n";
                    for (const auto& match : matches)
                    {
                        std::cout << match << "  ";
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
                    if (common_prefix.size() > current_input.size())
                        current_input = common_prefix;

                    cursor_pos = current_input.length();
                    refresh_line();
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

        std::istringstream iss(input);
        std::string command_name;
        iss >> command_name;

        if (command_name == "exit" || command_name == "quit")
            break;

        std::vector<std::string> params;
        std::string param;
        while (iss >> param)
        {
            params.push_back(param);
        }

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
