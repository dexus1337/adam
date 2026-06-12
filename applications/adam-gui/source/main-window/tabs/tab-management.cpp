/**
 * @file    tab-management.cpp
 * @author  dexus1337
 * @brief   Implementation of the management tab drawing entry point.
 * @version 1.0
 * @date    12.06.2026
 */

#include "tab-management.hpp"
#include "../main-window.hpp"

#include <imgui.h>
#include <algorithm>
#include <vector>
#include <mutex>
#include <string>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <chrono>

#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "configuration/parameters/configuration-parameter-double.hpp"
#include "configuration/parameters/configuration-parameter-string.hpp"
#include "configuration/parameters/configuration-parameter-list.hpp"
#include "controller/controller.hpp"
#include "module/module.hpp"
#include "data/port/port-input-replay.hpp"

namespace adam::gui
{
    void draw_tab_management(gui_controller& ctrl, adam::language lang)
    {
        static bool show_inspector = false;
        if (g_request_open_inspector)
        {
            show_inspector = true;
            g_request_open_inspector = false;
        }

        static size_t last_frame_inspector_count = 0;
        size_t current_inspector_count = 0;
        if (ctrl.is_commander_active())
        {
            current_inspector_count = ctrl.commander().get_inspectors().size() + 
                                      ctrl.commander().get_connection_input_inspectors().size() + 
                                      ctrl.commander().get_connection_output_inspectors().size();
        }

        if (last_frame_inspector_count > 0 && current_inspector_count == 0)
        {
            show_inspector = false;
        }
        last_frame_inspector_count = current_inspector_count;
        bool commander_active   = ctrl.is_commander_active();

        draw_delete_connection_modal(ctrl, lang);
        draw_create_connection_modal(ctrl, lang);
        draw_add_create_port_modal(ctrl, lang);
        draw_add_create_processor_modal(ctrl, lang);

        auto& reg_view = ctrl.commander().registry();
        std::lock_guard<const adam::registry_view> lock(reg_view);

        const auto& connections = reg_view.get_connections();
        
        static adam::configuration_parameter_integer* sort_mode_param = dynamic_cast<adam::configuration_parameter_integer*>(ctrl.get_parameters().get("connection_sort_mode"_ct));
        int sort_mode = static_cast<int>(sort_mode_param->get_value());
        
        draw_top_control_bar(ctrl, lang, sort_mode, show_inspector);

        ImGui::Spacing();

        static std::vector<std::pair<adam::string_hash, adam::connection_view*>> sorted_connections;
        sorted_connections.clear();
        for (const auto& [hash, conn] : connections)
            sorted_connections.push_back({hash, conn.get()});

        std::sort(sorted_connections.begin(), sorted_connections.end(), [sort_mode](const auto& a, const auto& b)
        {
            if (sort_mode == 0) // Name (Asc)
                return std::strcmp(a.second->name.c_str(), b.second->name.c_str()) < 0;
            else if (sort_mode == 1) // Name (Desc)
                return std::strcmp(a.second->name.c_str(), b.second->name.c_str()) > 0;
            else if (sort_mode == 2) // Date (Asc)
                return a.second->created < b.second->created;
            else if (sort_mode == 3) // Date (Desc)
                return a.second->created > b.second->created;
            else if (sort_mode == 4) // Edited (Asc)
                return a.second->edited < b.second->edited;
            else if (sort_mode == 5) // Edited (Desc)
                return a.second->edited > b.second->edited;
            else if (sort_mode == 6) // Custom
                return a.second->sorting_index < b.second->sorting_index;
            return false;
        });

        bool g_is_dragging_connection = false;

        if (sort_mode == 6)
        {
            if (const ImGuiPayload* payload = ImGui::GetDragDropPayload())
            {
                if (payload->IsDataType("DND_CONNECTION"))
                {
                    g_is_dragging_connection = true;
                    adam::string_hash dragged_hash = *(const adam::string_hash*)payload->Data;
                    
                    if (g_active_drag_hash != dragged_hash)
                    {
                        g_active_drag_hash = dragged_hash;
                        auto it = std::find_if(sorted_connections.begin(), sorted_connections.end(), 
                        [dragged_hash](const auto& pair)
                        {
                            return pair.first == dragged_hash;
                        });
                        if (it != sorted_connections.end())
                            g_active_drag_target_index = std::distance(sorted_connections.begin(), it);
                        else
                            g_active_drag_target_index = 0;
                    }

                    auto it = std::find_if(sorted_connections.begin(), sorted_connections.end(), 
                    [dragged_hash](const auto& pair)
                    {
                        return pair.first == dragged_hash;
                    });
                    if (it != sorted_connections.end())
                    {
                        auto item = *it;
                        sorted_connections.erase(it);
                        if (g_active_drag_target_index > sorted_connections.size())
                            g_active_drag_target_index = sorted_connections.size();
                        sorted_connections.insert(sorted_connections.begin() + g_active_drag_target_index, item);
                    }
                }
            }
            else
            {
                g_active_drag_hash = 0;
            }
        }

        float avail_w = ImGui::GetContentRegionAvail().x;
        float content_h = ImGui::GetContentRegionAvail().y;

        static float left_ratio = 0.66f;
        float dpi_scale     = ImGui::GetStyle()._MainScale;
        float left_w        = avail_w * left_ratio;
        
        if (avail_w > 0.0f)
        {
            if (left_w < 100.0f * dpi_scale) left_w = 100.0f * dpi_scale;
            if (show_inspector && left_w > avail_w - 100.0f * dpi_scale) left_w = avail_w - 100.0f * dpi_scale;
            left_ratio = left_w / avail_w;
        }

        if (show_inspector) 
            ImGui::BeginChild("ConnectionsRegion", ImVec2(left_w, content_h), false);

        static float card_width = ImGui::GetContentRegionAvail().x;
        
        draw_connections_list(ctrl, lang, sort_mode, card_width, sorted_connections, g_is_dragging_connection);

        if (sort_mode == 6 && g_is_dragging_connection && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            for (uint32_t new_idx = 0; new_idx < sorted_connections.size(); ++new_idx)
            {
                if (sorted_connections[new_idx].second->sorting_index != new_idx)
                {
                    if (commander_active)
                    {
                        adam::string_hash h = sorted_connections[new_idx].first;
                        ctrl.enqueue_commander_action([&ctrl, h, new_idx]() { ctrl.commander().request_connection_sorting_index_change(h, new_idx); });
                    }
                    sorted_connections[new_idx].second->sorting_index = new_idx; 
                }
            }
            g_active_drag_hash = 0;
            g_is_dragging_connection = false;
        }

        if (const ImGuiPayload* payload = ImGui::GetDragDropPayload())
        {
            if (payload->IsDataType("DND_CONNECTION") && payload->Data)
            {
                adam::string_hash dragged_hash = *(const adam::string_hash*)payload->Data;
                auto it = reg_view.get_connections().find(dragged_hash);
                if (it != reg_view.get_connections().end())
                {
                    ImGui::SetNextWindowPos(ImVec2(ImGui::GetMousePos().x - g_connection_drag_offset.x, ImGui::GetMousePos().y - g_connection_drag_offset.y));
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
                    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.7f); // Restore native ImGui drag preview transparency
                    if (ImGui::Begin("##drag_preview", nullptr, ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize))
                    {
                        draw_connection_card(ctrl, lang, sort_mode, dragged_hash, it->second.get(), true, card_width);
                    }
                    ImGui::End();
                    ImGui::PopStyleVar();
                    ImGui::PopStyleColor();
                    ImGui::PopStyleVar(2);
                }
            }
            else if (payload->IsDataType("DND_PROCESSOR") && payload->Data)
            {
                auto* p_data = (const DragProcessorPayload*)payload->Data;
                adam::string_hash conn_hash = p_data->connection;
                adam::string_hash proc_hash = p_data->processor;
                
                auto proc_it = reg_view.get_processors().find(proc_hash);
                if (proc_it != reg_view.get_processors().end())
                {
                    const char* name = proc_it->second->name.c_str();
                    bool is_unavail = proc_it->second->is_unavailable;

                    ImGui::SetNextWindowPos(ImVec2(ImGui::GetMousePos().x - g_processor_drag_offset.x, ImGui::GetMousePos().y - g_processor_drag_offset.y));
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
                    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.7f);
                    if (ImGui::Begin("##drag_processor_preview", nullptr, ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize))
                    {
                        float char_width = ImGui::CalcTextSize("x").x;
                        float desired_node_w = char_width * 40.0f + ImGui::GetStyle().FramePadding.x * 4.0f;
                        float approx_avail_x = card_width - ImGui::GetStyle().WindowPadding.x * 2.0f;
                        float port_w = std::min(desired_node_w, approx_avail_x * 0.25f);
                        if (port_w < char_width * 15.0f) port_w = char_width * 15.0f;
                        float proc_w = port_w;
                        float node_h = ImGui::GetTextLineHeight() * 2.0f;

                        ImVec2 cur_pos = ImGui::GetCursorScreenPos();
                        ImDrawList* draw_list = ImGui::GetWindowDrawList();

                        ImColor col = get_gui_color(gui_color_id::node_processor);
                        col.Value.w *= 0.7f;
                        if (is_unavail) col.Value.w *= 0.4f;

                        connection_pin_data p_in, p_out;
                        std::vector<expanded_port_draw_info> deferred;

                        auto conn_it = reg_view.get_connections().find(conn_hash);
                        auto* conn = conn_it != reg_view.get_connections().end() ? conn_it->second.get() : nullptr;

                        draw_connection_node(
                            ctrl,
                            lang,
                            conn,
                            conn_hash,
                            dpi_scale,
                            draw_list,
                            cur_pos,
                            proc_w,
                            0.0f,
                            proc_w,
                            node_h,
                            node_h,
                            1,
                            proc_w,
                            true, // is_drag_preview
                            name,
                            node_type_processor,
                            0,
                            0.0f,
                            col,
                            p_in,
                            p_out,
                            is_unavail,
                            nullptr,
                            0, // port_hash = 0 avoids tooltip or action triggers
                            0.0f,
                            deferred
                        );

                        ImGui::Dummy(ImVec2(proc_w, node_h));
                    }
                    ImGui::End();
                    ImGui::PopStyleVar();
                    ImGui::PopStyleColor();
                    ImGui::PopStyleVar(2);
                }
            }
        }

        if (!commander_active) ImGui::BeginDisabled();
        if (ImGui::Button(get_gui_string(gui_string_id::btn_create_connection, lang), ImVec2(-1.0f, ImGui::GetFrameHeight() * 1.5f)))
        {
            if (ImGui::GetIO().KeyShift)
            {
                ImGui::OpenPopup(get_gui_string(gui_string_id::dlg_create_connection, lang));
            }
            else
            {
                std::string default_name = "connection_" + std::to_string(connections.size());
                size_t suffix = connections.size();
                while (connections.find(adam::string_hashed(default_name.c_str()).get_hash()) != connections.end())
                {
                    suffix++;
                    default_name = "connection_" + std::to_string(suffix);
                }
                adam::string_hashed new_conn_name(default_name.c_str());
                ctrl.enqueue_commander_action([&ctrl, new_conn_name]() { ctrl.commander().request_connection_create(new_conn_name); });
            }
        }
        if (!commander_active) ImGui::EndDisabled();

        if (show_inspector)
        {
            ImGui::EndChild();
            draw_inspector_subwindow(ctrl, lang, left_w, avail_w, content_h);
            if (avail_w > 0.0f)
            {
                left_ratio = left_w / avail_w;
            }
        }
    }
}
