#pragma once

#include <adam-sdk.hpp>

namespace adam::cli
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
        desc_mod_scan,
        desc_mod_load,
        usage_mod_load,
        desc_mod_unload,
        usage_mod_unload,
        desc_mod_list,
        desc_port_create,
        usage_port_create,
        desc_port_destroy,
        usage_port_destroy,
        desc_port_start,
        usage_port_start,
        desc_port_stop,
        usage_port_stop,
        desc_port_list,
        desc_conn_create,
        usage_conn_create,
        desc_conn_destroy,
        usage_conn_destroy,
        desc_conn_start,
        usage_conn_start,
        desc_conn_stop,
        usage_conn_stop,
        desc_conn_add_port,
        usage_conn_add_port,
        desc_conn_rm_port,
        usage_conn_rm_port,
        desc_conn_list,
        desc_clean,
        desc_clear,
        desc_exit,
        desc_quit
    };

    const char* get_cli_string(cmd_string_id id, adam::language lang);
}