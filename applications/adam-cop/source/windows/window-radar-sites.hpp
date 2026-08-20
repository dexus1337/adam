#pragma once

/**
 * @file    window-radar-sites.hpp
 * @author  dexus1337
 * @brief   Header for the radar sites management window in adam-cop.
 * @version 1.0
 * @date    20.08.2026
 */

#include <adam-sdk.hpp>

namespace adam::cop
{
    class cop_controller;
    class world_map;

    /**
     * @brief Renders the Radar Sites management window.
     * @param ctrl Reference to the COP controller.
     * @param map Reference to the world map instance.
     * @param lang Current active user language.
     * @param p_show_sites Configuration parameter controlling window visibility.
     * @param picking_site_coords_hash Reference to active map picking state (or 0 if idle).
     */
    void draw_radar_sites_window(cop_controller& ctrl, world_map& map, adam::language lang, 
                                 adam::configuration_parameter_boolean* p_show_sites,
                                 adam::string_hash& picking_site_coords_hash);
}
