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
    }

    float geo_location::get_lat() const
    {
        auto* p = get_parameter<adam::configuration_parameter_double>("lat"_ct);
        return p ? static_cast<float>(p->get_value()) : 0.0f;
    }

    float geo_location::get_lon() const
    {
        auto* p = get_parameter<adam::configuration_parameter_double>("lon"_ct);
        return p ? static_cast<float>(p->get_value()) : 0.0f;
    }

    std::string geo_location::get_label() const
    {
        auto* p = get_parameter<adam::configuration_parameter_string>("name"_ct);
        return p ? p->get_value().data() : "";
    }

    void geo_location::set_lat(float lat)
    {
        if (auto* p = get_parameter<adam::configuration_parameter_double>("lat"_ct)) p->set_value(static_cast<double>(lat));
    }

    void geo_location::set_lon(float lon)
    {
        if (auto* p = get_parameter<adam::configuration_parameter_double>("lon"_ct)) p->set_value(static_cast<double>(lon));
    }

    void geo_location::set_label(const std::string& name)
    {
        if (auto* p = get_parameter<adam::configuration_parameter_string>("name"_ct)) p->set_value(adam::string_hashed(name));
    }
}
