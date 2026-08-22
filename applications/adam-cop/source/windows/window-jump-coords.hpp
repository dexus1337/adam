#pragma once

/**
 * @file    window-jump-coords.hpp
 * @author  dexus1337
 * @brief   Header for the coordinate jump helper window in adam-cop.
 * @version 1.0
 * @date    20.08.2026
 */

#include <adam-core.hpp>

namespace adam::cop
{
    class world_map;

    /**
     * @brief Renders the Jump to Coordinates utility window.
     * @param map Reference to the world map instance.
     * @param lang Current active user language.
     * @param p_show_jump_coords Configuration parameter controlling window visibility.
     * @param jump_lat Reference to the target latitude.
     * @param jump_lon Reference to the target longitude.
     */
    void draw_jump_coords_window(world_map& map, adam::language lang, 
                                adam::configuration_parameter_boolean* p_show_jump_coords,
                                float& jump_lat, float& jump_lon);
}
