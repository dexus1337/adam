#include <adam-sdk.hpp>

#include "terminal-manager.hpp"
#include "cmd-strings.hpp"
#include "command-database.hpp"
#include "default-commands.hpp"

#include <csignal>
#include <iostream>
#include <atomic>
#include <string>
#include <vector>
#include <sstream>
#include <mutex>
#include <thread>

#ifdef ADAM_PLATFORM_LINUX
void signal_handler(int signal) 
{
    if (signal == SIGINT) 
    {
    }
}
#endif

struct app_context
{
    adam::commander cmd;
    adam::logger lg;
    adam::logger_sink lgsnk;
    adam::cmd::command_database db;
};

bool setup(app_context& ctx)
{
    adam::language lang = adam::language_english;

    if (!ctx.cmd.connect())
    {
        adam::stream_log(adam::log::fatal, adam::cmd::get_cmd_string(adam::cmd::cmd_string_id::cannot_connect_controller, lang), std::cerr);
        return false;
    }
    
    lang = ctx.cmd.get_language();
    adam::stream_log(adam::log::info, adam::cmd::get_cmd_string(adam::cmd::cmd_string_id::connected_controller, lang), std::cout);

    if (!ctx.lg.connect())
    {
        adam::stream_log(adam::log::warning, adam::cmd::get_cmd_string(adam::cmd::cmd_string_id::failed_connect_logger, lang), std::cout);
    }
    else
    {
        adam::stream_log(adam::log::info, adam::cmd::get_cmd_string(adam::cmd::cmd_string_id::connected_logger, lang), std::cout);
    }
    
    if (!ctx.lgsnk.connect())
    {
        adam::stream_log(adam::log::warning, adam::cmd::get_cmd_string(adam::cmd::cmd_string_id::failed_connect_logger_sink, lang), std::cout);
    }
    else
    {
        adam::stream_log(adam::log::info, adam::cmd::get_cmd_string(adam::cmd::cmd_string_id::connected_logger_sink, lang), std::cout);
    }
    
    adam::cmd::register_default_commands(ctx.db, ctx.lgsnk);

    return true;
}

void teardown(app_context& ctx)
{
    adam::language lang = ctx.cmd.is_active() ? ctx.cmd.get_language() : adam::language_english;

    if (!ctx.lgsnk.destroy())
        adam::stream_log(adam::log::warning, adam::cmd::get_cmd_string(adam::cmd::cmd_string_id::failed_destroy_logger_sink, lang), std::cout);

    if (!ctx.lg.destroy())
        adam::stream_log(adam::log::warning, adam::cmd::get_cmd_string(adam::cmd::cmd_string_id::failed_destroy_logger, lang), std::cout);
    
    if (!ctx.cmd.destroy())
        adam::stream_log(adam::log::error, adam::cmd::get_cmd_string(adam::cmd::cmd_string_id::failed_destroy_commander, lang), std::cerr);

    adam::stream_log(adam::log::info, adam::cmd::get_cmd_string(adam::cmd::cmd_string_id::exiting, lang), std::cout);
}

int main() 
{
    #ifdef ADAM_PLATFORM_LINUX
    std::signal(SIGINT, signal_handler);
    #endif

    app_context ctx;
    if (!setup(ctx))
        return 1;

    terminal_manager term_mgr;
    std::mutex console_mutex;
    std::string current_input;

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
            std::cout << "> " << current_input << std::flush;
        }
    });

    std::cout << "> " << std::flush;

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
            std::cout << "\n";
            
            if (input.empty())
            {
                std::cout << "> " << std::flush;
                continue;
            }
        }
        else if (c == '\b' || c == 127) // Backspace
        {
            std::lock_guard<std::mutex> lock(console_mutex);
            if (!current_input.empty())
            {
                current_input.pop_back();
                std::cout << "\r\033[2K> " << current_input << std::flush;
            }
            continue;
        }
        else if (c >= 32 && c <= 126) // Printable characters
        {
            std::lock_guard<std::mutex> lock(console_mutex);
            current_input += static_cast<char>(c);
            std::cout << "\r\033[2K> " << current_input << std::flush;
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
            adam::stream_log(adam::log::warning, adam::cmd::get_cmd_string(adam::cmd::cmd_string_id::unknown_command, ctx.cmd.get_language()), std::cout);
        }
        
        {
            std::lock_guard<std::mutex> lock(console_mutex);
            std::cout << "> " << std::flush;
        }
    }

    ctx.lgsnk.queue().disable();
    logger_sink_watcher.join();

    teardown(ctx);

    return 0;
}
