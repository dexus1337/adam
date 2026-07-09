/**
 * @file    inspector.cpp
 * @author  dexus1337
 * @brief   Source containing diagnostic data telemetry inspector drawing implementations.
 * @version 1.0
 * @date    12.06.2026
 */

#include "inspector.hpp"
#include "shared-state.hpp"
#include "../../main-window.hpp"
#include "controller/controller.hpp"
#include "configuration/parameters/configuration-parameter-string.hpp"
#include <algorithm>
#include <vector>
#include <map>
#include <cmath> // For std::min, std::max
#include <mutex> // For std::lock_guard
#include <string> // For std::string
#include <unordered_set> // For std::unordered_set

namespace adam::gui
{
    // Helper to draw a single row in the inspector frames table
    static void draw_inspector_table_row(
        const adam::gui::inspected_buffer& ib,
        int actual_index,
        const std::vector<uint8_t>& data_pool,
        std::set<size_t>& expanded_nodes
    )
    {
        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        // Use ImGuiTreeNodeFlags_DefaultOpen if it's already expanded
        bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)actual_index, ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_OpenOnArrow | (expanded_nodes.count(actual_index) ? ImGuiTreeNodeFlags_DefaultOpen : 0), "");

        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", actual_index);

        ImGui::TableSetColumnIndex(2);
        ImGui::TextUnformatted(adam::get_log_time_string(ib.timestamp).c_str());

        ImGui::TableSetColumnIndex(3);
        ImGui::Text("%zu B", static_cast<size_t>(ib.size));

        ImGui::TableSetColumnIndex(4);
        if (g_mono_font) ImGui::PushFont(g_mono_font);

        char preview_hex[64];
        char preview_ascii[32];
        size_t preview_len = std::min((size_t)ib.size, (size_t)16);
        fill_hex_preview(data_pool.data() + ib.offset, preview_len, 16, preview_hex, sizeof(preview_hex), preview_ascii, sizeof(preview_ascii));

        ImGui::TextUnformatted(preview_hex);
        
        ImGui::TableSetColumnIndex(5);
        ImGui::TextUnformatted(preview_ascii);
        
        if (g_mono_font) ImGui::PopFont();

        if (node_open)
        {
            expanded_nodes.insert(actual_index);
            ImGui::TreePop();
        }
        else
        {
            expanded_nodes.erase(actual_index);
        }
    }

    static void draw_inspector_hex_dump
    (
        const uint8_t* data,
        size_t size,
        int actual_index,
        float inspector_height,
        float dpi_scale,
        adam::language lang
    )
    {
        size_t display_len = size;
        size_t num_rows = (display_len + 15) / 16;

        if (g_mono_font) ImGui::PushFont(g_mono_font);
        float line_h = ImGui::GetTextLineHeight();
        if (g_mono_font) ImGui::PopFont();

        float item_spacing_y = ImGui::GetStyle().ItemSpacing.y;
        float window_border_size = ImGui::GetStyle().WindowBorderSize;

        float text_content_h = num_rows > 0 ? (line_h * num_rows + item_spacing_y * (num_rows - 1)) : 0.0f;
        float child_padding_h = (4.0f * dpi_scale) * 2.0f;
        float child_border_h = window_border_size * 2.0f;
        
        float calc_h = text_content_h + child_padding_h + child_border_h;

        float button_h = ImGui::GetFrameHeight();
        float spacing_h = item_spacing_y;
        float container_padding_h = (6.0f * dpi_scale) * 2.0f;
        float container_border_h = window_border_size * 2.0f;
        float reserved_container_elements_h = button_h + spacing_h + container_padding_h + container_border_h;

        float max_container_h = inspector_height * 0.5f;
        if (max_container_h < 110.0f * dpi_scale) max_container_h = 110.0f * dpi_scale;
        
        float absolute_max_h = inspector_height - (button_h + spacing_h + 8.0f * dpi_scale);
        if (max_container_h > absolute_max_h) max_container_h = absolute_max_h;
        if (max_container_h < 80.0f * dpi_scale) max_container_h = 80.0f * dpi_scale;

        float max_child_h = max_container_h - reserved_container_elements_h;
        if (max_child_h < 30.0f * dpi_scale) max_child_h = 30.0f * dpi_scale;

        float child_h = std::min(calc_h, max_child_h);
        float container_h = child_h + reserved_container_elements_h;

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.0f * dpi_scale, 6.0f * dpi_scale));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f * dpi_scale);

        ImGui::PushID(actual_index);
        if (ImGui::BeginChild("##hex_container", ImVec2(-FLT_MIN, container_h), true))
        {
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f * dpi_scale, 2.0f * dpi_scale));

            float avail_w = ImGui::GetContentRegionAvail().x;
            float button_w = (avail_w - ImGui::GetStyle().ItemSpacing.x * 2.0f) / 3.0f;

            if (ImGui::Button(get_gui_string(gui_string_id::btn_copy_hex, lang), ImVec2(button_w, button_h)))
            {
                std::string copy_str;
                copy_str.reserve(size * 3);
                for (size_t j = 0; j < size; ++j)
                {
                    char hex[4];
                    snprintf(hex, sizeof(hex), "%02X ", data[j]);
                    copy_str += hex;
                }
                if (!copy_str.empty()) copy_str.pop_back();
                ImGui::SetClipboardText(copy_str.c_str());
            }
            ImGui::SameLine();

            if (ImGui::Button(get_gui_string(gui_string_id::btn_copy_ascii, lang), ImVec2(button_w, button_h)))
            {
                std::string copy_str;
                copy_str.reserve(size);
                for (size_t j = 0; j < size; ++j)
                {
                    char c = data[j];
                    if (c >= 32 && c <= 126) copy_str += c;
                    else copy_str += '.';
                }
                ImGui::SetClipboardText(copy_str.c_str());
            }
            ImGui::SameLine();

            if (ImGui::Button(get_gui_string(gui_string_id::btn_copy_hex_dump, lang), ImVec2(button_w, button_h)))
            {
                std::string copy_str;
                copy_str.reserve(num_rows * 80);
                for (size_t offset = 0; offset < display_len; offset += 16)
                {
                    char line_buf[256];
                    int printed = snprintf(line_buf, sizeof(line_buf), "%04X:  ", static_cast<unsigned int>(offset));
                    size_t chunk = std::min((size_t)16, display_len - offset);
                    for (size_t j = 0; j < 16; ++j)
                    {
                        if (j == 8) line_buf[printed++] = ' ';
                        if (j < chunk) printed += snprintf(line_buf + printed, sizeof(line_buf) - printed, "%02X ", data[offset + j]);
                        else printed += snprintf(line_buf + printed, sizeof(line_buf) - printed, "   ");
                    }
                    line_buf[printed++] = ' ';
                    line_buf[printed++] = ' ';
                    line_buf[printed++] = '|';
                    for (size_t j = 0; j < chunk; ++j)
                    {
                        char c = data[offset + j];
                        line_buf[printed++] = (c >= 32 && c <= 126) ? c : '.';
                    }
                    line_buf[printed++] = '|';
                    line_buf[printed++] = '\n';
                    line_buf[printed] = '\0';
                    copy_str += line_buf;
                }
                ImGui::SetClipboardText(copy_str.c_str());
            }

            ImGui::PopStyleVar();

            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0.15f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f * dpi_scale, 4.0f * dpi_scale));

            if (ImGui::BeginChild("##hex_child", ImVec2(-FLT_MIN, child_h), true, ImGuiWindowFlags_HorizontalScrollbar))
            {
                if (g_mono_font) ImGui::PushFont(g_mono_font);

                static constexpr const char* dummy_line = "0000:  00 11 22 33 44 55 66 77  88 99 AA BB CC DD EE FF   |0123456789ABCDEF|";
                float text_w = ImGui::CalcTextSize(dummy_line).x;
                float avail_w2 = ImGui::GetContentRegionAvail().x;
                float offset_x = (avail_w2 - text_w) / 2.0f;
                if (offset_x < 4.0f * dpi_scale) offset_x = 4.0f * dpi_scale;

                ImGuiListClipper hex_clipper;
                hex_clipper.Begin(static_cast<int>(num_rows), line_h);
                while (hex_clipper.Step())
                {
                    for (int row = hex_clipper.DisplayStart; row < hex_clipper.DisplayEnd; ++row)
                    {
                        size_t offset = row * 16;
                        size_t chunk = std::min((size_t)16, display_len - offset);

                        char line_buf[256];
                        int printed = snprintf(line_buf, sizeof(line_buf), "%04X:  ", static_cast<unsigned int>(offset));

                        for (size_t j = 0; j < 16; ++j)
                        {
                            if (j == 8)
                            {
                                line_buf[printed++] = ' ';
                            }
                            if (j < chunk)
                            {
                                printed += snprintf(line_buf + printed, sizeof(line_buf) - printed, "%02X ", data[offset + j]);
                            }
                            else
                            {
                                printed += snprintf(line_buf + printed, sizeof(line_buf) - printed, "   ");
                            }
                        }

                        line_buf[printed++] = ' ';
                        line_buf[printed++] = ' ';
                        line_buf[printed++] = '|';

                        for (size_t j = 0; j < 16; ++j)
                        {
                            if (j < chunk)
                            {
                                char c = data[offset + j];
                                if (c >= 32 && c <= 126)
                                    line_buf[printed++] = c;
                                else
                                    line_buf[printed++] = '.';
                            }
                            else
                            {
                                line_buf[printed++] = ' ';
                            }
                        }
                        line_buf[printed++] = '|';
                        line_buf[printed] = '\0';

                        ImGui::SetCursorPosX(offset_x);
                        ImGui::TextUnformatted(line_buf);
                    }
                }

                if (g_mono_font) ImGui::PopFont();
            }
            ImGui::EndChild();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
        }
        ImGui::EndChild();
        ImGui::PopID();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor();
    }

    static void draw_inspector_frames_table
    (
        gui_controller& ctrl,
        const char* name,
        adam::string_hash hash,
        std::map<adam::string_hash, adam::gui::inspection_port_data>& data_map,
        float inspector_height,
        float dpi_scale,
        adam::language lang,
        uint64_t id_modifier
    )
    {
        std::lock_guard<std::mutex> buffer_lock(adam::gui::g_inspection_data.mtx);
        auto& port_data = data_map[hash];
        const auto& buffers = port_data.buffers;
        const auto& data_pool = port_data.data_pool;
        auto& expanded_nodes = port_data.expanded_nodes;

        ImGui::PushID((const void*)(intptr_t)(hash ^ id_modifier));
        
        ImGui::BeginChild("##outer_child", ImVec2(0, inspector_height), true);

        auto setup_inner_columns = [&]() 
        {
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, ImGui::GetTextLineHeight() + ImGui::GetStyle().FramePadding.x * 2.0f);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_index, lang), ImGuiTableColumnFlags_WidthFixed, 55.0f * dpi_scale);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_timestamp, lang), ImGuiTableColumnFlags_WidthFixed, 120.0f * dpi_scale);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_size, lang), ImGuiTableColumnFlags_WidthFixed, 75.0f * dpi_scale);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_preview_hex, lang), ImGuiTableColumnFlags_WidthStretch, 0.75f);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_preview_ascii, lang), ImGuiTableColumnFlags_WidthStretch, 0.25f);
        };

        // Draw the static header table above the scrollable region
        float header_w = ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ScrollbarSize;
        bool header_table_begun = ImGui::BeginTable("InspectorTableHeader", 6, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg, ImVec2(header_w, 0));
        if (header_table_begun)
        {
            setup_inner_columns();
            ImGui::TableHeadersRow();
            ImGui::EndTable();
        }
        
        float inner_scroll_h = -(ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y);
        ImGui::BeginChild("##inner_child", ImVec2(0, inner_scroll_h), false);

        bool active_user_scrolling = ImGui::GetIO().MouseWheel != 0.0f || 
                                     (ImGui::IsMouseDown(ImGuiMouseButton_Left) && 
                                      (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) || 
                                       ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)));

        if (active_user_scrolling && ImGui::GetScrollMaxY() > 0.0f && ImGui::GetScrollY() < ImGui::GetScrollMaxY() - 5.0f * dpi_scale)
        {
            port_data.was_at_bottom = false;
        }
        bool auto_scroll = port_data.was_at_bottom;

        bool table_begun = ImGui::BeginTable("InspectorTableInner", 6, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg);
        
        if (table_begun)
        {
            setup_inner_columns();
        }

        float row_height = ImGui::GetTextLineHeight() + ImGui::GetStyle().CellPadding.y * 2.0f;

        // Remove any expanded nodes that are no longer valid (e.g., buffer was cleared)
        std::erase_if(expanded_nodes, [&](size_t idx) { return idx >= buffers.size(); });

        ImGuiListClipper clipper;
        clipper.Begin(static_cast<int>(buffers.size()), row_height);
        while (clipper.Step())
        {
            for (int row_idx = clipper.DisplayStart; row_idx < clipper.DisplayEnd; ++row_idx)
            {
                const auto& ib = buffers[row_idx];
                draw_inspector_table_row(ib, row_idx, data_pool, (std::set<size_t>&)expanded_nodes);
                
                if (expanded_nodes.count(row_idx))
                {
                    if (table_begun) { ImGui::EndTable(); table_begun = false; }
                    
                    ImGui::PushID(row_idx);
                    draw_inspector_hex_dump(data_pool.data() + ib.offset, ib.size, row_idx, inspector_height, dpi_scale, lang);
                    ImGui::PopID();
                    
                    table_begun = ImGui::BeginTable("InspectorTableInner", 6, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg);
                    if (table_begun) setup_inner_columns();
                }
            }
        }

        if (table_begun)
        {
            ImGui::EndTable();
        }

        if (auto_scroll && ImGui::GetScrollMaxY() > 0.0f)
        {
            ImGui::SetScrollY(ImGui::GetScrollMaxY());
        }

        if (!auto_scroll)
        {
            port_data.was_at_bottom = (ImGui::GetScrollMaxY() == 0.0f || ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 5.0f * dpi_scale);
        }
        
        ImGui::EndChild();

        char clear_btn_text[512];
        if (lang == adam::language_german)
        {
            snprintf(clear_btn_text, sizeof(clear_btn_text), "Daten löschen für \"%s\"", name);
        }
        else
        {
            snprintf(clear_btn_text, sizeof(clear_btn_text), "Clear Data for \"%s\"", name);
        }

        if (ImGui::Button(clear_btn_text, ImVec2(-1.0f, 0.0f)))
        {
            port_data.buffers.clear();
            port_data.data_pool.clear();
            port_data.data_pool.shrink_to_fit();
            port_data.expanded_nodes.clear();
        }
        
        ImGui::EndChild();
        
        ImGui::Spacing();
        ImGui::PopID();
    }

    void draw_inspector_view(gui_controller& ctrl, adam::language lang)
    {
        if (!ctrl.is_commander_active())
        {
            return;
        }

        auto& reg = ctrl.commander().registry();
        auto& inspectors = ctrl.commander().get_inspectors();
        const auto& ports = reg.get_ports();
        
        float dpi_scale = ImGui::GetStyle()._MainScale;
        auto* s_theme_param = dynamic_cast<adam::configuration_parameter_string*>(ctrl.get_parameters().get("theme"_ct));
        const bool s_is_light_theme = s_theme_param && s_theme_param->get_value() == "default-light"_ct;

        auto format_bytes_to_buf = [](uint64_t bytes, char* buf, size_t buf_size) 
        {
            if (bytes < 1024) snprintf(buf, buf_size, "%llu B", (unsigned long long)bytes);
            else if (bytes < 1024 * 1024) snprintf(buf, buf_size, "%.2f KB", bytes / 1024.0);
            else if (bytes < 1024 * 1024 * 1024) snprintf(buf, buf_size, "%.2f MB", bytes / (1024.0 * 1024.0));
            else snprintf(buf, buf_size, "%.2f GB", bytes / (1024.0 * 1024.0 * 1024.0));
        };

        const auto& connections = reg.get_connections();
        auto& connection_input_inspectors = ctrl.commander().get_connection_input_inspectors();
        auto& connection_output_inspectors = ctrl.commander().get_connection_output_inspectors();

        for (auto it = g_expanded_inspector_connections_input.begin(); it != g_expanded_inspector_connections_input.end(); )
        {
            if (connections.find(*it) == connections.end()) it = g_expanded_inspector_connections_input.erase(it);
            else ++it;
        }
        for (auto it = g_pending_inspector_connections_input.begin(); it != g_pending_inspector_connections_input.end(); )
        {
            if (connections.find(*it) == connections.end()) it = g_pending_inspector_connections_input.erase(it);
            else ++it;
        }

        for (auto it = g_expanded_inspector_connections_output.begin(); it != g_expanded_inspector_connections_output.end(); )
        {
            if (connections.find(*it) == connections.end()) it = g_expanded_inspector_connections_output.erase(it);
            else ++it;
        }
        for (auto it = g_pending_inspector_connections_output.begin(); it != g_pending_inspector_connections_output.end(); )
        {
            if (connections.find(*it) == connections.end()) it = g_pending_inspector_connections_output.erase(it);
            else ++it;
        }

        for (auto it = g_expanded_inspector_ports.begin(); it != g_expanded_inspector_ports.end(); )
        {
            if (ports.find(*it) == ports.end()) it = g_expanded_inspector_ports.erase(it);
            else ++it;
        }

        for (auto it = g_pending_inspector_ports.begin(); it != g_pending_inspector_ports.end(); )
        {
            if (ports.find(*it) == ports.end()) it = g_pending_inspector_ports.erase(it);
            else ++it;
        }

        if (g_port_to_expand_in_inspector != 0)
        {
            g_expanded_inspector_ports.insert(g_port_to_expand_in_inspector);
            g_port_to_expand_in_inspector = 0;
        }

        if (g_connection_input_to_expand_in_inspector != 0)
        {
            g_expanded_inspector_connections_input.insert(g_connection_input_to_expand_in_inspector);
            g_connection_input_to_expand_in_inspector = 0;
        }

        if (g_connection_output_to_expand_in_inspector != 0)
        {
            g_expanded_inspector_connections_output.insert(g_connection_output_to_expand_in_inspector);
            g_connection_output_to_expand_in_inspector = 0;
        }

        size_t num_expanded = 0;
        for (const auto& [hash, p_view] : ports)
        {
            if (g_expanded_inspector_ports.count(hash))
            {
                bool has_data = false;
                {
                    std::lock_guard<std::mutex> buffer_lock(adam::gui::g_inspection_data.mtx);
                    auto it = adam::gui::g_inspection_data.ports.find(hash);
                    if (it != adam::gui::g_inspection_data.ports.end() && !it->second.buffers.empty()) has_data = true;
                }
                bool has_inspector = inspectors.find(hash) != inspectors.end();
                if (has_inspector || has_data)
                {
                    num_expanded++;
                    g_pending_inspector_ports.erase(hash);
                }
                else
                {
                    if (g_pending_inspector_ports.count(hash))
                        num_expanded++;
                    else
                        g_expanded_inspector_ports.erase(hash);
                }
            }
        }

        for (const auto& [hash, conn] : connections)
        {
            if (g_expanded_inspector_connections_input.count(hash))
            {
                bool has_data = false;
                {
                    std::lock_guard<std::mutex> buffer_lock(adam::gui::g_inspection_data.mtx);
                    auto it = adam::gui::g_inspection_data.connections_input.find(hash);
                    if (it != adam::gui::g_inspection_data.connections_input.end() && !it->second.buffers.empty()) has_data = true;
                }
                bool has_inspector = connection_input_inspectors.find(hash) != connection_input_inspectors.end();
                if (has_inspector || has_data)
                {
                    num_expanded++;
                    g_pending_inspector_connections_input.erase(hash);
                }
                else
                {
                    if (g_pending_inspector_connections_input.count(hash)) num_expanded++;
                    else g_expanded_inspector_connections_input.erase(hash);
                }
            }

            if (g_expanded_inspector_connections_output.count(hash))
            {
                bool has_data = false;
                {
                    std::lock_guard<std::mutex> buffer_lock(adam::gui::g_inspection_data.mtx);
                    auto it = adam::gui::g_inspection_data.connections_output.find(hash);
                    if (it != adam::gui::g_inspection_data.connections_output.end() && !it->second.buffers.empty()) has_data = true;
                }
                bool has_inspector = connection_output_inspectors.find(hash) != connection_output_inspectors.end();
                if (has_inspector || has_data)
                {
                    num_expanded++;
                    g_pending_inspector_connections_output.erase(hash);
                }
                else
                {
                    if (g_pending_inspector_connections_output.count(hash)) num_expanded++;
                    else g_expanded_inspector_connections_output.erase(hash);
                }
            }
        }

        float spacing_h = ImGui::GetStyle().ItemSpacing.y;
        float table_row_h = ImGui::GetTextLineHeight() + ImGui::GetStyle().CellPadding.y * 2.0f;
        float row_h = ImGui::GetFrameHeight();
        if (table_row_h < row_h)
        {
            table_row_h = row_h;
        }

        float base_outer_h = 0.0f;
        if (!connections.empty())
        {
            base_outer_h += table_row_h * (connections.size() * 2 + 1);
        }
        if (!ports.empty())
        {
            base_outer_h += table_row_h * (ports.size() + 1);
        }
        if (!connections.empty() && !ports.empty())
        {
            base_outer_h += spacing_h;
        }
        base_outer_h += num_expanded * 3.0f * spacing_h;
        base_outer_h += 4.0f * dpi_scale;

        float sticky_button_h = ImGui::GetFrameHeight() * 1.5f + spacing_h;
        float avail_for_inner = ImGui::GetContentRegionAvail().y - base_outer_h - sticky_button_h;
        float min_inspector_height = 250.0f * dpi_scale;
        float inspector_height = min_inspector_height;

        if (num_expanded > 0)
        {
            float per_node_exp_size = avail_for_inner / num_expanded;
            if (per_node_exp_size > min_inspector_height)
            {
                inspector_height = per_node_exp_size;
            }
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::BeginChild("InspectorScrollContent", ImVec2(0, -sticky_button_h), false);
        ImGui::PopStyleVar();

        bool conn_table_open = ImGui::BeginTable("InspectorConnectionsTable", 7, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable);
        if (conn_table_open)
        {
            auto setup_columns = [dpi_scale, lang]() 
            {
                auto fixed_single_size = ImGui::GetTextLineHeight() + ImGui::GetStyle().CellPadding.x * 2.f;
                ImGui::TableSetupScrollFreeze(0, 3);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, fixed_single_size);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, ImGui::GetTextLineHeight() + ImGui::GetStyle().CellPadding.x);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_inspect, lang),       ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, fixed_single_size);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::lbl_connection, lang),    ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_messages, lang),      ImGuiTableColumnFlags_WidthFixed, 80.0f * dpi_scale);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_size, lang),          ImGuiTableColumnFlags_WidthFixed, 80.0f * dpi_scale);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_last_received, lang), ImGuiTableColumnFlags_WidthStretch);
            };

            setup_columns();
            ImGui::TableHeadersRow();

            size_t connection_idx = 0;
            size_t total_connections = connections.size();
            for (const auto& [conn_hash, c_view] : connections)
            {
                connection_idx++;
                bool is_last_connection = (connection_idx == total_connections);
                
                // Input Row
                {
                    bool has_inspector = connection_input_inspectors.find(conn_hash) != connection_input_inspectors.end();
                    bool has_data = false;
                    size_t msg_count = 0;
                    size_t total_size = 0;
                    uint64_t last_ts = 0;
                    char preview_hex[32] = "";

                    {
                        std::lock_guard<std::mutex> buffer_lock(adam::gui::g_inspection_data.mtx);
                        auto it = adam::gui::g_inspection_data.connections_input.find(conn_hash);
                        if (it != adam::gui::g_inspection_data.connections_input.end())
                        {
                            const auto& port_data = it->second;
                            const auto& buffers = port_data.buffers;
                            msg_count = buffers.size();
                            if (msg_count > 0)
                            {
                                has_data = true;
                                total_size = port_data.data_pool.size();
                                const auto& last_b = buffers.back();
                                last_ts = last_b.timestamp;
                                
                                size_t preview_len = std::min((size_t)last_b.size, (size_t)8);
                                char preview_hex_buf[32];
                                char preview_ascii_dummy[16];
                                fill_hex_preview(port_data.data_pool.data() + last_b.offset, preview_len, 8, preview_hex_buf, sizeof(preview_hex_buf), preview_ascii_dummy, sizeof(preview_ascii_dummy));
                                snprintf(preview_hex, sizeof(preview_hex), "%s", preview_hex_buf);
                            }
                        }
                    }

                    bool node_open = (has_inspector || has_data) && (g_expanded_inspector_connections_input.count(conn_hash) > 0);
                    char name_buf[256];
                    snprintf(name_buf, sizeof(name_buf), "%s [Input]", c_view->name.c_str());

                    if (conn_table_open)
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        bool pushed_id = false;
                        
                        if (has_inspector || has_data)
                        {
                            ImGui::PushID((const void*)(intptr_t)(conn_hash ^ 0x9990));
                            pushed_id = true;
                            
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered));
                            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
                            if (ImGui::ArrowButton("##node_in", node_open ? ImGuiDir_Down : ImGuiDir_Right))
                            {
                                if (node_open)
                                    g_expanded_inspector_connections_input.erase(conn_hash);
                                else
                                    g_expanded_inspector_connections_input.insert(conn_hash);
                                node_open = !node_open;
                            }
                            ImGui::PopStyleColor(3);
                        }
                        
                        ImGui::TableSetColumnIndex(1);
                        ImColor pin_col = s_is_light_theme ? get_gui_color(gui_color_id::node_connection_line_light) : get_gui_color(gui_color_id::node_connection_line);
                        if (c_view->started)
                            pin_col = get_gui_color(gui_color_id::node_pin_active);
                        ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
                        float dot_radius = ImGui::GetTextLineHeight() / 2 - ImGui::GetStyle().CellPadding.y;
                        ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(cursor_pos.x + dot_radius + ImGui::GetStyle().CellPadding.x, cursor_pos.y + ImGui::GetFrameHeight() * 0.5f), dot_radius, pin_col);
                        
                        ImGui::TableSetColumnIndex(2);
                        bool inspect_val = has_inspector;
                        ImGui::PushID((const void*)(intptr_t)(conn_hash ^ 0x9991));
                        if (ImGui::Checkbox("##inspect_in", &inspect_val))
                        {
                            if (inspect_val)
                            {
                                ctrl.enqueue_commander_action([&ctrl, conn_hash]() 
                                {
                                    auto& cmdr = ctrl.commander();
                                    if (cmdr.get_connection_input_inspectors().find(conn_hash) == cmdr.get_connection_input_inspectors().end())
                                    {
                                        adam::data_inspector* new_inspector = nullptr;
                                        cmdr.request_connection_input_inspector_create(conn_hash, make_inspector_connection_input_buffer_callback(conn_hash), new_inspector);
                                    }
                                });
                            }
                            else
                            {
                                g_expanded_inspector_connections_input.erase(conn_hash);
                                g_pending_inspector_connections_input.erase(conn_hash);
                                ctrl.enqueue_commander_action([&ctrl, conn_hash]() 
                                {
                                    auto& cmdr = ctrl.commander();
                                    auto it = cmdr.connection_input_inspectors().find(conn_hash);
                                    if (it != cmdr.connection_input_inspectors().end())
                                        cmdr.request_connection_input_inspector_destroy(it->second);
                                });
                            }
                        }
                        ImGui::PopID();
                        
                        ImGui::TableSetColumnIndex(3);
                        ImGui::TextUnformatted(name_buf);
                        
                        ImGui::TableSetColumnIndex(4);
                        if (has_inspector || has_data) ImGui::Text("%zu", msg_count);
                        
                        ImGui::TableSetColumnIndex(5);
                        char total_size_buf[64] = "";
                        if (has_inspector || has_data)
                        {
                            format_bytes_to_buf(total_size, total_size_buf, sizeof(total_size_buf));
                            ImGui::TextUnformatted(total_size_buf);
                        }
                        
                        ImGui::TableSetColumnIndex(6);
                        if (has_inspector || has_data)
                        {
                            if (msg_count > 0)
                                ImGui::Text("%s | %s", adam::get_log_time_string(last_ts).c_str(), preview_hex);
                            else
                                ImGui::TextDisabled("%s", get_gui_string(gui_string_id::lbl_no_data, lang));
                        }
                            
                        if (pushed_id) ImGui::PopID();
                    }
                        
                    if (node_open)
                    {
                        if (conn_table_open) ImGui::EndTable();
                        draw_inspector_frames_table(ctrl, name_buf, conn_hash, adam::gui::g_inspection_data.connections_input, inspector_height, dpi_scale, lang, (uint64_t)0x1111111111111111ULL);
                        conn_table_open = ImGui::BeginTable("InspectorConnectionsTable", 7, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable);
                        if (conn_table_open) setup_columns();
                    }
                }

                // Output Row
                {
                    bool has_inspector = connection_output_inspectors.find(conn_hash) != connection_output_inspectors.end();
                    bool has_data = false;
                    size_t msg_count = 0;
                    size_t total_size = 0;
                    uint64_t last_ts = 0;
                    char preview_hex[32] = "";

                    {
                        std::lock_guard<std::mutex> buffer_lock(adam::gui::g_inspection_data.mtx);
                        auto it = adam::gui::g_inspection_data.connections_output.find(conn_hash);
                        if (it != adam::gui::g_inspection_data.connections_output.end())
                        {
                            const auto& port_data = it->second;
                            const auto& buffers = port_data.buffers;
                            msg_count = buffers.size();
                            if (msg_count > 0)
                            {
                                has_data = true;
                                total_size = port_data.data_pool.size();
                                const auto& last_b = buffers.back();
                                last_ts = last_b.timestamp;
                                
                                size_t preview_len = std::min((size_t)last_b.size, (size_t)8);
                                char preview_hex_buf[32];
                                char preview_ascii_dummy[16];
                                fill_hex_preview(port_data.data_pool.data() + last_b.offset, preview_len, 8, preview_hex_buf, sizeof(preview_hex_buf), preview_ascii_dummy, sizeof(preview_ascii_dummy));
                                snprintf(preview_hex, sizeof(preview_hex), "%s", preview_hex_buf);
                            }
                        }
                    }

                    bool node_open = (has_inspector || has_data) && (g_expanded_inspector_connections_output.count(conn_hash) > 0);
                    char name_buf[256];
                    snprintf(name_buf, sizeof(name_buf), "%s [Output]", c_view->name.c_str());

                    if (conn_table_open)
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        bool pushed_id = false;
                        
                        if (has_inspector || has_data)
                        {
                            ImGui::PushID((const void*)(intptr_t)(conn_hash ^ 0x9992));
                            pushed_id = true;
                            
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered));
                            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
                            if (ImGui::ArrowButton("##node_out", node_open ? ImGuiDir_Down : ImGuiDir_Right))
                            {
                                if (node_open)
                                    g_expanded_inspector_connections_output.erase(conn_hash);
                                else
                                    g_expanded_inspector_connections_output.insert(conn_hash);
                                node_open = !node_open;
                            }
                            ImGui::PopStyleColor(3);
                        }
                        
                        ImGui::TableSetColumnIndex(1);
                        ImColor pin_col = s_is_light_theme ? get_gui_color(gui_color_id::node_connection_line_light) : get_gui_color(gui_color_id::node_connection_line);
                        if (c_view->started)
                            pin_col = get_gui_color(gui_color_id::node_pin_active);
                        ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
                        float dot_radius = ImGui::GetTextLineHeight() / 2 - ImGui::GetStyle().CellPadding.y;
                        ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(cursor_pos.x + dot_radius + ImGui::GetStyle().CellPadding.x, cursor_pos.y + ImGui::GetFrameHeight() * 0.5f), dot_radius, pin_col);
                        
                        ImGui::TableSetColumnIndex(2);
                        bool inspect_val = has_inspector;
                        ImGui::PushID((const void*)(intptr_t)(conn_hash ^ 0x9993));
                        if (ImGui::Checkbox("##inspect_out", &inspect_val))
                        {
                            if (inspect_val)
                            {
                                ctrl.enqueue_commander_action([&ctrl, conn_hash]() 
                                {
                                    auto& cmdr = ctrl.commander();
                                    if (cmdr.get_connection_output_inspectors().find(conn_hash) == cmdr.get_connection_output_inspectors().end())
                                    {
                                        adam::data_inspector* new_inspector = nullptr;
                                        cmdr.request_connection_output_inspector_create(conn_hash, make_inspector_connection_output_buffer_callback(conn_hash), new_inspector);
                                    }
                                });
                            }
                            else
                            {
                                g_expanded_inspector_connections_output.erase(conn_hash);
                                g_pending_inspector_connections_output.erase(conn_hash);
                                ctrl.enqueue_commander_action([&ctrl, conn_hash]() 
                                {
                                    auto& cmdr = ctrl.commander();
                                    auto it = cmdr.connection_output_inspectors().find(conn_hash);
                                    if (it != cmdr.connection_output_inspectors().end())
                                        cmdr.request_connection_output_inspector_destroy(it->second);
                                });
                            }
                        }
                        ImGui::PopID();
                        
                        ImGui::TableSetColumnIndex(3);
                        ImGui::TextUnformatted(name_buf);
                        
                        ImGui::TableSetColumnIndex(4);
                        if (has_inspector || has_data) ImGui::Text("%zu", msg_count);
                        
                        ImGui::TableSetColumnIndex(5);
                        char total_size_buf[64] = "";
                        if (has_inspector || has_data)
                        {
                            format_bytes_to_buf(total_size, total_size_buf, sizeof(total_size_buf));
                            ImGui::TextUnformatted(total_size_buf);
                        }
                        
                        ImGui::TableSetColumnIndex(6);
                        if (has_inspector || has_data)
                        {
                            if (msg_count > 0)
                                ImGui::Text("%s | %s", adam::get_log_time_string(last_ts).c_str(), preview_hex);
                            else
                                ImGui::TextDisabled("%s", get_gui_string(gui_string_id::lbl_no_data, lang));
                        }
                            
                        if (pushed_id) ImGui::PopID();
                    }
                        
                    if (node_open)
                    {
                        if (conn_table_open) ImGui::EndTable();
                        draw_inspector_frames_table(ctrl, name_buf, conn_hash, adam::gui::g_inspection_data.connections_output, inspector_height, dpi_scale, lang, (uint64_t)0x2222222222222222ULL);
                        if (!is_last_connection)
                        {
                            conn_table_open = ImGui::BeginTable("InspectorConnectionsTable", 7, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable);
                            if (conn_table_open) setup_columns();
                        }
                        else
                        {
                            conn_table_open = false;
                        }
                    }
                }
            }

            if (conn_table_open) ImGui::EndTable();
        }

        ImGui::Spacing();

        bool table_open = ImGui::BeginTable("InspectorPortsTable", 7, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable);
        if (table_open)
        {
            auto setup_columns = [dpi_scale, lang]() 
            {
                auto fixed_single_size = ImGui::GetTextLineHeight() + ImGui::GetStyle().CellPadding.x * 2.f;
                ImGui::TableSetupScrollFreeze(0, 3);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, fixed_single_size);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, ImGui::GetTextLineHeight() + ImGui::GetStyle().CellPadding.x);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_inspect, lang),       ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, fixed_single_size);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_port_name, lang),     ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_messages, lang),      ImGuiTableColumnFlags_WidthFixed, 80.0f * dpi_scale);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_size, lang),          ImGuiTableColumnFlags_WidthFixed, 80.0f * dpi_scale);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_last_received, lang), ImGuiTableColumnFlags_WidthStretch);
            };

            setup_columns();
            ImGui::TableHeadersRow();

            size_t port_idx = 0;
            size_t total_ports = ports.size();
            for (const auto& [port_hash, p_view] : ports)
            {
                port_idx++;
                bool is_last_port = (port_idx == total_ports);
                bool has_inspector = inspectors.find(port_hash) != inspectors.end();
                bool has_data = false;
                size_t msg_count = 0;
                size_t total_size = 0;
                uint64_t last_ts = 0;
                char preview_hex[32] = "";

                {
                    std::lock_guard<std::mutex> buffer_lock(adam::gui::g_inspection_data.mtx);
                    auto it = adam::gui::g_inspection_data.ports.find(port_hash);
                    if (it != adam::gui::g_inspection_data.ports.end())
                    {
                        const auto& port_data = it->second;
                        const auto& buffers = port_data.buffers;
                        msg_count = buffers.size();
                        if (msg_count > 0)
                        {
                            has_data = true;
                            total_size = port_data.data_pool.size();
                            const auto& last_b = buffers.back();
                            last_ts = last_b.timestamp;
                            
                            size_t preview_len = std::min((size_t)last_b.size, (size_t)8);
                            char preview_hex_buf[32];
                            char preview_ascii_dummy[16];
                            fill_hex_preview(port_data.data_pool.data() + last_b.offset, preview_len, 8, preview_hex_buf, sizeof(preview_hex_buf), preview_ascii_dummy, sizeof(preview_ascii_dummy));
                            snprintf(preview_hex, sizeof(preview_hex), "%s", preview_hex_buf);
                        }
                    }
                }

                bool node_open = (has_inspector || has_data) && (g_expanded_inspector_ports.count(port_hash) > 0);

                if (table_open)
                {
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    bool pushed_id = false;
                    if (has_inspector || has_data)
                    {
                        ImGui::PushID((const void*)(intptr_t)(port_hash ^ 0x9999));
                        pushed_id = true;
                        
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
                        if (ImGui::ArrowButton("##node", node_open ? ImGuiDir_Down : ImGuiDir_Right))
                        {
                            if (node_open)
                                g_expanded_inspector_ports.erase(port_hash);
                            else
                                g_expanded_inspector_ports.insert(port_hash);
                            node_open = !node_open;
                        }
                        ImGui::PopStyleColor(3);
                    }
                
                    ImGui::TableSetColumnIndex(1);
                    ImColor pin_col = s_is_light_theme ? get_gui_color(gui_color_id::node_connection_line_light) : get_gui_color(gui_color_id::node_connection_line);
                    if (p_view->started)
                    {
                        if (p_view->statistic_buffer)
                        {
                            auto* stats = p_view->statistic_buffer->data_as<adam::port::state_buffer_data>();
                            if (stats->cur_state == adam::port::state_running)
                                pin_col = get_gui_color(gui_color_id::node_pin_active);
                            else if (stats->cur_state == adam::port::state_inactive)
                                pin_col = get_gui_color(gui_color_id::log_warning);
                        }
                        else
                        {
                            pin_col = get_gui_color(gui_color_id::node_pin_active);
                        }
                    }

                    ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
                    float dot_radius = ImGui::GetTextLineHeight() / 2 - ImGui::GetStyle().CellPadding.y;
                    ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(cursor_pos.x + dot_radius + ImGui::GetStyle().CellPadding.x, cursor_pos.y + ImGui::GetFrameHeight() * 0.5f), dot_radius, pin_col);
                    
                    ImGui::TableSetColumnIndex(2);
                    bool inspect_val = has_inspector;
                    ImGui::PushID((const void*)(intptr_t)port_hash);
                    if (ImGui::Checkbox("##inspect", &inspect_val))
                    {
                        if (inspect_val)
                        {
                            ctrl.enqueue_commander_action([&ctrl, port_hash]() 
                            {
                                auto& cmdr = ctrl.commander();
                                if (cmdr.inspectors().find(port_hash) == cmdr.inspectors().end())
                                {
                                    adam::data_inspector* new_inspector = nullptr;
                                    cmdr.request_inspector_create(port_hash, make_inspector_buffer_callback(port_hash), new_inspector);
                                }
                            });
                        }
                        else
                        {
                            g_expanded_inspector_ports.erase(port_hash);
                            g_pending_inspector_ports.erase(port_hash);

                            ctrl.enqueue_commander_action([&ctrl, port_hash]() 
                            {
                                auto& cmdr = ctrl.commander();
                                auto it = cmdr.inspectors().find(port_hash);
                                if (it != cmdr.inspectors().end())
                                {
                                    cmdr.request_inspector_destroy(it->second);
                                }
                            });
                        }
                    }
                    ImGui::PopID();

                    ImGui::TableSetColumnIndex(3);
                    ImGui::TextUnformatted(p_view->name.c_str());

                    if (has_inspector || has_data)
                    {
                        ImGui::TableSetColumnIndex(4);
                        ImGui::Text("%zu", msg_count);

                        ImGui::TableSetColumnIndex(5);
                        char total_size_buf[64];
                        format_bytes_to_buf(total_size, total_size_buf, sizeof(total_size_buf));
                        ImGui::TextUnformatted(total_size_buf);

                        ImGui::TableSetColumnIndex(6);
                        if (msg_count > 0)
                        {
                            ImGui::Text("%s | %s", adam::get_log_time_string(last_ts).c_str(), preview_hex);
                        }
                        else
                        {
                            ImGui::TextDisabled("%s", get_gui_string(gui_string_id::lbl_no_data, lang));
                        }
                    }

                    if (pushed_id)
                    {
                        ImGui::PopID();
                    }
                }

                if (node_open)
                {
                    if (table_open) ImGui::EndTable();
                    
                    draw_inspector_frames_table(ctrl, p_view->name.c_str(), port_hash, adam::gui::g_inspection_data.ports, inspector_height, dpi_scale, lang, (uint64_t)0x3333333333333333ULL);

                    if (!is_last_port)
                    {
                        table_open = ImGui::BeginTable("InspectorPortsTable", 7, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable);
                        if (table_open)
                        {
                            setup_columns();
                        }
                    }
                    else
                    {
                        table_open = false;
                    }
                }
            }

            if (table_open)
            {
                ImGui::EndTable();
            }
        }

        ImGui::EndChild();

        bool has_any_buffered_data = false;
        {
            std::lock_guard<std::mutex> buffer_lock(adam::gui::g_inspection_data.mtx);
            for (const auto& [hash, port_data] : adam::gui::g_inspection_data.ports)
            {
                if (!port_data.buffers.empty())
                {
                    has_any_buffered_data = true;
                    break;
                }
            }
            if (!has_any_buffered_data)
            {
                for (const auto& [hash, port_data] : adam::gui::g_inspection_data.connections_input)
                {
                    if (!port_data.buffers.empty())
                    {
                        has_any_buffered_data = true;
                        break;
                    }
                }
            }
            if (!has_any_buffered_data)
            {
                for (const auto& [hash, port_data] : adam::gui::g_inspection_data.connections_output)
                {
                    if (!port_data.buffers.empty())
                    {
                        has_any_buffered_data = true;
                        break;
                    }
                }
            }
        }

        ImGui::BeginDisabled(!has_any_buffered_data);
        if (ImGui::Button(get_gui_string(gui_string_id::btn_clear_all_data, lang), ImVec2(-1.0f, ImGui::GetFrameHeight() * 1.5f)))
        {
            std::lock_guard<std::mutex> buffer_lock(adam::gui::g_inspection_data.mtx);
            adam::gui::g_inspection_data.ports.clear();
            adam::gui::g_inspection_data.connections_input.clear();
            adam::gui::g_inspection_data.connections_output.clear();
        }
        ImGui::EndDisabled();
    }

    void draw_inspector_subwindow
    (
        gui_controller& ctrl,
        adam::language lang,
        float& left_w,
        float avail_w,
        float content_h
    )
    {
        ImGui::SameLine();

        float dpi_scale     = ImGui::GetStyle()._MainScale;
        float splitter_w    = 4.0f * dpi_scale;
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_Separator));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_SeparatorHovered));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_SeparatorActive));
        ImGui::Button("##vsplitter", ImVec2(splitter_w, content_h));
        ImGui::PopStyleColor(3);

        if (ImGui::IsItemActive())
        {
            left_w += ImGui::GetIO().MouseDelta.x;
            if (left_w < 100.0f * dpi_scale) left_w = 100.0f * dpi_scale;
            if (left_w > avail_w - 100.0f * dpi_scale) left_w = avail_w - 100.0f * dpi_scale;
        }
        if (ImGui::IsItemHovered())
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

        ImGui::SameLine();

        ImGui::BeginChild("InspectorRegion", ImVec2(0, content_h), false);
        draw_inspector_view(ctrl, lang);
        ImGui::EndChild();
    }
}
