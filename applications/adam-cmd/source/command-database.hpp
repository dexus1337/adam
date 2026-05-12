#pragma once

#include <adam-sdk.hpp>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <mutex>
#include "cmd-strings.hpp"

namespace adam::cmd
{
    using command_handler = std::function<void(const std::vector<std::string>&, adam::commander&, std::mutex&)>;

    struct command_info
    {
        cmd_string_id desc_id;
        command_handler handler;
    };

    class command_database
    {
    public:
        void register_command(const std::string& name, cmd_string_id desc_id, command_handler handler);
        bool execute_command(const std::string& name, const std::vector<std::string>& args, adam::commander& cmd, std::mutex& console_mutex);
        
        const std::unordered_map<std::string, command_info>& get_commands() const { return m_commands; }
        
    private:
        std::unordered_map<std::string, command_info> m_commands;
    };
}