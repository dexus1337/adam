#include "resources/language-strings.hpp"


#include <unordered_map>


namespace adam
{
    std::string_view language_strings::get_controller_status_text(controller::status stat, language lang)
    {
        static const std::unordered_map<int, std::array<std::string_view, languages_count>> translations =
        {
            {
                static_cast<int>(controller::status::status_success),
                {
                    "Success", "Erfolg"
                }
            },
            {
                static_cast<int>(controller::status::status_queue_existing),
                {
                    "A requested queue already exists!",                        "Die angeforderte Warteschlange existiert bereits!"
                }
            },
            {
                static_cast<int>(controller::status::status_queue_not_existing),
                {
                    "The requested queue does not exists!",                     "Die angeforderte Warteschlange existiert nicht!"
                }
            },
            {
                static_cast<int>(controller::status::status_queue_unavailable),
                {
                    "The requested queue is not available!",                    "Die angeforderte Warteschlange ist nicht verfügbar!"
                }
            },
            {
                static_cast<int>(controller::status::status_queue_unauthorized),
                {
                    "The authorization for the requested qeue failed!",         "Die Authentifizierung für angeforderte Warteschlange ist fehlgeschlagen!"
                }
            },
            {
                static_cast<int>(controller::status::status_queue_failed_create),
                {
                    "The requested queue could not be correctly created!",      "Die angeforderte Warteschlange konnte nicht korrekt erstellt werden!"
                }
            },
            {
                static_cast<int>(controller::status::status_queue_failed_destroy),
                {
                    "The requested queue could not be correctly destroyed!",    "Die angeforderte Warteschlange konnte nicht korrekt entfernt werden!"
                }
            },
            {
                static_cast<int>(controller::status::status_unknown_master_request),
                {
                    "An unknown request was pushed to the master queue!",       "Eine unbekannte Aufforderung wurde and die Haupt-Warteschlange gesendet!"
                }
            }
        };

        auto val    = static_cast<int>(stat);
        auto it     = translations.find(val);

        if (it != translations.end())
            return it->second[static_cast<int>(lang)];
        
        return unknown_type_message("controller::status", val, lang);
    }

    std::string_view language_strings::get_response_type_text(response_status typ, language lang)
    {
        static const std::unordered_map<int, std::array<std::string_view, languages_count>> translations =
        {
            {
                static_cast<int>(response_status::success),
                {
                    "Last submitted request successfully executed!",    "Die letzte Anforderung wurde mit Erfolg ausgeführt."
                }
            },
            {
                static_cast<int>(response_status::unknown),
                {
                    "Last submitted request was unknown!",              "Die letzte Anforderung ist unbekannt!"
                }
            },
            {
                static_cast<int>(response_status::invalid),
                {
                    "Last submitted request was invalid!",              "Die letzte Anforderung ist fehlerhaft!"
                }
            },
            {
                static_cast<int>(response_status::failed),
                {
                    "Last submitted request failed!",                   "Die letzte Anforderung ist fehlgeschlagen!"
                }
            },
        };

        auto val    = static_cast<int>(typ);
        auto it     = translations.find(val);

        if (it != translations.end())
            return it->second[static_cast<int>(lang)];
        
        return unknown_type_message("response_status", val, lang);
    }

    std::string_view language_strings::unknown_type_message(std::string_view type, int val, language lang)
    {
        // Use thread_local to safely return a string_view to a persistent object without dangling
        thread_local std::string fallback_msg;

        switch (lang)
        {
        default:
        case language_english:
            fallback_msg = std::format("No available text for value {:d} in status of type \"{}\"", val, type);
            break;
        case language_german:
            fallback_msg = std::format("Kein hinterlegter Text für Wert {:d} im Status-Typ \"{}\"", val, type);
            break;
        }

        return fallback_msg;
    }
}