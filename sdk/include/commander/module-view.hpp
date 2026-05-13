#pragma once

/**
 * @file    module-view.hpp
 * @author  dexus1337
 * @brief   Defines a view for module elements in the commander.
 * @version 1.0
 * @date    13.05.2026
 */

#include "api/sdk-api.hpp"
#include "controller/registry-module-manager.hpp"
#include <vector>
#include <string>

namespace adam 
{
    /**
     * @class module_view
     * @brief Holds a local view of the controller's available, unavailable, and loaded modules.
     */
    class ADAM_SDK_API module_view
    {
    public:
        registry_module_manager::map_available_modules&       available()     { return m_available_modules; }
        registry_module_manager::map_unavailable_modules&     unavailable()   { return m_unavailable_modules; }
        registry_module_manager::map_loaded_modules&          loaded()        { return m_loaded_modules; }
        std::vector<string_hashed>&                             paths()         { return m_paths; }

        const registry_module_manager::map_available_modules&     get_available()     const { return m_available_modules; }
        const registry_module_manager::map_unavailable_modules&   get_unavailable()   const { return m_unavailable_modules; }
        const registry_module_manager::map_loaded_modules&        get_loaded()        const { return m_loaded_modules; }
        const std::vector<string_hashed>&                           get_paths()         const { return m_paths; }

        void clear()
        {
            m_available_modules.clear();
            m_unavailable_modules.clear();
            m_loaded_modules.clear();
            m_paths.clear();
        }

    private:
        registry_module_manager::map_available_modules    m_available_modules;
        registry_module_manager::map_unavailable_modules  m_unavailable_modules;
        registry_module_manager::map_loaded_modules       m_loaded_modules;
        std::vector<string_hashed>                          m_paths;
    };
}