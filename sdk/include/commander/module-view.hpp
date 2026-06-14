#pragma once

/**
 * @file    module-view.hpp
 * @author  dexus1337
 * @brief   Defines a view for module elements in the commander.
 * @version 1.0
 * @date    13.05.2026
 */

#include "api/api-sdk.hpp"
#include "types/string-hashed.hpp"
#include "data/port.hpp"
#include "resources/language.hpp"

#include <vector>
#include <string>
#include <tuple>
#include <utility>
#include <unordered_map>
#include <array>
#include <atomic>
#include <mutex>

#include "os/os.hpp"

namespace adam 
{
    class module;

    struct port_info
    {
        string_hashed name;
        port::direction direction;
    };

    struct processor_info
    {
        string_hashed name;
        bool is_filter;
    };

    struct module_info
    {
        string_hashed name;
        string_hashed path;
        uint32_t version;
        std::array<std::string, static_cast<size_t>(languages_count)> descriptions;
        std::vector<string_hashed> data_formats;
        std::vector<port_info> ports;
        std::vector<processor_info> processors;
    };

    /**
     * @class module_view
     * @brief Holds a local view of the controller's available, unavailable, and loaded modules.
     */
    class ADAM_SDK_API module_view
    {
    public:
        using map_available_modules   = std::unordered_map<string_hashed, std::pair<uint32_t, string_hashed>>;
        using map_unavailable_modules = std::unordered_map<string_hashed, std::tuple<uint32_t, string_hashed, uint8_t>>;
        using map_loaded_modules      = std::unordered_map<string_hashed, std::pair<uint32_t, module*>>;
        using map_internal_modules    = std::unordered_map<string_hashed, const module*>;
        using map_database            = std::unordered_map<string_hashed, module_info>;

        module_view();

        map_available_modules&              available()     { return m_available_modules; }
        map_unavailable_modules&            unavailable()   { return m_unavailable_modules; }
        map_loaded_modules&                 loaded()        { return m_loaded_modules; }
        map_internal_modules&               internal()      { return m_internal_modules; }
        map_database&                       database()      { return m_database; }
        std::vector<string_hashed>&         paths()         { return m_paths; }

        const map_available_modules&        get_available()     const { return m_available_modules; }
        const map_unavailable_modules&      get_unavailable()   const { return m_unavailable_modules; }
        const map_loaded_modules&           get_loaded()        const { return m_loaded_modules; }
        const map_internal_modules&         get_internal()      const { return m_internal_modules; }
        const map_database&                 database()          const { return m_database; }
        const std::vector<string_hashed>&   get_paths()         const { return m_paths; }

        void lock()   const { spinlock::acquire(m_lock); }
        void unlock() const { spinlock::release(m_lock); }

        /** @brief Extracts the type and module names for a given port hash. */
        void extract_port_type_and_module(string_hash type_hash, string_hash module_hash, string_hashed& out_type, string_hashed& out_module) const;

        /** @brief Extracts the datatype and module names for a given data format hash. */
        void extract_datatype_and_module(string_hash datatype_hash, string_hash module_hash, string_hashed& out_datatype, string_hashed& out_module) const;

        /** @brief Extracts the type and module names for a given data processor hash. */
        void extract_processor_type_and_module(string_hash type_hash, string_hash module_hash, string_hashed& out_type, string_hashed& out_module) const;

        void update_module_database(const string_hashed& name, const string_hashed& path, uint32_t version, module* mod = nullptr);

        void register_internal_module(const module* mod);
        const module* get_module(string_hash name_hash) const;
        bool is_module_loaded(string_hash name_hash) const;
        bool is_module_internal(string_hash name_hash) const;

        void load_module(const string_hashed& name, const string_hashed& path, uint32_t version);

        void unload_module(const string_hashed& name);

        void clear();

    private:
        mutable std::atomic_flag                            m_lock = ATOMIC_FLAG_INIT;
        map_available_modules                               m_available_modules;
        map_unavailable_modules                             m_unavailable_modules;
        map_loaded_modules                                  m_loaded_modules;
        std::unordered_map<string_hashed, void*>            m_handles;
        std::unordered_map<string_hashed, module_info>      m_database;
        std::vector<string_hashed>                          m_paths;
        std::unordered_map<string_hashed, const module*>    m_internal_modules;
    };
}