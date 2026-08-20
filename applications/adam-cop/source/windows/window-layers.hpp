#pragma once

/**
 * @file    window-layers.hpp
 * @author  dexus1337
 * @brief   Header for the layers, projection and overlays control panel in adam-cop.
 * @version 1.0
 * @date    20.08.2026
 */

#include <adam-sdk.hpp>
#include "../map/world-map.hpp"
#include <array>

namespace adam::cop
{
    struct map_layer_config_params
    {
        adam::configuration_parameter_integer* provider = nullptr;
        adam::configuration_parameter_double*  opacity  = nullptr;
        adam::configuration_parameter_boolean* visible  = nullptr;
    };

    struct layers_window_context
    {
        map_render_options&                     map_options;
        std::array<map_layer_config_params, 4>& map_layer_params;
        adam::configuration_parameter_boolean*  p_show_control_panel = nullptr;
        adam::configuration_parameter_integer*  p_map_projection     = nullptr;
        adam::configuration_parameter_boolean*  p_show_grid          = nullptr;
        adam::configuration_parameter_boolean*  p_show_coastlines    = nullptr;
        adam::configuration_parameter_boolean*  p_show_scale_bar     = nullptr;
    };

    /**
     * @brief Renders the Layers & Overlays control panel window.
     * @param ctx Reference to the layers configuration context.
     * @param lang Current active user language.
     */
    void draw_layers_window(layers_window_context& ctx, adam::language lang);
}
