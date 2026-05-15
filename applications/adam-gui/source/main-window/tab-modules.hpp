#pragma once

#include <adam-sdk.hpp>

namespace adam::gui 
{
    class gui_controller;
    void render_tab_modules(gui_controller& ctrl, adam::language lang, int module_paths_table_id, int modules_table_id);
}