#pragma once

#include <adam-sdk.hpp>

namespace adam::gui
{
    class gui_controller;

    void render_inspector_view(gui_controller& ctrl, adam::language lang);
    void render_inspector_subwindow(gui_controller& ctrl, adam::language lang, float& left_w, float avail_w, float content_h);
}
