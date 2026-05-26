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
#include "data/port/port.hpp"
#include "resources/language.hpp"

#include <vector>
#include <string>
#include <tuple>
#include <utility>
#include <unordered_map>
#include <array>
#include <atomic>
#include <mutex>

#ifdef ADAM_CPU_X64
#include <immintrin.h>
#endif

namespace adam 
{
    struct port_info
    {
        string_hash name_hash;
        std::string type_name_str;
        port::direction direction;
    };

    struct processor_info
    {
        string_hash hash;
        std::string name_str;
    };

    struct module_info
    {
        string_hashed name;
        string_hashed path;
        uint32_t version;
        std::array<std::string, static_cast<size_t>(languages_count)> descriptions;
        std::vector<string_hashed> data_formats;
        std::vector<port_info> ports;
        std::vector<processor_info> filters;
        std::vector<processor_info> converters;
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
        using map_loaded_modules      = std::unordered_map<string_hashed, std::pair<uint32_t, string_hashed>>;

        map_available_modules&              available()     { return m_available_modules; }
        map_unavailable_modules&            unavailable()   { return m_unavailable_modules; }
        map_loaded_modules&                 loaded()        { return m_loaded_modules; }
        std::vector<string_hashed>&         paths()         { return m_paths; }
        std::unordered_map<string_hashed, module_info>& database() { return m_database; }

        const map_available_modules&        get_available()     const { return m_available_modules; }
        const map_unavailable_modules&      get_unavailable()   const { return m_unavailable_modules; }
        const map_loaded_modules&           get_loaded()        const { return m_loaded_modules; }
        const std::vector<string_hashed>&   get_paths()         const { return m_paths; }
        const std::unordered_map<string_hashed, module_info>& database() const { return m_database; }

        void lock() const
        {
            while (m_lock.test_and_set(std::memory_order_acquire))
            {
                while (m_lock.test(std::memory_order_relaxed))
                {
                    #ifdef ADAM_CPU_X64
                    _mm_pause(); // Hardware pause (no OS yield) to reduce power and memory bus saturation
                    #endif
                }
            }
        }

        void unlock() const { m_lock.clear(std::memory_order_release); }

        /** @brief Extracts the type and module names for a given port hash. */
        void extract_port_type_and_module(string_hash type_hash, string_hash module_hash, string_hashed& out_type, string_hashed& out_module) const;

        /** @brief Extracts the datatype and module names for a given data format hash. */
        void extract_datatype_and_module(string_hash datatype_hash, string_hash module_hash, string_hashed& out_datatype, string_hashed& out_module) const;

        void update_module_database(const string_hashed& name, const string_hashed& path, uint32_t version);

        void load_module(const string_hashed& name, const string_hashed& path, uint32_t version);

        void unload_module(const string_hashed& name);

        void clear();

    private:
        mutable std::atomic_flag    m_lock = ATOMIC_FLAG_INIT;
        map_available_modules       m_available_modules;
        map_unavailable_modules     m_unavailable_modules;
        map_loaded_modules          m_loaded_modules;
        std::unordered_map<string_hashed, module_info> m_database;
        std::vector<string_hashed>  m_paths;
    };
}