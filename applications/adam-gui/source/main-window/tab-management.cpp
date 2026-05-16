#include "tab-management.hpp"
#include "main-window.hpp"

#include <imgui.h>
#include <algorithm>
#include <vector>
#include <mutex>

namespace adam::gui 
{
    void render_tab_management(gui_controller& ctrl, adam::language lang)
    {
        bool commander_active   = ctrl.is_commander_active();
        float dpi_scale         = ImGui::GetStyle()._MainScale;

        if (ImGui::BeginPopupModal(get_gui_string(gui_string_id::dlg_create_connection, lang), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            static char conn_name[256] = "";
            ImGui::InputText(get_gui_string(gui_string_id::tbl_name, lang), conn_name, sizeof(conn_name));

            ImGui::Spacing();
            if (ImGui::Button(get_gui_string(gui_string_id::btn_create, lang), ImVec2(120.0f * dpi_scale, 0.0f)))
            {
                if (conn_name[0] != '\0')
                {
                    ctrl.get_commander().request_connection_create(adam::string_hashed(&conn_name[0]));
                    conn_name[0] = '\0';
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button(get_gui_string(gui_string_id::btn_cancel, lang), ImVec2(120.0f * dpi_scale, 0.0f)))
                ImGui::CloseCurrentPopup();
            
            ImGui::EndPopup();
        }

        auto& reg_view = ctrl.get_commander().registry();

        std::lock_guard<const adam::registry_view> lock(reg_view);

        #ifdef ADAM_BUILD_DEBUG
        // Inject a test connection if it doesn't exist to test layout
        static bool test_injected = false;
        if (!test_injected)
        {
            auto conn = std::make_unique<adam::connection_view>();
            conn->name = adam::string_hashed("Debug_Connection");
            
            auto p_in1 = std::make_unique<adam::port_view>(); p_in1->name = adam::string_hashed("Video_In"); p_in1->type = adam::string_hashed("adam::port_input_internal");
            auto p_in2 = std::make_unique<adam::port_view>(); p_in2->name = adam::string_hashed("Audio_In"); p_in2->type = adam::string_hashed("adam::port_input_internal");
            
            auto p_out1 = std::make_unique<adam::port_view>(); p_out1->name = adam::string_hashed("Stream_Out"); p_out1->type = adam::string_hashed("adam::port_output_internal");
            auto p_out2 = std::make_unique<adam::port_view>(); p_out2->name = adam::string_hashed("Preview_Out"); p_out2->type = adam::string_hashed("adam::port_output_internal");

            conn->inputs.push_back(p_in1->name.get_hash());
            conn->inputs.push_back(p_in2->name.get_hash());
            conn->outputs.push_back(p_out1->name.get_hash());
            conn->outputs.push_back(p_out2->name.get_hash());

            // Mock processor
            conn->filters.push_back(1234);

            reg_view.ports()[p_in1->name.get_hash()] = std::move(p_in1);
            reg_view.ports()[p_in2->name.get_hash()] = std::move(p_in2);
            reg_view.ports()[p_out1->name.get_hash()] = std::move(p_out1);
            reg_view.ports()[p_out2->name.get_hash()] = std::move(p_out2);

            reg_view.connections()[conn->name.get_hash()] = std::move(conn);

            auto conn2 = std::make_unique<adam::connection_view>();
            conn2->name = adam::string_hashed("Converter_Connection");
            
            auto p2_in1 = std::make_unique<adam::port_view>(); p2_in1->name = adam::string_hashed("Raw_Data_In"); p2_in1->type = adam::string_hashed("adam::port_input_internal");
            auto p2_out1 = std::make_unique<adam::port_view>(); p2_out1->name = adam::string_hashed("Processed_Out"); p2_out1->type = adam::string_hashed("adam::port_output_internal");

            conn2->inputs.push_back(p2_in1->name.get_hash());
            conn2->outputs.push_back(p2_out1->name.get_hash());
            
            // Mock converter
            conn2->converters.push_back(5678);

            reg_view.ports()[p2_in1->name.get_hash()] = std::move(p2_in1);
            reg_view.ports()[p2_out1->name.get_hash()] = std::move(p2_out1);
            
            reg_view.connections()[conn2->name.get_hash()] = std::move(conn2);

            auto conn3 = std::make_unique<adam::connection_view>();
            conn3->name = adam::string_hashed("Splitter_Connection");
            
            auto p3_in1 = std::make_unique<adam::port_view>(); p3_in1->name = adam::string_hashed("Sensor_1"); p3_in1->type = adam::string_hashed("adam::port_input_internal");
            auto p3_in2 = std::make_unique<adam::port_view>(); p3_in2->name = adam::string_hashed("Sensor_2"); p3_in2->type = adam::string_hashed("adam::port_input_internal");
            
            auto p3_out1 = std::make_unique<adam::port_view>(); p3_out1->name = adam::string_hashed("Log_Out"); p3_out1->type = adam::string_hashed("adam::port_output_internal");
            auto p3_out2 = std::make_unique<adam::port_view>(); p3_out2->name = adam::string_hashed("Display_Out"); p3_out2->type = adam::string_hashed("adam::port_output_internal");
            auto p3_out3 = std::make_unique<adam::port_view>(); p3_out3->name = adam::string_hashed("Network_Out"); p3_out3->type = adam::string_hashed("adam::port_output_internal");

            conn3->inputs.push_back(p3_in1->name.get_hash());
            conn3->inputs.push_back(p3_in2->name.get_hash());
            conn3->outputs.push_back(p3_out1->name.get_hash());
            conn3->outputs.push_back(p3_out2->name.get_hash());
            conn3->outputs.push_back(p3_out3->name.get_hash());

            reg_view.ports()[p3_in1->name.get_hash()] = std::move(p3_in1);
            reg_view.ports()[p3_in2->name.get_hash()] = std::move(p3_in2);
            reg_view.ports()[p3_out1->name.get_hash()] = std::move(p3_out1);
            reg_view.ports()[p3_out2->name.get_hash()] = std::move(p3_out2);
            reg_view.ports()[p3_out3->name.get_hash()] = std::move(p3_out3);
            
            reg_view.connections()[conn3->name.get_hash()] = std::move(conn3);
            
            auto conn4 = std::make_unique<adam::connection_view>();
            conn4->name = adam::string_hashed("Complex_Filter_Connection");
            
            auto p4_in1 = std::make_unique<adam::port_view>(); p4_in1->name = adam::string_hashed("Mic_1"); p4_in1->type = adam::string_hashed("adam::port_input_internal");
            auto p4_in2 = std::make_unique<adam::port_view>(); p4_in2->name = adam::string_hashed("Mic_2"); p4_in2->type = adam::string_hashed("adam::port_input_internal");
            auto p4_in3 = std::make_unique<adam::port_view>(); p4_in3->name = adam::string_hashed("Mic_3"); p4_in3->type = adam::string_hashed("adam::port_input_internal");
            
            auto p4_out1 = std::make_unique<adam::port_view>(); p4_out1->name = adam::string_hashed("Mixed_Audio_Out"); p4_out1->type = adam::string_hashed("adam::port_output_internal");

            conn4->inputs.push_back(p4_in1->name.get_hash());
            conn4->inputs.push_back(p4_in2->name.get_hash());
            conn4->inputs.push_back(p4_in3->name.get_hash());
            conn4->outputs.push_back(p4_out1->name.get_hash());

            // Mock filters
            conn4->filters.push_back(1111);
            conn4->filters.push_back(2222);

            reg_view.ports()[p4_in1->name.get_hash()] = std::move(p4_in1);
            reg_view.ports()[p4_in2->name.get_hash()] = std::move(p4_in2);
            reg_view.ports()[p4_in3->name.get_hash()] = std::move(p4_in3);
            reg_view.ports()[p4_out1->name.get_hash()] = std::move(p4_out1);
            
            reg_view.connections()[conn4->name.get_hash()] = std::move(conn4);
            
            auto conn5 = std::make_unique<adam::connection_view>();
            conn5->name = adam::string_hashed("Massive_Connection");
            
            auto p5_in1 = std::make_unique<adam::port_view>(); p5_in1->name = adam::string_hashed("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"); p5_in1->type = adam::string_hashed("adam::port_input_internal");
            auto p5_in2 = std::make_unique<adam::port_view>(); p5_in2->name = adam::string_hashed("In_2"); p5_in2->type = adam::string_hashed("adam::port_input_internal");
            auto p5_in3 = std::make_unique<adam::port_view>(); p5_in3->name = adam::string_hashed("In_3"); p5_in3->type = adam::string_hashed("adam::port_input_internal");
            auto p5_in4 = std::make_unique<adam::port_view>(); p5_in4->name = adam::string_hashed("In_4"); p5_in4->type = adam::string_hashed("adam::port_input_internal");
            auto p5_in5 = std::make_unique<adam::port_view>(); p5_in5->name = adam::string_hashed("In_5"); p5_in5->type = adam::string_hashed("adam::port_input_internal");
            
            auto p5_out1 = std::make_unique<adam::port_view>(); p5_out1->name = adam::string_hashed("Out_1"); p5_out1->type = adam::string_hashed("adam::port_output_internal");
            auto p5_out2 = std::make_unique<adam::port_view>(); p5_out2->name = adam::string_hashed("Out_2"); p5_out2->type = adam::string_hashed("adam::port_output_internal");

            conn5->inputs.push_back(p5_in1->name.get_hash());
            conn5->inputs.push_back(p5_in2->name.get_hash());
            conn5->inputs.push_back(p5_in3->name.get_hash());
            conn5->inputs.push_back(p5_in4->name.get_hash());
            conn5->inputs.push_back(p5_in5->name.get_hash());

            conn5->outputs.push_back(p5_out1->name.get_hash());
            conn5->outputs.push_back(p5_out2->name.get_hash());

            for (int i = 0; i < 10; ++i)
                conn5->filters.push_back(10000 + i);

            reg_view.ports()[p5_in1->name.get_hash()] = std::move(p5_in1);
            reg_view.ports()[p5_in2->name.get_hash()] = std::move(p5_in2);
            reg_view.ports()[p5_in3->name.get_hash()] = std::move(p5_in3);
            reg_view.ports()[p5_in4->name.get_hash()] = std::move(p5_in4);
            reg_view.ports()[p5_in5->name.get_hash()] = std::move(p5_in5);
            reg_view.ports()[p5_out1->name.get_hash()] = std::move(p5_out1);
            reg_view.ports()[p5_out2->name.get_hash()] = std::move(p5_out2);
            
            reg_view.connections()[conn5->name.get_hash()] = std::move(conn5);
            
            test_injected = true;
        }
        #endif

        const auto& connections = reg_view.get_connections();
        const auto& ports = reg_view.get_ports();

        // Speed optimization: Reusing vectors instead of allocating per card
        static std::vector<std::vector<ImVec2>> stage_pins_in;
        static std::vector<std::vector<ImVec2>> stage_pins_out;

        if (ImGui::BeginChild("ConnectionsList", ImVec2(0, -(ImGui::GetFrameHeight() * 1.5f + ImGui::GetStyle().ItemSpacing.y)), true))
        {
            for (const auto& [hash, conn] : connections)
            {
                ImGui::PushID(static_cast<int>(hash));
                
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f * dpi_scale);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f * dpi_scale, 8.0f * dpi_scale));
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));

