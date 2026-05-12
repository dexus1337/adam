#include "default-commands.hpp"
#include "cmd-strings.hpp"

#include <iostream>
#include <iomanip>
#include <algorithm>

namespace adam::cmd
{
    void register_default_commands(command_database& db, adam::logger_sink& lgsnk)
    {
        command_database* p_db = &db;
        adam::logger_sink* p_lgsnk = &lgsnk;

        db.register_command("help", cmd_string_id::desc_help, 
            [p_db](const std::vector<std::string>&, adam::commander& c, std::mutex& console_mutex) 
        {
            std::lock_guard<std::mutex> lock(console_mutex);
            std::cout << "\r\033[2K\n" << get_cmd_string(cmd_string_id::available_commands, c.get_language()) << "\n";
            
            std::vector<std::pair<std::string, std::string>> cmds;
            for (const auto& [name, info] : p_db->get_commands())
            {
                cmds.push_back({name, get_cmd_string(info.desc_id, c.get_language())});
            }
            std::sort(cmds.begin(), cmds.end());

            for (const auto& [name, desc] : cmds)
            {
                std::cout << "  " << std::left << std::setw(15) << name << " - " << desc << "\n";
            }
            std::cout << std::endl;
        });

        db.register_command("setlang", cmd_string_id::desc_setlang, 
            [](const std::vector<std::string>& params, adam::commander& c, std::mutex& console_mutex) 
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
                adam::stream_log(adam::log::warning, adam::cmd::get_cmd_string(adam::cmd::cmd_string_id::usage_setlang, c.get_language()), std::cout);
            }
        });

        db.register_command("loglvl", cmd_string_id::desc_loglvl, 
            [p_lgsnk](const std::vector<std::string>& params, adam::commander& c, std::mutex& console_mutex) 
        {
            if (params.size() == 1)
            {
                const std::string& val = params[0];
                adam::log::level new_level;
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
                        adam::stream_log(adam::log::info, adam::cmd::get_cmd_string(adam::cmd::cmd_string_id::log_level_updated, c.get_language()), std::cout);
                    }
                    return;
                }
            }
            
            std::lock_guard<std::mutex> lock(console_mutex);
            adam::stream_log(adam::log::warning, adam::cmd::get_cmd_string(adam::cmd::cmd_string_id::usage_loglvl, c.get_language()), std::cout);
        });
        
        // Register exit and quit so they appear in the help listing (actual logic is handled natively in the loop)
        db.register_command("exit", cmd_string_id::desc_exit, [](const std::vector<std::string>&, adam::commander&, std::mutex&){});
        db.register_command("quit", cmd_string_id::desc_quit, [](const std::vector<std::string>&, adam::commander&, std::mutex&){});
    }
}