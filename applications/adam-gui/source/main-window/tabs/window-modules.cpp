/**
 * @file    window-modules.cpp
 * @author  dexus1337
 * @brief   Implementation of the modules window drawing functions.
 * @version 1.0
 * @date    12.06.2026
 */

#include "window-modules.hpp"
#include "../main-window.hpp"
#include "module/module.hpp"
#include "commander/messages/message-structs.hpp"

#include <imgui.h>
#include <unordered_map>
#include <vector>
#include <cstring>
#include <algorithm>
#include <mutex>

namespace adam::gui 
{
    void draw_window_modules
    (
        gui_controller& ctrl,
        adam::language lang,
        int module_paths_table_id,
        int modules_table_id
    )
    {
        float dpi_scale = ImGui::GetStyle()._MainScale;

        bool commander_active = ctrl.is_commander_active();
        
        float content_w = ImGui::GetContentRegionAvail().x;
        float panel_height = ImGui::GetContentRegionAvail().y * 0.333f;
        if (panel_height < 220.0f * dpi_scale) panel_height = 220.0f * dpi_scale;
        
        float btn_h = ImGui::GetFrameHeight() * 1.5f;

        // --- TOP PANEL (Paths + Settings) ---
        ImGui::BeginChild("##TopPanel", ImVec2(content_w, panel_height), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        
        ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_module_paths, lang));
        ImGui::Separator();

        float btn_add_width = std::max(ImGui::CalcTextSize(get_gui_string(gui_string_id::btn_add_path, adam::language_english)).x,
                                       ImGui::CalcTextSize(get_gui_string(gui_string_id::btn_add_path, adam::language_german)).x) + ImGui::GetStyle().FramePadding.x * 2.0f;

        if (!commander_active) ImGui::BeginDisabled();
        
