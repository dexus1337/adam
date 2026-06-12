#pragma once

/**
 * @file    tab-information.hpp
 * @author  dexus1337
 * @brief   Header for the information tab drawing functions.
 * @version 1.0
 * @date    12.06.2026
 */

#include <adam-sdk.hpp>

namespace adam::gui 
{
    class gui_controller;
    void draw_tab_information(gui_controller& ctrl, adam::language lang);
}
