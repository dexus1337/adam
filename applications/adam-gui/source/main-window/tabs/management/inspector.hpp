#pragma once

/**
 * @file    inspector.hpp
 * @author  dexus1337
 * @brief   Header containing diagnostic data telemetry inspector drawing declarations.
 * @version 1.0
 * @date    12.06.2026
 */

#include <adam-sdk.hpp>

namespace adam::gui
{
    class gui_controller;

    void draw_inspector_view(gui_controller& ctrl, adam::language lang);

    void draw_inspector_subwindow
    (
        gui_controller& ctrl,
        adam::language lang,
        float& left_w,
        float avail_w,
        float content_h
    );
}
