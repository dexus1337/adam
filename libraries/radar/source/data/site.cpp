#include "data/site.hpp"
#include <random>

using namespace adam::string_hashed_ct_literals;

namespace adam::lib::radar
{
    uint32_t site::generate_random_color()
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<uint32_t> dist(50, 255);

        uint32_t r = dist(gen);
        uint32_t g = dist(gen);
        uint32_t b = dist(gen);

        return (r << 16) | (g << 8) | b;
    }

    static adam::configuration_parameter_list build_site_params(const adam::configuration_parameter_list& child_params)
    {
        adam::configuration_parameter_list p(child_params);
        p.add(std::make_unique<adam::configuration_parameter_boolean>("enabled"_ct, true));
        p.add(std::make_unique<adam::configuration_parameter_integer>("color"_ct, 0));
        p.add(std::make_unique<adam::configuration_parameter_integer>("sacsic"_ct, 0));
        p.add(std::make_unique<adam::configuration_parameter_double>("range_nm"_ct, 120.0));
        p.add(std::make_unique<adam::configuration_parameter_boolean>("auto_retrieve_coords"_ct, false));
        p.add(std::make_unique<adam::configuration_parameter_boolean>("auto_calc_range"_ct, false));
        return p;
    }

    site::site(const adam::string_hashed& item_name, const adam::configuration_parameter_list& child_params)
        : geo_location(item_name, build_site_params(child_params))
        , m_p_enabled(get_parameter<adam::configuration_parameter_boolean>("enabled"_ct))
        , m_p_color(get_parameter<adam::configuration_parameter_integer>("color"_ct))
        , m_p_sacsic(get_parameter<adam::configuration_parameter_integer>("sacsic"_ct))
        , m_p_range_nm(get_parameter<adam::configuration_parameter_double>("range_nm"_ct))
        , m_p_auto_retrieve_coords(get_parameter<adam::configuration_parameter_boolean>("auto_retrieve_coords"_ct))
        , m_p_auto_calc_range(get_parameter<adam::configuration_parameter_boolean>("auto_calc_range"_ct))
    {
        if (get_color() == 0)
        {
            set_color(generate_random_color());
        }
    }
}