                size_t max_rows = std::max(conn->inputs.size(), conn->outputs.size());
                if (max_rows == 0) max_rows = 1;
                
                float node_h = ImGui::GetTextLineHeight() * 2.0f;
                float row_height = node_h + 10.0f * dpi_scale;
                
                float base_height = ImGui::GetTextLineHeight() + ImGui::GetFrameHeight() + 42.0f * dpi_scale;
                float child_height = base_height + static_cast<float>(max_rows) * row_height;

                if (ImGui::BeginChild("ConnCard", ImVec2(0, child_height), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
                {
                    ImDrawList* draw_list = ImGui::GetWindowDrawList();

                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "CONNECTION");
                    ImGui::SameLine();

                    char name_buf[256];
                    std::strncpy(name_buf, conn->name.c_str(), sizeof(name_buf));
                    name_buf[sizeof(name_buf) - 1] = '\0';

                    ImGui::SetNextItemWidth(250.0f * dpi_scale);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
                    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImGui::GetStyleColorVec4(ImGuiCol_FrameBgHovered));
                    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImGui::GetStyleColorVec4(ImGuiCol_FrameBgActive));
                    
                    ImGui::PushID("conn_name_edit");
                    bool enter_pressed = ImGui::InputText("##edit", name_buf, sizeof(name_buf), ImGuiInputTextFlags_EnterReturnsTrue);
                    bool deactivated = ImGui::IsItemDeactivatedAfterEdit();
                    ImGui::PopID();
                    
