/**
 * @file    window-analysis.cpp
 * @author  dexus1337
 * @brief   Implementation of diagnostic data telemetry analysis window.
 * @version 1.0
 * @date    12.06.2026
 */

#include "window-analysis.hpp"
#include "management/inspector.hpp"

namespace adam::gui
{
    void draw_window_analysis(gui_controller& ctrl, adam::language lang)
    {
        draw_inspector_view(ctrl, lang);
    }
}
