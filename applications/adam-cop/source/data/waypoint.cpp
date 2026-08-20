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
        cache_waypoint_parameters();
    }

    bool waypoint::is_enabled() const
    {
        if (!m_p_enabled) return true;
        return m_p_enabled->get_value();
    }

    void waypoint::set_enabled(bool enabled)
    {
        if (!m_p_enabled) return;
        m_p_enabled->set_value(enabled);
    }

    uint32_t waypoint::get_color() const
    {
        if (!m_p_color) return 0xFF5919;
        return static_cast<uint32_t>(m_p_color->get_value());
    }

    void waypoint::set_color(uint32_t color)
    {
        if (!m_p_color) return;
        m_p_color->set_value(static_cast<int64_t>(color));
    }

    void waypoint::cache_waypoint_parameters()
    {
        m_p_enabled = get_parameter<adam::configuration_parameter_boolean>("enabled"_ct);
        m_p_color   = get_parameter<adam::configuration_parameter_integer>("color"_ct);
    }
}
