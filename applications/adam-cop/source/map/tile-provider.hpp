#pragma once

/**
 * @file    tile-provider.hpp
 * @author  dexus1337
 * @brief   Raster tile map provider definitions for adam-cop
 * @version 1.0
 * @date    05.08.2026
 */

#include <string>
#include <vector>

namespace adam::cop
{
    enum class tile_provider_type
    {
        cartodb_dark = 0,
        openstreetmap,
        esri_satellite,
        opentopomap,
        vector_only
    };

    struct tile_provider_info
    {
        tile_provider_type type;
        const char* name;
        const char* url_template;
        const char* user_agent;
        const char* folder_name;
        int max_zoom;
    };

    /** @brief Retrieves metadata info for a given tile provider */
    tile_provider_info get_tile_provider_info(tile_provider_type type);

    /** @brief Constructs tile URL for given provider and tile coordinates (z, x, y) */
    std::string build_tile_url(tile_provider_type type, int z, int x, int y);

    /** @brief Constructs local disk cache file path for given provider and tile coordinates */
    std::string build_tile_cache_path(tile_provider_type type, int z, int x, int y);
}
