#include "tab-modules.hpp"
#include "main-window.hpp"

#include <imgui.h>
#include <unordered_map>
#include <vector>
#include <cstring>
#include <algorithm>
#include <mutex>

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
                std::lock_guard<const adam::module_view> lg(ctrl.get_commander().modules());

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
        if (ImGui::BeginTable("ModulesTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, ImGui::GetTextLineHeight() + ImGui::GetStyle().FramePadding.x * 2.0f);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_status, lang), ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_name, lang), ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_path, lang), ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_version, lang), ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_load, lang), ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableHeadersRow();

            auto draw_module_row = [&](const char* name, int status, const char* path, uint32_t version, uint8_t reason, const adam::module_info* mod_info_ptr)
            {
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::AlignTextToFramePadding();

                bool open = false;
                if (mod_info_ptr)
                {
                    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_OpenOnArrow;
                    open = ImGui::TreeNodeEx(name, flags, "%s", "");
                }

                ImGui::TableSetColumnIndex(1);
                ImGui::AlignTextToFramePadding();
                if (status == 0)
                {
                    ImGui::TextColored(get_gui_color(gui_color_id::log_trace), "%s", get_gui_string(gui_string_id::stat_available, lang));
                }
                else if (status == 1)
                {
                    ImGui::TextColored(get_gui_color(gui_color_id::log_info), "%s", get_gui_string(gui_string_id::stat_loaded, lang));
                }
                else if (status == 2)
                {
                    ImGui::TextColored(get_gui_color(gui_color_id::log_warning), "%s", get_gui_string(gui_string_id::stat_unavailable, lang));
                    if (ImGui::IsItemHovered())
                    {
                        if (reason == 1) // adam::module::basic_info::incompat_reason_sdk_too_old
                            ImGui::SetTooltip("%s", get_gui_string(gui_string_id::tt_incompat_sdk, lang));
                        else
                            ImGui::SetTooltip("%s", get_gui_string(gui_string_id::tt_incompat_unknown, lang));
                    }
                }

                ImGui::TableSetColumnIndex(2);
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted(name);

                ImGui::TableSetColumnIndex(3);
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted((!path || path[0] == '\0') ? "N/A" : path);

                ImGui::TableSetColumnIndex(4);
                ImGui::AlignTextToFramePadding();
                if (version != 0)
                {
                    ImGui::Text("%d.%d.%d", adam::get_major(version), adam::get_minor(version), adam::get_patch(version));
                }
                else
                {
                    ImGui::TextUnformatted("N/A");
                }

                ImGui::TableSetColumnIndex(5);
                bool is_loaded = (status == 1);
                bool checkbox_val = is_loaded;
                ImGui::PushID(name);
                if (status == 2) ImGui::BeginDisabled();
                if (ImGui::Checkbox("##load", &checkbox_val))
                {
                    if (checkbox_val)
                        ctrl.get_commander().request_module_load(adam::string_hashed(name));
                    else
                        ctrl.get_commander().request_module_unload(adam::string_hashed(name));
                }
                if (status == 2) ImGui::EndDisabled();
                ImGui::PopID();

                if (open)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Dummy(ImVec2(0.0f, 0.0f)); // Dummy to create the cell

                    ImGui::TableSetColumnIndex(1);
                    ImGui::Dummy(ImVec2(0.0f, 0.0f));

                    ImGui::TableSetColumnIndex(2);
                    ImGui::Dummy(ImVec2(0.0f, 0.0f));

                    ImGui::TableSetColumnIndex(3);
                    ImGui::Dummy(ImVec2(0.0f, 0.0f));

                    ImGui::TableSetColumnIndex(4);
                    ImGui::Dummy(ImVec2(0.0f, 0.0f));

                    ImGui::TableSetColumnIndex(5);
                    ImGui::Dummy(ImVec2(0.0f, 0.0f));

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::SetColumnWidth(0, ImGui::GetColumnWidth(0) + ImGui::GetColumnWidth(1) + ImGui::GetColumnWidth(2) + ImGui::GetColumnWidth(3) + ImGui::GetColumnWidth(4) + ImGui::GetColumnWidth(5));
                    ImGui::TableSetColumnIndex(1);
                    ImGui::SetColumnWidth(1, 0);
                    ImGui::TableSetColumnIndex(2);
                    ImGui::SetColumnWidth(2, 0);
                    ImGui::TableSetColumnIndex(3);
                    ImGui::SetColumnWidth(3, 0);
                    ImGui::TableSetColumnIndex(4);
                    ImGui::SetColumnWidth(4, 0);
                    ImGui::TableSetColumnIndex(5);
                    ImGui::SetColumnWidth(5, 0);

                    ImGui::TableSetColumnIndex(0);

                    if (mod_info_ptr)
                    {
                        const std::string& desc = mod_info_ptr->descriptions[static_cast<size_t>(lang)];
                        if (!desc.empty())
                        {
                            ImGui::TextWrapped("%s", desc.c_str());
                        }
                        else
                        {
                            ImGui::TextDisabled("%s", get_gui_string(gui_string_id::lbl_no_description, lang));
                        }

                        bool has_formats = !mod_info_ptr->data_formats.empty();
                        bool has_ports = !mod_info_ptr->ports.empty();
                        bool has_processors = !mod_info_ptr->filters.empty() || !mod_info_ptr->converters.empty();

                        if (has_formats || has_ports || has_processors)
                        {
                            ImGui::Spacing();

                            float available_width = ImGui::GetContentRegionAvail().x;
                            int num_items = (has_formats ? 1 : 0) + (has_ports ? 1 : 0) + (has_processors ? 1 : 0);
                            float item_width = (num_items > 0) ? (available_width - ImGui::GetStyle().ItemSpacing.x * (num_items - 1)) / num_items : available_width;

                            if (has_formats)
                            {
                                ImGui::BeginGroup();
                                ImGui::Text("%s:", get_gui_string(gui_string_id::lbl_available_data_formats, lang));
                                if (ImGui::BeginTable("##FormatsTable", 1, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp, ImVec2(item_width, 0)))
                                {
                                    ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_name, lang));
                                    ImGui::TableHeadersRow();
                                    for (const auto& format_name : mod_info_ptr->data_formats)
                                    {
                                        ImGui::TableNextRow();
                                        ImGui::TableSetColumnIndex(0);
                                        ImGui::TextUnformatted(format_name.c_str());
                                    }
                                    ImGui::EndTable();
                                }
                                ImGui::EndGroup();
                                if (has_ports || has_processors) ImGui::SameLine();
                            }

                            if (has_ports)
                            {
                                ImGui::BeginGroup();
                                ImGui::Text("%s:", get_gui_string(gui_string_id::lbl_available_ports, lang));
                                if (ImGui::BeginTable("##PortsTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp, ImVec2(item_width, 0)))
                                {
                                    ImGui::TableSetupColumn(get_gui_string(gui_string_id::lbl_port_type, lang), ImGuiTableColumnFlags_WidthStretch, 0.7f);
                                    ImGui::TableSetupColumn(get_gui_string(gui_string_id::lbl_port_direction, lang), ImGuiTableColumnFlags_WidthStretch, 0.3f);
                                    ImGui::TableHeadersRow();

                                    auto draw_dir_badge = [](const char* label, ImVec4 color, bool first)
                                    {
                                        if (!first) ImGui::SameLine();
                                        ImVec4 bg_col = color;
                                        bg_col.w = 0.6f;
                                        ImGui::PushStyleColor(ImGuiCol_Button, bg_col);
                                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bg_col);
                                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, bg_col);
                                        ImGui::SmallButton(label);
                                        ImGui::PopStyleColor(3);
                                    };

                                    for (const auto& p : mod_info_ptr->ports)
                                    {
                                        ImGui::TableNextRow();

                                        ImGui::TableSetColumnIndex(0);
                                        ImGui::AlignTextToFramePadding();
                                        if (!p.type_name_str.empty())
                                            ImGui::TextUnformatted(p.type_name_str.c_str());
                                        else
                                            ImGui::Text("Unknown (Hash: 0x%llx)", static_cast<unsigned long long>(p.name_hash));

                                        ImGui::TableSetColumnIndex(1);
                                        bool first = true;
                                        if ((p.direction & adam::port_direction::input) != adam::port_direction::none)
                                        {
                                            draw_dir_badge(get_gui_string(gui_string_id::lbl_badge_input, lang), get_gui_color(gui_color_id::node_input), first);
                                            first = false;
                                        }

                                        if ((p.direction & adam::port_direction::output) != adam::port_direction::none)
                                        {
                                            draw_dir_badge(get_gui_string(gui_string_id::lbl_badge_output, lang), get_gui_color(gui_color_id::node_output), first);
                                        }
                                    }
                                    ImGui::EndTable();
                                }
                                ImGui::EndGroup();
                                if (has_processors) ImGui::SameLine();
                            }

                            if (has_processors)
                            {
                                ImGui::BeginGroup();
                                ImGui::Text("%s:", get_gui_string(gui_string_id::lbl_available_processors, lang));
                                if (ImGui::BeginTable("##ProcessorsTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp, ImVec2(item_width, 0)))
                                {
                                    ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_name, lang));
                                    ImGui::TableSetupColumn(get_gui_string(gui_string_id::lbl_processor_type, lang));
                                    ImGui::TableHeadersRow();

                                    for (const auto& proc : mod_info_ptr->filters)
                                    {
                                        ImGui::TableNextRow();
                                        ImGui::TableSetColumnIndex(0);
                                        ImGui::TextUnformatted(proc.name_str.c_str());
                                        ImGui::TableSetColumnIndex(1);
                                        ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_filter, lang));
                                    }
                                    for (const auto& proc : mod_info_ptr->converters)
                                    {
                                        ImGui::TableNextRow();
                                        ImGui::TableSetColumnIndex(0);
                                        ImGui::TextUnformatted(proc.name_str.c_str());
                                        ImGui::TableSetColumnIndex(1);
                                        ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_converter, lang));
                                    }
                                    ImGui::EndTable();
                                }
                                ImGui::EndGroup();
                            }
                        }

                        ImGui::Spacing();
                    }
                    ImGui::TreePop();
                }
            };

            if (commander_active)
            {
                // Speed Optimization: avoid map string allocations
                struct module_gui_info
                {
                    int status; // 0=Avail, 1=Loaded, 2=Unavail
                    uint32_t version;
                    const char* path;
                    uint8_t reason;
                    const char* name;
                    const adam::module_info* mod_info_ptr;
                };
                
                std::unordered_map<uint64_t, module_gui_info> merged;

                std::lock_guard<const adam::module_view> lg(ctrl.get_commander().modules());
                const auto& db = ctrl.get_commander().get_modules().database();

                for (const auto& [name_hash, data] : ctrl.get_commander().get_modules().get_available())
                {
                    auto it = db.find(name_hash);
                    const adam::module_info* info_ptr = (it != db.end()) ? &it->second : nullptr;
                    merged[name_hash.get_hash()] = { 0, data.first, data.second.c_str(), 0, name_hash.c_str(), info_ptr };
                }
                    
                for (const auto& [name_hash, data] : ctrl.get_commander().get_modules().get_loaded())
                {
                    uint64_t hash = name_hash.get_hash();
                    auto it = db.find(name_hash);
                    const adam::module_info* info_ptr = (it != db.end()) ? &it->second : nullptr;
                    if (merged.find(hash) != merged.end())
                    {
                        merged[hash].status = 1;
                        merged[hash].mod_info_ptr = info_ptr;
                    }
                    else
                    {
                        merged[hash] = { 1, data.first, data.second.c_str(), 0, name_hash.c_str(), info_ptr };
                    }
                }
                    
                for (const auto& [name_hash, data] : ctrl.get_commander().get_modules().get_unavailable())
                {
                    auto it = db.find(name_hash);
                    const adam::module_info* info_ptr = (it != db.end()) ? &it->second : nullptr;
                    merged[name_hash.get_hash()] = { 2, std::get<0>(data), std::get<1>(data).c_str(), std::get<2>(data), name_hash.c_str(), info_ptr };
                }
                
                std::vector<const module_gui_info*> sorted_modules;
                sorted_modules.reserve(merged.size());
                for (const auto& kv : merged) 
                    sorted_modules.push_back(&kv.second);
                    
                std::sort(sorted_modules.begin(), sorted_modules.end(), [](const module_gui_info* a, const module_gui_info* b)
                {
                    return std::strcmp(a->name, b->name) < 0;
                });
                    
                for (const auto* info : sorted_modules)
                    draw_module_row(info->name, info->status, info->path, info->version, info->reason, info->mod_info_ptr);
            }

            ImGui::EndTable();
        }
        ImGui::PopID();
    }
}