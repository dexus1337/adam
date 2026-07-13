#pragma once

#include "command-database.hpp"
#include "cli-settings.hpp"

namespace adam::cli
{
    /**
     * @brief Registers default application commands like help, setlang, and loglvl.
     */
    void register_default_commands(command_database& db, adam::logger_sink& lgsnk, cli_settings& settings);
}