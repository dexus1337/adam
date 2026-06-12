/**
 * @file    connection-card.cpp
 * @author  dexus1337
 * @brief   Implementation of connection card rendering and layout logic.
 * @version 1.0
 * @date    12.06.2026
 */

#include "connection-card.hpp"
#include "shared-state.hpp"
#include "node.hpp"
#include "../../main-window.hpp"
#include "controller/controller.hpp"
#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "configuration/parameters/configuration-parameter-double.hpp"
#include "configuration/parameters/configuration-parameter-string.hpp"
#include <algorithm>
#include <vector>
#include <cstring>
#include <cmath>

namespace adam::gui
{
    void draw_top_control_bar
    (
        gui_controller& ctrl,
        adam::language lang,
        int& sort_mode,
        bool& show_inspector
    )
    {
        float dpi_scale = ImGui::GetStyle()._MainScale;
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_sort_by, lang));
        ImGui::SameLine();
        const char* sort_options[] = 
        {
            get_gui_string(gui_string_id::sort_name_asc, lang),
            get_gui_string(gui_string_id::sort_name_desc, lang),
            get_gui_string(gui_string_id::sort_date_asc, lang),
            get_gui_string(gui_string_id::sort_date_desc, lang),
            get_gui_string(gui_string_id::sort_edited_asc, lang),
            get_gui_string(gui_string_id::sort_edited_desc, lang),
            get_gui_string(gui_string_id::sort_custom, lang)
        };
        ImGui::SetNextItemWidth(200.0f * dpi_scale);
        if (ImGui::Combo("##SortMode", &sort_mode, sort_options, IM_ARRAYSIZE(sort_options)))
        {
            auto* sort_mode_param = dynamic_cast<adam::configuration_parameter_integer*>(ctrl.get_parameters().get("connection_sort_mode"_ct));
            if (sort_mode_param)
                sort_mode_param->set_value(static_cast<int64_t>(sort_mode));
        }

        const char* inspector_lbl = get_gui_string(gui_string_id::lbl_data_inspector, lang);
        float right_align = ImGui::GetWindowWidth() - ImGui::CalcTextSize(inspector_lbl).x - ImGui::GetFrameHeight() - ImGui::GetStyle().WindowPadding.x * 2.0f;
        if (right_align > ImGui::GetCursorPosX())
        {
            ImGui::SameLine(right_align);
        }
        else
        {
            ImGui::SameLine();
        }
        ImGui::Checkbox(inspector_lbl, &show_inspector);
    }

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
        bool is_unavailable
    )
    {
        bool commander_active = ctrl.is_commander_active();
        const auto& connections = ctrl.commander().registry().get_connections();

        ImGui::AlignTextToFramePadding();

        char name_buf[max_name_length];
        std::strncpy(name_buf, conn->name.c_str(), sizeof(name_buf));
        name_buf[sizeof(name_buf) - 1] = '\0';

        ImGui::PushID("conn_name_edit");
        bool enter_pressed = false;
        bool deactivated = false;
        if (!is_drag_preview)
        {
            ImGui::SetNextItemWidth(port_w);
            enter_pressed = ImGui::InputText("##edit", name_buf, sizeof(name_buf), ImGuiInputTextFlags_EnterReturnsTrue);
            deactivated = ImGui::IsItemDeactivatedAfterEdit();
        }
        else
        {
            ImGui::TextUnformatted(name_buf);
        }
        ImGui::PopID();

        if (enter_pressed || deactivated)
        {
            if (name_buf[0] != '\0' && conn->name != string_hashed(&name_buf[0]))
            {
                adam::string_hash proposed_hash = adam::string_hashed(&name_buf[0]).get_hash();
                if (connections.find(proposed_hash) == connections.end())
                {
                    adam::string_hash old_hash = conn->name.get_hash();
                    adam::string_hashed new_name(&name_buf[0]);
                    ctrl.enqueue_commander_action([&ctrl, old_hash, new_name]() { ctrl.commander().request_connection_rename(old_hash, new_name); });
                }
            }
        }

        ImGui::SameLine();

        ImVec4 conn_color_vec;
        if (conn->color == 0)
        {
            conn_color_vec = is_drag_preview ? ImVec4(0.2f, 0.2f, 0.2f, 1.0f) : ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
        }
        else
        {
            conn_color_vec.x = ((conn->color >> 16) & 0xFF) / 255.0f;
            conn_color_vec.y = ((conn->color >> 8) & 0xFF) / 255.0f;
            conn_color_vec.z = (conn->color & 0xFF) / 255.0f;
            conn_color_vec.w = 1.0f;
        }

        if (!is_drag_preview)
        {
            ImGui::PushID("conn_color_edit");
                
            if (ImGui::ColorButton("##color_btn", conn_color_vec, ImGuiColorEditFlags_NoTooltip))
            {
                ImGui::OpenPopup("ColorPickerPopup");
            }
            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
            {
                conn->color = 0;
                if (commander_active)
                {
                    adam::string_hash h = conn->name.get_hash();
                    ctrl.enqueue_commander_action([&ctrl, h]() { ctrl.commander().request_connection_color_change(h, 0); });
                }
            }
                
            if (ImGui::BeginPopup("ColorPickerPopup"))
            {
                float picker_col[3] = { conn_color_vec.x, conn_color_vec.y, conn_color_vec.z };
                    
                if (conn->color == 0) 
                {
                    picker_col[0] = 1.0f; picker_col[1] = 1.0f; picker_col[2] = 1.0f;
                }

                if (ImGui::ColorPicker3("##picker", picker_col, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview))
                {
                    uint32_t new_color = (static_cast<uint32_t>(picker_col[0] * 255.0f + 0.5f) << 16) |
                                         (static_cast<uint32_t>(picker_col[1] * 255.0f + 0.5f) << 8) |
                                         (static_cast<uint32_t>(picker_col[2] * 255.0f + 0.5f));
                    if (new_color == 0) new_color = 0x000001; 
                    conn->color = new_color;
                }
                    
                if (ImGui::IsItemDeactivatedAfterEdit())
                {
                    if (commander_active)
                    {
                        adam::string_hash h = conn->name.get_hash();
                        uint32_t c = conn->color;
                        ctrl.enqueue_commander_action([&ctrl, h, c]() { ctrl.commander().request_connection_color_change(h, c); });
                    }
                }
                    
                ImGui::EndPopup();
            }
            ImGui::PopID();
        }
        else
        {
            ImGui::ColorButton("##color_preview", conn_color_vec, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop);
        }

        if (!is_unavailable)
        {
            const char* btn_start_str = get_gui_string(gui_string_id::btn_start, lang);
            const char* btn_stop_str  = get_gui_string(gui_string_id::btn_stop, lang);
                
            ImGui::SameLine();

            bool started = conn->started;
            bool can_start = !started && conn->valid_chain;

            if (!can_start) ImGui::BeginDisabled();
            if (ImGui::Button(btn_start_str) && !is_drag_preview)
            {
                adam::string_hash h = conn->name.get_hash();
                ctrl.enqueue_commander_action([&ctrl, h]() { ctrl.commander().request_connection_start(h); });
            }
            if (!can_start) ImGui::EndDisabled();
                
            ImGui::SameLine();
            if (!started) ImGui::BeginDisabled();
            if (ImGui::Button(btn_stop_str) && !is_drag_preview)
            {
                adam::string_hash h = conn->name.get_hash();
                ctrl.enqueue_commander_action([&ctrl, h]() { ctrl.commander().request_connection_stop(h); });
            }
            if (!started) ImGui::EndDisabled();
        }
        else
        {
            ImGui::SameLine();
            ImGui::TextColored(get_gui_color(gui_color_id::log_warning), "%s", get_gui_string(gui_string_id::stat_unavailable, lang));
        }

        const char* btn_delete_str = get_gui_string(gui_string_id::btn_delete, lang);
        float btn_delete_w = ImGui::CalcTextSize(btn_delete_str).x + ImGui::GetStyle().FramePadding.x * 2.0f;
            
        ImGui::SameLine(ImGui::GetWindowWidth() - btn_delete_w - ImGui::GetStyle().WindowPadding.x);
            
        if (ImGui::Button(btn_delete_str) && !is_drag_preview)
        {
            if (conn->inputs.empty() && conn->outputs.empty() && conn->processors.empty())
            {
                adam::string_hash h = conn->name.get_hash();
                ctrl.enqueue_commander_action([&ctrl, h]() { ctrl.commander().request_connection_destroy(h); });
            }
            else
            {
                g_connection_to_delete = conn->name;
                g_request_delete_popup = true;
            }
        }

        if (sort_mode == 6 && !is_drag_preview)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_Separator));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_SeparatorHovered));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_SeparatorActive));
            ImGui::Button("##drag_handle", ImVec2(-1.0f, 4.0f * dpi_scale));
            ImGui::PopStyleColor(3);

            if (ImGui::IsItemActivated())
            {
                ImVec2 mouse_pos = ImGui::GetMousePos();
                ImVec2 win_pos = ImGui::GetWindowPos();
                g_connection_drag_offset = ImVec2(mouse_pos.x - win_pos.x, mouse_pos.y - win_pos.y);
            }

            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID | ImGuiDragDropFlags_SourceNoPreviewTooltip))
            {
                ImGui::SetDragDropPayload("DND_CONNECTION", &hash, sizeof(adam::string_hash));
                ImGui::EndDragDropSource();
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
            }
        }
        else
        {
            ImGui::Separator();
        }
    }

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
    )
    {
        ImColor line_col = is_light_theme ? get_gui_color(gui_color_id::node_connection_line_light) : get_gui_color(gui_color_id::node_connection_line);
        ImColor grey_col = is_light_theme ? ImColor(210, 210, 210, 80) : ImColor(70, 70, 70, 80);
        float line_thickness = 5.f * dpi_scale;

        if (conn->processors.empty() && !conn->inputs.empty() && !conn->outputs.empty() && (conn->inputs.size() > 1 || conn->outputs.size() > 1))
        {
            float center_y = cur_pos.y + static_cast<float>(max_rows) * row_height * 0.5f;
            float mid_x = cur_pos.x + avail_x * 0.5f;
            ImVec2 p_mid(mid_x, center_y);
                
            for (const auto& pin_out_data : stage_pins_out[0])
            {
                ImVec2 pin_out = pin_out_data.pos;
                float b_strength = (mid_x - pin_out.x) * 0.5f;
                ImColor cur_line_col = conn->valid_chain ? line_col : grey_col;

                draw_list->AddBezierCubic(
                    pin_out, ImVec2(pin_out.x + b_strength, pin_out.y), ImVec2(mid_x - b_strength, center_y), p_mid,
                    cur_line_col, line_thickness);
            }
                
            for (const auto& pin_in_data : stage_pins_in[1])
            {
                ImVec2 pin_in = pin_in_data.pos;
                float b_strength = (pin_in.x - mid_x) * 0.5f;
                ImColor cur_line_col = conn->valid_chain ? line_col : grey_col;

                draw_list->AddBezierCubic(
                    p_mid, ImVec2(mid_x + b_strength, center_y), ImVec2(pin_in.x - b_strength, pin_in.y), pin_in,
                    cur_line_col, line_thickness);
            }
        }
        else
        {
            for (int s = 0; s < total_stages - 1; ++s)
            {
                for (const auto& pin_out_data : stage_pins_out[s])
                {
                    ImVec2 pin_out = pin_out_data.pos;
                    for (const auto& pin_in_data : stage_pins_in[s + 1])
                    {
                        ImVec2 pin_in = pin_in_data.pos;
                        float b_strength = (pin_in.x - pin_out.x) * 0.5f;
                        ImColor cur_line_col = conn->valid_chain ? line_col : grey_col;

                        draw_list->AddBezierCubic(
                            pin_out, ImVec2(pin_out.x + b_strength, pin_out.y), ImVec2(pin_in.x - b_strength, pin_in.y), pin_in,
                            cur_line_col, line_thickness
                        );
                    }
                }
            }
        }

        auto draw_pin_dot = [&](const connection_pin_data& pin)
        {
            float dot_radius = ImGui::GetTextLineHeight() / 2 - ImGui::GetStyle().CellPadding.y;
            draw_list->AddCircleFilled(pin.pos, dot_radius, pin.col);
        };

        for (int s = 0; s < total_stages; ++s)
        {
            if (s > 0)
            {
                for (const auto& pin_in : stage_pins_in[s]) draw_pin_dot(pin_in);
            }
            if (s < total_stages - 1)
            {
                for (const auto& pin_out : stage_pins_out[s]) draw_pin_dot(pin_out);
            }
        }
    }

    void draw_connection_card_footer
    (
        gui_controller& ctrl,
        adam::language lang,
        adam::connection_view* conn,
        bool is_drag_preview,
        float port_w,
        float current_y,
        float start_x,
        float avail_x,
        float avail_w
    )
    {
        (void)ctrl;
        const char* btn_add_input_str = get_gui_string(gui_string_id::btn_add_input, lang);
        const char* btn_add_processor_str = get_gui_string(gui_string_id::btn_add_processor, lang);
        const char* btn_add_output_str = get_gui_string(gui_string_id::btn_add_output, lang);

        float add_in_w = ImGui::CalcTextSize(btn_add_input_str).x + ImGui::GetStyle().FramePadding.x * 2.0f;
        float in_x = start_x + port_w * 0.5f - add_in_w * 0.5f;
        ImGui::SetCursorPos(ImVec2(std::max(start_x, in_x), current_y));
        if (ImGui::Button(btn_add_input_str) && !is_drag_preview)
        {
            g_target_connection = conn->name;
            g_target_direction = adam::port::direction_in;
            g_request_port_popup = true;
        }

        if (!conn->inputs.empty())
        {
            float add_out_w = ImGui::CalcTextSize(btn_add_output_str).x + ImGui::GetStyle().FramePadding.x * 2.0f;
            float out_x = start_x + avail_x - port_w * 0.5f - add_out_w * 0.5f;
            ImGui::SetCursorPos(ImVec2(std::min(start_x + avail_x - add_out_w, out_x), current_y));
            if (ImGui::Button(btn_add_output_str) && !is_drag_preview)
            {
                g_target_connection = conn->name;
                g_target_direction = adam::port::direction_out;
                g_request_port_popup = true;
            }

            if (!conn->outputs.empty())
            {
                float add_proc_w = ImGui::CalcTextSize(btn_add_processor_str).x + ImGui::GetStyle().FramePadding.x * 2.0f;
                float center_x = (avail_w - add_proc_w) * 0.5f;
                ImGui::SetCursorPos(ImVec2(center_x, current_y));
                if (ImGui::Button(btn_add_processor_str) && !is_drag_preview)
                {
                    g_target_connection = conn->name;
                    g_request_processor_popup = true;
                }
            }
        }
    }

    void draw_connection_card
    (
        gui_controller& ctrl,
        adam::language lang,
        int sort_mode,
        adam::string_hash hash,
        adam::connection_view* conn,
        bool is_drag_preview,
        float card_w
    )
    {
        if (!conn) return;

        float dpi_scale = ImGui::GetStyle()._MainScale;
        auto& reg_view = ctrl.commander().registry();
        const auto& ports = reg_view.get_ports();

        bool is_unavailable = conn->is_unavailable;
        bool input_missing = false;
        bool output_missing = false;

        auto* theme_param = dynamic_cast<adam::configuration_parameter_string*>(ctrl.get_parameters().get("theme"_ct));
        bool is_light_theme = theme_param && theme_param->get_value() == "default-light"_ct;

        auto& stage_pins_in = is_drag_preview ? g_stage_pins_in_preview : g_stage_pins_in_normal;
        auto& stage_pins_out = is_drag_preview ? g_stage_pins_out_preview : g_stage_pins_out_normal;

        ImGui::PushID((const void*)(intptr_t)(is_drag_preview ? hash + 0x1000000 : hash));
            
        bool is_being_dragged = false;
        if (!is_drag_preview && sort_mode == 6) 
        {
            if (const ImGuiPayload* payload = ImGui::GetDragDropPayload()) 
            {
                if (payload->IsDataType("DND_CONNECTION") && *(adam::string_hash*)payload->Data == hash) 
                {
                    is_being_dragged = true;
                }
            }
        }


        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f * dpi_scale);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f * dpi_scale, 8.0f * dpi_scale));
            
        ImVec4 bg = is_drag_preview ? ImVec4(0.2f, 0.2f, 0.2f, 0.9f) : ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
            
        if (conn->color != 0)
        {
            bg.x = ((conn->color >> 16) & 0xFF) / 255.0f * 0.35f + bg.x * 0.65f;
            bg.y = ((conn->color >> 8) & 0xFF) / 255.0f * 0.35f + bg.y * 0.65f;
            bg.z = (conn->color & 0xFF) / 255.0f * 0.35f + bg.z * 0.65f;
        }
            
        if (is_being_dragged || is_unavailable)
        {
            bg.w *= 0.4f;
        }
            
        ImGui::PushStyleColor(ImGuiCol_ChildBg, bg);

        size_t max_rows = std::max(conn->inputs.size(), conn->outputs.size());
        if (max_rows == 0 && !conn->processors.empty()) max_rows = 1;
            
        float node_h = ImGui::GetTextLineHeight() * 2.0f;
        float row_height = node_h + 10.0f * dpi_scale;
            
        float base_height = ImGui::GetStyle().WindowPadding.y * 2.0f;
        base_height += ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y;
        base_height += (sort_mode == 6 && !is_drag_preview ? 4.0f * dpi_scale : 1.0f) + ImGui::GetStyle().ItemSpacing.y;

        if (!is_drag_preview)
            base_height += ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y;

        if (max_rows > 0 || !conn->processors.empty())
            base_height += ImGui::GetStyle().ItemSpacing.y * 2.0f + 1.0f;
        
        base_height += ImGui::GetFrameHeight();

        int total_stages = 2 + static_cast<int>(conn->processors.size());

        float in_offset = (static_cast<float>(max_rows) - static_cast<float>(conn->inputs.size())) * 0.5f;
        float out_offset = (static_cast<float>(max_rows) - static_cast<float>(conn->outputs.size())) * 0.5f;

        float in_col_bottom = (in_offset + static_cast<float>(conn->inputs.size())) * row_height;
        float out_col_bottom = (out_offset + static_cast<float>(conn->outputs.size())) * row_height;

        float max_proc_h = static_cast<float>(max_rows) * row_height;
        if (!is_drag_preview)
        {
            for (auto pid : conn->inputs)
            {
                uint64_t uid = get_unique_node_id(pid, hash, 0);
                if (g_expanded_nodes.count(uid)) in_col_bottom += get_expanded_node_height(uid, dpi_scale);
            }

            int stage = 1;
            for (auto fid : conn->processors)
            {
                uint64_t uid = get_unique_node_id(fid, hash, stage);
                if (g_expanded_nodes.count(uid))
                {
                    float h = static_cast<float>(max_rows) * row_height + get_expanded_node_height(uid, dpi_scale);
                    if (h > max_proc_h) max_proc_h = h;
                }
                stage++;
            }

            for (auto pid : conn->outputs)
            {
                uint64_t uid = get_unique_node_id(pid, hash, total_stages - 1);
                if (g_expanded_nodes.count(uid)) out_col_bottom += get_expanded_node_height(uid, dpi_scale);
            }
        }
        
        float max_col_h = std::max({in_col_bottom, out_col_bottom, max_proc_h});
        float child_height = base_height + max_col_h;

        if (!is_drag_preview) ImGui::BeginGroup();

        static std::vector<expanded_port_draw_info> deferred_expansions;
        deferred_expansions.clear();

        if (is_being_dragged) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.4f);

        if (ImGui::BeginChild(is_drag_preview ? "ConnCardPreview" : "ConnCard", ImVec2(card_w, child_height), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
        {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            float avail_x = ImGui::GetContentRegionAvail().x;
                
            float char_width = ImGui::CalcTextSize("x").x;
            float desired_node_w = char_width * 40.0f + ImGui::GetStyle().FramePadding.x * 4.0f;
                
            float port_w = std::min(desired_node_w, avail_x * 0.25f);
            if (port_w < char_width * 15.0f) port_w = char_width * 15.0f;
                
            float proc_w = port_w;
            bool compact_processors = false;
                
            if (total_stages > 2)
            {
                float gap_if_large = (avail_x - static_cast<float>(total_stages) * port_w) / static_cast<float>(total_stages - 1);
                if (gap_if_large < 10.0f * dpi_scale)
                {
                    proc_w = char_width * 5.0f + ImGui::GetStyle().FramePadding.x * 4.0f;
                    compact_processors = true;
                        
                    float gap_if_compact = (avail_x - 2.0f * port_w - static_cast<float>(conn->processors.size()) * proc_w) / static_cast<float>(total_stages - 1);
                    if (gap_if_compact < 10.0f * dpi_scale)
                    {
                        float available_for_procs = avail_x - 2.0f * port_w - static_cast<float>(total_stages - 1) * 10.0f * dpi_scale;
                        if (available_for_procs > 0.0f)
                            proc_w = available_for_procs / static_cast<float>(conn->processors.size());
                        else
                            proc_w = 4.0f * dpi_scale; 
                    }
                }
            }

            draw_connection_card_header(ctrl, lang, sort_mode, hash, conn, is_drag_preview, dpi_scale, port_w, is_unavailable);

            ImVec2 cur_pos = ImGui::GetCursorScreenPos();

            if (!is_drag_preview)
            {
                static std::vector<std::pair<adam::string_hashed, adam::string_hashed>> available_formats;
                static int s_formats_frame = -1;
                {
                    std::lock_guard<const adam::module_view> mod_lg(ctrl.commander().modules());
                    const auto& loaded_modules = ctrl.commander().get_modules().get_loaded();

                    if (ImGui::GetFrameCount() != s_formats_frame)
                    {
                        s_formats_frame = ImGui::GetFrameCount();
                        available_formats.clear();
                        available_formats.push_back({ "transparent"_ct, "internal"_ct });
                        for (const auto& [mod_name, mod_info] : ctrl.commander().get_modules().database())
                        {
                            if (loaded_modules.find(mod_name) != loaded_modules.end())
                            {
                                for (const auto& fmt : mod_info.data_formats)
                                    available_formats.push_back({ fmt, mod_name });
                            }
                        }
                    }

                     if (is_unavailable)
                     {
                         bool is_in_transparent = conn->input_format.get_hash() == "transparent"_ct.get_hash();
                         if (!is_in_transparent && conn->input_format_module.get_hash() != "internal"_ct.get_hash() && loaded_modules.find(conn->input_format_module.get_hash()) == loaded_modules.end())
                         {
                             input_missing = true;
                         }
                         
                         bool is_out_transparent = conn->output_format.get_hash() == "transparent"_ct.get_hash();
                         if (!is_out_transparent && conn->output_format_module.get_hash() != "internal"_ct.get_hash() && loaded_modules.find(conn->output_format_module.get_hash()) == loaded_modules.end())
                         {
                             output_missing = true;
                         }
                     }
                }

                ImGui::SetCursorScreenPos(ImVec2(cur_pos.x, cur_pos.y));
                char in_fmt_str[256];
                 if (conn->input_format == "transparent"_ct)
                 {
                     snprintf(in_fmt_str, sizeof(in_fmt_str), "%s", get_gui_string(gui_string_id::lbl_data_format_transparent_none, lang));
                 }
                 else
                 {
                     snprintf(in_fmt_str, sizeof(in_fmt_str), "%s", conn->input_format.c_str());
                 }
                 if (!conn->input_format_module.empty() && conn->input_format_module != "internal"_ct)
                 {
                     size_t len = strlen(in_fmt_str);
                     snprintf(in_fmt_str + len, sizeof(in_fmt_str) - len, " [%s]", conn->input_format_module.c_str());
                 }

                if (input_missing)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, get_gui_color(gui_color_id::log_warning));
                }

                float cb_size = ImGui::GetFrameHeight();
                const char* inspect_lbl = get_gui_string(gui_string_id::col_inspect, lang);
                float inspect_w = ImGui::CalcTextSize(inspect_lbl).x + cb_size + ImGui::GetStyle().ItemInnerSpacing.x;

                ImGui::SetNextItemWidth(port_w - inspect_w - ImGui::GetStyle().ItemSpacing.x);
                ImGui::PushID("conn_input_format_combo");
                bool in_combo_open = ImGui::BeginCombo("##InputFormatCombo", in_fmt_str);
                
                if (input_missing)
                {
                    ImGui::PopStyleColor();
                    if (ImGui::IsItemHovered())
                    {
                        ImGui::BeginTooltip();
                        ImGui::Text(get_gui_string(gui_string_id::tt_module_missing, lang), conn->input_format_module.c_str());
                        ImGui::EndTooltip();
                    }
                }

                if (in_combo_open)
                {
                    for (const auto& [fmt, mod] : available_formats)
                    {
                         char item_str[256];
                         if (fmt == "transparent"_ct)
                             snprintf(item_str, sizeof(item_str), "%s", get_gui_string(gui_string_id::lbl_data_format_transparent_none, lang));
                         else
                             snprintf(item_str, sizeof(item_str), "%s", fmt.c_str());
                         if (!mod.empty() && mod != "internal"_ct)
                         {
                             size_t len = strlen(item_str);
                             snprintf(item_str + len, sizeof(item_str) - len, " [%s]", mod.c_str());
                         }
                        
                        bool is_selected = (fmt == conn->input_format && mod == conn->input_format_module);
                        if (ImGui::Selectable(item_str, is_selected))
                        {
                            if (!is_selected)
                            {
                                ctrl.enqueue_commander_action([&ctrl, hash, fmt, mod]()
                                {
                                    ctrl.commander().request_connection_set_input_data_format(hash, fmt.get_hash(), mod.get_hash());
                                });
                            }
                        }
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
                ImGui::PopID();

                ImGui::SameLine();
                bool input_inspect_val = ctrl.commander().get_connection_input_inspectors().find(hash) != ctrl.commander().get_connection_input_inspectors().end();
                ImGui::PushID("conn_input_inspect");
                if (ImGui::Checkbox(inspect_lbl, &input_inspect_val))
                {
                    if (input_inspect_val)
                    {
                        ctrl.enqueue_commander_action([&ctrl, hash]() 
                        {
                            auto& cmdr = ctrl.commander();
                            if (cmdr.get_connection_input_inspectors().find(hash) == cmdr.get_connection_input_inspectors().end())
                            {
                                adam::data_inspector* new_inspector = nullptr;
                                cmdr.request_connection_input_inspector_create(hash, make_inspector_connection_input_buffer_callback(hash), new_inspector);
                            }
                        });
                        g_request_open_inspector = true;
                        g_connection_input_to_expand_in_inspector = hash;
                        g_pending_inspector_connections_input.insert(hash);
                    }
                    else
                    {
                        g_expanded_inspector_connections_input.erase(hash);
                        g_pending_inspector_connections_input.erase(hash);
                        ctrl.enqueue_commander_action([&ctrl, hash]() 
                        {
                            auto& cmdr = ctrl.commander();
                            auto it = cmdr.connection_input_inspectors().find(hash);
                            if (it != cmdr.connection_input_inspectors().end())
                            {
                                cmdr.request_connection_input_inspector_destroy(it->second);
                            }
                        });
                    }
                }
                ImGui::PopID();

                ImGui::SetCursorScreenPos(ImVec2(cur_pos.x + avail_x - port_w, cur_pos.y));
                char out_fmt_str[256];
                 if (conn->output_format == "transparent"_ct)
                 {
                     snprintf(out_fmt_str, sizeof(out_fmt_str), "%s", get_gui_string(gui_string_id::lbl_data_format_transparent_none, lang));
                 }
                 else
                 {
                     snprintf(out_fmt_str, sizeof(out_fmt_str), "%s", conn->output_format.c_str());
                 }
                 if (!conn->output_format_module.empty() && conn->output_format_module != "internal"_ct)
                 {
                     size_t len = strlen(out_fmt_str);
                     snprintf(out_fmt_str + len, sizeof(out_fmt_str) - len, " [%s]", conn->output_format_module.c_str());
                 }

                if (output_missing)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, get_gui_color(gui_color_id::log_warning));
                }

                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted(inspect_lbl);
                ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
                bool output_inspect_val = ctrl.commander().get_connection_output_inspectors().find(hash) != ctrl.commander().get_connection_output_inspectors().end();
                ImGui::PushID("conn_output_inspect");
                if (ImGui::Checkbox("##inspect_out", &output_inspect_val))
                {
                    if (output_inspect_val)
                    {
                        ctrl.enqueue_commander_action([&ctrl, hash]() 
                        {
                            auto& cmdr = ctrl.commander();
                            if (cmdr.get_connection_output_inspectors().find(hash) == cmdr.get_connection_output_inspectors().end())
                            {
                                adam::data_inspector* new_inspector = nullptr;
                                cmdr.request_connection_output_inspector_create(hash, make_inspector_connection_output_buffer_callback(hash), new_inspector);
                            }
                        });
                        g_request_open_inspector = true;
                        g_connection_output_to_expand_in_inspector = hash;
                        g_pending_inspector_connections_output.insert(hash);
                    }
                    else
                    {
                        g_expanded_inspector_connections_output.erase(hash);
                        g_pending_inspector_connections_output.erase(hash);
                        ctrl.enqueue_commander_action([&ctrl, hash]() 
                        {
                            auto& cmdr = ctrl.commander();
                            auto it = cmdr.connection_output_inspectors().find(hash);
                            if (it != cmdr.connection_output_inspectors().end())
                            {
                                cmdr.request_connection_output_inspector_destroy(it->second);
                            }
                        });
                    }
                }
                ImGui::PopID();

                ImGui::SameLine();
                ImGui::SetNextItemWidth(port_w - inspect_w - ImGui::GetStyle().ItemSpacing.x);
                ImGui::PushID("conn_output_format_combo");
                bool out_combo_open = ImGui::BeginCombo("##OutputFormatCombo", out_fmt_str);

                if (output_missing)
                {
                    ImGui::PopStyleColor();
                    if (ImGui::IsItemHovered())
                    {
                        ImGui::BeginTooltip();
                        ImGui::Text(get_gui_string(gui_string_id::tt_module_missing, lang), conn->output_format_module.c_str());
                        ImGui::EndTooltip();
                    }
                }

                if (out_combo_open)
                {
                    for (const auto& [fmt, mod] : available_formats)
                    {
                         char item_str[256];
                         if (fmt == "transparent"_ct)
                             snprintf(item_str, sizeof(item_str), "%s", get_gui_string(gui_string_id::lbl_data_format_transparent_none, lang));
                         else
                             snprintf(item_str, sizeof(item_str), "%s", fmt.c_str());
                         if (!mod.empty() && mod != "internal"_ct)
                         {
                             size_t len = strlen(item_str);
                             snprintf(item_str + len, sizeof(item_str) - len, " [%s]", mod.c_str());
                         }
                        
                        bool is_selected = (fmt == conn->output_format && mod == conn->output_format_module);
                        if (ImGui::Selectable(item_str, is_selected))
                        {
                            if (!is_selected)
                            {
                                ctrl.enqueue_commander_action([&ctrl, hash, fmt, mod]()
                                {
                                    ctrl.commander().request_connection_set_output_data_format(hash, fmt.get_hash(), mod.get_hash());
                                });
                            }
                        }
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
                ImGui::PopID();

                cur_pos.y += ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y;
            }
                
            float total_node_widths = 2.0f * port_w + static_cast<float>(std::max(0, total_stages - 2)) * proc_w;
            float gap = (total_stages > 1) ? (avail_x - total_node_widths) / static_cast<float>(total_stages - 1) : 0.0f;
                
            if (stage_pins_in.size() < static_cast<size_t>(total_stages))
            {
                stage_pins_in.resize(total_stages);
                stage_pins_out.resize(total_stages);
            }
            for (int i = 0; i < total_stages; ++i)
            {
                stage_pins_in[i].clear();
                stage_pins_out[i].clear();
            }

            // Determine dragging / reordering / snapping for processor
            bool is_dragging_processor = false;
            bool inside_card = false;
            ImVec2 mouse_pos = ImGui::GetMousePos();
            if (!is_drag_preview)
            {
                if (const ImGuiPayload* payload = ImGui::GetDragDropPayload())
                {
                    if (payload->IsDataType("DND_PROCESSOR"))
                    {
                        auto* p_data = (const DragProcessorPayload*)payload->Data;
                        if (p_data->connection == hash)
                        {
                            is_dragging_processor = true;
                            g_is_dragging_processor = true;
                            g_dragged_processor_conn_hash = hash;
                            g_dragged_processor_hash = p_data->processor;
                            
                            if (g_dragged_processor_original_index == -1)
                            {
                                auto& procs = conn->processors;
                                auto it = std::find(procs.begin(), procs.end(), g_dragged_processor_hash);
                                if (it != procs.end())
                                {
                                    g_dragged_processor_original_index = static_cast<int>(std::distance(procs.begin(), it));
                                }
                            }

                            // Bounding box of the child window
                            ImVec2 card_min = ImGui::GetWindowPos();
                            ImVec2 card_max = ImVec2(card_min.x + card_w, card_min.y + child_height);
                            inside_card = (mouse_pos.x >= card_min.x && mouse_pos.x <= card_max.x &&
                                           mouse_pos.y >= card_min.y && mouse_pos.y <= card_max.y);
                        }
                    }
                }
            }

            if (g_is_dragging_processor && g_dragged_processor_conn_hash == hash)
            {
                int target_idx = -1;
                if (inside_card)
                {
                    int num_processors = static_cast<int>(conn->processors.size());
                    if (num_processors > 0)
                    {
                        // Check if mouse is hovering over input port
                        if (mouse_pos.x >= cur_pos.x && mouse_pos.x <= cur_pos.x + port_w)
                        {
                            target_idx = 0;
                        }
                        // Check if mouse is hovering over output port
                        else if (mouse_pos.x >= cur_pos.x + avail_x - port_w && mouse_pos.x <= cur_pos.x + avail_x)
                        {
                            target_idx = num_processors - 1;
                        }
                        else
                        {
                            // Check if mouse is hovering over any processor slot
                            float slot_0_center_x = cur_pos.x + port_w + gap + proc_w * 0.5f;
                            for (int i = 0; i < num_processors; ++i)
                            {
                                float slot_left = slot_0_center_x - proc_w * 0.5f + static_cast<float>(i) * (proc_w + gap);
                                float slot_right = slot_left + proc_w;
                                if (mouse_pos.x >= slot_left && mouse_pos.x <= slot_right)
                                {
                                    target_idx = i;
                                    break;
                                }
                            }
                        }
                    }
                }
                else
                {
                    // Dragged outside -> snap back to original position
                    target_idx = g_dragged_processor_original_index;
                }

                if (target_idx != -1)
                {
                    auto& procs = conn->processors;
                    auto it = std::find(procs.begin(), procs.end(), g_dragged_processor_hash);
                    if (it != procs.end())
                    {
                        int current_idx = static_cast<int>(std::distance(procs.begin(), it));
                        if (target_idx != current_idx)
                        {
                            procs.erase(it);
                            if (target_idx > static_cast<int>(procs.size()))
                                target_idx = static_cast<int>(procs.size());
                            procs.insert(procs.begin() + target_idx, g_dragged_processor_hash);
                        }
                    }
                }
            }

            if (!is_drag_preview && g_is_dragging_processor && g_dragged_processor_conn_hash == hash)
            {
                if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
                {
                    if (inside_card)
                    {
                        auto it = std::find(conn->processors.begin(), conn->processors.end(), g_dragged_processor_hash);
                        if (it != conn->processors.end())
                        {
                            uint32_t final_idx = static_cast<uint32_t>(std::distance(conn->processors.begin(), it));
                            if (ctrl.is_commander_active())
                            {
                                ctrl.enqueue_commander_action([&ctrl, hash, proc_hash = g_dragged_processor_hash, final_idx]()
                                {
                                    ctrl.commander().request_connection_processor_reorder(hash, proc_hash, final_idx);
                                });
                            }
                        }
                    }
                    else
                    {
                        // Dragged outside and released -> restore to original index silently
                        auto& procs = conn->processors;
                        auto it = std::find(procs.begin(), procs.end(), g_dragged_processor_hash);
                        if (it != procs.end())
                        {
                            procs.erase(it);
                            int target_idx = g_dragged_processor_original_index;
                            if (target_idx >= 0)
                            {
                                if (target_idx > static_cast<int>(procs.size()))
                                    target_idx = static_cast<int>(procs.size());
                                procs.insert(procs.begin() + target_idx, g_dragged_processor_hash);
                            }
                        }
                    }
                    g_is_dragging_processor = false;
                    g_dragged_processor_conn_hash = 0;
                    g_dragged_processor_hash = 0;
                    g_active_processor_drag_target_index = -1;
                    g_dragged_processor_original_index = -1;
                }
                else if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
                {
                    // Safety cleanup if we missed the release event
                    g_is_dragging_processor = false;
                    g_dragged_processor_conn_hash = 0;
                    g_dragged_processor_hash = 0;
                    g_active_processor_drag_target_index = -1;
                    g_dragged_processor_original_index = -1;
                }
            }

            ImColor in_col = get_gui_color(gui_color_id::node_input);
            ImColor proc_col = get_gui_color(gui_color_id::node_processor);
            ImColor out_col = get_gui_color(gui_color_id::node_output);

            float current_in_y = in_offset * row_height;
            for (auto pid : conn->inputs)
            {
                connection_pin_data p_in, p_out;
                auto it = ports.find(pid);
                bool is_unavail = (it != ports.end() && it->second->is_unavailable);
                ImColor col = in_col;
                if (is_unavail) col.Value.w *= 0.4f;

                const char* mod_name = "Unknown";
                if (is_unavail && it->second->type_module.get_hash() != 0)
                    mod_name = it->second->type_module.c_str();

                bool is_exp = draw_connection_node(ctrl, lang, conn, hash, dpi_scale, draw_list, cur_pos, port_w, gap, proc_w, node_h, row_height, total_stages, avail_x, is_drag_preview, it != ports.end() ? it->second->name.c_str() : "Unknown Input", node_type_input, 0, 0.0f, col, p_in, p_out, is_unavail, mod_name, pid, current_in_y, deferred_expansions);
                
                if (!conn->input_format.empty())
                    p_out.format_name = conn->input_format.c_str();
                else
                    p_out.format_name = "transparent";
                stage_pins_out[0].push_back(p_out);
                current_in_y += row_height;

                uint64_t uid = get_unique_node_id(pid, hash, 0);
                if (is_exp)
                    current_in_y += get_expanded_node_height(uid, dpi_scale);
            }

            int current_stage = 1;
            int processor_idx = 1;
            float proc_offset = (static_cast<float>(max_rows) - 1.0f) * 0.5f;
            float proc_extra_y = proc_offset * row_height;
            for (auto fid : conn->processors)
            {
                auto proc_it = ctrl.commander().registry().get_processors().find(fid);
                const char* proc_name = "Unknown Processor";
                if (proc_it != ctrl.commander().registry().get_processors().end())
                    proc_name = proc_it->second->name.c_str();

                bool is_unavail = (proc_it != ctrl.commander().registry().get_processors().end() && proc_it->second->is_unavailable);
                ImColor col = proc_col;
                if (is_unavail) col.Value.w *= 0.4f;
                if (g_is_dragging_processor && g_dragged_processor_conn_hash == hash && fid == g_dragged_processor_hash)
                {
                    col.Value.w *= 0.4f;
                }

                const char* mod_name = "Unknown";
                if (is_unavail && proc_it->second->module_name.get_hash() != 0)
                    mod_name = proc_it->second->module_name.c_str();

                connection_pin_data p_in, p_out;
                bool is_node_drag_preview = is_drag_preview || (g_is_dragging_processor && g_dragged_processor_conn_hash == hash && fid == g_dragged_processor_hash);
                if (compact_processors)
                {
                    char short_name[16];
                    snprintf(short_name, sizeof(short_name), "%02d   ", processor_idx);
                    draw_connection_node(ctrl, lang, conn, hash, dpi_scale, draw_list, cur_pos, port_w, gap, proc_w, node_h, row_height, total_stages, avail_x, is_node_drag_preview, short_name, node_type_processor, current_stage, 0.0f, col, p_in, p_out, is_unavail, mod_name, fid, proc_extra_y, deferred_expansions);
                }
                else
                {
                    draw_connection_node(ctrl, lang, conn, hash, dpi_scale, draw_list, cur_pos, port_w, gap, proc_w, node_h, row_height, total_stages, avail_x, is_node_drag_preview, proc_name, node_type_processor, current_stage, 0.0f, col, p_in, p_out, is_unavail, mod_name, fid, proc_extra_y, deferred_expansions);
                }
                p_in.format_name = "transparent";
                p_out.format_name = "transparent";
                stage_pins_in[current_stage].push_back(p_in);
                stage_pins_out[current_stage].push_back(p_out);
                current_stage++;
                processor_idx++;
            }

            float current_out_y = out_offset * row_height;
            for (auto pid : conn->outputs)
            {
                connection_pin_data p_in, p_out;
                auto it = ports.find(pid);
                bool is_unavail = (it != ports.end() && it->second->is_unavailable);
                ImColor col = out_col;
                if (is_unavail) col.Value.w *= 0.4f;

                const char* mod_name = "Unknown";
                if (is_unavail && it->second->type_module.get_hash() != 0)
                    mod_name = it->second->type_module.c_str();

                bool is_exp = draw_connection_node(ctrl, lang, conn, hash, dpi_scale, draw_list, cur_pos, port_w, gap, proc_w, node_h, row_height, total_stages, avail_x, is_drag_preview, it != ports.end() ? it->second->name.c_str() : "Unknown Output", node_type_output, total_stages - 1, 0.0f, col, p_in, p_out, is_unavail, mod_name, pid, current_out_y, deferred_expansions);
                if (!conn->output_format.empty())
                    p_in.format_name = conn->output_format.c_str();
                else
                    p_in.format_name = "transparent";
                stage_pins_in[total_stages - 1].push_back(p_in);
                current_out_y += row_height;

                uint64_t uid = get_unique_node_id(pid, hash, total_stages - 1);
                if (is_exp)
                    current_out_y += get_expanded_node_height(uid, dpi_scale);
            }

            draw_connection_lines(draw_list, dpi_scale, is_light_theme, total_stages, max_rows, row_height, avail_x, cur_pos, stage_pins_in, stage_pins_out, conn);

            for (const auto& info : deferred_expansions)
            {
                draw_expanded_port_node(ctrl, lang, ctrl.commander().registry(), dpi_scale, draw_list, info);
            }

            ImGui::SetCursorScreenPos(ImVec2(cur_pos.x, cur_pos.y + max_col_h));
                
            if (max_rows > 0 || !conn->processors.empty())
            {
                ImGui::Spacing();
                ImGui::Separator();
            }
                
            float avail_w = ImGui::GetWindowWidth();
            float current_y = ImGui::GetCursorPosY();
            float start_x = ImGui::GetStyle().WindowPadding.x;

            draw_connection_card_footer(ctrl, lang, conn, is_drag_preview, port_w, current_y, start_x, avail_x, avail_w);
        }
        ImGui::EndChild();

        if (is_being_dragged) ImGui::PopStyleVar();

        if (!is_drag_preview)
        {
            ImGui::Spacing();
            ImGui::EndGroup();
        }

        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
        ImGui::PopID();
    }

    void draw_connections_list
    (
        gui_controller& ctrl,
        adam::language lang,
        int sort_mode,
        float& card_width,
        const std::vector<std::pair<adam::string_hash, adam::connection_view*>>& sorted_connections,
        bool is_dragging_connection
    )
    {
        if (!ImGui::BeginChild("ConnectionsList", ImVec2(0, -(ImGui::GetFrameHeight() * 1.5f + ImGui::GetStyle().ItemSpacing.y)), false))
        {
            return;
        }

        card_width = ImGui::GetContentRegionAvail().x;

        for (size_t i = 0; i < sorted_connections.size(); ++i)
        {
            auto hash = sorted_connections[i].first;
            auto* conn = sorted_connections[i].second;

            draw_connection_card(ctrl, lang, sort_mode, hash, conn, false, card_width);

            if (sort_mode == 6 && is_dragging_connection)
            {
                ImVec2 min = ImGui::GetItemRectMin();
                ImVec2 max = ImGui::GetItemRectMax();
                ImVec2 mouse_pos = ImGui::GetMousePos();
                
                if (mouse_pos.y >= min.y && mouse_pos.y <= max.y)
                {
                    g_active_drag_target_index = i;
                }
            }
        }
        ImGui::EndChild();
    }
}
