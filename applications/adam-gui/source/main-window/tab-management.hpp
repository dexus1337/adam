#pragma once

#include <adam-sdk.hpp>
#include <vector>
#include <utility>

namespace adam 
{
    struct connection_view;
}

namespace adam::gui 
{
    class gui_controller;

    void render_delete_connection_modal(gui_controller& ctrl, adam::language lang);
    void render_create_connection_modal(gui_controller& ctrl, adam::language lang);
    void render_add_create_port_modal(gui_controller& ctrl, adam::language lang);
    void render_add_create_processor_modal(gui_controller& ctrl, adam::language lang);
    void render_top_control_bar(gui_controller& ctrl, adam::language lang, int& sort_mode, bool& show_inspector);
    void render_connection_card(gui_controller& ctrl, adam::language lang, int sort_mode, adam::string_hash hash, adam::connection_view* conn, bool is_drag_preview, float card_w);
    void render_connections_list(gui_controller& ctrl, adam::language lang, int sort_mode, float& card_width, const std::vector<std::pair<adam::string_hash, adam::connection_view*>>& sorted_connections, bool is_dragging_connection);

    void render_inspector_view(gui_controller& ctrl, adam::language lang);
    void render_inspector_subwindow(gui_controller& ctrl, adam::language lang, float& left_w, float avail_w, float content_h);
    void render_tab_management(gui_controller& ctrl, adam::language lang);
}