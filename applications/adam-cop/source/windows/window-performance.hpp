#pragma once

/**
 * @file    window-performance.hpp
 * @author  dexus1337
 * @brief   Header for the performance overlay HUD in adam-cop.
 * @version 1.0
 * @date    20.08.2026
 */

#include <adam-core.hpp>

namespace adam::cop
{
    struct perf_overlay_params
    {
        adam::configuration_parameter_boolean* p_show_performance   = nullptr;
        adam::configuration_parameter_integer* p_perf_ovly_location = nullptr;
        adam::configuration_parameter_double*  p_perf_ovly_x        = nullptr;
        adam::configuration_parameter_double*  p_perf_ovly_y        = nullptr;
        adam::configuration_parameter_integer* p_perf_ovly_content  = nullptr;
        adam::configuration_parameter_integer* p_gui_mode           = nullptr;
    };

    /**
     * @brief Renders the floating Performance Overlay (FPS, CPU, RAM).
     * @param params Reference to performance overlay parameters.
     * @param lang Current active user language.
     */
    void draw_performance_overlay(perf_overlay_params& params, adam::language lang);
}
