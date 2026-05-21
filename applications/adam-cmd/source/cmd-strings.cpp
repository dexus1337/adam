#include "cmd-strings.hpp"

#include <unordered_map>
#include <array>

namespace adam::cmd
{
    const char* get_cmd_string(cmd_string_id id, adam::language lang)
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
            { cmd_string_id::desc_exit,                   { "Exits the application cleanly", "Beendet die Anwendung sauber" } },
            { cmd_string_id::desc_quit,                   { "Exits the application cleanly", "Beendet die Anwendung sauber" } }
        };

        auto it = translations.find(id);
        if (it != translations.end())
            return it->second[lang];
        
        return language_strings::unknown_type_message("cmd_string_id", static_cast<int>(id), lang).data();
    }
}