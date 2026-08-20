#include "site.hpp"

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
    }
}
