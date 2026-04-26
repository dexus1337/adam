#pragma once

/**
 * @file        controller.hpp
 * @author      dexus1337
 * @brief       Defines the core ADAM controller class which manages the entire system
 * @version     1.0
 * @date        25.04.2026
 */

 
#include "api/api.hpp"

#include <unordered_map>

#include "string/string-hashed.hpp"


namespace adam 
{
    class module;

    /**
     * @class controller
     * @brief The main controller class for the ADAM system, responsible for managing all components and orchestrating their interactions.
     */
    class ADAM_SDK_API controller 
    {
    public:

        /**
         * @brief Constructs a new controller object.
         */
        controller();

        /**
         * @brief Destroys the controller object and cleans up resources.
         */
        ~controller();

        /** @brief Retrieves a reference to the map of all available modules. */
        const std::unordered_map<string_hashed, string_hashed>& get_available_modules() const { return m_available_modules; }

        /** @brief Retrieves a reference to the map of all loaded modules. */
        const std::unordered_map<string_hashed, const module*>& get_loaded_modules() const { return m_loaded_modules; }

        /** @brief Retrieves a pointer to a loaded module by its hashed name. */
        const module* get_loaded_module(const string_hashed& name) const;

        /** @brief Scans the specified directory for module shared libraries, loads them, and registers their modules in the system. */
        bool scan_for_modules(std::string_view directory = "");

        /** @brief Loads a module by its hashed name from the available modules and registers it in the loaded modules map. */
        bool load_module(const string_hashed& name, const module** out_module = nullptr);

    protected:

        std::unordered_map<string_hashed, string_hashed> m_available_modules;   /**< A map of available modules in the system, indexed by their hashed string names for efficient lookup. */
        std::unordered_map<string_hashed, const module*> m_loaded_modules;      /**< A map of loaded modules in the system, indexed by their hashed string names for efficient lookup. */

        memory_manager& m_memory_manager = memory_manager::get();               /**< Reference to the memory manager singleton for managing memory buffers across modules. */
    };
}