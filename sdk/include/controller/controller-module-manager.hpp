#pragma once

/**
 * @file    controller-module-manager.hpp
 * @author  dexus1337
 * @brief   Defines a manager for handling external modules within the ADAM system.
 * @version 1.0
 * @date    08.05.2026
 */


#include "api/api.hpp"
#include "module/module.hpp"
#include "types/string-hashed.hpp"
#include "resources/language.hpp"

#include <unordered_map>
#include <string_view>
#include <utility>

namespace adam 
{
    class controller;
    class module;

    /**
     * @class   controller_module_manager
     * @brief   Manages the discovery, loading, and lifecycle of external modules in the ADAM system.
     */
    class ADAM_SDK_API controller_module_manager
    {
    public:
        enum log_event
        {
            module_requires_newer_sdk,
            module_requires_newer_sdk_cannot_load
        };

        static std::string_view get_log_event_text(log_event event, language lang);

        using map_available_modules     = std::unordered_map<string_hashed, std::pair<uint32_t, string_hashed>>; /**< A type alias for a map of available modules, storing the version and file path indexed by hashed name. */
        using map_unavailable_modules   = std::unordered_map<string_hashed, std::pair<uint32_t, string_hashed>>; /**< A type alias for a map of unavailable modules, storing the incompatability reason and file path indexed by hashed name. */
        using map_loaded_modules        = std::unordered_map<string_hashed, const module*>;                      /**< A type alias for a map of loaded modules, indexed by their hashed string names. */

        /** @brief Constructs a new module manager object. */
        controller_module_manager(controller& ctrl);

        /** @brief Destroys the module manager object and cleans up resources. */
        ~controller_module_manager();

        /** @brief Retrieves a reference to the map of all available modules. */
        const map_available_modules& get_available_modules() const { return m_available_modules; }

        /** @brief Retrieves a reference to the map of all unavailable modules. */
        const map_unavailable_modules& get_unavailable_modules() const { return m_unavailable_modules; }

        /** @brief Retrieves a reference to the map of all loaded modules. */
        const map_loaded_modules& get_loaded_modules() const { return m_loaded_modules; }

        /** @brief Retrieves a pointer to a loaded module by its hashed name. */
        const module* get_loaded_module(const string_hashed& name) const;

        /** @brief Scans the specified directory for module shared libraries, and registers them in the system. */
        bool scan_for_modules(string_hashed::view directory = "");

        /** @brief Loads a module by its hashed name from the available modules and registers it in the loaded modules map. */
        bool load_module(const string_hashed& name, const module** out_module = nullptr);

    private:
        controller&             m_controller;           /**< A reference to the controller, used for accessing shared resources and logging. */
        map_available_modules   m_available_modules;    /**< A map of available modules in the system, indexed by their hashed string names for efficient lookup. */
        map_unavailable_modules m_unavailable_modules;  /**< A map of unavailable modules in the system, indexed by their hashed string names for efficient lookup. */
        map_loaded_modules      m_loaded_modules;       /**< A map of loaded modules in the system, indexed by their hashed string names for efficient lookup. */
    };
}