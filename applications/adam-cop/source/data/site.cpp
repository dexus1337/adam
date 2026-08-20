#include "site.hpp"

using namespace adam::string_hashed_ct_literals;

namespace adam::cop
{
    static adam::configuration_parameter_list build_site_params()
    {
        adam::configuration_parameter_list p;
        p.add(std::make_unique<adam::configuration_parameter_integer>("sacsic"_ct));
        return p;
    }

    site::site(const adam::string_hashed& item_name)
        : geo_location(item_name, build_site_params())
    {
        cache_site_parameters();
    }

    int64_t site::get_sacsic() const
    {
        if (!m_p_sacsic) return 0;
        return m_p_sacsic->get_value();
    }

    void site::set_sacsic(int64_t sacsic)
    {
        if (!m_p_sacsic) return;
        m_p_sacsic->set_value(sacsic);
    }

    void site::cache_site_parameters()
    {
        m_p_sacsic = get_parameter<adam::configuration_parameter_integer>("sacsic"_ct);
    }
}
