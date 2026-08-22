#pragma once

/**
 * @file    window-configuration.hpp
 * @author  dexus1337
 * @brief   Header for the configuration window drawing functions.
 * @version 1.0
 * @date    12.06.2026
 */

#include <adam-core.hpp>

namespace adam::gui 
{
    class gui_controller;
    void draw_window_configuration(gui_controller& ctrl, adam::language lang);
}
