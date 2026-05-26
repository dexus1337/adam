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
        
        bool has_input = (new_path[0] != '\0');
        if (!has_input) ImGui::BeginDisabled();
        if (ImGui::Button(get_gui_string(gui_string_id::btn_add_path, lang), ImVec2(btn_add_width, 0)))
        {
            if (commander_active)
            {
                ctrl.commander().request_module_path_add(adam::string_hashed(&new_path[0]));
                new_path[0] = '\0';
            }
        }
        if (!has_input) ImGui::EndDisabled();
        
        if (!commander_active) ImGui::EndDisabled();
        
        ImGui::Spacing();
        
        float top_height = ImGui::GetContentRegionAvail().y * 0.35f;
        ImGui::PushID(module_paths_table_id);
        if (ImGui::BeginTable("ModulePathsTable", 3, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY, ImVec2(0, top_height)))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_index, lang), ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize("99").x);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_path, lang), ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("##Actions", ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize(get_gui_string(gui_string_id::btn_remove_path, lang)).x + ImGui::GetStyle().FramePadding.x * 2.0f);
            ImGui::TableHeadersRow();

            if (commander_active)
            {
                std::lock_guard<const adam::module_view> lg(ctrl.commander().modules());

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
                        ctrl.commander().request_module_path_remove(i);
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
                ctrl.commander().request_module_scan();
        }
        if (!commander_active) ImGui::EndDisabled();
            
        ImGui::Spacing();
        
        ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_modules, lang));
        ImGui::Separator();

        ImGui::PushID(modules_table_id);
        if (ImGui::BeginTable("ModulesTable", 6, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, ImGui::GetTextLineHeight() + ImGui::GetStyle().FramePadding.x * 2.0f);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_status, lang), ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_name, lang), ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_path, lang), ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoClip);
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
                    // The SpanFullWidth flag is not behaving as expected, so we use a manual approach.
                    open = ImGui::TreeNodeEx(name, ImGuiTreeNodeFlags_OpenOnArrow, "%s", "");
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
                        ctrl.commander().request_module_load(adam::string_hashed(name));
                    else
                        ctrl.commander().request_module_unload(adam::string_hashed(name));
                }
                if (status == 2) ImGui::EndDisabled();
                ImGui::PopID();

                if (open)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(3); // Use the stretch column (Path) as the base
                                                   // The NoClip flag will allow it to visually overflow into the remaining columns.

                    ImGui::Spacing();

                    if (mod_info_ptr)
                    {
                        // Calculate width to span from the path column to the end of the table
                        float span_width = ImGui::GetWindowContentRegionMax().x - ImGui::GetCursorPos().x;
                        if (span_width < 100.0f) span_width = 100.0f;

                        ImGui::BeginGroup();
                        const std::string& desc = mod_info_ptr->descriptions[static_cast<size_t>(lang)];
                        if (!desc.empty())
                        {
                            ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + span_width);
                            ImGui::TextUnformatted(desc.c_str());
                            ImGui::PopTextWrapPos();
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
                            ImGui::Spacing();

                            int num_items = (has_formats ? 1 : 0) + (has_ports ? 1 : 0) + (has_processors ? 1 : 0);
                            
                            float start_x = ImGui::GetCursorPosX();
                            float col_width = span_width / static_cast<float>(num_items);
                            float current_x = start_x;

                            if (has_formats)
                            {
                                    ImGui::SetCursorPosX(current_x);
                                    ImGui::BeginGroup();
                                    ImGui::TextColored(get_gui_color(gui_color_id::log_info), "%s", get_gui_string(gui_string_id::lbl_available_data_formats, lang));
                                    for (const auto& format_name : mod_info_ptr->data_formats)
                                    {
                                        ImGui::BulletText("%s", format_name.c_str());
                                    }
                                    ImGui::EndGroup();
                                    current_x += col_width;
                                    if (has_ports || has_processors) ImGui::SameLine();
                            }

                            if (has_ports)
                            {
                                    ImGui::SetCursorPosX(current_x);
                                    ImGui::BeginGroup();
                                    ImGui::TextColored(get_gui_color(gui_color_id::log_info), "%s", get_gui_string(gui_string_id::lbl_available_ports, lang));

                                    auto draw_dir_badge = [](const char* label, ImVec4 color)
                                    {
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
                                        ImGui::AlignTextToFramePadding();
                                        ImGui::Bullet();
                                        ImGui::SameLine();

                                        if ((p.direction & adam::port::direction_in) != adam::port::direction_invalid)
                                        {
                                            draw_dir_badge(get_gui_string(gui_string_id::lbl_badge_input, lang), get_gui_color(gui_color_id::node_input));
                                            ImGui::SameLine();
                                        }

                                        if ((p.direction & adam::port::direction_out) != adam::port::direction_invalid)
                                        {
                                            draw_dir_badge(get_gui_string(gui_string_id::lbl_badge_output, lang), get_gui_color(gui_color_id::node_output));
                                            ImGui::SameLine();
                                        }

                                        if (!p.type_name_str.empty())
                                            ImGui::TextUnformatted(p.type_name_str.c_str());
                                        else
                                            ImGui::Text("Unknown (Hash: 0x%llx)", static_cast<unsigned long long>(p.name_hash));
                                    }
                                    ImGui::EndGroup();
                                    current_x += col_width;
                                    if (has_processors) ImGui::SameLine();
                            }

                            if (has_processors)
                            {
                                    ImGui::SetCursorPosX(current_x);
                                    ImGui::BeginGroup();
                                    ImGui::TextColored(get_gui_color(gui_color_id::log_info), "%s", get_gui_string(gui_string_id::lbl_available_processors, lang));

                                    for (const auto& proc : mod_info_ptr->filters)
                                    {
                                        ImGui::BulletText("%s ", proc.name_str.c_str());
                                        ImGui::SameLine();
                                        ImGui::TextDisabled("[%s]", get_gui_string(gui_string_id::lbl_filter, lang));
                                    }
                                    for (const auto& proc : mod_info_ptr->converters)
                                    {
                                        ImGui::BulletText("%s ", proc.name_str.c_str());
                                        ImGui::SameLine();
                                        ImGui::TextDisabled("[%s]", get_gui_string(gui_string_id::lbl_converter, lang));
                                    }
                                    ImGui::EndGroup();
                            }
                            
                        }

                        ImGui::Spacing();
                        ImGui::EndGroup();
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
                
                // Speed Optimization: static vector avoids allocations, and modules are mutually exclusive between states
                static std::vector<module_gui_info> sorted_modules;
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
                    sorted_modules.push_back({ 1, data.first, data.second.c_str(), 0, name_hash.c_str(), (it != db.end()) ? &it->second : nullptr });
                }
                    
                for (const auto& [name_hash, data] : unavail)
                {
                    auto it = db.find(name_hash);
                    sorted_modules.push_back({ 2, std::get<0>(data), std::get<1>(data).c_str(), std::get<2>(data), name_hash.c_str(), (it != db.end()) ? &it->second : nullptr });
                }
                
                std::sort(sorted_modules.begin(), sorted_modules.end(), [](const module_gui_info& a, const module_gui_info& b)
                {
                    return std::strcmp(a.name, b.name) < 0;
                });
                    
                for (const auto& info : sorted_modules)
                    draw_module_row(info.name, info.status, info.path, info.version, info.reason, info.mod_info_ptr);
            }

            ImGui::EndTable();
        }
        ImGui::PopID();
    }
}