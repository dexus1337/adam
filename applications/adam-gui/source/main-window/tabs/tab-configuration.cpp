/**
 * @file    tab-configuration.cpp
 * @author  Antigravity
 * @brief   Implementation of the configuration tab drawing functions.
 */

#include "tab-configuration.hpp"
#include "../main-window.hpp"

#include <imgui.h>
#include <unordered_map>
#include <vector>
#include <cstring>
#include <algorithm>
#include <mutex>
#include <ctime>

namespace adam::gui 
{
    void draw_tab_configuration(gui_controller& ctrl, adam::language lang)
    {
        bool commander_active = ctrl.is_commander_active();
        
        static bool open_save_popup                         = false;
        static char save_filename[adam::max_name_length]    = "adam-config.bin";
        static char save_name[adam::max_name_length]        = "default";
        static char save_desc[adam::max_description_length] = "";
        static uint32_t save_path_idx                       = 0;
        static char export_popup_filename[adam::max_name_length] = "adam-config.bin";

        float dpi_scale = ImGui::GetStyle()._MainScale;

        static bool initial_sync = true;
        if (initial_sync && commander_active)
        {
            std::strncpy(save_name, ctrl.commander().get_config_name().c_str(), sizeof(save_name) - 1);
            save_name[sizeof(save_name) - 1] = '\0';
            std::strncpy(save_desc, ctrl.commander().get_config_description().c_str(), sizeof(save_desc) - 1);
            save_desc[sizeof(save_desc) - 1] = '\0';
            initial_sync = false;
        }

        float content_w = ImGui::GetContentRegionAvail().x;
        float panel_height = ImGui::GetContentRegionAvail().y * 0.35f;
        if (panel_height < 220.0f * dpi_scale) panel_height = 220.0f * dpi_scale;
        
        float half_w = (content_w - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
        float btn_h = ImGui::GetFrameHeight() * 1.5f;

        // --- LEFT PANEL (Paths) ---
        ImGui::BeginChild("##LeftPathPanel", ImVec2(half_w, panel_height), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        
        ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_configuration_paths, lang));
        ImGui::Separator();
        
        float btn_add_width = std::max(ImGui::CalcTextSize(get_gui_string(gui_string_id::btn_add_path, adam::language_english)).x,
                                       ImGui::CalcTextSize(get_gui_string(gui_string_id::btn_add_path, adam::language_german)).x) + ImGui::GetStyle().FramePadding.x * 2.0f;

        if (!commander_active) ImGui::BeginDisabled();
        static char new_path[384] = "";
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - btn_add_width - ImGui::GetStyle().ItemSpacing.x);
        ImGui::InputTextWithHint("##NewConfigPath", get_gui_string(gui_string_id::ph_new_path, lang), new_path, sizeof(new_path));
        ImGui::SameLine();
        
        bool has_input = (new_path[0] != '\0');
        if (!has_input) ImGui::BeginDisabled();
        if (ImGui::Button(get_gui_string(gui_string_id::btn_add_path, lang), ImVec2(btn_add_width, 0)))
        {
            if (commander_active)
            {
                ctrl.enqueue_commander_action([&ctrl, path_str = std::string(new_path)]() 
                {
                    ctrl.commander().request_config_path_add(adam::string_hashed(path_str.c_str()));
                });
                new_path[0] = '\0';
            }
        }
        if (!has_input) ImGui::EndDisabled();
        
        if (!commander_active) ImGui::EndDisabled();
        
        ImGui::Spacing();
        
        // Calculate remaining table height to prevent overlap with scan button
        float table_h = panel_height - ImGui::GetCursorPosY() - btn_h - ImGui::GetStyle().ItemSpacing.y - ImGui::GetStyle().WindowPadding.y * 2.0f;
        if (table_h < 60.0f * dpi_scale) table_h = 60.0f * dpi_scale;
        
