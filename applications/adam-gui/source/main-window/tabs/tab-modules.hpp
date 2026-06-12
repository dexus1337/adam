#pragma once

/**
 * @file    tab-modules.hpp
 * @author  dexus1337
 * @brief   Header for the modules tab drawing functions.
 * @version 1.0
 * @date    12.06.2026
 */

#include <adam-sdk.hpp>

namespace adam::gui 
{
    class gui_controller;

    void draw_tab_modules
    (
        gui_controller& ctrl,
        adam::language lang,
        int module_paths_table_id,
        int modules_table_id
    );
}
