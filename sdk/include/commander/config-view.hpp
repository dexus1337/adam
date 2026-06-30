#pragma once

/**
 * @file    config-view.hpp
 * @author  Antigravity
 * @brief   Defines a view for configurations in the commander.
 */

#include "api/api-sdk.hpp"
#include "types/string-hashed.hpp"
#include "os/os.hpp"

#include <vector>
#include <string>
#include <unordered_map>
#include <atomic>

namespace adam 
{
    struct config_info
    {
        uint32_t path_idx;
        std::string filename;   ///< Bare filename only (e.g. "my_config.bin")
        string_hashed name;
        string_hashed description;
        uint64_t created;
        uint64_t modified;
        uint32_t port_count;
        uint32_t processor_count;
        uint32_t connection_count;
    };

    /**
     * @class config_view
     * @brief Holds a local view of the controller's available configurations and configuration paths.
     */
    class ADAM_SDK_API config_view
    {
    public:
        using map_available_configs = std::unordered_map<string_hashed, config_info>;

        config_view() = default;

        map_available_configs&              available()     { return m_available_configs; }
        std::vector<string_hashed>&         paths()         { return m_paths; }

        const map_available_configs&        get_available() const { return m_available_configs; }
        const std::vector<string_hashed>&   get_paths()     const { return m_paths; }

        void lock()   const { spinlock::acquire(m_lock); }
        void unlock() const { spinlock::release(m_lock); }

        void clear();

    private:
        map_available_configs       m_available_configs;
        std::vector<string_hashed>  m_paths;
        mutable std::atomic_flag    m_lock = ATOMIC_FLAG_INIT;
    };
}
