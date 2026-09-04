/**
 * @file    window-data-sources.cpp
 * @author  dexus1337
 * @brief   Implementation of the data sources / ASTERIX streams window for adam-cop.
 * @version 1.0
 * @date    20.08.2026
 */

#include "window-data-sources.hpp"
#include "../cop-controller.hpp"
#include "../cop-strings.hpp"
#include "../data/radar-stream.hpp"
#include "main-window.hpp"
#include <imgui.h>
#include <lib-imgui.hpp>
#include <renderer-setup.hpp>
#include <vector>
#include <string>

namespace adam::cop
{
    struct radar_stream_item
    {
        adam::string_hash   conn_hash;
        adam::string_hashed conn_name;
        bool                is_input;
        adam::string_hashed format_name;
        bool                started;
    };

    /**
     * @brief Collects all registered input and output connections running ASTERIX protocol.
     */
    static std::vector<radar_stream_item> collect_asterix_streams(cop_controller& ctrl)
    {
        std::vector<radar_stream_item> streams;
        std::lock_guard<const adam::registry_view> reg_lock(ctrl.commander().registry());

        for (const auto& [conn_hash, conn] : ctrl.commander().registry().get_connections())
        {
            if (!conn)
            {
                continue;
            }

            if (conn->input_format == "asterix"_ct)
            {
                streams.push_back({ conn_hash, conn->name, true, conn->input_format, conn->started });
            }

            if (conn->output_format == "asterix"_ct)
            {
                streams.push_back({ conn_hash, conn->name, false, conn->output_format, conn->started });
            }
        }
        return streams;
    }

    void draw_data_sources_window(cop_controller& ctrl, adam::language lang, adam::configuration_parameter_boolean* p_show_data_sources)
    {
        if (!p_show_data_sources || !p_show_data_sources->get_value())
        {
            return;
        }

        std::string title_asterix = std::string(get_cop_string(wnd_data_sources, lang)) + "###Asterix";
        if (!ImGui::Begin(title_asterix.c_str(), &p_show_data_sources->value()))
        {
            ImGui::End();
            return;
        }

        std::vector<radar_stream_item> streams = collect_asterix_streams(ctrl);

        if (streams.empty())
        {
            ImGui::TextDisabled("%s", get_cop_string(lbl_no_asterix_streams, lang));
            ImGui::Spacing();
            ImGui::TextWrapped("Configure ASTERIX connections in ADAM and start them to stream radar telemetry into the COP.");
            ImGui::End();
            return;
        }

        size_t active_stream_count = 0;
        for (const auto& s : streams)
        {
            if (ctrl.is_radar_stream_enabled(s.conn_hash, s.is_input))
            {
                active_stream_count++;
            }
        }

        if (ImGui::Button(get_cop_string(btn_enable_all, lang)))
        {
            ctrl.enable_all_radar_streams(true);
        }
        ImGui::SameLine();
        if (ImGui::Button(get_cop_string(btn_disable_all, lang)))
        {
            ctrl.enable_all_radar_streams(false);
        }
        ImGui::SameLine();
        if (ImGui::Button(get_cop_string(btn_clear_stats, lang)))
        {
            ctrl.clear_radar_stream_stats();
        }
        ImGui::SameLine();
        ImGui::TextDisabled(get_cop_string(lbl_active_streams, lang), active_stream_count, streams.size());

        ImGui::Separator();

        float dpi_scale = adam::lib::imgui::get_current_dpi_scale();
        if (dpi_scale <= 0.0f)
        {
            dpi_scale = 1.0f;
        }

        bool table_open = ImGui::BeginTable("RadarDataSourcesTable", 5, 
            ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY);
        
        if (table_open)
        {
            auto fixed_single_size = ImGui::GetTextLineHeight() + ImGui::GetStyle().CellPadding.x * 2.0f;
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, ImGui::GetTextLineHeight() + ImGui::GetStyle().CellPadding.x);
            ImGui::TableSetupColumn(get_cop_string(col_on, lang),            ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, fixed_single_size);
            ImGui::TableSetupColumn(get_cop_string(col_endpoint, lang),      ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn(get_cop_string(col_messages, lang),      ImGuiTableColumnFlags_WidthFixed, 100.0f * dpi_scale);
            ImGui::TableSetupColumn(get_cop_string(col_size, lang),          ImGuiTableColumnFlags_WidthFixed, 100.0f * dpi_scale);

            ImGui::TableHeadersRow();

            for (const auto& s : streams)
            {
                ImGui::TableNextRow();

                // Status Dot
                ImGui::TableSetColumnIndex(0);
                ImColor pin_col = get_cop_color(cop_color_id::node_connection_line);
                if (s.started)
                {
                    pin_col = get_cop_color(cop_color_id::node_pin_active);
                }
                ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
                float dot_radius = ImGui::GetTextLineHeight() / 2.0f - ImGui::GetStyle().CellPadding.y;
                ImGui::GetWindowDrawList()->AddCircleFilled(
                    ImVec2(cursor_pos.x + dot_radius + ImGui::GetStyle().CellPadding.x, cursor_pos.y + ImGui::GetFrameHeight() * 0.5f), 
                    dot_radius, pin_col);

                // Stream / Inspect Checkbox
                ImGui::TableSetColumnIndex(1);
                bool is_enabled = ctrl.is_radar_stream_enabled(s.conn_hash, s.is_input);
                ImGui::PushID(reinterpret_cast<const void*>(static_cast<intptr_t>(s.conn_hash ^ (s.is_input ? 0x4321 : 0x8765))));
                if (ImGui::Checkbox("##stream_enable", &is_enabled))
                {
                    ctrl.set_radar_stream_enabled(s.conn_hash, s.is_input, is_enabled);
                }
                ImGui::PopID();

                // Name & Endpoint Label
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%s %s", s.conn_name.c_str(), s.is_input ? get_cop_string(lbl_input_endpoint, lang) : get_cop_string(lbl_output_endpoint, lang));

                // Stats: Messages, Size
                radar_stream_stats stats;
                bool has_stats = ctrl.get_radar_stream_stats(s.conn_hash, s.is_input, stats);
                if (has_stats && stats.msg_count > 0)
                {
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("%llu", static_cast<unsigned long long>(stats.msg_count));

                    ImGui::TableSetColumnIndex(4);
                    char size_buf[64] = "";
                    format_stream_bytes_to_buf(stats.total_bytes, size_buf, sizeof(size_buf));
                    ImGui::TextUnformatted(size_buf);
                }
                else
                {
                    ImGui::TableSetColumnIndex(3);
                    ImGui::TextDisabled("0");

                    ImGui::TableSetColumnIndex(4);
                    ImGui::TextDisabled("0 B");
                }
            }

            ImGui::EndTable();
        }

        ImGui::End();
    }
}
