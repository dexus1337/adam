#pragma once

/**
 * @file    window-about.hpp
 * @author  dexus1337
 * @brief   Header for the about window drawing functions.
 * @version 1.0
 * @date    12.06.2026
 */

#include <adam-core.hpp>

namespace adam::gui 
{
    class gui_controller;
    void draw_about_dialog(gui_controller& ctrl, adam::language lang, bool& show_about);
}
