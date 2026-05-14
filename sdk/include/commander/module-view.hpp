#pragma once

/**
 * @file    module-view.hpp
 * @author  dexus1337
 * @brief   Defines a view for module elements in the commander.
 * @version 1.0
 * @date    13.05.2026
 */

#include "api/sdk-api.hpp"
#include "types/string-hashed.hpp"

#include <vector>
#include <string>
#include <tuple>
#include <utility>
#include <unordered_map>

namespace adam 
{
    class module;

    /**
     * @class module_view
     * @brief Holds a local view of the controller's available, unavailable, and loaded modules.
     */
    class ADAM_SDK_API module_view
    {
    public:
        using map_available_modules   = std::unordered_map<string_hashed, std::pair<uint32_t, string_hashed>>;
        using map_unavailable_modules = std::unordered_map<string_hashed, std::tuple<uint32_t, string_hashed, uint8_t>>;
        using map_loaded_modules      = std::unordered_map<string_hashed, const module*>;
        using handle_map              = std::unordered_map<string_hashed, void*>;

        map_available_modules&              available()     { return m_available_modules; }
        map_unavailable_modules&            unavailable()   { return m_unavailable_modules; }
        map_loaded_modules&                 loaded()        { return m_loaded_modules; }
        handle_map&                         handles()       { return m_handles; }
        std::vector<string_hashed>&         paths()         { return m_paths; }

        const map_available_modules&        get_available()     const { return m_available_modules; }
        const map_unavailable_modules&      get_unavailable()   const { return m_unavailable_modules; }
        const map_loaded_modules&           get_loaded()        const { return m_loaded_modules; }
        const handle_map&                   get_handles()       const { return m_handles; }
        const std::vector<string_hashed>&   get_paths()         const { return m_paths; }

        /** @brief Extracts the type and module names for a given port hash. */
        void extract_port_type_and_module(string_hashed::hash_datatype type_hash, string_hashed::hash_datatype module_hash, string_hashed& out_type, string_hashed& out_module) const;

        void load_module(const string_hashed& name, const string_hashed& path);

        void unload_module(const string_hashed& name);

        void clear();

    private:
        map_available_modules       m_available_modules;
        map_unavailable_modules     m_unavailable_modules;
        map_loaded_modules          m_loaded_modules;
        handle_map                  m_handles;
        std::vector<string_hashed>  m_paths;
    };
}