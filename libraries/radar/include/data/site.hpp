#pragma once

/**
 * @file    site.hpp
 * @author  dexus1337
 * @brief   Site configuration item representing a radar site
 * @version 1.0
 * @date    09.08.2026
 */

#include "geo-location.hpp"

namespace adam::lib::radar
{
    class site : public geo_location
    {
    public:
        site(const adam::string_hashed& item_name);
        virtual ~site() = default;

        bool is_enabled() const { return m_p_enabled->get_value(); }
        void set_enabled(bool enabled) { m_p_enabled->set_value(enabled); }

        uint32_t get_color() const { return static_cast<uint32_t>(m_p_color->get_value()); }
        void set_color(uint32_t color) { m_p_color->set_value(static_cast<int64_t>(color)); }

        int64_t get_sac() const { return m_p_sac->get_value(); }
        void set_sac(int64_t sac) { m_p_sac->set_value(sac); m_p_sacsic->set_value((sac << 8) | (m_p_sic->get_value() & 0xFF)); }

        int64_t get_sic() const { return m_p_sic->get_value(); }
        void set_sic(int64_t sic) { m_p_sic->set_value(sic); m_p_sacsic->set_value(((m_p_sac->get_value() & 0xFF) << 8) | (sic & 0xFF)); }

        int64_t get_sacsic() const { return m_p_sacsic->get_value(); }
        void set_sacsic(int64_t sacsic) { m_p_sacsic->set_value(sacsic); m_p_sac->set_value((sacsic >> 8) & 0xFF); m_p_sic->set_value(sacsic & 0xFF); }

        bool get_auto_retrieve_coords() const { return m_p_auto_retrieve_coords->get_value(); }
        void set_auto_retrieve_coords(bool val) { m_p_auto_retrieve_coords->set_value(val); }

        double get_range_nm() const { return m_p_range_nm->get_value(); }
        void set_range_nm(double range) { m_p_range_nm->set_value(range); }

        bool get_auto_calc_range() const { return m_p_auto_calc_range->get_value(); }
        void set_auto_calc_range(bool val) { m_p_auto_calc_range->set_value(val); }

        bool get_show_range() const { return m_p_show_range->get_value(); }
        void set_show_range(bool val) { m_p_show_range->set_value(val); }

        double get_range_alpha() const { return m_p_range_alpha->get_value(); }
        void set_range_alpha(double val) { m_p_range_alpha->set_value(val); }

        bool get_show_sector_crossings() const { return m_p_show_sector_crossings->get_value(); }
        void set_show_sector_crossings(bool val) { m_p_show_sector_crossings->set_value(val); }

        double get_sector_crossings_alpha() const { return m_p_sector_crossings_alpha->get_value(); }
        void set_sector_crossings_alpha(double val) { m_p_sector_crossings_alpha->set_value(val); }

    protected:
        adam::configuration_parameter_boolean* m_p_enabled                = nullptr;
        adam::configuration_parameter_integer* m_p_color                  = nullptr;
        adam::configuration_parameter_integer* m_p_sac                    = nullptr;
        adam::configuration_parameter_integer* m_p_sic                    = nullptr;
        adam::configuration_parameter_integer* m_p_sacsic                 = nullptr;
        adam::configuration_parameter_boolean* m_p_auto_retrieve_coords   = nullptr;
        adam::configuration_parameter_double*  m_p_range_nm               = nullptr;
        adam::configuration_parameter_boolean* m_p_auto_calc_range        = nullptr;
        adam::configuration_parameter_boolean* m_p_show_range             = nullptr;
        adam::configuration_parameter_double*  m_p_range_alpha            = nullptr;
        adam::configuration_parameter_boolean* m_p_show_sector_crossings  = nullptr;
        adam::configuration_parameter_double*  m_p_sector_crossings_alpha = nullptr;
    };
}
