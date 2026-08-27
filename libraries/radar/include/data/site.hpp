#pragma once

/**
 * @file    site.hpp
 * @author  dexus1337
 * @brief   Site configuration item representing a radar site
 * @version 1.0
 * @date    09.08.2026
 */

#include "geo-location.hpp"
#include <atomic>

namespace adam::lib::radar
{
    class site : public geo_location
    {
    public:
        site(const adam::string_hashed& item_name, const adam::configuration_parameter_list& child_params = adam::configuration_parameter_list());
        virtual ~site() = default;

        static uint32_t generate_random_color();

        inline bool     get_auto_calc_range()       const { return m_p_auto_calc_range->get_value(); }
        inline bool     get_auto_retrieve_coords()  const { return m_p_auto_retrieve_coords->get_value(); }
        inline uint32_t get_color()                 const { return static_cast<uint32_t>(m_p_color->get_value()); }
        inline double   get_current_azimuth_deg()   const { return m_current_azimuth_deg.load(std::memory_order_relaxed); }
        inline uint8_t  get_current_sector()        const { return m_current_sector.load(std::memory_order_relaxed); }
        inline bool     is_enabled()                const { return m_p_enabled->get_value(); }
        inline uint64_t get_last_north_marker_time()const { return m_last_north_marker_time.load(std::memory_order_relaxed); }
        inline uint64_t get_last_sector_time()      const { return m_last_sector_time.load(std::memory_order_relaxed); }
        inline double   get_range_nm()              const { return m_p_range_nm->get_value(); }
        inline double   get_rotation_period_s()     const { return m_rotation_period_s.load(std::memory_order_relaxed); }
        inline double   get_rotation_rpm()          const { double p = m_rotation_period_s.load(std::memory_order_relaxed); return p > 0.0 ? (60.0 / p) : 0.0; }
        inline int64_t  get_sac()                   const { return (m_p_sacsic->get_value() >> 8) & 0xFF; }
        inline int64_t  get_sacsic()                const { return m_p_sacsic->get_value(); }
        inline int64_t  get_sic()                   const { return m_p_sacsic->get_value() & 0xFF; }
        inline bool     has_live_rotation()         const { return m_last_sector_time.load(std::memory_order_relaxed) != 0; }

        inline void     set_auto_calc_range(bool val)             { m_p_auto_calc_range->set_value(val); }
        inline void     set_auto_retrieve_coords(bool val)        { m_p_auto_retrieve_coords->set_value(val); }
        inline void     set_color(uint32_t color)                 { m_p_color->set_value(static_cast<int64_t>(color)); }
        inline void     set_current_azimuth_deg(double deg)       { m_current_azimuth_deg.store(deg, std::memory_order_relaxed); }
        inline void     set_current_sector(uint8_t sector)        { m_current_sector.store(sector, std::memory_order_relaxed); }
        inline void     set_enabled(bool enabled)                 { m_p_enabled->set_value(enabled); }
        inline void     set_last_north_marker_time(uint64_t t)    { m_last_north_marker_time.store(t, std::memory_order_relaxed); }
        inline void     set_last_sector_time(uint64_t t)          { m_last_sector_time.store(t, std::memory_order_relaxed); }
        inline void     set_range_nm(double range)                { m_p_range_nm->set_value(range); }
        inline void     set_rotation_period_s(double period)      { m_rotation_period_s.store(period, std::memory_order_relaxed); }
        inline void     set_sac(int64_t sac)                      { m_p_sacsic->set_value(((sac & 0xFF) << 8) | (m_p_sacsic->get_value() & 0xFF)); }
        inline void     set_sacsic(int64_t sacsic)                { m_p_sacsic->set_value(sacsic); }
        inline void     set_sic(int64_t sic)                      { m_p_sacsic->set_value((m_p_sacsic->get_value() & 0xFF00) | (sic & 0xFF)); }

    protected:
        adam::configuration_parameter_boolean* m_p_enabled              = nullptr;
        adam::configuration_parameter_integer* m_p_color                = nullptr;
        adam::configuration_parameter_integer* m_p_sacsic               = nullptr;
        adam::configuration_parameter_boolean* m_p_auto_retrieve_coords = nullptr;
        adam::configuration_parameter_double*  m_p_range_nm             = nullptr;
        adam::configuration_parameter_boolean* m_p_auto_calc_range      = nullptr;

        std::atomic<uint8_t>                   m_current_sector         = 0;
        std::atomic<double>                    m_current_azimuth_deg    = 0.0;
        std::atomic<double>                    m_rotation_period_s      = 0.0;
        std::atomic<uint64_t>                  m_last_sector_time       = 0;
        std::atomic<uint64_t>                  m_last_north_marker_time = 0;
    };
}
