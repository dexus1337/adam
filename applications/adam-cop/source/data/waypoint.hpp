#pragma once

/**
 * @file    waypoint.hpp
 * @author  dexus1337
 * @brief   Waypoint configuration item representing a map marker
 * @version 1.0
 * @date    09.08.2026
 */

#include "geo-location.hpp"

namespace adam::cop
{
    class waypoint : public geo_location
    {
    public:
        waypoint(const adam::string_hashed& item_name);
        virtual ~waypoint() = default;

        bool is_enabled() const;
        void set_enabled(bool enabled);

        uint32_t get_color() const;
        void set_color(uint32_t color);
    };
}
