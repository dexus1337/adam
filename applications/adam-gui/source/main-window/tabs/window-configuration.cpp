/**
 * @file    window-configuration.cpp
 * @author  dexus1337
 * @brief   Implementation of the configuration window drawing functions.
 * @version 1.0
 * @date    12.06.2026
 */

#include "window-configuration.hpp"
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
    void draw_window_configuration(gui_controller& ctrl, adam::language lang)
    {
        bool commander_active = ctrl.is_commander_active();
        
        static bool open_save_popup                         = false;
        static char save_filename[adam::max_name_length]    = "adam-config.adamcfg";
        static char save_name[adam::max_name_length]        = "default";
        static char save_desc[adam::max_description_length] = "";
        static uint32_t save_path_idx                       = 0;
        static char export_popup_filename[adam::max_name_length] = "adam-config.adamcfg";

        float dpi_scale = ImGui::GetStyle()._MainScale;

        static bool initial_sync = true;
        if (initial_sync && commander_active)
        {
            std::strncpy(save_name, ctrl.commander().configs().get_name().c_str(), sizeof(save_name) - 1);
            save_name[sizeof(save_name) - 1] = '\0';
            std::strncpy(save_desc, ctrl.commander().configs().get_description().c_str(), sizeof(save_desc) - 1);
            save_desc[sizeof(save_desc) - 1] = '\0';
            initial_sync = false;
        }

        float content_w = ImGui::GetContentRegionAvail().x;
        float panel_height = ImGui::GetContentRegionAvail().y * 0.333f;
        if (panel_height < 220.0f * dpi_scale) panel_height = 220.0f * dpi_scale;
        
        float separator_pad = ImGui::GetStyle().ItemSpacing.x;
        float half_w = (content_w - separator_pad * 2.0f - 1.0f) * 0.5f;
        float btn_h = ImGui::GetFrameHeight() * 1.5f;

        // --- LEFT PANEL (Export Metadata) ---
        ImGui::BeginChild("##LeftMetadataPanel", ImVec2(half_w, panel_height), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        
        ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_config_settings, lang));
        ImGui::Separator();
        
        if (!commander_active) ImGui::BeginDisabled();
        
        float label_w = ImGui::CalcTextSize(get_gui_string(gui_string_id::lbl_config_description, lang)).x + 10.0f * dpi_scale;
        
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_config_name, lang));
        ImGui::SameLine(label_w);
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::InputText("##ConfigName", save_name, sizeof(save_name));
        
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_config_description, lang));
        ImGui::SameLine(label_w);
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::InputText("##ConfigDesc", save_desc, sizeof(save_desc));

        ImGui::Spacing();
        
        if (ImGui::Button(get_gui_string(gui_string_id::btn_save, lang), ImVec2(-1.0f, btn_h)))
        {
            ctrl.enqueue_commander_action([&ctrl, name = std::string(save_name), desc = std::string(save_desc)]() 
            {
                ctrl.commander().request_config_save(name, desc);
            });
        }
        
        if (!commander_active) ImGui::EndDisabled();
        
        ImGui::EndChild();

        ImGui::SameLine();
        
        // Vertical Splitter Line
        ImVec2 p_sep = ImGui::GetCursorScreenPos();
        ImGui::GetWindowDrawList()->AddLine(ImVec2(p_sep.x + separator_pad, p_sep.y), ImVec2(p_sep.x + separator_pad, p_sep.y + panel_height), ImGui::GetColorU32(ImGuiCol_Separator));
        ImGui::Dummy(ImVec2(separator_pad * 2.0f, panel_height));
        
        ImGui::SameLine();

        // --- RIGHT PANEL (Paths & Actions) ---
        ImGui::BeginChild("##RightPathsPanel", ImVec2(0.0f, panel_height), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        
        ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_configuration_paths, lang));
        ImGui::Separator();
        
        static char new_config_path[adam::max_name_length] = "";
        
        if (!commander_active) ImGui::BeginDisabled();
        
        float btn_add_w = ImGui::CalcTextSize(get_gui_string(gui_string_id::btn_add_path, lang)).x + ImGui::GetStyle().FramePadding.x * 2.0f;
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - btn_add_w - ImGui::GetStyle().ItemSpacing.x);
        ImGui::InputTextWithHint("##NewConfigPath", get_gui_string(gui_string_id::ph_new_path, lang), new_config_path, sizeof(new_config_path));
        
        ImGui::SameLine();
        
        if (ImGui::Button(get_gui_string(gui_string_id::btn_add_path, lang)))
        {
            if (new_config_path[0] != '\0')
            {
                std::string path_str(new_config_path);
                ctrl.enqueue_commander_action([&ctrl, path_str]() 
                {
                    ctrl.commander().request_config_path_add(adam::string_hashed(path_str.c_str()));
                });
                new_config_path[0] = '\0';
            }
        }
        
        if (ImGui::BeginTable("ConfigPathsTable", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0.0f, panel_height - ImGui::GetCursorPosY() - btn_h - ImGui::GetStyle().ItemSpacing.y)))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_path, lang), ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("##Action", ImGuiTableColumnFlags_WidthFixed, 60.0f * dpi_scale);
            ImGui::TableHeadersRow();

            std::vector<adam::string_hashed> paths;
            if (commander_active)
            {
                std::lock_guard<const adam::config_view> lg(ctrl.commander().configs());
                paths = ctrl.get_commander().get_configs().get_paths();
            }

            for (size_t i = 0; i < paths.size(); ++i)
            {
                ImGui::TableNextRow();
                
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(paths[i].c_str());
                
                ImGui::TableSetColumnIndex(1);
                ImGui::PushID(static_cast<int>(i));
                if (ImGui::Button(get_gui_string(gui_string_id::btn_remove_path, lang)))
                {
                    ctrl.enqueue_commander_action([&ctrl, idx = static_cast<uint32_t>(i)]() 
                    {
                        ctrl.commander().request_config_path_remove(idx);
                    });
                }
                ImGui::PopID();
            }
            ImGui::EndTable();
        }

        if (ImGui::Button(get_gui_string(gui_string_id::btn_scan_configs, lang), ImVec2(-1.0f, btn_h)))
        {
            ctrl.enqueue_commander_action([&ctrl]() 
            {
                ctrl.commander().request_config_scan();
            });
        }
        
        if (!commander_active) ImGui::EndDisabled();
        
        ImGui::EndChild();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // --- BOTTOM PANEL (Available Configuration Files Table) ---
        ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_available_configurations, lang));
        ImGui::Separator();

        if (!commander_active) ImGui::BeginDisabled();

        if (ImGui::BeginTable("AvailableConfigsTable", 7, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY, ImVec2(0.0f, ImGui::GetContentRegionAvail().y)))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_config_name, lang), ImGuiTableColumnFlags_WidthStretch, 0.15f);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_filename, lang), ImGuiTableColumnFlags_WidthStretch, 0.15f);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_description, lang), ImGuiTableColumnFlags_WidthStretch, 0.35f);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_created, lang), ImGuiTableColumnFlags_WidthFixed, 130.0f * dpi_scale);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_modified, lang), ImGuiTableColumnFlags_WidthFixed, 130.0f * dpi_scale);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_counts, lang), ImGuiTableColumnFlags_WidthFixed, 80.0f * dpi_scale);
            ImGui::TableSetupColumn("##Actions", ImGuiTableColumnFlags_WidthFixed, 220.0f * dpi_scale);
            ImGui::TableHeadersRow();

            std::vector<adam::config_info> available_configs;
            if (commander_active)
            {
                std::lock_guard<const adam::config_view> lg(ctrl.commander().configs());
                for (const auto& [name_hash, info] : ctrl.get_commander().get_configs().get_available())
                    available_configs.push_back(info);
            }

            static uint32_t confirm_delete_idx = UINT32_MAX;

            for (size_t i = 0; i < available_configs.size(); ++i)
            {
                const auto& cfg = available_configs[i];
                ImGui::TableNextRow();

                // Name
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(cfg.name.c_str());

                // Filename
                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(cfg.filename.c_str());

                // Description
                ImGui::TableSetColumnIndex(2);
                if (cfg.description.empty())
                    ImGui::TextDisabled("%s", get_gui_string(gui_string_id::lbl_no_description, lang));
                else
                    ImGui::TextUnformatted(cfg.description.c_str());

                // Created Timestamp
                ImGui::TableSetColumnIndex(3);
                if (cfg.created > 0)
                {
                    std::time_t t = static_cast<std::time_t>(cfg.created / 1000);
                    char time_buf[64];
                    std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M", std::localtime(&t));
                    ImGui::TextUnformatted(time_buf);
                }
                else
                {
                    ImGui::TextDisabled("-");
                }

                // Modified Timestamp
                ImGui::TableSetColumnIndex(4);
                if (cfg.modified > 0)
                {
                    std::time_t t = static_cast<std::time_t>(cfg.modified / 1000);
                    char time_buf[64];
                    std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M", std::localtime(&t));
                    ImGui::TextUnformatted(time_buf);
                }
                else
                {
                    ImGui::TextDisabled("-");
                }

                // Counts (Conns/Ports/Procs)
                ImGui::TableSetColumnIndex(5);
                ImGui::Text("%llu/%llu/%llu", static_cast<unsigned long long>(cfg.connection_count), static_cast<unsigned long long>(cfg.port_count), static_cast<unsigned long long>(cfg.processor_count));

                // Actions Column (Load, Export, Delete)
                ImGui::TableSetColumnIndex(6);
                ImGui::PushID(static_cast<int>(i));

                float action_btn_w = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 2.0f) / 3.0f;
                
                if (ImGui::Button(get_gui_string(gui_string_id::btn_load_config, lang), ImVec2(action_btn_w, 0)))
                {
                    ctrl.enqueue_commander_action([&ctrl, p_idx = cfg.path_idx, fname = cfg.filename]() 
                    {
                        ctrl.commander().request_config_import(p_idx, adam::string_hashed(fname.c_str()));
                    });
                }
                ImGui::SameLine();
                
                if (ImGui::Button(get_gui_string(gui_string_id::btn_export_config, lang), ImVec2(action_btn_w, 0)))
                {
                    save_path_idx = cfg.path_idx;
                    std::strncpy(export_popup_filename, cfg.filename.c_str(), sizeof(export_popup_filename) - 1);
                    export_popup_filename[sizeof(export_popup_filename) - 1] = '\0';
                    std::strncpy(save_name, cfg.name.c_str(), sizeof(save_name) - 1);
                    save_name[sizeof(save_name) - 1] = '\0';
                    std::strncpy(save_desc, cfg.description.c_str(), sizeof(save_desc) - 1);
                    save_desc[sizeof(save_desc) - 1] = '\0';

                    open_save_popup = true;
                }
                ImGui::SameLine();

                if (ImGui::Button(get_gui_string(gui_string_id::btn_delete_config, lang), ImVec2(action_btn_w, 0)))
                {
                    confirm_delete_idx = static_cast<uint32_t>(i);
                    ImGui::OpenPopup(get_gui_string(gui_string_id::dlg_delete_config_confirm, lang));
                }

                if (confirm_delete_idx == static_cast<uint32_t>(i) && ImGui::BeginPopupModal(get_gui_string(gui_string_id::dlg_delete_config_confirm, lang), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
                {
                    ImGui::Text("Are you sure you want to delete configuration '%s'?", cfg.name.c_str());
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    int modal_btn = draw_modal_buttons({
                        { get_gui_string(gui_string_id::btn_delete, lang) },
                        { get_gui_string(gui_string_id::btn_cancel, lang) }
                    });

                    if (modal_btn == 0)
                    {
                        ctrl.enqueue_commander_action([&ctrl, p_idx = cfg.path_idx, fname = cfg.filename]() 
                        {
                            ctrl.commander().request_config_delete(p_idx, adam::string_hashed(fname.c_str()));
                        });
                        confirm_delete_idx = UINT32_MAX;
                        ImGui::CloseCurrentPopup();
                    }
                    else if (modal_btn == 1)
                    {
                        confirm_delete_idx = UINT32_MAX;
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }

                ImGui::PopID();
            }

            ImGui::EndTable();
        }

        if (!commander_active) ImGui::EndDisabled();

        // Modal for Configuration Export
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

            int btn = draw_modal_buttons({
                { "OK" },
                { "Cancel" }
            });

            if (btn == 0)
            {
                ctrl.enqueue_commander_action([&ctrl, path_idx = save_path_idx, filename = std::string(export_popup_filename), name = std::string(save_name), desc = std::string(save_desc)]() 
                {
                    ctrl.commander().request_config_export(path_idx, adam::string_hashed(filename.c_str()), name, desc);
                });
                ImGui::CloseCurrentPopup();
            }
            else if (btn == 1)
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
}