        if (ImGui::BeginTable("ConfigPathsTable", 3, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY, ImVec2(0, table_h)))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_index, lang), ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize("99").x);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_path, lang), ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("##Actions", ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize(get_gui_string(gui_string_id::btn_remove_path, lang)).x + ImGui::GetStyle().FramePadding.x * 2.0f);
            ImGui::TableHeadersRow();

            if (commander_active)
            {
                std::lock_guard<const adam::config_view> lg(ctrl.commander().configs());

                const auto& paths = ctrl.get_commander().get_configs().get_paths();
                for (size_t i = 0; i < paths.size(); ++i)
                {
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("%zu", i);

                    ImGui::TableSetColumnIndex(1);
                    ImGui::AlignTextToFramePadding();
                    ImGui::TextUnformatted(paths[i].c_str());

                    ImGui::TableSetColumnIndex(2);
                    ImGui::PushID(static_cast<int>(i));
                    if (i == 0) ImGui::BeginDisabled();
                    if (ImGui::Button(get_gui_string(gui_string_id::btn_remove_path, lang), ImVec2(-1.0f, 0.0f)))
                    {
                        ctrl.enqueue_commander_action([&ctrl, idx = static_cast<uint32_t>(i)]() 
                        {
                            ctrl.commander().request_config_path_remove(idx);
                        });
                    }
                    if (i == 0) ImGui::EndDisabled();
                    ImGui::PopID();
                }
            }
            ImGui::EndTable();
        }
        
        // Scan button at the bottom of the left child window
        ImGui::SetCursorPosY(panel_height - btn_h - ImGui::GetStyle().WindowPadding.y);
        
        if (!commander_active) ImGui::BeginDisabled();
        if (ImGui::Button(get_gui_string(gui_string_id::btn_scan_configs, lang), ImVec2(-1.0f, btn_h)))
        {
            ctrl.enqueue_commander_action([&ctrl]() 
            {
                ctrl.commander().request_config_scan();
            });
        }
        if (!commander_active) ImGui::EndDisabled();
        
        ImGui::EndChild();
        
        ImGui::SameLine();
        
        // --- RIGHT PANEL (Export Metadata) ---
        ImGui::BeginChild("##RightMetadataPanel", ImVec2(half_w, panel_height), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        
        ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_config_settings, lang));
        ImGui::Separator();
        
        if (!commander_active) ImGui::BeginDisabled();
        
        float label_w = ImGui::CalcTextSize(get_gui_string(gui_string_id::lbl_config_description, lang)).x + 10.0f * dpi_scale;
        
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_config_name, lang));
        ImGui::SameLine(label_w);
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::InputText("##save_name", save_name, sizeof(save_name));

        ImGui::Spacing();

        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_config_description, lang));
        ImGui::SameLine(label_w);
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::InputText("##save_desc", save_desc, sizeof(save_desc));
        
        if (!commander_active) ImGui::EndDisabled();
        
        // Save and Export buttons at the bottom of the right child window
        ImGui::SetCursorPosY(panel_height - btn_h - ImGui::GetStyle().WindowPadding.y);
        
        if (!commander_active) ImGui::BeginDisabled();
        
        float right_btn_w = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
        
        if (ImGui::Button(get_gui_string(gui_string_id::btn_save, lang), ImVec2(right_btn_w, btn_h)))
        {
            ctrl.enqueue_commander_action([&ctrl, name = std::string(save_name), desc = std::string(save_desc)]() 
            {
                ctrl.commander().request_config_save(name, desc);
            });
        }
        ImGui::SameLine();

        if (ImGui::Button(get_gui_string(gui_string_id::btn_export, lang), ImVec2(-1.0f, btn_h)))
        {
            std::strncpy(export_popup_filename, save_filename, sizeof(export_popup_filename));
            export_popup_filename[sizeof(export_popup_filename) - 1] = '\0';
            open_save_popup = true;
        }
        
        if (!commander_active) ImGui::EndDisabled();
        
        ImGui::EndChild();

        ImGui::Spacing();
        ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_available_configurations, lang));
        ImGui::Spacing();
        
        bool do_load = false;
        uint32_t load_path_idx = 0;
        std::string load_filename;
        std::string export_filename;
        std::string export_name;
        std::string export_description;

        if (ImGui::BeginTable("ConfigsTable", 8, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY, ImVec2(0, 0)))
        {
            float width_name     = 150.0f * dpi_scale;
            float width_filename = 300.0f * dpi_scale;
            float width_created  = 130.0f * dpi_scale;
            float width_modified = 130.0f * dpi_scale;
            float width_counts   = 240.0f * dpi_scale;
            float width_import   = 100.0f * dpi_scale;
            float width_export   = 100.0f * dpi_scale;

            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_config_name, lang), ImGuiTableColumnFlags_WidthFixed, width_name);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_filename, lang), ImGuiTableColumnFlags_WidthFixed, width_filename);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_description, lang), ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_created, lang), ImGuiTableColumnFlags_WidthFixed, width_created);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_modified, lang), ImGuiTableColumnFlags_WidthFixed, width_modified);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_counts, lang), ImGuiTableColumnFlags_WidthFixed, width_counts);
            ImGui::TableSetupColumn("##ImportColumn", ImGuiTableColumnFlags_WidthFixed, width_import);
            ImGui::TableSetupColumn("##ExportColumn", ImGuiTableColumnFlags_WidthFixed, width_export);
            ImGui::TableHeadersRow();

            if (commander_active)
            {
                std::lock_guard<const adam::config_view> lg(ctrl.commander().configs());

                const auto& configs = ctrl.get_commander().get_configs().get_available();
                int idx = 0;
                for (const auto& [filepath, cfg] : configs)
                {
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    ImGui::AlignTextToFramePadding();
                    ImGui::TextUnformatted(cfg.name.c_str());

                    ImGui::TableSetColumnIndex(1);
                    ImGui::AlignTextToFramePadding();
                    // The map key 'filepath' is already the full path+filename built in request_initial_data
                    ImGui::TextUnformatted(filepath.c_str());

                    ImGui::TableSetColumnIndex(2);
                    ImGui::AlignTextToFramePadding();
                    ImGui::TextUnformatted(cfg.description.c_str());

                    auto format_time = [](uint64_t ts) -> std::string 
                    {
                        if (ts == 0) return "-";
                        std::time_t t = static_cast<std::time_t>(ts);
                        std::tm tm_val;
                        #ifdef ADAM_PLATFORM_WINDOWS
                        localtime_s(&tm_val, &t);
                        #else
                        localtime_r(&t, &tm_val);
                        #endif
                        char buf[64];
                        std::strftime(buf, sizeof(buf), "%d.%m.%Y %H:%M", &tm_val);
                        return buf;
                    };

                    ImGui::TableSetColumnIndex(3);
                    ImGui::AlignTextToFramePadding();
                    ImGui::TextUnformatted(format_time(cfg.created).c_str());

                    ImGui::TableSetColumnIndex(4);
                    ImGui::AlignTextToFramePadding();
                    ImGui::TextUnformatted(format_time(cfg.modified).c_str());

                    ImGui::TableSetColumnIndex(5);
                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("%u / %u / %u", cfg.port_count, cfg.processor_count, cfg.connection_count);

                    ImGui::PushID(idx++);

                    ImGui::TableSetColumnIndex(6);
                    if (ImGui::Button(get_gui_string(gui_string_id::btn_load_config, lang), ImVec2(-1.0f, 0.0f)))
                    {
                        do_load = true;
                        load_path_idx = cfg.path_idx;
                        load_filename = cfg.filename;

                        save_path_idx = cfg.path_idx;
                        std::strncpy(save_filename, cfg.filename.c_str(), sizeof(save_filename));
                        save_filename[sizeof(save_filename) - 1] = '\0';
                        std::strncpy(save_name, cfg.name.c_str(), sizeof(save_name));
                        save_name[sizeof(save_name) - 1] = '\0';
                        std::strncpy(save_desc, cfg.description.c_str(), sizeof(save_desc));
                        save_desc[sizeof(save_desc) - 1] = '\0';
                    }
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("%s", get_gui_string(gui_string_id::btn_load_config, lang));

                    ImGui::TableSetColumnIndex(7);
                    if (ImGui::Button(get_gui_string(gui_string_id::btn_export_config, lang), ImVec2(-1.0f, 0.0f)))
                    {
                        std::strncpy(save_filename, cfg.filename.c_str(), sizeof(save_filename));
                        save_filename[sizeof(save_filename) - 1] = '\0';
                        std::strncpy(save_name, cfg.name.c_str(), sizeof(save_name));
                        save_name[sizeof(save_name) - 1] = '\0';
                        std::strncpy(save_desc, cfg.description.c_str(), sizeof(save_desc));
                        save_desc[sizeof(save_desc) - 1] = '\0';
                        save_path_idx = cfg.path_idx;
                        open_save_popup = true;
                    }
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("%s", filepath.c_str());

                    ImGui::PopID();
                }
            }
            ImGui::EndTable();
        }

        if (do_load)
        {
            ctrl.enqueue_commander_action([&ctrl, load_path_idx, load_filename]() 
            {
                ctrl.commander().request_config_import(load_path_idx, adam::string_hashed(load_filename.c_str()));
            });
        }

        // Save Config Popup Dialog
        if (open_save_popup)
        {
            ImGui::OpenPopup(get_gui_string(gui_string_id::dlg_save_config, lang));
            open_save_popup = false;
        }

        if (ImGui::BeginPopupModal(get_gui_string(gui_string_id::dlg_save_config, lang), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            // Lock and read available configuration paths to show in the dropdown combo box
            int selected_path_idx = static_cast<int>(save_path_idx);
            {
                std::lock_guard<const adam::config_view> lg(ctrl.commander().configs());
                const auto& paths = ctrl.get_commander().get_configs().get_paths();
                
                std::string combo_preview = "Invalid Path Index";
                if (selected_path_idx >= 0 && selected_path_idx < static_cast<int>(paths.size()))
                {
                    combo_preview = paths[selected_path_idx].c_str();
                }
                
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted("Path");
                ImGui::SameLine(ImGui::GetFontSize() * 7.0f);
                ImGui::SetNextItemWidth(250.0f * dpi_scale);
                if (ImGui::BeginCombo("##PathCombo", combo_preview.c_str()))
                {
                    for (int i = 0; i < static_cast<int>(paths.size()); ++i)
                    {
                        const bool is_selected = (selected_path_idx == i);
                        if (ImGui::Selectable(paths[i].c_str(), is_selected))
                        {
                            save_path_idx = static_cast<uint32_t>(i);
                        }
                        if (is_selected)
                        {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }
            }

            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Filename");
            ImGui::SameLine(ImGui::GetFontSize() * 7.0f);
            ImGui::SetNextItemWidth(250.0f * dpi_scale);
            ImGui::InputText("##ExportFilename", export_popup_filename, sizeof(export_popup_filename));

            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_config_name, lang));
            ImGui::SameLine(ImGui::GetFontSize() * 7.0f);
            ImGui::SetNextItemWidth(250.0f * dpi_scale);
            ImGui::InputText("##ExportName", save_name, sizeof(save_name));

            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_config_description, lang));
            ImGui::SameLine(ImGui::GetFontSize() * 7.0f);
            ImGui::SetNextItemWidth(250.0f * dpi_scale);
            ImGui::InputText("##ExportDesc", save_desc, sizeof(save_desc));

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("OK", ImVec2(120, 0)))
            {
                ctrl.enqueue_commander_action([&ctrl, path_idx = save_path_idx, filename = std::string(export_popup_filename), name = std::string(save_name), desc = std::string(save_desc)]() 
                {
                    ctrl.commander().request_config_export(path_idx, adam::string_hashed(filename.c_str()), name, desc);
                });
                ImGui::CloseCurrentPopup();
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
}
