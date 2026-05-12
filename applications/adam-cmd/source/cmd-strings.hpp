#pragma once

#include <adam-sdk.hpp>

namespace adam::cmd
{
    enum class cmd_string_id
    {
        cannot_connect_controller,
        connected_controller,
        failed_connect_logger,
        connected_logger,
        failed_connect_logger_sink,
        connected_logger_sink,
        usage_setlang,
        usage_loglvl,
        log_level_updated,
        failed_destroy_logger_sink,
        failed_destroy_logger,
        failed_destroy_commander,
        exiting,
        unknown_command,
        available_commands,
        desc_help,
        desc_setlang,
        desc_loglvl,
        desc_exit,
        desc_quit
    };

    const char* get_cmd_string(cmd_string_id id, adam::language lang);
}