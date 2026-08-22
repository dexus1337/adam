#pragma once

/**
 * @file    window-log.hpp
 * @author  dexus1337
 * @brief   Header for log window drawing functions.
 * @version 1.0
 * @date    12.06.2026
 */

#include <adam-core.hpp>

namespace adam::gui 
{
    class gui_controller;

    void draw_window_log(gui_controller& ctrl, adam::language lang, int log_table_id);
}
