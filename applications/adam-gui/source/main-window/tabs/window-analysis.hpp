#pragma once

/**
 * @file    window-analysis.hpp
 * @author  dexus1337
 * @brief   Header containing diagnostic data telemetry analysis window declarations.
 * @version 1.0
 * @date    12.06.2026
 */

#include <adam-sdk.hpp>

namespace adam::gui
{
    class gui_controller;

    void draw_window_analysis(gui_controller& ctrl, adam::language lang);
}
