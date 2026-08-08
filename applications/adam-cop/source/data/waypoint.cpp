#include "waypoint.hpp"

using namespace adam::string_hashed_ct_literals;

namespace adam::cop
{
    static adam::configuration_parameter_list build_waypoint_params()
    {
        adam::configuration_parameter_list p;
        p.add(std::make_unique<adam::configuration_parameter_boolean>("enabled"_ct, true));
        p.add(std::make_unique<adam::configuration_parameter_integer>("color"_ct, 0xFF5919)); // Orange default
        return p;
    }

    waypoint::waypoint(const adam::string_hashed& item_name)
        : geo_location(item_name, build_waypoint_params())
    {
    }

    bool waypoint::is_enabled() const
    {
        auto* p = get_parameter<adam::configuration_parameter_boolean>("enabled"_ct);
        return p ? p->get_value() : true;
    }

    void waypoint::set_enabled(bool enabled)
    {
        if (auto* p = get_parameter<adam::configuration_parameter_boolean>("enabled"_ct)) p->set_value(enabled);
    }

    uint32_t waypoint::get_color() const
    {
        auto* p = get_parameter<adam::configuration_parameter_integer>("color"_ct);
        return p ? static_cast<uint32_t>(p->get_value()) : 0xFF5919;
    }

    void waypoint::set_color(uint32_t color)
    {
        if (auto* p = get_parameter<adam::configuration_parameter_integer>("color"_ct)) p->set_value(static_cast<int64_t>(color));
    }
}
