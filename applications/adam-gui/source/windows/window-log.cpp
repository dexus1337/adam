/**
 * @file    window-log.cpp
 * @author  dexus1337
 * @brief   Implementation of the log window drawing functions.
 * @version 1.0
 * @date    12.06.2026
 */

#include "window-log.hpp"
#include "main-window.hpp"

#include <imgui.h>
#include <algorithm>
#include <vector>
#include <string>

#include <cctype>

namespace adam::gui 
{
    void draw_window_log(gui_controller& ctrl, adam::language lang, int log_table_id)
    {
        auto* p_show_log = static_cast<adam::configuration_parameter_boolean*>(ctrl.get_parameters().get("show_log"_ct));
        auto* p_log_level = static_cast<adam::configuration_parameter_integer*>(ctrl.get_parameters().get("log_level"_ct));

        if (!ImGui::Begin(get_gui_string(gui_string_id::wnd_log_console, lang), &p_show_log->value()))
        {
            ImGui::End();
            return;
        }

        float dpi_scale = ImGui::GetStyle()._MainScale;

        // --- Sort Combo (Left) ---
        static adam::configuration_parameter_integer* sort_mode_param = dynamic_cast<adam::configuration_parameter_integer*>(ctrl.get_parameters().get("log_sort_mode"_ct));
        int sort_mode = static_cast<int>(sort_mode_param->get_value());

        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_sort_by, lang));
        ImGui::SameLine();
        const char* sort_options[] = 
        {
            get_gui_string(gui_string_id::sort_log_time_asc, lang),
            get_gui_string(gui_string_id::sort_log_time_desc, lang),
            get_gui_string(gui_string_id::sort_log_severity_asc, lang),
            get_gui_string(gui_string_id::sort_log_severity_desc, lang)
        };
        ImGui::SetNextItemWidth(200.0f * dpi_scale);
        if (ImGui::Combo("##LogSortMode", &sort_mode, sort_options, IM_ARRAYSIZE(sort_options)))
        {
            if (sort_mode_param)
                sort_mode_param->set_value(static_cast<int64_t>(sort_mode));
        }

        // --- Right Controls (Log Level & Clear) ---
        float max_combo_text_width = std::max(ImGui::CalcTextSize("Warning").x, ImGui::CalcTextSize("Warnung").x);
        float combo_width = max_combo_text_width + ImGui::GetFrameHeight() + ImGui::GetStyle().FramePadding.x * 2.0f;
        const char* clear_log_text = get_gui_string(gui_string_id::btn_clear_log, lang);
        float btn_width = ImGui::CalcTextSize(clear_log_text, NULL, true).x + ImGui::GetStyle().FramePadding.x * 2.0f;
        float right_align_offset = combo_width + btn_width + ImGui::GetStyle().ItemSpacing.x;

        // --- Search Bar (Center) ---
        static adam::configuration_parameter_string* search_param = dynamic_cast<adam::configuration_parameter_string*>(ctrl.get_parameters().get("log_search"_ct));
        static std::string search_str = search_param ? std::string(search_param->get_value().c_str()) : "";

        float center_pos = 0.0f;
        float search_width = 0.0f;
        get_search_bar_layout(lang, ImGui::GetContentRegionAvail().x, center_pos, search_width);
        
        // Ensure center_pos doesn't overlap left controls
        if (center_pos < ImGui::GetCursorPosX()) center_pos = ImGui::GetCursorPosX() + ImGui::GetStyle().ItemSpacing.x;
        
        // If center search overlaps right controls, we might need to adjust, but let's assume it fits
        float right_controls_x = ImGui::GetWindowContentRegionMax().x - right_align_offset;
        if (center_pos + search_width > right_controls_x - ImGui::GetStyle().ItemSpacing.x)
        {
            center_pos = ImGui::GetCursorPosX() + ImGui::GetStyle().ItemSpacing.x;
            search_width = right_controls_x - center_pos - ImGui::GetStyle().ItemSpacing.x;
        }

        ImGui::SameLine(center_pos);
        ImGui::SetNextItemWidth(search_width);
        char search_buf[256];
        strncpy(search_buf, search_str.c_str(), sizeof(search_buf) - 1);
        search_buf[sizeof(search_buf) - 1] = '\0';
        if (ImGui::InputTextWithHint("##LogSearch", get_gui_string(gui_string_id::search_hint, lang), search_buf, sizeof(search_buf)))
        {
            search_str = search_buf;
            if (search_param)
                search_param->set_value(adam::string_hashed(search_str));
        }

        ImGui::SameLine(right_controls_x);
        int current_log_level = static_cast<int>(p_log_level->get_value());
        ImGui::SetNextItemWidth(combo_width);
        if (ImGui::Combo("##LogLevel", &current_log_level, get_gui_string(gui_string_id::combo_log_level_options, lang)))
        {
            p_log_level->set_value(static_cast<int64_t>(current_log_level));
            if (ctrl.get_log_sink().is_active() && ctrl.log_sink().queue().metadata())
            {
                ctrl.log_sink().queue().metadata()->store(static_cast<adam::log::level>(current_log_level + 1), std::memory_order_relaxed);
            }
        }
        
        ImGui::SameLine();
        
        if (ImGui::Button(clear_log_text))
        {
            ctrl.clear_log_history();
        }
        
        ImGui::PushID(log_table_id);
        if (ImGui::BeginTable("LogTable", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0.0f, 0.0f)))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_time, lang), ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_level, lang), ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_message, lang), ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();

            auto log_history_raw = ctrl.get_log_history();
            
            std::string search_lower = search_str;
            std::transform(search_lower.begin(), search_lower.end(), search_lower.begin(), [](unsigned char c) { return std::tolower(c); });

            std::vector<log_entry> log_history;
            for (const auto& log_line : log_history_raw)
            {
                if (!search_lower.empty())
                {
                    std::string msg_lower = log_line.text.c_str();
                    std::transform(msg_lower.begin(), msg_lower.end(), msg_lower.begin(), [](unsigned char c) { return std::tolower(c); });
                    if (msg_lower.find(search_lower) == std::string::npos) continue;
                }
                log_history.push_back(log_line);
            }

            std::sort(log_history.begin(), log_history.end(), [sort_mode](const log_entry& a, const log_entry& b)
            {
                if (sort_mode == 0) // Time Asc
                    return a.timestamp < b.timestamp;
                else if (sort_mode == 1) // Time Desc
                    return a.timestamp > b.timestamp;
                else if (sort_mode == 2) // Severity Asc
                    return static_cast<int>(a.level) < static_cast<int>(b.level);
                else if (sort_mode == 3) // Severity Desc
                    return static_cast<int>(a.level) > static_cast<int>(b.level);
                return false;
            });

            int i = 0;
            for (const auto& log_line : log_history)
            {
                ImGui::TableNextRow();
                
                const char* level_text = "";
                float r, g, b;
                adam::get_log_appearance(log_line.level, level_text, r, g, b);
                
                ImVec4 color;
                switch (log_line.level)
                {
                    case adam::log::level::trace:   color = get_gui_color(gui_color_id::log_trace); break;
                    case adam::log::level::info:    color = get_gui_color(gui_color_id::log_info); break;
                    case adam::log::level::warning: color = get_gui_color(gui_color_id::log_warning); break;
                    case adam::log::level::error:   color = get_gui_color(gui_color_id::log_error); break;
                    default:                        color = ImVec4(r, g, b, 1.0f); break; // Fallback to SDK defaults
                }

                ImGui::TableSetColumnIndex(0);
                ImGui::PushID(i);
                std::string time_str = adam::get_log_time_string(log_line.timestamp);
                ImGui::Selectable(time_str.c_str(), false, ImGuiSelectableFlags_SpanAllColumns);
                
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    ImGui::SetClipboardText(log_line.text.c_str());
                }

                if (ImGui::BeginPopupContextItem())
                {
                    if (ImGui::MenuItem(get_gui_string(gui_string_id::btn_copy_message, lang)))
                    {
                        ImGui::SetClipboardText(log_line.text.c_str());
                    }
                    if (ImGui::MenuItem(get_gui_string(gui_string_id::btn_copy_row, lang)))
                    {
                        char row_str[1024];
                        snprintf(row_str, sizeof(row_str), "[%s] [%s] %s", 
                                 time_str.c_str(),
                                 level_text,
                                 log_line.text.c_str());
                        ImGui::SetClipboardText(row_str);
                    }
                    ImGui::EndPopup();
                }
                ImGui::PopID();

                ImGui::TableSetColumnIndex(1);
                ImGui::TextColored(color, "%s", level_text);

                ImGui::TableSetColumnIndex(2);
                ImGui::TextUnformatted(log_line.text.c_str());

                i++;
            }

            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f);
                
            ImGui::EndTable();
        }
        ImGui::PopID();

        ImGui::End();
    }
}
