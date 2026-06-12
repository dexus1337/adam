#pragma once

#include <adam-sdk.hpp>
#include <imgui.h>
#include <vector>
#include "shared-state.hpp"

namespace adam::gui
{
    class gui_controller;

    float get_expanded_node_height(uint64_t uid, float dpi_scale);

    void draw_expanded_port_node(
        gui_controller& ctrl,
        adam::language lang,
        const adam::registry_view& registry,
        float dpi_scale,
        ImDrawList* draw_list,
        const expanded_port_render_info& info);

    bool draw_connection_node(
        gui_controller& ctrl,
        adam::language lang,
        adam::connection_view* conn,
        adam::string_hash hash,
        float dpi_scale,
        ImDrawList* draw_list,
        ImVec2 cur_pos,
        float port_w,
        float gap,
        float proc_w,
        float node_h,
        float row_height,
        int total_stages,
        float avail_x,
        bool is_drag_preview,
        const char* name,
        node_type type,
        int stage,
        float row,
        ImColor color,
        connection_pin_data& out_pin_in,
        connection_pin_data& out_pin_out,
        bool is_unavail,
        const char* unavail_module,
        adam::string_hash port_hash,
        float extra_y,
        std::vector<expanded_port_render_info>& deferred_expansions);
}
