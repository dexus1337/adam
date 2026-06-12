#pragma once

#include <adam-sdk.hpp>
#include <vector>
#include <utility>
#include <imgui.h>

namespace adam::gui
{
    class gui_controller;

    void render_top_control_bar(gui_controller& ctrl, adam::language lang, int& sort_mode, bool& show_inspector);

    void draw_connection_card_header(
        gui_controller& ctrl,
        adam::language lang,
        adam::string_hash hash,
        adam::connection_view* conn,
        bool is_unavailable,
        float card_width,
        float dpi_scale);

    void draw_connection_lines(
        ImDrawList* draw_list,
        int total_stages,
        float dpi_scale);

    void draw_connection_card_footer(
        gui_controller& ctrl,
        adam::language lang,
        adam::string_hash hash,
        adam::connection_view* conn,
        bool is_unavailable,
        float card_width,
        float dpi_scale);

    void render_connection_card(
        gui_controller& ctrl,
        adam::language lang,
        int sort_mode,
        adam::string_hash hash,
        adam::connection_view* conn,
        bool is_drag_preview,
        float card_w);

    void render_connections_list(
        gui_controller& ctrl,
        adam::language lang,
        int sort_mode,
        float& card_width,
        const std::vector<std::pair<adam::string_hash, adam::connection_view*>>& sorted_connections,
        bool is_dragging_connection);
}
