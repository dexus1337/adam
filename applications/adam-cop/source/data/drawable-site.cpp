/**
 * @file    drawable-site.cpp
 * @author  dexus1337
 * @brief   Implementation of drawable_site for adam-cop
 * @version 1.0
 * @date    27.08.2026
 */

#include "drawable-site.hpp"

using namespace adam::string_hashed_ct_literals;

namespace adam::cop
{
    static adam::configuration_parameter_list build_drawable_site_params(const adam::configuration_parameter_list& child_params)
    {
        adam::configuration_parameter_list p(child_params);
        p.add(std::make_unique<adam::configuration_parameter_boolean>("show_range"_ct, true));
        p.add(std::make_unique<adam::configuration_parameter_double>("range_alpha"_ct, 0.25));
        p.add(std::make_unique<adam::configuration_parameter_boolean>("show_sector_crossings"_ct, false));
        p.add(std::make_unique<adam::configuration_parameter_double>("sector_crossings_alpha"_ct, 0.60));
        return p;
    }

    drawable_site::drawable_site(const adam::string_hashed& item_name, const adam::configuration_parameter_list& child_params)
        : site(item_name, build_drawable_site_params(child_params))
        , m_p_show_range(get_parameter<adam::configuration_parameter_boolean>("show_range"_ct))
        , m_p_range_alpha(get_parameter<adam::configuration_parameter_double>("range_alpha"_ct))
        , m_p_show_sector_crossings(get_parameter<adam::configuration_parameter_boolean>("show_sector_crossings"_ct))
        , m_p_sector_crossings_alpha(get_parameter<adam::configuration_parameter_double>("sector_crossings_alpha"_ct))
    {
    }
}