        static char new_path[adam::max_path_length] = "";
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - btn_add_width - ImGui::GetStyle().ItemSpacing.x);
        ImGui::InputTextWithHint("##NewPath", get_gui_string(gui_string_id::ph_new_path, lang), new_path, sizeof(new_path));
        ImGui::SameLine();
        
        bool has_input = (new_path[0] != '\0');
        if (!has_input) ImGui::BeginDisabled();
        if (ImGui::Button(get_gui_string(gui_string_id::btn_add_path, lang), ImVec2(btn_add_width, 0)))
        {
            if (commander_active)
            {
                ctrl.enqueue_commander_action([&ctrl, path_str = std::string(new_path)]() 
                {
                    ctrl.commander().request_module_path_add(adam::string_hashed(path_str.c_str()));
                });
                new_path[0] = '\0';
            }
        }
        if (!has_input) ImGui::EndDisabled();

        float table_h = panel_height - ImGui::GetCursorPosY() - btn_h - ImGui::GetStyle().ItemSpacing.y;
        if (table_h < 60.0f * dpi_scale) table_h = 60.0f * dpi_scale;

        ImGui::PushID(module_paths_table_id);
        if (ImGui::BeginTable("ModulePathsTable", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0.0f, table_h)))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_path, lang), ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("##Remove", ImGuiTableColumnFlags_WidthFixed, 60.0f * dpi_scale);
            ImGui::TableHeadersRow();

            std::vector<adam::string_hashed> paths;
            if (commander_active)
            {
                std::lock_guard<const adam::module_view> lg(ctrl.commander().modules());
                paths = ctrl.get_commander().get_modules().get_paths();
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
                    if (commander_active)
                    {
                        ctrl.enqueue_commander_action([&ctrl, idx = static_cast<uint32_t>(i)]() 
                        {
                            ctrl.commander().request_module_path_remove(idx);
                        });
                    }
                }
                ImGui::PopID();
            }

            ImGui::EndTable();
        }
        ImGui::PopID();

        if (ImGui::Button(get_gui_string(gui_string_id::btn_scan_modules, lang), ImVec2(-1.0f, btn_h)))
        {
            if (commander_active)
            {
                ctrl.enqueue_commander_action([&ctrl]() 
                {
                    ctrl.commander().request_module_scan();
                });
            }
        }
        
        if (!commander_active) ImGui::EndDisabled();

        ImGui::EndChild();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // --- BOTTOM PANEL (Modules Table) ---
        ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_modules, lang));
        ImGui::Separator();

        ImGui::PushID(modules_table_id);
        if (ImGui::BeginTable("ModulesTable", 6, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY, ImVec2(0.0f, ImGui::GetContentRegionAvail().y)))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_load, lang), ImGuiTableColumnFlags_WidthFixed, 50.0f * dpi_scale);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_name, lang), ImGuiTableColumnFlags_WidthStretch, 0.20f);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_status, lang), ImGuiTableColumnFlags_WidthStretch, 0.15f);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_version, lang), ImGuiTableColumnFlags_WidthFixed, 80.0f * dpi_scale);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_path, lang), ImGuiTableColumnFlags_WidthStretch, 0.40f);
            ImGui::TableSetupColumn("##Details", ImGuiTableColumnFlags_WidthFixed, 24.0f * dpi_scale);
            ImGui::TableHeadersRow();

            auto draw_module_row = [&](const char* name, int status, const char* path, uint32_t version, uint32_t reason, const adam::module_info* mod_info_ptr, bool is_last_row)
            {
                (void)is_last_row;
                ImGui::TableNextRow();

                // Col 0: Load/Unload Checkbox / Switch
                ImGui::TableSetColumnIndex(0);
                ImGui::PushID(name);
                
                bool is_loaded = (status == 1);
                bool can_toggle = (status == 0 || status == 1);

                if (!commander_active || !can_toggle) ImGui::BeginDisabled();
                
                if (ImGui::Checkbox("##LoadCheck", &is_loaded))
                {
                    if (commander_active)
                    {
                        std::string mod_name(name);
                        ctrl.enqueue_commander_action([&ctrl, mod_name, is_loaded]() 
                        {
                            if (is_loaded)
                                ctrl.commander().request_module_load(adam::string_hashed(mod_name.c_str()));
                            else
                                ctrl.commander().request_module_unload(adam::string_hashed(mod_name.c_str()));
                        });
                    }
                }
                
                if (!commander_active || !can_toggle) ImGui::EndDisabled();
                
                ImGui::PopID();

                // Col 1: Name
                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(name);

                // Col 2: Status
                ImGui::TableSetColumnIndex(2);
                if (status == 0) // Available
                {
                    ImGui::TextColored(get_gui_color(gui_color_id::log_trace), "%s", get_gui_string(gui_string_id::stat_available, lang));
                }
                else if (status == 1) // Loaded
                {
                    ImGui::TextColored(get_gui_color(gui_color_id::log_info), "%s", get_gui_string(gui_string_id::stat_loaded, lang));
                }
                else // Unavailable
                {
                    ImGui::TextColored(get_gui_color(gui_color_id::log_error), "%s", get_gui_string(gui_string_id::stat_unavailable, lang));
                    if (ImGui::IsItemHovered())
                    {
                        ImGui::BeginTooltip();
                        if (reason == static_cast<uint32_t>(adam::module::basic_info::incompat_reason_sdk_too_old))
                            ImGui::TextUnformatted(get_gui_string(gui_string_id::tt_incompat_sdk, lang));
                        else
                            ImGui::TextUnformatted(get_gui_string(gui_string_id::tt_incompat_unknown, lang));
                        ImGui::EndTooltip();
                    }
                }

                // Col 3: Version
                ImGui::TableSetColumnIndex(3);
                if (status == 1 || status == 0)
                {
                    uint8_t v_maj = (version >> 16) & 0xFF;
                    uint8_t v_min = (version >> 8) & 0xFF;
                    uint8_t v_pat = version & 0xFF;
                    ImGui::Text("%u.%u.%u", v_maj, v_min, v_pat);
                }
                else
                {
                    ImGui::TextDisabled("-");
                }

                // Col 4: Path
                ImGui::TableSetColumnIndex(4);
                ImGui::TextUnformatted(path);

                // Col 5: Details Tooltip (Ports & Processors defined by module)
                ImGui::TableSetColumnIndex(5);
                if (mod_info_ptr)
                {
                    ImGui::TextDisabled("(?)");
                    if (ImGui::IsItemHovered())
                    {
                        ImGui::BeginTooltip();
                        
                        ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_available_ports, lang));
                        ImGui::Separator();
                        if (mod_info_ptr->ports.empty())
                        {
                            ImGui::TextDisabled("None");
                        }
                        else
                        {
                            for (const auto& port_entry : mod_info_ptr->ports)
                            {
                                const char* dir_str = (port_entry.direction == adam::port::direction_in) ? get_gui_string(gui_string_id::lbl_badge_input, lang) :
                                                      (port_entry.direction == adam::port::direction_out) ? get_gui_string(gui_string_id::lbl_badge_output, lang) : "In/Out";
                                ImGui::BulletText("%s [%s]", port_entry.name.c_str(), dir_str);
                            }
                        }
                        
                        ImGui::Spacing();
                        ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_available_data_formats, lang));
                        ImGui::Separator();
                        if (mod_info_ptr->data_formats.empty())
                        {
                            ImGui::TextDisabled("None");
                        }
                        else
                        {
                            for (const auto& fmt_entry : mod_info_ptr->data_formats)
                            {
                                ImGui::BulletText("%s", fmt_entry.c_str());
                            }
                        }

                        ImGui::Spacing();
                        ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_available_processors, lang));
                        ImGui::Separator();
                        if (mod_info_ptr->processors.empty())
                        {
                            ImGui::TextDisabled("None");
                        }
                        else
                        {
                            for (const auto& proc_entry : mod_info_ptr->processors)
                            {
                                const char* type_str = proc_entry.is_filter ? get_gui_string(gui_string_id::lbl_filter, lang) : get_gui_string(gui_string_id::lbl_converter, lang);
                                ImGui::BulletText("%s [%s]", proc_entry.name.c_str(), type_str);
                            }
                        }

                        ImGui::EndTooltip();
                    }
                }
            };

            bool table_open = true;

            struct module_gui_info
            {
                int status;
                uint32_t version;
                const char* path;
                uint32_t reason;
                const char* name;
                const adam::module_info* mod_info_ptr;
            };

            static std::vector<module_gui_info> sorted_modules;

            if (commander_active)
            {
                sorted_modules.clear();

                std::lock_guard<const adam::module_view> lg(ctrl.commander().modules());

                const auto& modules_manager = ctrl.get_commander().get_modules();
                const auto& db = modules_manager.database();
                const auto& avail = modules_manager.get_available();
                const auto& loaded = modules_manager.get_loaded();
                const auto& unavail = modules_manager.get_unavailable();

                sorted_modules.reserve(avail.size() + loaded.size() + unavail.size());

                for (const auto& [name_hash, data] : avail)
                {
                    auto it = db.find(name_hash);
                    sorted_modules.push_back({ 0, data.first, data.second.c_str(), 0, name_hash.c_str(), (it != db.end()) ? &it->second : nullptr });
                }
                    
                for (const auto& [name_hash, data] : loaded)
                {
                    auto it = db.find(name_hash);
                    sorted_modules.push_back({ 1, data.first, (data.second ? data.second->get_filepath().c_str() : ""), 0, name_hash.c_str(), (it != db.end()) ? &it->second : nullptr });
                }
                    
                for (const auto& [name_hash, data] : unavail)
                {
                    auto it = db.find(name_hash);
                    sorted_modules.push_back({ 2, std::get<0>(data), std::get<1>(data).c_str(), static_cast<uint32_t>(std::get<2>(data)), name_hash.c_str(), (it != db.end()) ? &it->second : nullptr });
                }
                
                std::sort(sorted_modules.begin(), sorted_modules.end(), [](const module_gui_info& a, const module_gui_info& b)
                {
                    return std::strcmp(a.name, b.name) < 0;
                });
                    
                for (size_t i = 0; i < sorted_modules.size(); ++i)
                {
                    const auto& info = sorted_modules[i];
                    draw_module_row(info.name, info.status, info.path, info.version, info.reason, info.mod_info_ptr, i == sorted_modules.size() - 1);
                }
            }

            if (table_open)
                ImGui::EndTable();
        }
        ImGui::PopID();
    }
}
