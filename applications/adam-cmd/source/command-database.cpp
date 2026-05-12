#include "command-database.hpp"

namespace adam::cmd
{
    void command_database::register_command(const std::string& name, cmd_string_id desc_id, command_handler handler)
    {
        m_commands[name] = { desc_id, handler };
    }

    bool command_database::execute_command(const std::string& name, const std::vector<std::string>& args, adam::commander& cmd, std::mutex& console_mutex)
    {
        auto it = m_commands.find(name);
        if (it != m_commands.end())
        {
            it->second.handler(args, cmd, console_mutex);
            return true;
        }
        
        return false;
    }
}