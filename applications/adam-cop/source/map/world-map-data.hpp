#pragma once

/**
 * @file    world-map-data.hpp
 * @author  dexus1337
 * @brief   Embedded vector coastline and landmass coordinate datasets
 * @version 1.0
 * @date    05.08.2026
 */

#include <vector>
#include <cstdint>

namespace adam::cop
{
    struct geo_point
    {
        float lat; // -90.0f to +90.0f (degrees N)
        float lon; // -180.0f to +180.0f (degrees E)
    };

    struct map_polyline
    {
        std::vector<geo_point> points;
        bool is_closed = false;
    };

    /** @brief Retrieves the static global vector map dataset */
    const std::vector<map_polyline>& get_world_map_vector_data();
}