                    ImGui::PopStyleColor(3);

                    if (enter_pressed || deactivated)
                    {
                        if (name_buf[0] != '\0' && conn->name != string_hashed(&name_buf[0]))
                        {
                            ctrl.get_commander().request_connection_rename(conn->name, adam::string_hashed(&name_buf[0]));
                        }
                    }

                    const char* btn_start_str = get_gui_string(gui_string_id::btn_start, lang);
                    const char* btn_stop_str  = get_gui_string(gui_string_id::btn_stop, lang);
                    
                    float btn_start_w = ImGui::CalcTextSize(btn_start_str).x + ImGui::GetStyle().FramePadding.x * 2.0f;
                    float btn_stop_w  = ImGui::CalcTextSize(btn_stop_str).x + ImGui::GetStyle().FramePadding.x * 2.0f;
                    float total_btn_w = btn_start_w + btn_stop_w + ImGui::GetStyle().ItemSpacing.x;
                    
                    ImGui::SameLine(ImGui::GetWindowWidth() - total_btn_w - ImGui::GetStyle().WindowPadding.x);
                    
                    bool is_active = conn->is_active;
                    if (is_active) ImGui::BeginDisabled();
                    if (ImGui::Button(btn_start_str)) { ctrl.get_commander().request_connection_start(conn->name); }
                    if (is_active) ImGui::EndDisabled();
                    
