#pragma once

/**
 * @file    site.hpp
 * @author  dexus1337
 * @brief   Site configuration item representing a radar site
 * @version 1.0
 * @date    09.08.2026
 */

#include "geo-location.hpp"

namespace adam::cop
{
    class site : public geo_location
    {
    public:
        site(const adam::string_hashed& item_name);
        virtual ~site() = default;
    };
}
