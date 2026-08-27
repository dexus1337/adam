#pragma once

/**
 * @file    drawable-site.hpp
 * @author  dexus1337
 * @brief   Radar site configuration item with rendering options for adam-cop
 * @version 1.0
 * @date    27.08.2026
 */

#include <adam-core.hpp>
#include <lib-radar.hpp>

namespace adam::cop
{
    class drawable_site : public adam::lib::radar::site
    {
    public:
        drawable_site(const adam::string_hashed& item_name, const adam::configuration_parameter_list& child_params = adam::configuration_parameter_list());
        virtual ~drawable_site() = default;

        inline bool   get_show_range()              const { return m_p_show_range->get_value(); }
        inline double get_range_alpha()             const { return m_p_range_alpha->get_value(); }
        inline bool   get_show_sector_crossings()   const { return m_p_show_sector_crossings->get_value(); }
        inline double get_sector_crossings_alpha()  const { return m_p_sector_crossings_alpha->get_value(); }

        inline void   set_show_range(bool val)               { m_p_show_range->set_value(val); }
        inline void   set_range_alpha(double val)            { m_p_range_alpha->set_value(val); }
        inline void   set_show_sector_crossings(bool val)    { m_p_show_sector_crossings->set_value(val); }
        inline void   set_sector_crossings_alpha(double val) { m_p_sector_crossings_alpha->set_value(val); }

    protected:
        adam::configuration_parameter_boolean* m_p_show_range             = nullptr;
        adam::configuration_parameter_double*  m_p_range_alpha            = nullptr;
        adam::configuration_parameter_boolean* m_p_show_sector_crossings  = nullptr;
        adam::configuration_parameter_double*  m_p_sector_crossings_alpha = nullptr;
    };
}
