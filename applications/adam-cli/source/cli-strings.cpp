#include "cli-strings.hpp"

#include <unordered_map>
#include <array>

namespace adam::cli
{
    const char* get_cli_string(cmd_string_id id, adam::language lang)
    {
        static const std::unordered_map<cmd_string_id, std::array<const char*, adam::languages_count>> translations =
        {
            { cmd_string_id::cannot_connect_controller,   { "Cannot connect to controller", "Verbindung zum Controller fehlgeschlagen" } },
            { cmd_string_id::connected_controller,        { "Connected to controller!", "Verbunden mit Controller!" } },
            { cmd_string_id::failed_connect_logger,       { "Failed to connect to the logger.", "Verbindung zum Logger fehlgeschlagen." } },
            { cmd_string_id::connected_logger,            { "Connected to logger!", "Verbunden mit Logger!" } },
            { cmd_string_id::failed_connect_logger_sink,  { "Failed to connect to the logger sink.", "Verbindung zum Logger Sink fehlgeschlagen." } },
            { cmd_string_id::connected_logger_sink,       { "Connected to logger sink!", "Verbunden mit Logger Sink!" } },
            { cmd_string_id::usage_setlang,               { "Usage: setlang <de|en>", "Verwendung: setlang <de|en>" } },
            { cmd_string_id::usage_loglvl,                { "Usage: loglvl <0|1|2|3> or <trace|info|warn|error>", "Verwendung: loglvl <0|1|2|3> oder <trace|info|warn|error>" } },
            { cmd_string_id::log_level_updated,           { "Log level updated.", "Protokollstufe aktualisiert." } },
            { cmd_string_id::failed_destroy_logger_sink,  { "Failed to destroy the logger sink.", "Logger Sink konnte nicht zerstört werden." } },
            { cmd_string_id::failed_destroy_logger,       { "Failed to destroy the logger.", "Logger konnte nicht zerstört werden." } },
            { cmd_string_id::failed_destroy_commander,    { "Failed to destroy the commander.", "Commander konnte nicht zerstört werden." } },
            { cmd_string_id::exiting,                     { "Exiting!", "Beenden!" } },
            { cmd_string_id::unknown_command,             { "Unknown command.", "Unbekannter Befehl." } },
            { cmd_string_id::available_commands,          { "Available commands:", "Verfügbare Befehle:" } },
            { cmd_string_id::desc_help,                   { "Lists all available commands and their descriptions", "Listet alle verfügbaren Befehle und deren Beschreibungen auf" } },
            { cmd_string_id::desc_setlang,                { "Changes the application language (de, en)", "Ändert die Anwendungssprache (de, en)" } },
            { cmd_string_id::desc_loglvl,                 { "Sets the logging level (0=trace, 1=info, 2=warn, 3=error)", "Legt die Protokollierungsstufe fest (0=trace, 1=info, 2=warn, 3=error)" } },
            { cmd_string_id::desc_mod_scan,               { "Scans for new modules in module paths", "Sucht nach neuen Modulen in den Modulpfaden" } },
            { cmd_string_id::desc_mod_load,               { "Loads a module by name", "Lädt ein Modul nach Namen" } },
            { cmd_string_id::usage_mod_load,              { "Usage: mod_load <name>", "Verwendung: mod_load <name>" } },
            { cmd_string_id::desc_mod_unload,             { "Unloads a module by name", "Entlädt ein Modul nach Namen" } },
            { cmd_string_id::usage_mod_unload,            { "Usage: mod_unload <name>", "Verwendung: mod_unload <name>" } },
            { cmd_string_id::desc_mod_list,               { "Lists all modules", "Listet alle Module auf" } },
            { cmd_string_id::desc_port_create,            { "Creates a new port", "Erstellt einen neuen Port" } },
            { cmd_string_id::usage_port_create,           { "Usage: port_create <name> <type> [module]", "Verwendung: port_create <name> <type> [module]" } },
            { cmd_string_id::desc_port_destroy,           { "Destroys a port by name", "Zerstört einen Port nach Namen" } },
            { cmd_string_id::usage_port_destroy,          { "Usage: port_destroy <name>", "Verwendung: port_destroy <name>" } },
            { cmd_string_id::desc_port_start,             { "Starts a port by name", "Startet einen Port nach Namen" } },
            { cmd_string_id::usage_port_start,            { "Usage: port_start <name>", "Verwendung: port_start <name>" } },
            { cmd_string_id::desc_port_stop,              { "Stops a port by name", "Stoppt einen Port nach Namen" } },
            { cmd_string_id::usage_port_stop,             { "Usage: port_stop <name>", "Verwendung: port_stop <name>" } },
            { cmd_string_id::desc_port_list,              { "Lists all ports", "Listet alle Ports auf" } },
            { cmd_string_id::desc_conn_create,            { "Creates a new connection", "Erstellt eine neue Verbindung" } },
            { cmd_string_id::usage_conn_create,           { "Usage: conn_create <name>", "Verwendung: conn_create <name>" } },
            { cmd_string_id::desc_conn_destroy,           { "Destroys a connection by name", "Zerstört eine Verbindung nach Namen" } },
            { cmd_string_id::usage_conn_destroy,          { "Usage: conn_destroy <name>", "Verwendung: conn_destroy <name>" } },
            { cmd_string_id::desc_conn_start,             { "Starts a connection by name", "Startet eine Verbindung nach Namen" } },
            { cmd_string_id::usage_conn_start,            { "Usage: conn_start <name>", "Verwendung: conn_start <name>" } },
            { cmd_string_id::desc_conn_stop,              { "Stops a connection by name", "Stoppt eine Verbindung nach Namen" } },
            { cmd_string_id::usage_conn_stop,             { "Usage: conn_stop <name>", "Verwendung: conn_stop <name>" } },
            { cmd_string_id::desc_conn_add_port,          { "Adds a port to a connection", "Fügt einen Port zu einer Verbindung hinzu" } },
            { cmd_string_id::usage_conn_add_port,         { "Usage: conn_add_port <conn_name> <port_name> <in|out>", "Verwendung: conn_add_port <conn_name> <port_name> <in|out>" } },
            { cmd_string_id::desc_conn_rm_port,           { "Removes a port from a connection", "Entfernt einen Port aus einer Verbindung" } },
            { cmd_string_id::usage_conn_rm_port,          { "Usage: conn_rm_port <conn_name> <port_name> <in|out>", "Verwendung: conn_rm_port <conn_name> <port_name> <in|out>" } },
            { cmd_string_id::desc_conn_list,              { "Lists all connections", "Listet alle Verbindungen auf" } },
            { cmd_string_id::desc_port_rename,            { "Renames a port", "Benennt einen Port um" } },
            { cmd_string_id::usage_port_rename,           { "Usage: port_rename <old_name> <new_name>", "Verwendung: port_rename <old_name> <new_name>" } },
            { cmd_string_id::desc_port_params,            { "Lists all parameters of a port", "Listet alle Parameter eines Ports auf" } },
            { cmd_string_id::usage_port_params,           { "Usage: port_params <port_name>", "Verwendung: port_params <port_name>" } },
            { cmd_string_id::desc_port_set_param,         { "Sets a parameter of a port", "Setzt einen Parameter eines Ports" } },
            { cmd_string_id::usage_port_set_param,        { "Usage: port_set_param <port_name> <param_name> <value>", "Verwendung: port_set_param <port_name> <param_name> <value>" } },
            { cmd_string_id::desc_port_inject,            { "Injects hex data into a port", "Injiziert Hex-Daten in einen Port" } },
            { cmd_string_id::usage_port_inject,           { "Usage: port_inject <port_name> <hex_data> <in|out>", "Verwendung: port_inject <port_name> <hex_data> <in|out>" } },
            { cmd_string_id::desc_conn_rename,            { "Renames a connection", "Benennt eine Verbindung um" } },
            { cmd_string_id::usage_conn_rename,           { "Usage: conn_rename <old_name> <new_name>", "Verwendung: conn_rename <old_name> <new_name>" } },
            { cmd_string_id::desc_conn_set_fmt_in,        { "Sets the input format of a connection", "Setzt das Eingabeformat einer Verbindung" } },
            { cmd_string_id::usage_conn_set_fmt_in,       { "Usage: conn_set_fmt_in <conn_name> <format> [module]", "Verwendung: conn_set_fmt_in <conn_name> <format> [module]" } },
            { cmd_string_id::desc_conn_set_fmt_out,       { "Sets the output format of a connection", "Setzt das Ausgabeformat einer Verbindung" } },
            { cmd_string_id::usage_conn_set_fmt_out,      { "Usage: conn_set_fmt_out <conn_name> <format> [module]", "Verwendung: conn_set_fmt_out <conn_name> <format> [module]" } },
            { cmd_string_id::desc_mod_path_add,           { "Adds a module search path", "Fügt einen Modul-Suchpfad hinzu" } },
            { cmd_string_id::usage_mod_path_add,          { "Usage: mod_path_add <path>", "Verwendung: mod_path_add <path>" } },
            { cmd_string_id::desc_mod_path_rm,            { "Removes a module search path by index", "Entfernt einen Modul-Suchpfad nach Index" } },
            { cmd_string_id::usage_mod_path_rm,           { "Usage: mod_path_rm <index>", "Verwendung: mod_path_rm <index>" } },
            { cmd_string_id::err_param_invalid,           { "Invalid parameter value or out of range.", "Ungültiger Parameterwert oder außerhalb des Bereichs." } },
            { cmd_string_id::err_param_not_found,         { "Parameter not found.", "Parameter nicht gefunden." } },
            { cmd_string_id::desc_clean,                  { "Clears the terminal screen", "Leert den Terminalbildschirm" } },
            { cmd_string_id::desc_clear,                  { "Clears the terminal screen", "Leert den Terminalbildschirm" } },
            { cmd_string_id::desc_exit,                   { "Exits the application cleanly", "Beendet die Anwendung sauber" } },
            { cmd_string_id::desc_quit,                   { "Exits the application cleanly", "Beendet die Anwendung sauber" } }
        };

        auto it = translations.find(id);
        if (it != translations.end())
            return it->second[lang];
        
        return language_strings::unknown_type_message("cmd_string_id", static_cast<int>(id), lang).data();
    }
}