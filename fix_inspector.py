import sys
import re

file_path = r'c:\Users\PC\Documents\git\dexus1337\adam\applications\adam-gui\source\main-window\tabs\management\inspector.cpp'
with open(file_path, 'r', encoding='utf-8') as f:
    content = f.read()

start_marker = 'ImGui::BeginChild("##outer_child", ImVec2(0, current_height), true);'
end_marker = 'char clear_btn_text[512];'

start_idx = content.find(start_marker)
end_idx = content.find(end_marker)

if start_idx == -1 or end_idx == -1:
    print('Markers not found!')
    sys.exit(1)

new_code = '''ImGui::BeginChild("##outer_child", ImVec2(0, current_height), true);

        bool has_analyzer = !port_data.analyzer_columns.empty();

        if (has_analyzer)
        {
            bool is_expandable = port_data.analyzer_ptr && port_data.analyzer_ptr->is_row_expandable();
            int num_cols = (int)port_data.analyzer_columns.size() + (is_expandable ? 1 : 0);

            auto setup_analyzer_columns = [&]()
            {
                if (is_expandable) {
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoClip, ImGui::GetTextLineHeight() + ImGui::GetStyle().FramePadding.x * 2.0f);
                }
                for (const auto& col_name : port_data.analyzer_columns)
                {
                    ImGui::TableSetupColumn(col_name.c_str());
                }
            };

            bool active_user_scrolling = ImGui::GetIO().MouseWheel != 0.0f || 
                                         (ImGui::IsMouseDown(ImGuiMouseButton_Left) && 
                                          (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) || 
                                           ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)));

            if (active_user_scrolling && ImGui::GetScrollMaxY() > 0.0f && ImGui::GetScrollY() < ImGui::GetScrollMaxY() - 5.0f * dpi_scale)
            {
                port_data.was_at_bottom = false;
            }
            bool auto_scroll = port_data.was_at_bottom;

            float inner_scroll_h = -(ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y);
            ImGuiTableFlags flags = ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_ScrollY;
            
            if (ImGui::BeginTable("InspectorAnalyzerTable", num_cols, flags, ImVec2(0, inner_scroll_h)))
            {
                setup_analyzer_columns();
                ImGui::TableHeadersRow();

                const auto& flat_rows = port_data.parsed_flat_rows;

                float row_height = ImGui::GetTextLineHeight() + ImGui::GetStyle().CellPadding.y * 2.0f;
                ImGuiListClipper clipper;
                
                std::erase_if(port_data.expanded_nodes, [&](size_t idx) { return idx >= flat_rows.size(); });

                clipper.Begin(static_cast<int>(flat_rows.size()), row_height);
                while (clipper.Step())
                {
                    for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
                    {
                        size_t b_idx = flat_rows[i].first;
                        size_t r_idx = flat_rows[i].second;
                        
                        ImGui::TableNextRow();
                        
                        int current_col = 0;

                        bool is_open = false;
                        if (is_expandable)
                        {
                            ImGui::TableSetColumnIndex(0);
                            ImGui::PushID(i);
                            is_open = ImGui::TreeNodeEx("##row_node", ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_OpenOnArrow | (port_data.expanded_nodes.count(i) ? ImGuiTreeNodeFlags_DefaultOpen : 0));
                            if (is_open)
                            {
                                if (!port_data.expanded_nodes.count(i)) port_data.expanded_nodes.insert(i);
                            }
                            else
                            {
                                if (is_open) port_data.expanded_nodes.erase(i);
                            }
                            ImGui::PopID();
                            current_col = 1;
                        }

                        if (r_idx == SIZE_MAX)
                        {
                            ImGui::TableSetColumnIndex(current_col);
                            char unparsed_buf[128];
                            snprintf(unparsed_buf, sizeof(unparsed_buf), "%s (Size: %zu)", get_gui_string(gui_string_id::lbl_no_data, lang), (size_t)port_data.buffers[b_idx].size);
                            ImGui::TextDisabled("%s", unparsed_buf);
                        }
                        else
                        {
                            const auto& row = port_data.parsed_data[b_idx][r_idx];
                            size_t text_idx = 0;
                            for (size_t c = 0; c < port_data.analyzer_columns.size(); ++c)
                            {
                                ImGui::TableSetColumnIndex(current_col + (int)c);
                                adam::analyzer::column_type c_type = adam::analyzer::column_text;
                                if (c < port_data.analyzer_column_types.size())
                                {
                                    c_type = port_data.analyzer_column_types[c];
                                }
                                
                                if (c_type == adam::analyzer::column_frame_id)
                                {
                                    ImGui::Text("%zu", b_idx);
                                }
                                else if (c_type == adam::analyzer::column_timestamp)
                                {
                                    ImGui::TextUnformatted(adam::get_log_time_string(port_data.buffers[b_idx].timestamp).c_str());
                                }
                                else
                                {
                                    if (text_idx < row.columns.size())
                                    {
                                        ImGui::TextUnformatted(row.columns[text_idx].c_str());
                                    }
                                    text_idx++;
                                }
                            }
                        }

                        if (is_expandable && port_data.expanded_nodes.count(i))
                        {
                            const auto& row_obj = port_data.parsed_data[b_idx][r_idx];
                            
                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);

                            ImGui::PushID(i);
                            float expand_indent = ImGui::GetTextLineHeight() + ImGui::GetStyle().CellPadding.x * 2.0f;
                            float sub_table_w = ImGui::GetWindowContentRegionMax().x - ImGui::GetCursorPos().x - ImGui::GetStyle().WindowPadding.x;
                            ImGui::Indent(expand_indent);

                            if (!row_obj.expansions.empty())
                            {
                                for (size_t exp_idx = 0; exp_idx < row_obj.expansions.size(); ++exp_idx)
                                {
                                    const auto& exp = row_obj.expansions[exp_idx];
                                    ImGui::PushID((int)exp_idx);
                                    if (exp.data_type == adam::analyzer::expanded_data::type_text)
                                    {
                                        ImGui::TextWrapped("%s", exp.text_content.c_str());
                                    }
                                    else if (exp.data_type == adam::analyzer::expanded_data::type_table)
                                    {
                                        const auto& ext_cols = port_data.analyzer_ptr->get_expandable_columns();
                                        if (ImGui::BeginTable("##sub_table", (int)ext_cols.size(), ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchSame, ImVec2(sub_table_w, 0)))
                                        {
                                            for (const auto& h : ext_cols) ImGui::TableSetupColumn(h.c_str());
                                            ImGui::TableHeadersRow();
                                            
                                            ImGuiListClipper sub_clipper;
                                            sub_clipper.Begin(static_cast<int>(exp.table_rows.size()), row_height);
                                            while (sub_clipper.Step())
                                            {
                                                for (int r = sub_clipper.DisplayStart; r < sub_clipper.DisplayEnd; ++r)
                                                {
                                                    const auto& row_v = exp.table_rows[r];
                                                    ImGui::TableNextRow();
                                                    for (size_t c = 0; c < row_v.columns.size() && c < ext_cols.size(); ++c)
                                                    {
                                                        ImGui::TableSetColumnIndex((int)c);
                                                        ImGui::TextUnformatted(row_v.columns[c].c_str());
                                                    }
                                                }
                                            }
                                            ImGui::EndTable();
                                        }
                                    }
                                    ImGui::PopID();
                                }
                            }
                            else
                            {
                                ImGui::TextDisabled("No expanded data available.");
                            }
                            ImGui::Unindent(expand_indent);
                            ImGui::PopID();
                        }
                    }
                }

                if (auto_scroll && ImGui::GetScrollMaxY() > 0.0f)
                {
                    ImGui::SetScrollHereY(1.0f);
                }

                if (!auto_scroll)
                {
                    port_data.was_at_bottom = (ImGui::GetScrollMaxY() == 0.0f || ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 5.0f * dpi_scale);
                }

                ImGui::EndTable();
            }
        }
        else
        {
            auto setup_inner_columns = [&]() 
            {
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoClip, ImGui::GetTextLineHeight() + ImGui::GetStyle().FramePadding.x * 2.0f);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_index, lang), ImGuiTableColumnFlags_WidthFixed, 55.0f * dpi_scale);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_timestamp, lang), ImGuiTableColumnFlags_WidthFixed, 120.0f * dpi_scale);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_size, lang), ImGuiTableColumnFlags_WidthFixed, 75.0f * dpi_scale);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_preview_hex, lang), ImGuiTableColumnFlags_WidthStretch, 0.75f);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_preview_ascii, lang), ImGuiTableColumnFlags_WidthStretch, 0.25f);
            };

            bool active_user_scrolling = ImGui::GetIO().MouseWheel != 0.0f || 
                                         (ImGui::IsMouseDown(ImGuiMouseButton_Left) && 
                                          (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) || 
                                           ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)));

            if (active_user_scrolling && ImGui::GetScrollMaxY() > 0.0f && ImGui::GetScrollY() < ImGui::GetScrollMaxY() - 5.0f * dpi_scale)
            {
                port_data.was_at_bottom = false;
            }
            bool auto_scroll = port_data.was_at_bottom;

            float inner_scroll_h = -(ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y);
            ImGuiTableFlags flags = ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_ScrollY;
            
            if (ImGui::BeginTable("InspectorTable", 6, flags, ImVec2(0, inner_scroll_h)))
            {
                setup_inner_columns();
                ImGui::TableHeadersRow();

                float row_height = ImGui::GetTextLineHeight() + ImGui::GetStyle().CellPadding.y * 2.0f;

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
                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);
                            
                            ImGui::PushID(row_idx);
                            draw_inspector_hex_dump(data_pool.data() + ib.offset, ib.size, row_idx, current_height, dpi_scale, lang);
                            ImGui::PopID();
                        }
                    }
                }

                if (auto_scroll && ImGui::GetScrollMaxY() > 0.0f)
                {
                    ImGui::SetScrollHereY(1.0f);
                }

                if (!auto_scroll)
                {
                    port_data.was_at_bottom = (ImGui::GetScrollMaxY() == 0.0f || ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 5.0f * dpi_scale);
                }

                ImGui::EndTable();
            }
        }

        ImGui::EndChild();

        '''

with open(file_path, 'w', encoding='utf-8') as f:
    f.write(content[:start_idx] + new_code + content[end_idx:])

print('Success')
