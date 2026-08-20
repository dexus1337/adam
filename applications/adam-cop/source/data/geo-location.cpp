#include "geo-location.hpp"

using namespace adam::string_hashed_ct_literals;

namespace adam::cop
{
    static adam::configuration_parameter_list build_geo_location_params(const adam::configuration_parameter_list& child_params)
    {
        adam::configuration_parameter_list p(child_params);
        if (!p.get("lat"_ct)) p.add(std::make_unique<adam::configuration_parameter_double>("lat"_ct, 0.0));
        if (!p.get("lon"_ct)) p.add(std::make_unique<adam::configuration_parameter_double>("lon"_ct, 0.0));
        if (!p.get("name"_ct)) p.add(std::make_unique<adam::configuration_parameter_string>("name"_ct, ""_ct));
        return p;
    }

    geo_location::geo_location(const adam::string_hashed& item_name, const adam::configuration_parameter_list& child_params)
        : adam::configuration_item(item_name, build_geo_location_params(child_params))
    {
        cache_geo_parameters();
    }

    float geo_location::get_lat() const
    {
        if (!m_p_lat) return 0.0f;
        return static_cast<float>(m_p_lat->get_value());
    }

    float geo_location::get_lon() const
    {
        if (!m_p_lon) return 0.0f;
        return static_cast<float>(m_p_lon->get_value());
    }

    const adam::string_hashed& geo_location::get_label() const
    {
        if (!m_p_name)
        {
            static const adam::string_hashed empty_name("");
            return empty_name;
        }

        return m_p_name->get_value();
    }

    void geo_location::set_lat(float lat)
    {
        if (!m_p_lat) return;
        m_p_lat->set_value(static_cast<double>(lat));
    }

    void geo_location::set_lon(float lon)
    {
        if (!m_p_lon) return;
        m_p_lon->set_value(static_cast<double>(lon));
    }

    void geo_location::set_label(const adam::string_hashed& name)
    {
        if (!m_p_name) return;
        m_p_name->set_value(name);
    }

    void geo_location::cache_geo_parameters()
    {
        m_p_lat  = get_parameter<adam::configuration_parameter_double>("lat"_ct);
        m_p_lon  = get_parameter<adam::configuration_parameter_double>("lon"_ct);
        m_p_name = get_parameter<adam::configuration_parameter_string>("name"_ct);
    }
}
