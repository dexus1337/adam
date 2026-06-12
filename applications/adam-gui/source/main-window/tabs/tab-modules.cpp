#include "tab-modules.hpp"
#include "../main-window.hpp"

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
        bool table_open = ImGui::BeginTable("ModulesTable", 6, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable);
        if (table_open)
        {
            auto setup_columns = [&]()
            {
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, ImGui::GetTextLineHeight() + ImGui::GetStyle().FramePadding.x * 2.0f);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_load, lang), ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_status, lang), ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_name, lang), ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_path, lang), ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoClip);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_version, lang), ImGuiTableColumnFlags_WidthFixed);
            };

            setup_columns();
            ImGui::TableHeadersRow();

            auto draw_module_row = [&](const char* name, int status, const char* path, uint32_t version, uint8_t reason, const adam::module_info* mod_info_ptr, bool is_last)
            {
                if (!table_open)
                    return;

                ImGui::TableNextRow();

                // ---- BEGIN Expandable  ----
                ImGui::TableSetColumnIndex(0);
                ImGui::AlignTextToFramePadding();

                bool open = false;
                if (mod_info_ptr)
                {
                    // The SpanFullWidth flag is not behaving as expected, so we use a manual approach.
                    uint64_t mod_hash = adam::string_hashed(name).get_hash();
                    ImGui::SetNextItemStorageID(static_cast<ImGuiID>(mod_hash ^ (mod_hash >> 32)));
                    open = ImGui::TreeNodeEx(name, ImGuiTreeNodeFlags_OpenOnArrow, "");
                }
                // ---- END Expandable  ----

                // ---- BEGIN Load  ----
                ImGui::TableSetColumnIndex(1);
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
                // ---- END Load  ----

                // ---- BEGIN Status  ----
                ImGui::TableSetColumnIndex(2);
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
                // ---- END Status  ----

                // ---- BEGIN Name  ----
                ImGui::TableSetColumnIndex(3);
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted(name);
                // ---- END Name  ----

                // ---- BEGIN Path  ----
                ImGui::TableSetColumnIndex(4);
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted((!path || path[0] == '\0') ? "N/A" : path);
                // ---- END Path  ----

                // ---- BEGIN Version  ----
                ImGui::TableSetColumnIndex(5);
                ImGui::AlignTextToFramePadding();
                if (version != 0)
                {
                    ImGui::Text("%d.%d.%d", adam::get_major(version), adam::get_minor(version), adam::get_patch(version));
                }
                else
                {
                    ImGui::TextUnformatted("N/A");
                }
                // ---- END Version  ----

                if (open)
                {
                    ImGui::TreePop();
                    ImGui::EndTable();

                    if (mod_info_ptr)
                    {
                        ImGui::PushID(name);
                        if (ImGui::BeginChild("##ModuleDetailsChild", ImVec2(0, 150.0f * ImGui::GetStyle()._MainScale), true))
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
                            bool has_processors = !mod_info_ptr->processors.empty();

                            if (has_formats || has_ports || has_processors)
                            {
                                ImGui::Spacing();
                                ImGui::Spacing();

                                if (ImGui::BeginTable("ModuleDetailsTable", 2, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
                                {
                                    ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_type, lang), ImGuiTableColumnFlags_WidthFixed, 150.0f * ImGui::GetStyle()._MainScale);
                                    ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_name, lang), ImGuiTableColumnFlags_WidthStretch);
                                    ImGui::TableHeadersRow();

                                    for (const auto& format_name : mod_info_ptr->data_formats)
                                    {
                                        ImGui::TableNextRow();
                                        ImGui::TableSetColumnIndex(0);
                                        ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_data_format, lang));
                                        ImGui::TableSetColumnIndex(1);
                                        ImGui::TextUnformatted(format_name.c_str());
                                    }

                                    for (const auto& [port_name, port_dir] : mod_info_ptr->ports)
                                    {
                                        ImGui::TableNextRow();
                                        ImGui::TableSetColumnIndex(0);
                                        
                                        bool is_in = (port_dir & adam::port::direction_in) != adam::port::direction_invalid;
                                        bool is_out = (port_dir & adam::port::direction_out) != adam::port::direction_invalid;
                                        
                                        if (is_in && is_out)
                                            ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_inout_port, lang));
                                        else if (is_in)
                                            ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_input_port, lang));
                                        else if (is_out)
                                            ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_output_port, lang));
                                        else
                                            ImGui::TextUnformatted("Port");

                                        ImGui::TableSetColumnIndex(1);
                                        if (!port_name.empty())
                                            ImGui::TextUnformatted(port_name.c_str());
                                        else
                                            ImGui::Text("Unknown (Hash: 0x%llx)", static_cast<unsigned long long>(port_name.get_hash()));
                                    }

                                    for (const auto& [processor_name, is_filter] : mod_info_ptr->processors)
                                    {
                                        ImGui::TableNextRow();
                                        ImGui::TableSetColumnIndex(0);
                                        if (is_filter)
                                            ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_filter, lang));
                                        else
                                            ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_converter, lang));
                                        ImGui::TableSetColumnIndex(1);
                                        if (!processor_name.empty())
                                            ImGui::TextUnformatted(processor_name.c_str());
                                        else
                                            ImGui::Text("Unknown (Hash: 0x%llx)", static_cast<unsigned long long>(processor_name.get_hash()));
                                    }

                                    ImGui::EndTable();
                                }
                            }

                            ImGui::Spacing();
                        }
                        ImGui::EndChild();
                        ImGui::PopID();
                    }
                    
                    if (!is_last)
                    {
                        table_open = ImGui::BeginTable("ModulesTable", 6, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable);
                        if (table_open)
                        {
                            setup_columns();
                        }
                    }
                    else
                    {
                        table_open = false;
                    }
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
                    sorted_modules.push_back({ 1, data.first, data.second->get_filepath().c_str(), 0, name_hash.c_str(), (it != db.end()) ? &it->second : nullptr });
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
