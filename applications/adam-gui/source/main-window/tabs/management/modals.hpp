#pragma once

#include <adam-sdk.hpp>

namespace adam::gui
{
    class gui_controller;

    void render_delete_connection_modal(gui_controller& ctrl, adam::language lang);
    void render_create_connection_modal(gui_controller& ctrl, adam::language lang);
    void render_add_create_port_modal(gui_controller& ctrl, adam::language lang);
    void render_add_create_processor_modal(gui_controller& ctrl, adam::language lang);
}
