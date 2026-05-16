#include "tab-modules.hpp"
#include "main-window.hpp"

#include <imgui.h>
#include <unordered_map>
#include <vector>
#include <cstring>
#include <algorithm>

namespace adam::gui 
{
    void render_tab_modules(gui_controller& ctrl, adam::language lang, int module_paths_table_id, int modules_table_id)
    {
        bool commander_active = ctrl.is_commander_active();
        
        ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_module_paths, lang));
        ImGui::Separator();

        float btn_add_width = std::max(ImGui::CalcTextSize(get_gui_string(gui_string_id::btn_add_path, adam::language_english)).x,
                                       ImGui::CalcTextSize(get_gui_string(gui_string_id::btn_add_path, adam::language_german)).x) + ImGui::GetStyle().FramePadding.x * 2.0f;

        if (!commander_active) ImGui::BeginDisabled();
        
        static char new_path[384] = "";
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - btn_add_width - ImGui::GetStyle().ItemSpacing.x);
        ImGui::InputTextWithHint("##NewPath", get_gui_string(gui_string_id::ph_new_path, lang), new_path, sizeof(new_path));
        ImGui::SameLine();
        if (ImGui::Button(get_gui_string(gui_string_id::btn_add_path, lang), ImVec2(btn_add_width, 0)))
        {
            if (new_path[0] != '\0' && commander_active)
            {
                ctrl.get_commander().request_module_path_add(adam::string_hashed(&new_path[0]));
                new_path[0] = '\0';
            }
        }
        
        if (!commander_active) ImGui::EndDisabled();
        
        ImGui::Spacing();
        
        float top_height = ImGui::GetContentRegionAvail().y * 0.35f;
        ImGui::PushID(module_paths_table_id);
        if (ImGui::BeginTable("ModulePathsTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY, ImVec2(0, top_height)))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_index, lang), ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize("99").x);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_path, lang), ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("##Actions", ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize(get_gui_string(gui_string_id::btn_remove_path, lang)).x + ImGui::GetStyle().FramePadding.x * 2.0f);
            ImGui::TableHeadersRow();

            if (commander_active)
            {
                const auto& paths = ctrl.get_commander().get_modules().get_paths();
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
                        ctrl.get_commander().request_module_path_remove(i);
                    if (i == 0) ImGui::EndDisabled();
                    ImGui::PopID();
                }
            }
            ImGui::EndTable();
        }
        ImGui::PopID();
        
        ImGui::Spacing();
        
        if (!commander_active) ImGui::BeginDisabled();
        if (ImGui::Button(get_gui_string(gui_string_id::btn_scan_modules, lang), ImVec2(-1.0f, ImGui::GetFrameHeight() * 1.5f)))
        {
            if (commander_active)
                ctrl.get_commander().request_module_scan();
        }
        if (!commander_active) ImGui::EndDisabled();
            
        ImGui::Spacing();
        
        ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_modules, lang));
        ImGui::Separator();

        ImGui::PushID(modules_table_id);
        if (ImGui::BeginTable("ModulesTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_status, lang), ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_name, lang), ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_path, lang), ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_version, lang), ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_load, lang), ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableHeadersRow();

            auto draw_module_row = [&](const char* name, int status, const char* path, uint32_t version, uint8_t reason) {
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::AlignTextToFramePadding();
                if (status == 0) {
                    ImGui::TextColored(get_gui_color(gui_color_id::log_trace), "%s", get_gui_string(gui_string_id::stat_available, lang));
                } else if (status == 1) {
                    ImGui::TextColored(get_gui_color(gui_color_id::log_info), "%s", get_gui_string(gui_string_id::stat_loaded, lang));
                } else if (status == 2) {
                    ImGui::TextColored(get_gui_color(gui_color_id::log_warning), "%s", get_gui_string(gui_string_id::stat_unavailable, lang));
                    if (ImGui::IsItemHovered())
                    {
                        if (reason == 1) // adam::module::basic_info::incompat_reason_sdk_too_old
                            ImGui::SetTooltip("%s", get_gui_string(gui_string_id::tt_incompat_sdk, lang));
                        else
                            ImGui::SetTooltip("%s", get_gui_string(gui_string_id::tt_incompat_unknown, lang));
                    }
                }

                ImGui::TableSetColumnIndex(1);
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted(name);

                ImGui::TableSetColumnIndex(2);
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted((!path || path[0] == '\0') ? "N/A" : path);

                ImGui::TableSetColumnIndex(3);
                ImGui::AlignTextToFramePadding();
                if (version != 0) {
                    ImGui::Text("%d.%d.%d", adam::get_major(version), adam::get_minor(version), adam::get_patch(version));
                } else {
                    ImGui::TextUnformatted("N/A");
                }

                ImGui::TableSetColumnIndex(4);
                bool is_loaded = (status == 1);
                bool checkbox_val = is_loaded;
                ImGui::PushID(name);
                if (status == 2) ImGui::BeginDisabled();
                if (ImGui::Checkbox("##load", &checkbox_val)) {
                    if (checkbox_val)
                        ctrl.get_commander().request_module_load(adam::string_hashed(name));
                    else
                        ctrl.get_commander().request_module_unload(adam::string_hashed(name));
                }
                if (status == 2) ImGui::EndDisabled();
                ImGui::PopID();
            };

            if (commander_active)
            {
                // Speed Optimization: avoid map string allocations
                struct module_gui_info {
                    int status; // 0=Avail, 1=Loaded, 2=Unavail
                    uint32_t version;
                    const char* path;
                    uint8_t reason;
                    const char* name;
                };
                
                std::unordered_map<uint64_t, module_gui_info> merged;

                for (const auto& [name_hash, data] : ctrl.get_commander().get_modules().get_available())
                    merged[name_hash.get_hash()] = { 0, data.first, data.second.c_str(), 0, name_hash.c_str() };
                    
                for (const auto& [name_hash, data] : ctrl.get_commander().get_modules().get_loaded()) {
                    uint64_t hash = name_hash.get_hash();
                    if (merged.find(hash) != merged.end())
                        merged[hash].status = 1;
                    else
                    {
                        if (data)
                            merged[hash] = { 1, data->get_version(), data->get_filepath().c_str(), 0, name_hash.c_str() };
                        else
                            merged[hash] = { 1, 0, "", 0, name_hash.c_str() };
                    }
                }
                    
                for (const auto& [name_hash, data] : ctrl.get_commander().get_modules().get_unavailable())
                    merged[name_hash.get_hash()] = { 2, std::get<0>(data), std::get<1>(data).c_str(), std::get<2>(data), name_hash.c_str() };
                
                std::vector<const module_gui_info*> sorted_modules;
                sorted_modules.reserve(merged.size());
                for (const auto& kv : merged) 
                    sorted_modules.push_back(&kv.second);
                    
                std::sort(sorted_modules.begin(), sorted_modules.end(), [](const module_gui_info* a, const module_gui_info* b) {
                    return std::strcmp(a->name, b->name) < 0;
                });
                    
                for (const auto* info : sorted_modules)
                    draw_module_row(info->name, info->status, info->path, info->version, info->reason);
            }

            ImGui::EndTable();
        }
        ImGui::PopID();
    }
}