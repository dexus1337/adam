#pragma once

/**
 * @file    multi-sensor-tracker.hpp
 * @author  dexus1337
 * @brief   Multi-sensor tracker with direct flat atomic table SAC/SIC lookup
 * @version 1.0
 * @date    27.08.2026
 */

#include <adam-core.hpp>
#include "data/site.hpp"
#include <vector>
#include <atomic>
#include <memory>
#include <unordered_map>

namespace adam::lib::radar
{
    class multi_sensor_tracker
    {
    public:
        multi_sensor_tracker();
        virtual ~multi_sensor_tracker() = default;

        static inline uint16_t               make_sacsic_key(uint8_t sac, uint8_t sic)         { return (static_cast<uint16_t>(sac) << 8) | static_cast<uint16_t>(sic); }
        static inline uint16_t               make_sacsic_key(int64_t sacsic)                   { return static_cast<uint16_t>(sacsic & 0xFFFF); }

        inline site*                         get_site(uint8_t sac, uint8_t sic)          const { return m_sites_table[make_sacsic_key(sac, sic)].load(std::memory_order_relaxed); }
        inline site*                         get_site(int64_t sacsic)                    const { return m_sites_table[make_sacsic_key(sacsic)].load(std::memory_order_relaxed); }
        site*                                get_site(adam::string_hash name_hash)       const;
        inline bool                          has_site(uint8_t sac, uint8_t sic)          const { return m_sites_table[make_sacsic_key(sac, sic)].load(std::memory_order_relaxed) != nullptr; }
        inline bool                          has_site(int64_t sacsic)                    const { return m_sites_table[make_sacsic_key(sacsic)].load(std::memory_order_relaxed) != nullptr; }
        bool                                 has_site(adam::string_hash name_hash)       const;
        inline size_t                        get_site_count()                            const { return m_buf_list.size(); }
        std::vector<site*>                   get_sites()                                 const;

        void                                 register_site(site* s);
        void                                 unregister_site(site* s);
        void                                 unregister_site(uint8_t sac, uint8_t sic);
        void                                 unregister_site(uint16_t sacsic_key);
        inline void                          unregister_site(int64_t sacsic)             { unregister_site(make_sacsic_key(sacsic)); }
        void                                 unregister_site(adam::string_hash name_hash);
        void                                 clear_sites();

    private:
        void                                 sync_buffers();

        std::unique_ptr<std::atomic<site*>[]>             m_sites_table;
        mutable std::atomic_flag                          m_write_lock = ATOMIC_FLAG_INIT;
        std::vector<site*>                                m_sites_list;
        std::unordered_map<adam::string_hash, site*>      m_sites_by_name;

        adam::map_double_buffer<adam::string_hash, site*> m_buf_by_name;
        adam::vector_double_buffer<site*>                 m_buf_list;
    };
}
