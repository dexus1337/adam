#pragma once

/**
 * @file    modals.hpp
 * @author  dexus1337
 * @brief   Header containing popup modals drawing declarations for connection/port/processor creation/deletion.
 * @version 1.0
 * @date    12.06.2026
 */

#include <adam-sdk.hpp>

namespace adam::gui
{
    class gui_controller;

    void draw_delete_connection_modal(gui_controller& ctrl, adam::language lang);
    void draw_delete_port_modal(gui_controller& ctrl, adam::language lang);
    void draw_create_connection_modal(gui_controller& ctrl, adam::language lang);
    void draw_add_create_port_modal(gui_controller& ctrl, adam::language lang);
    void draw_add_create_processor_modal(gui_controller& ctrl, adam::language lang);
}
