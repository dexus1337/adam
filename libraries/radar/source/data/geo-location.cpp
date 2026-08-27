/**
 * @file    geo-location.cpp
 * @author  dexus1337
 * @brief   Base geographic location configuration item implementation
 * @version 1.0
 * @date    09.08.2026
 */

#include "data/geo-location.hpp"

using namespace adam::string_hashed_ct_literals;

namespace adam::lib::radar
{
    static adam::configuration_parameter_list build_geo_location_params(const adam::configuration_parameter_list& child_params)
    {
        adam::configuration_parameter_list p(child_params);
        p.add(std::make_unique<adam::configuration_parameter_double>("lat"_ct, 0.0));
        p.add(std::make_unique<adam::configuration_parameter_double>("lon"_ct, 0.0));
        p.add(std::make_unique<adam::configuration_parameter_string>("name"_ct, ""_ct));
        return p;
    }

    geo_location::geo_location(const adam::string_hashed& item_name, const adam::configuration_parameter_list& child_params)
        : adam::configuration_item(item_name, build_geo_location_params(child_params))
        , m_p_lat(get_parameter<adam::configuration_parameter_double>("lat"_ct))
        , m_p_lon(get_parameter<adam::configuration_parameter_double>("lon"_ct))
        , m_p_name(get_parameter<adam::configuration_parameter_string>("name"_ct))
    {
    }
}
