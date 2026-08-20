#pragma once

/**
 * @file    window-cache-stats.hpp
 * @author  dexus1337
 * @brief   Header for the tile cache statistics window in adam-cop.
 * @version 1.0
 * @date    20.08.2026
 */

#include <adam-sdk.hpp>

namespace adam::cop
{
    class world_map;

    /**
     * @brief Renders the Tile Cache Statistics debug / inspector window.
     * @param map Reference to the world map instance.
     * @param lang Current active user language.
     * @param p_show_cache_stats Configuration parameter controlling window visibility.
     */
    void draw_cache_stats_window(world_map& map, adam::language lang, adam::configuration_parameter_boolean* p_show_cache_stats);
}
