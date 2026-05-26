#pragma once

#include "command-database.hpp"

namespace adam::cmd
{
    /**
     * @brief Registers default application commands like help, setlang, and loglvl.
     */
    void register_default_commands(command_database& db, adam::logger_sink& lgsnk);
}