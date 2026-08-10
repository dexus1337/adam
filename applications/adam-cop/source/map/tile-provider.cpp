/**
 * @file    tile-provider.cpp
 * @author  dexus1337
 * @brief   Raster tile map provider implementation for adam-cop
 * @version 1.0
 * @date    05.08.2026
 */

#include "tile-provider.hpp"
#include <cstdio>
#include <cstring>
#include <filesystem>

namespace adam::cop
{
    tile_provider_info get_tile_provider_info(tile_provider_type type)
    {
        switch (type)
        {
            case tile_provider_type::cartodb_dark:
                return {
                    tile_provider_type::cartodb_dark,
                    "CartoDB Dark Matter (Tactical)",
                    "https://a.basemaps.cartocdn.com/dark_all/{z}/{x}/{y}.png",
                    "adam-cop/1.0",
                    "cartodb_dark",
                    19
                };
            case tile_provider_type::openstreetmap:
            default:
                return {
                    tile_provider_type::openstreetmap,
                    "OpenStreetMap Standard",
                    "https://tile.openstreetmap.org/{z}/{x}/{y}.png",
                    "adam-cop/1.0",
                    "openstreetmap",
                    19
                };
            case tile_provider_type::esri_satellite:
                return {
                    tile_provider_type::esri_satellite,
                    "Esri World Imagery (Satellite)",
                    "https://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile/{z}/{y}/{x}",
                    "adam-cop/1.0",
                    "esri_satellite",
                    18
                };
            case tile_provider_type::opentopomap:
                return {
                    tile_provider_type::opentopomap,
                    "OpenTopoMap (Topographic)",
                    "https://tile.opentopomap.org/{z}/{x}/{y}.png",
                    "adam-cop/1.0",
                    "opentopomap",
                    17
                };
        }
    }

    std::string build_tile_url(tile_provider_type type, int z, int x, int y)
    {
        tile_provider_info info = get_tile_provider_info(type);
        if (std::strlen(info.url_template) == 0)
        {
            return "";
        }

        std::string url = info.url_template;

        auto replace_all = [](std::string& str, const std::string& from, const std::string& to)
        {
            size_t start_pos = 0;
            while ((start_pos = str.find(from, start_pos)) != std::string::npos)
            {
                str.replace(start_pos, from.length(), to);
                start_pos += to.length();
            }
        };

        replace_all(url, "{z}", std::to_string(z));
        replace_all(url, "{x}", std::to_string(x));
        replace_all(url, "{y}", std::to_string(y));

        return url;
    }

    std::string build_tile_cache_path(tile_provider_type type, int z, int x, int y)
    {
        tile_provider_info info = get_tile_provider_info(type);
        std::filesystem::path p = "map_cache";
        p /= info.folder_name;
        p /= std::to_string(z);
        p /= std::to_string(x);
        p /= std::to_string(y) + ".png";
        return p.string();
    }
}