                    ImGui::SameLine();
                    if (!is_active) ImGui::BeginDisabled();
                    if (ImGui::Button(btn_stop_str)) { ctrl.get_commander().request_connection_stop(conn->name); }
                    if (!is_active) ImGui::EndDisabled();

                    ImGui::Separator();

                    float avail_x = ImGui::GetContentRegionAvail().x;
                    ImVec2 cur_pos = ImGui::GetCursorScreenPos();
                    
                    size_t num_processors = conn->filters.size() + conn->converters.size();
                    int total_stages = 2 + static_cast<int>(num_processors);
                    
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
                            
                            float gap_if_compact = (avail_x - 2.0f * port_w - static_cast<float>(num_processors) * proc_w) / static_cast<float>(total_stages - 1);
                            if (gap_if_compact < 10.0f * dpi_scale)
                            {
                                float available_for_procs = avail_x - 2.0f * port_w - static_cast<float>(total_stages - 1) * 10.0f * dpi_scale;
                                if (available_for_procs > 0.0f)
                                    proc_w = available_for_procs / static_cast<float>(num_processors);
                                else
                                    proc_w = 4.0f * dpi_scale; // Minimal clamping if window is exceptionally tiny
                            }
                        }
                    }
                    
                    float total_node_widths = 2.0f * port_w + static_cast<float>(std::max(0, total_stages - 2)) * proc_w;
                    float gap = (total_stages > 1) ? (avail_x - total_node_widths) / static_cast<float>(total_stages - 1) : 0.0f;
                    
                    auto draw_node = [&](const char* name, int stage, float row, ImColor color, ImVec2& out_pin_in, ImVec2& out_pin_out)
                    {
                        float current_node_w = (stage == 0 || stage == total_stages - 1) ? port_w : proc_w;
                        float node_x;
                        if (total_stages == 1) node_x = cur_pos.x;
                        else if (stage == 0) node_x = cur_pos.x;
                        else if (stage == total_stages - 1) node_x = cur_pos.x + avail_x - current_node_w;
                        else node_x = cur_pos.x + port_w + gap + static_cast<float>(stage - 1) * (proc_w + gap);
                        
                        float node_y = cur_pos.y + row * row_height + (row_height - node_h) * 0.5f;
                        
                        ImVec2 p_min(node_x, node_y);
                        ImVec2 p_max(node_x + current_node_w, node_y + node_h);
                        
                        draw_list->AddRectFilled(p_min, p_max, color, 6.0f * dpi_scale);
                        draw_list->AddRect(p_min, p_max, ImColor(color.Value.x * 1.2f, color.Value.y * 1.2f, color.Value.z * 1.2f), 6.0f * dpi_scale, 0, 1.5f * dpi_scale);
                        
                        // Text with clipping
                        float text_width = ImGui::CalcTextSize(name).x;
                        float text_x = p_min.x + (current_node_w - text_width) * 0.5f;
                        if (text_x < p_min.x + 8.0f * dpi_scale) text_x = p_min.x + 8.0f * dpi_scale;
                        ImVec2 text_pos(text_x, p_min.y + (node_h - ImGui::GetTextLineHeight()) * 0.5f);
                        draw_list->PushClipRect(p_min, p_max, true);
                        draw_list->AddText(text_pos, ImColor(255, 255, 255), name);
                        draw_list->PopClipRect();

                        // Pin positions
                        out_pin_in = ImVec2(p_min.x, p_min.y + node_h * 0.5f);
                        out_pin_out = ImVec2(p_max.x, p_min.y + node_h * 0.5f);

                        // Draw pins as circles
                        if (stage > 0)
                            draw_list->AddCircleFilled(out_pin_in, 4.0f * dpi_scale, ImColor(200, 200, 200, 200));
                        if (stage < total_stages - 1)
                            draw_list->AddCircleFilled(out_pin_out, 4.0f * dpi_scale, ImColor(200, 200, 200, 200));
                    };

                    // Speed Optimization
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

                    ImColor in_col(0x26, 0x76, 0xA6, 220);
                    ImColor proc_col(0xA6, 0x76, 0x26, 220);
                    ImColor out_col(0xA6, 0x26, 0x26, 220);

                    float in_offset = (static_cast<float>(max_rows) - static_cast<float>(conn->inputs.size())) * 0.5f;
                    float out_offset = (static_cast<float>(max_rows) - static_cast<float>(conn->outputs.size())) * 0.5f;

                    float row_val = in_offset;
                    for (auto pid : conn->inputs)
                    {
                        ImVec2 p_in, p_out;
                        auto it = ports.find(pid);
                        draw_node(it != ports.end() ? it->second->name.c_str() : "Unknown Input", 0, row_val, in_col, p_in, p_out);
                        stage_pins_out[0].push_back(p_out);
                        row_val += 1.0f;
                    }

                    int current_stage = 1;
                    int processor_idx = 1;
                    float proc_row_val = (static_cast<float>(max_rows) - 1.0f) * 0.5f;
                    for (auto fid : conn->filters)
                    {
                        (void)fid; // Unused
                        ImVec2 p_in, p_out;
                        if (compact_processors)
                        {
                            char short_name[16];
                            snprintf(short_name, sizeof(short_name), "%02d   ", processor_idx);
                            draw_node(short_name, current_stage, proc_row_val, proc_col, p_in, p_out);
                        }
                        else
                        {
                            draw_node("Filter", current_stage, proc_row_val, proc_col, p_in, p_out);
                        }
                        stage_pins_in[current_stage].push_back(p_in);
                        stage_pins_out[current_stage].push_back(p_out);
                        current_stage++;
                        processor_idx++;
                    }
                    for (auto cid : conn->converters)
                    {
                        (void)cid; // Unused
                        ImVec2 p_in, p_out;
                        if (compact_processors)
                        {
                            char short_name[16];
                            snprintf(short_name, sizeof(short_name), "%02d   ", processor_idx);
                            draw_node(short_name, current_stage, proc_row_val, proc_col, p_in, p_out);
                        }
                        else
                        {
                            draw_node("Converter", current_stage, proc_row_val, proc_col, p_in, p_out);
                        }
                        stage_pins_in[current_stage].push_back(p_in);
                        stage_pins_out[current_stage].push_back(p_out);
                        current_stage++;
                        processor_idx++;
                    }

                    row_val = out_offset;
                    for (auto pid : conn->outputs)
                    {
                        ImVec2 p_in, p_out;
                        auto it = ports.find(pid);
                        draw_node(it != ports.end() ? it->second->name.c_str() : "Unknown Output", total_stages - 1, row_val, out_col, p_in, p_out);
                        stage_pins_in[total_stages - 1].push_back(p_in);
                        row_val += 1.0f;
                    }

                    ImColor line_col(200, 200, 200, 180);
                    float line_thickness = 5.f * dpi_scale;
                    
                    if (num_processors == 0 && !conn->inputs.empty() && !conn->outputs.empty() && (conn->inputs.size() > 1 || conn->outputs.size() > 1))
                    {
                        float center_y = cur_pos.y + static_cast<float>(max_rows) * row_height * 0.5f;
                        float mid_x = cur_pos.x + avail_x * 0.5f;
                        
                        ImVec2 p_mid(mid_x, center_y);
                        
                        for (const auto& pin_out : stage_pins_out[0])
                        {
                            float b_strength = (mid_x - pin_out.x) * 0.5f;
                            draw_list->AddBezierCubic(
                                pin_out, ImVec2(pin_out.x + b_strength, pin_out.y), ImVec2(mid_x - b_strength, center_y), p_mid,
                                line_col, line_thickness);
                        }
                        
                        for (const auto& pin_in : stage_pins_in[1])
                        {
                            float b_strength = (pin_in.x - mid_x) * 0.5f;
                            draw_list->AddBezierCubic(
                                p_mid, ImVec2(mid_x + b_strength, center_y), ImVec2(pin_in.x - b_strength, pin_in.y), pin_in,
                                line_col, line_thickness);
                        }
                        
                        // Draw a smooth circular cap to cleanly join the intersecting lines
                        draw_list->AddCircleFilled(p_mid, line_thickness * 0.6f, line_col);
                    }
                    else
                    {
                        for (int s = 0; s < total_stages - 1; ++s)
                        {
                            for (const auto& pin_out : stage_pins_out[s])
                            {
                                for (const auto& pin_in : stage_pins_in[s + 1])
                                {
                                    float b_strength = (pin_in.x - pin_out.x) * 0.5f;
                                    draw_list->AddBezierCubic(
                                        pin_out, ImVec2(pin_out.x + b_strength, pin_out.y), ImVec2(pin_in.x - b_strength, pin_in.y), pin_in,
                                        line_col, line_thickness);
                                }
                            }
                        }
                    }

                    ImGui::SetCursorScreenPos(ImVec2(cur_pos.x, cur_pos.y + static_cast<float>(max_rows) * row_height));
                    
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();
                    
                    bool is_empty = conn->inputs.empty() && conn->outputs.empty() && conn->filters.empty() && conn->converters.empty();

                    float avail_w = ImGui::GetWindowWidth();
                    
                    if (is_empty)
                    {
                        float btn_w = ImGui::CalcTextSize("Add Input").x + ImGui::GetStyle().FramePadding.x * 2.0f;
                        ImGui::SetCursorPosX((avail_w - btn_w) * 0.5f);
                        ImGui::Button("Add Input");
                    }
                    else
                    {
                        float current_y = ImGui::GetCursorPosY();
                        float start_x = ImGui::GetStyle().WindowPadding.x;

                        float add_in_w = ImGui::CalcTextSize("Add Input").x + ImGui::GetStyle().FramePadding.x * 2.0f;
                        float in_x = start_x + port_w * 0.5f - add_in_w * 0.5f;
                        ImGui::SetCursorPos(ImVec2(in_x, current_y));
                        ImGui::Button("Add Input");

                        float add_proc_w = ImGui::CalcTextSize("Add Processor").x + ImGui::GetStyle().FramePadding.x * 2.0f;
                        float center_x = (avail_w - add_proc_w) * 0.5f;
                        ImGui::SetCursorPos(ImVec2(center_x, current_y));
                        ImGui::Button("Add Processor");

                        if (!conn->inputs.empty())
                        {
                            float add_out_w = ImGui::CalcTextSize("Add Output").x + ImGui::GetStyle().FramePadding.x * 2.0f;
                            float out_x = start_x + avail_x - port_w * 0.5f - add_out_w * 0.5f;
                            ImGui::SetCursorPos(ImVec2(out_x, current_y));
                            ImGui::Button("Add Output");
                        }
                    }
                }
                ImGui::EndChild();
                ImGui::PopStyleColor();
                ImGui::PopStyleVar(2);
                ImGui::PopID();
                
                ImGui::Spacing();
            }
        }
        ImGui::EndChild();

        if (!commander_active) ImGui::BeginDisabled();
        if (ImGui::Button(get_gui_string(gui_string_id::btn_create_connection, lang), ImVec2(-1.0f, ImGui::GetFrameHeight() * 1.5f)))
            ImGui::OpenPopup(get_gui_string(gui_string_id::dlg_create_connection, lang));
        if (!commander_active) ImGui::EndDisabled();
    }
}