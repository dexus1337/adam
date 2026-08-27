#include "data/site.hpp"

using namespace adam::string_hashed_ct_literals;

namespace adam::lib::radar
{
    static adam::configuration_parameter_list build_site_params()
    {
        adam::configuration_parameter_list p;
        p.add(std::make_unique<adam::configuration_parameter_boolean>("enabled"_ct, true));
        p.add(std::make_unique<adam::configuration_parameter_integer>("color"_ct, 0x00E5FF)); // Cyan default
        p.add(std::make_unique<adam::configuration_parameter_integer>("sac"_ct, 0));
        p.add(std::make_unique<adam::configuration_parameter_integer>("sic"_ct, 0));
        p.add(std::make_unique<adam::configuration_parameter_integer>("sacsic"_ct, 0));
        p.add(std::make_unique<adam::configuration_parameter_double>("range_nm"_ct, 120.0));
        p.add(std::make_unique<adam::configuration_parameter_boolean>("auto_retrieve_coords"_ct, false));
        p.add(std::make_unique<adam::configuration_parameter_boolean>("auto_calc_range"_ct, false));
        p.add(std::make_unique<adam::configuration_parameter_boolean>("show_range"_ct, true));
        p.add(std::make_unique<adam::configuration_parameter_double>("range_alpha"_ct, 0.25));
        p.add(std::make_unique<adam::configuration_parameter_boolean>("show_sector_crossings"_ct, false));
        p.add(std::make_unique<adam::configuration_parameter_double>("sector_crossings_alpha"_ct, 0.60));
        return p;
    }

    site::site(const adam::string_hashed& item_name)
        : geo_location(item_name, build_site_params())
        , m_p_enabled(get_parameter<adam::configuration_parameter_boolean>("enabled"_ct))
        , m_p_color(get_parameter<adam::configuration_parameter_integer>("color"_ct))
        , m_p_sac(get_parameter<adam::configuration_parameter_integer>("sac"_ct))
        , m_p_sic(get_parameter<adam::configuration_parameter_integer>("sic"_ct))
        , m_p_sacsic(get_parameter<adam::configuration_parameter_integer>("sacsic"_ct))
        , m_p_range_nm(get_parameter<adam::configuration_parameter_double>("range_nm"_ct))
        , m_p_auto_retrieve_coords(get_parameter<adam::configuration_parameter_boolean>("auto_retrieve_coords"_ct))
        , m_p_auto_calc_range(get_parameter<adam::configuration_parameter_boolean>("auto_calc_range"_ct))
        , m_p_show_range(get_parameter<adam::configuration_parameter_boolean>("show_range"_ct))
        , m_p_range_alpha(get_parameter<adam::configuration_parameter_double>("range_alpha"_ct))
        , m_p_show_sector_crossings(get_parameter<adam::configuration_parameter_boolean>("show_sector_crossings"_ct))
        , m_p_sector_crossings_alpha(get_parameter<adam::configuration_parameter_double>("sector_crossings_alpha"_ct))
    {
    }
}
