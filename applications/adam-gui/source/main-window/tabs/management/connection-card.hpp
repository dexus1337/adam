#pragma once

/**
 * @file    connection-card.hpp
 * @author  dexus1337
 * @brief   Header for connection card drawing functions.
 * @version 1.0
 * @date    12.06.2026
 */

#include <adam-sdk.hpp>
#include <vector>
#include <utility>
#include <imgui.h>
#include "shared-state.hpp"

namespace adam::gui
{
    class gui_controller;

    void draw_top_control_bar
    (
        gui_controller& ctrl,
        adam::language lang,
        int& sort_mode,
        bool& show_inspector
    );

    void draw_connection_card_header
    (
        gui_controller& ctrl,
        adam::language lang,
        int sort_mode,
        adam::string_hash hash,
        adam::connection_view* conn,
        bool is_drag_preview,
        float dpi_scale,
        float port_w,
        bool is_unavailable,
        const std::vector<std::pair<adam::string_hashed, adam::string_hashed>>& available_formats,
        bool input_missing,
        bool output_missing
    );

    void draw_connection_lines
    (
        ImDrawList* draw_list,
        float dpi_scale,
        bool is_light_theme,
        int total_stages,
        size_t max_rows,
        float row_height,
        float avail_x,
        const ImVec2& cur_pos,
        const std::vector<std::vector<connection_pin_data>>& stage_pins_in,
        const std::vector<std::vector<connection_pin_data>>& stage_pins_out,
        adam::connection_view* conn
    );

    void draw_connection_card
    (
        gui_controller& ctrl,
        adam::language lang,
        int sort_mode,
        adam::string_hash hash,
        adam::connection_view* conn,
        bool is_drag_preview,
        float card_w
    );

    void draw_connections_list
    (
        gui_controller& ctrl,
        adam::language lang,
        int sort_mode,
        float& card_width,
        const std::vector<std::pair<adam::string_hash, adam::connection_view*>>& sorted_connections,
        bool is_dragging_connection
    );
}
