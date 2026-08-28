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
        inline bool   get_show_azimuth()            const { return m_p_show_azimuth->get_value(); }
        inline double get_azimuth_alpha()           const { return m_p_azimuth_alpha->get_value(); }
        inline bool   get_show_sectors()            const { return m_p_show_sectors->get_value(); }
        inline double get_sectors_alpha()           const { return m_p_sectors_alpha->get_value(); }
        inline bool   get_show_name()               const { return m_p_show_name->get_value(); }
        inline bool   get_show_sacsic()             const { return m_p_show_sacsic->get_value(); }
        inline bool   get_show_rotation_duration()  const { return m_p_show_rotation_duration->get_value(); }

        inline void   set_show_range(bool val)              { m_p_show_range->set_value(val); }
        inline void   set_range_alpha(double val)           { m_p_range_alpha->set_value(val); }
        inline void   set_show_azimuth(bool val)            { m_p_show_azimuth->set_value(val); }
        inline void   set_azimuth_alpha(double val)         { m_p_azimuth_alpha->set_value(val); }
        inline void   set_show_sectors(bool val)            { m_p_show_sectors->set_value(val); }
        inline void   set_sectors_alpha(double val)         { m_p_sectors_alpha->set_value(val); }
        inline void   set_show_name(bool val)               { m_p_show_name->set_value(val); }
        inline void   set_show_sacsic(bool val)             { m_p_show_sacsic->set_value(val); }
        inline void   set_show_rotation_duration(bool val)  { m_p_show_rotation_duration->set_value(val); }

    protected:
        adam::configuration_parameter_boolean* m_p_show_range             = nullptr;
        adam::configuration_parameter_double*  m_p_range_alpha            = nullptr;
        adam::configuration_parameter_boolean* m_p_show_azimuth           = nullptr;
        adam::configuration_parameter_double*  m_p_azimuth_alpha          = nullptr;
        adam::configuration_parameter_boolean* m_p_show_sectors           = nullptr;
        adam::configuration_parameter_double*  m_p_sectors_alpha          = nullptr;
        adam::configuration_parameter_boolean* m_p_show_name              = nullptr;
        adam::configuration_parameter_boolean* m_p_show_sacsic            = nullptr;
        adam::configuration_parameter_boolean* m_p_show_rotation_duration = nullptr;
    };
}
