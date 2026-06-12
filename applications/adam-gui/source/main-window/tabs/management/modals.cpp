/**
 * @file    modals.cpp
 * @author  dexus1337
 * @brief   Implementation of popup modals for creating and deleting connections, ports, and processors.
 * @version 1.0
 * @date    12.06.2026
 */

#include "modals.hpp"
#include "shared-state.hpp"
#include "../../main-window.hpp"
#include "controller/controller.hpp"
#include <algorithm>
#include <vector>
#include <map>
#include <array>

namespace adam::gui
{
    void draw_delete_connection_modal(gui_controller& ctrl, adam::language lang)
    {
        float dpi_scale = ImGui::GetStyle()._MainScale;
        if (g_request_delete_popup)
        {
            ImGui::OpenPopup(get_gui_string(gui_string_id::dlg_delete_connection, lang));
            g_request_delete_popup = false;
        }

        if (!ImGui::BeginPopupModal(get_gui_string(gui_string_id::dlg_delete_connection, lang), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            return;
        }

        ImGui::TextUnformatted(get_gui_string(gui_string_id::msg_delete_connection_confirm, lang));
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button(get_gui_string(gui_string_id::btn_ok, lang), ImVec2(120.0f * dpi_scale, 0.0f)))
        {
            if (g_connection_to_delete.get_hash() != 0)
            {
                adam::string_hash h = g_connection_to_delete.get_hash();
                ctrl.enqueue_commander_action([&ctrl, h]() { ctrl.commander().request_connection_destroy(h); });
                g_connection_to_delete = adam::string_hashed("");
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button(get_gui_string(gui_string_id::btn_cancel, lang), ImVec2(120.0f * dpi_scale, 0.0f)))
        {
            g_connection_to_delete = adam::string_hashed("");
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    void draw_create_connection_modal(gui_controller& ctrl, adam::language lang)
    {
        float dpi_scale = ImGui::GetStyle()._MainScale;
        if (!ImGui::BeginPopupModal(get_gui_string(gui_string_id::dlg_create_connection, lang), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            return;
        }

        static char conn_name[max_name_length] = "";
        ImGui::InputText(get_gui_string(gui_string_id::tbl_name, lang), conn_name, sizeof(conn_name));

        ImGui::Spacing();
        
        bool can_create = conn_name[0] != '\0';
        if (can_create)
        {
            adam::string_hash proposed_hash = adam::string_hashed(&conn_name[0]).get_hash();
            if (ctrl.commander().registry().get_connections().find(proposed_hash) != ctrl.commander().registry().get_connections().end())
            {
                can_create = false;
            }
        }

        if (!can_create) ImGui::BeginDisabled();
        if (ImGui::Button(get_gui_string(gui_string_id::btn_create, lang), ImVec2(120.0f * dpi_scale, 0.0f)))
        {
            if (conn_name[0] != '\0')
            {
                adam::string_hashed new_name(&conn_name[0]);
                ctrl.enqueue_commander_action([&ctrl, new_name]() { ctrl.commander().request_connection_create(new_name); });
                conn_name[0] = '\0';
                ImGui::CloseCurrentPopup();
            }
        }
        if (!can_create) ImGui::EndDisabled();
        
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button(get_gui_string(gui_string_id::btn_cancel, lang), ImVec2(120.0f * dpi_scale, 0.0f)))
            ImGui::CloseCurrentPopup();
        
        ImGui::EndPopup();
    }

    void draw_add_create_port_modal(gui_controller& ctrl, adam::language lang)
    {
        float dpi_scale = ImGui::GetStyle()._MainScale;

        static std::map<std::string, std::vector<port_display_info>> existing_grouped;
        static std::map<std::string, std::vector<port_display_info>> new_grouped;
        static std::map<adam::string_hash, port_display_info> known_port_types;
        static std::map<adam::string_hash, std::array<char, max_name_length>> new_port_names;
        static const std::vector<adam::string_hash>* used_ports_ptr = nullptr;

        if (g_request_port_popup)
        {
            if (g_target_direction == adam::port::direction_in)
            {
                snprintf(g_port_popup_title, sizeof(g_port_popup_title), get_gui_string(gui_string_id::dlg_add_input_port_to, lang), g_target_connection.c_str());
            }
            else if (g_target_direction == adam::port::direction_out)
            {
                snprintf(g_port_popup_title, sizeof(g_port_popup_title), get_gui_string(gui_string_id::dlg_add_output_port_to, lang), g_target_connection.c_str());
            }
            else
            {
                snprintf(g_port_popup_title, sizeof(g_port_popup_title), "%s", get_gui_string(gui_string_id::dlg_add_port, lang));
            }

            existing_grouped.clear();
            new_grouped.clear();
            known_port_types.clear();
            new_port_names.clear();
            used_ports_ptr = nullptr;

            {
                std::lock_guard<const adam::module_view> mod_lock(ctrl.commander().modules());
                std::lock_guard<const adam::registry_view> reg_lock(ctrl.commander().registry());

                const auto& db              = ctrl.get_commander().get_modules().database();
                const auto& loaded_modules  = ctrl.get_commander().get_modules().get_loaded();
                const auto& reg_ports       = ctrl.commander().registry().get_ports();
                const auto& reg_conns       = ctrl.commander().registry().get_connections();

                auto conn_it = reg_conns.find(g_target_connection.get_hash());
                if (conn_it != reg_conns.end())
                {
                    if (g_target_direction == adam::port::direction_in)
                    {
                        used_ports_ptr = &conn_it->second->inputs;
                    }
                    else if (g_target_direction == adam::port::direction_out)
                    {
                        used_ports_ptr = &conn_it->second->outputs;
                    }
                }

                for (const auto& [mod_hash, mod_info] : db)
                {
                    bool is_loaded = loaded_modules.find(mod_hash) != loaded_modules.end();

                    for (const auto& [port_name, port_dir] : mod_info.ports)
                    {
                        port_display_info pdi;
                        pdi.module_hash = mod_hash.get_hash();
                        pdi.module_name = mod_info.name.c_str();
                        if (!port_name.empty())
                        {
                            pdi.port_name = port_name.c_str();
                            pdi.type_name = port_name.c_str();
                        }
                        else
                        {
                            char hash_buf[32];
                            snprintf(hash_buf, sizeof(hash_buf), "0x%llx", static_cast<unsigned long long>(port_name.get_hash()));
                            pdi.port_name = std::string("Unknown (Hash: ") + hash_buf + ")";
                            pdi.type_name = pdi.port_name;
                        }
                        pdi.port_hash = port_name.get_hash();
                        pdi.direction = port_dir;
                        pdi.is_unavailable = false;

                        known_port_types[port_name.get_hash()] = pdi;

                        if (is_loaded)
                        {
                            if (g_target_direction == adam::port::direction_invalid || (pdi.direction & g_target_direction) != adam::port::direction_invalid)
                            {
                                new_grouped[pdi.module_name].push_back(pdi);
                            }
                        }
                    }
                }

                {
                    port_display_info pdi;
                    pdi.module_hash = 0;
                    pdi.module_name = "Internal";
                    pdi.port_name = "internal";
                    pdi.type_name = "internal";
                    pdi.port_hash = ("internal"_ct).get_hash();
                    pdi.direction = adam::port::direction_inout;
                    pdi.is_unavailable = false;

                    if (g_target_direction == adam::port::direction_invalid || (pdi.direction & g_target_direction) != adam::port::direction_invalid)
                    {
                        new_grouped[pdi.module_name].push_back(pdi);
                        known_port_types[pdi.port_hash] = pdi;
                    }
                }

                for (const auto& [p_hash, p_view] : reg_ports)
                {
                    port_display_info pdi;
                    pdi.port_name = p_view->name.c_str();
                    pdi.port_hash = p_hash;
                    pdi.direction = p_view->direction;
                    pdi.is_unavailable = p_view->is_unavailable;

                    auto type_it = known_port_types.find(p_view->type.get_hash());
                    if (type_it != known_port_types.end())
                    {
                        pdi.module_name = type_it->second.module_name;
                        pdi.module_hash = type_it->second.module_hash;
                        pdi.type_name = type_it->second.type_name;
                    }
                    else
                    {
                        if (p_view->type.get_hash() == ("internal"_ct).get_hash())
                        {
                            pdi.module_name = "Internal";
                            pdi.module_hash = 0;
                            pdi.type_name = "internal";
                        }
                        else
                        {
                            if (p_view->type_module.get_hash() != 0)
                            {
                                pdi.module_name = p_view->type_module.c_str();
                                pdi.module_hash = p_view->type_module.get_hash();
                            }
                            else
                            {
                                pdi.module_name = "Unknown Module";
                                pdi.module_hash = 0;
                            }
                            
                            if (!p_view->type.empty())
                            {
                                pdi.type_name = p_view->type.c_str();
                            }
                            else
                            {
                                char hash_buf[32];
                                snprintf(hash_buf, sizeof(hash_buf), "0x%llx", static_cast<unsigned long long>(p_view->type.get_hash()));
                                pdi.type_name = std::string("Unknown (Hash: ") + hash_buf + ")";
                            }
                        }
                    }

                    if (g_target_direction == adam::port::direction_invalid || pdi.direction == adam::port::direction_invalid || (pdi.direction & g_target_direction) != adam::port::direction_invalid)
                    {
                        existing_grouped[pdi.module_name].push_back(pdi);
                    }
                }
            }

            ImGui::OpenPopup(g_port_popup_title);
            g_request_port_popup = false;
        }

        ImGui::SetNextWindowSize(ImVec2(1200.0f * dpi_scale, 600.0f * dpi_scale), ImGuiCond_Appearing);
        if (!ImGui::BeginPopupModal(g_port_popup_title, nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
        {
            return;
        }

        static int port_open_frame = -1;
        if (ImGui::IsWindowAppearing())
        {
            port_open_frame = ImGui::GetFrameCount();
        }

        if (ImGui::IsMouseClicked(0) && ImGui::GetFrameCount() > port_open_frame + 1)
        {
            if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows))
            {
                ImGui::CloseCurrentPopup();
            }
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Escape))
        {
            ImGui::CloseCurrentPopup();
        }

        float half_w = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
        float bottom_h = ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y * 4.0f + 4.0f * dpi_scale;
        float child_h = ImGui::GetContentRegionAvail().y - bottom_h;

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
        
        // -- LEFT PANEL: EXISTING PORTS --
        if (ImGui::BeginChild("ExistingPortsTableChild", ImVec2(half_w, child_h), true))
        {
            ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_existing_ports, lang));
            ImGui::Separator();

            if (ImGui::BeginTable("ExistingPortsTable", 4, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
            {
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::lbl_port_type, lang), ImGuiTableColumnFlags_WidthStretch, 0.4f);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::lbl_port_direction, lang), ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_name, lang), ImGuiTableColumnFlags_WidthStretch, 0.6f);
                ImGui::TableSetupColumn("##Action", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableHeadersRow();

                for (const auto& [mod_name, ports] : existing_grouped)
                {
                    ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextColored(get_gui_color(gui_color_id::log_info), "[%s]", mod_name.c_str());

                    for (const auto& pdi : ports)
                    {
                        bool is_used = used_ports_ptr && std::find(used_ports_ptr->begin(), used_ports_ptr->end(), pdi.port_hash) != used_ports_ptr->end();

                        ImGui::TableNextRow();
                        
                        if (is_used || pdi.is_unavailable)
                        {
                            ImGui::BeginDisabled();
                        }

                        ImGui::TableSetColumnIndex(0);
                        ImGui::Indent();
                        ImGui::TextUnformatted(pdi.type_name.c_str());
                        ImGui::Unindent();

                        ImGui::TableSetColumnIndex(1);
                        draw_direction_badge(pdi.direction, is_used, lang);

                        ImGui::TableSetColumnIndex(2);
                        ImGui::TextUnformatted(pdi.port_name.c_str());

                        ImGui::TableSetColumnIndex(3);
                        ImGui::PushID((const void*)(intptr_t)pdi.port_hash);
                        if (ImGui::Button(get_gui_string(gui_string_id::btn_add_path, lang)))
                        {
                            adam::string_hash conn_hash = g_target_connection.get_hash();
                            adam::string_hash port_hash = pdi.port_hash;
                            adam::port::direction dir = g_target_direction;
                            
                            ctrl.enqueue_commander_action([&ctrl, conn_hash, port_hash, dir]() { ctrl.commander().request_connection_port_add(conn_hash, port_hash, dir == adam::port::direction_in); });
                        }
                        if (pdi.is_unavailable && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                        {
                            ImGui::SetTooltip(get_gui_string(gui_string_id::tt_module_missing, lang), pdi.module_name.c_str());
                        }
                        ImGui::PopID();
                        
                        if (is_used || pdi.is_unavailable)
                        {
                            ImGui::EndDisabled();
                        }
                    }
                }
                ImGui::EndTable();
            }
        }
        ImGui::EndChild();

        ImGui::SameLine();

        // -- RIGHT PANEL: NEW PORTS --
        if (ImGui::BeginChild("NewPortsTableChild", ImVec2(half_w, child_h), true))
        {
            ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_new_ports, lang));
            ImGui::Separator();

            if (ImGui::BeginTable("NewPortsTable", 4, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
            {
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::lbl_port_type, lang), ImGuiTableColumnFlags_WidthStretch, 0.4f);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::lbl_port_direction, lang), ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_name, lang), ImGuiTableColumnFlags_WidthStretch, 0.6f);
                ImGui::TableSetupColumn("##Action", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableHeadersRow();

                for (const auto& [mod_name, ports] : new_grouped)
                {
                    ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextColored(get_gui_color(gui_color_id::log_info), "[%s]", mod_name.c_str());

                    for (const auto& pdi : ports)
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Indent();
                        ImGui::TextUnformatted(pdi.type_name.c_str());
                        ImGui::Unindent();

                        ImGui::TableSetColumnIndex(1);
                        draw_direction_badge(pdi.direction, false, lang);

                        ImGui::TableSetColumnIndex(2);
                        ImGui::PushID((const void*)(intptr_t)pdi.port_hash);
                        auto& name_buffer = new_port_names[pdi.port_hash];
                        ImGui::SetNextItemWidth(-1.0f);
                        ImGui::InputText("##NewName", name_buffer.data(), name_buffer.size());

                        ImGui::TableSetColumnIndex(3);
                        bool has_name = name_buffer[0] != '\0';
                        
                        if (has_name)
                        {
                            adam::string_hash proposed_hash = adam::string_hashed(name_buffer.data()).get_hash();
                            if (ctrl.commander().registry().get_ports().find(proposed_hash) != ctrl.commander().registry().get_ports().end())
                            {
                                has_name = false;
                            }
                        }
                        
                        if (!has_name)
                        {
                            ImGui::BeginDisabled();
                        }
                        
                        if (ImGui::Button(get_gui_string(gui_string_id::btn_create, lang)))
                        {
                            adam::string_hashed new_name(name_buffer.data());
                            adam::string_hash type_hash = pdi.port_hash;
                            adam::string_hash mod_hash = pdi.module_hash;
                            adam::string_hash conn_hash = g_target_connection.get_hash();
                            adam::string_hash new_port_hash = adam::string_hashed(name_buffer.data()).get_hash();
                            adam::port::direction dir = g_target_direction;
                            
                            ctrl.enqueue_commander_action([&ctrl, new_name, type_hash, mod_hash, conn_hash, new_port_hash, dir]() { ctrl.commander().request_port_create(new_name, type_hash, mod_hash); ctrl.commander().request_connection_port_add(conn_hash, new_port_hash, dir == adam::port::direction_in); });

                            name_buffer[0] = '\0';
                        }
                        
                        if (!has_name)
                        {
                            ImGui::EndDisabled();
                        }
                        ImGui::PopID();
                    }
                }
                ImGui::EndTable();
            }
        }
        ImGui::EndChild();

        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        if (ImGui::Button(get_gui_string(gui_string_id::btn_cancel, lang), ImVec2(-1.0f, 0.0f)))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    void draw_add_create_processor_modal(gui_controller& ctrl, adam::language lang)
    {
        float dpi_scale = ImGui::GetStyle()._MainScale;

        static std::map<std::string, std::vector<processor_display_info>> existing_filters;
        static std::map<std::string, std::vector<processor_display_info>> existing_converters;
        static std::map<std::string, std::vector<processor_display_info>> new_filters;
        static std::map<std::string, std::vector<processor_display_info>> new_converters;
        static std::map<adam::string_hash, processor_display_info> known_proc_types;
        static std::map<adam::string_hash, std::array<char, max_name_length>> new_proc_names;
        static const std::vector<adam::string_hash>* used_processors_ptr = nullptr;

        if (g_request_processor_popup)
        {
            snprintf(g_processor_popup_title, sizeof(g_processor_popup_title), "%s", get_gui_string(gui_string_id::btn_add_processor, lang));

            existing_filters.clear();
            existing_converters.clear();
            new_filters.clear();
            new_converters.clear();
            known_proc_types.clear();
            new_proc_names.clear();
            used_processors_ptr = nullptr;

            {
                std::lock_guard<const adam::module_view> mod_lock(ctrl.commander().modules());
                std::lock_guard<const adam::registry_view> reg_lock(ctrl.commander().registry());

                const auto& db              = ctrl.get_commander().get_modules().database();
                const auto& loaded_modules  = ctrl.get_commander().get_modules().get_loaded();
                const auto& reg_procs       = ctrl.commander().registry().get_processors();
                const auto& reg_conns       = ctrl.commander().registry().get_connections();

                auto conn_it = reg_conns.find(g_target_connection.get_hash());
                if (conn_it != reg_conns.end())
                {
                    used_processors_ptr = &conn_it->second->processors;
                }

                for (const auto& [mod_hash, mod_info] : db)
                {
                    bool is_loaded = loaded_modules.find(mod_hash) != loaded_modules.end();

                    for (const auto& proc : mod_info.processors)
                    {
                        processor_display_info pdi;
                        pdi.module_hash = mod_hash.get_hash();
                        pdi.module_name = mod_info.name.c_str();
                        if (!proc.name.empty())
                        {
                            pdi.proc_name = proc.name.c_str();
                            pdi.type_name = proc.name.c_str();
                        }
                        else
                        {
                            char hash_buf[32];
                            snprintf(hash_buf, sizeof(hash_buf), "0x%llx", static_cast<unsigned long long>(proc.name.get_hash()));
                            pdi.proc_name = std::string("Unknown (Hash: ") + hash_buf + ")";
                            pdi.type_name = pdi.proc_name;
                        }
                        pdi.proc_hash = proc.name.get_hash();
                        pdi.is_filter = proc.is_filter;
                        pdi.is_unavailable = false;

                        known_proc_types[proc.name.get_hash()] = pdi;

                        if (is_loaded)
                        {
                            if (pdi.is_filter)
                                new_filters[pdi.module_name].push_back(pdi);
                            else
                                new_converters[pdi.module_name].push_back(pdi);
                        }
                    }
                }

                for (const auto& [p_hash, p_view] : reg_procs)
                {
                    processor_display_info pdi;
                    pdi.proc_name = p_view->name.c_str();
                    pdi.proc_hash = p_hash;
                    pdi.is_filter = p_view->is_filter;
                    pdi.is_unavailable = p_view->is_unavailable;

                    auto type_it = known_proc_types.find(p_view->type.get_hash());
                    if (type_it != known_proc_types.end())
                    {
                        pdi.module_name = type_it->second.module_name;
                        pdi.module_hash = type_it->second.module_hash;
                        pdi.type_name = type_it->second.type_name;
                    }
                    else
                    {
                        if (p_view->module_name.get_hash() != 0)
                        {
                            pdi.module_name = p_view->module_name.c_str();
                            pdi.module_hash = p_view->module_name.get_hash();
                        }
                        else
                        {
                            pdi.module_name = "Unknown Module";
                            pdi.module_hash = 0;
                        }
                        
                        if (!p_view->type.empty())
                        {
                            pdi.type_name = p_view->type.c_str();
                        }
                        else
                        {
                            char hash_buf[32];
                            snprintf(hash_buf, sizeof(hash_buf), "0x%llx", static_cast<unsigned long long>(p_view->type.get_hash()));
                            pdi.type_name = std::string("Unknown (Hash: ") + hash_buf + ")";
                        }
                    }

                    if (pdi.is_filter)
                        existing_filters[pdi.module_name].push_back(pdi);
                    else
                        existing_converters[pdi.module_name].push_back(pdi);
                }
            }

            ImGui::OpenPopup(g_processor_popup_title);
            g_request_processor_popup = false;
        }

        ImGui::SetNextWindowSize(ImVec2(1200.0f * dpi_scale, 600.0f * dpi_scale), ImGuiCond_Appearing);
        if (!ImGui::BeginPopupModal(g_processor_popup_title, nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
        {
            return;
        }

        static int proc_open_frame = -1;
        if (ImGui::IsWindowAppearing())
        {
            proc_open_frame = ImGui::GetFrameCount();
        }

        if (ImGui::IsMouseClicked(0) && ImGui::GetFrameCount() > proc_open_frame + 1)
        {
            if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows))
            {
                ImGui::CloseCurrentPopup();
            }
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Escape))
        {
            ImGui::CloseCurrentPopup();
        }

        float half_w = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
        float bottom_h = ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y * 4.0f + 4.0f * dpi_scale;
        float child_h = ImGui::GetContentRegionAvail().y - bottom_h;

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
        
        // Helper lambda for existing tables content
        auto draw_existing_table_content = [&](const char* title, const std::map<std::string, std::vector<processor_display_info>>& grouped_data, float width, float height) {
            ImGui::TextUnformatted(title);
            ImGui::Separator();

            if (ImGui::BeginTable(title, 3, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(width, height)))
            {
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::lbl_processor_type, lang), ImGuiTableColumnFlags_WidthStretch, 0.4f);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_name, lang), ImGuiTableColumnFlags_WidthStretch, 0.6f);
                ImGui::TableSetupColumn("##Action", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableHeadersRow();

                for (const auto& [mod_name, procs] : grouped_data)
                {
                    ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextColored(get_gui_color(gui_color_id::log_info), "[%s]", mod_name.c_str());

                    for (const auto& pdi : procs)
                    {
                        bool is_used = used_processors_ptr && std::find(used_processors_ptr->begin(), used_processors_ptr->end(), pdi.proc_hash) != used_processors_ptr->end();

                        ImGui::TableNextRow();
                        
                        if (is_used || pdi.is_unavailable)
                        {
                            ImGui::BeginDisabled();
                        }

                        ImGui::TableSetColumnIndex(0);
                        ImGui::Indent();
                        ImGui::TextUnformatted(pdi.type_name.c_str());
                        ImGui::Unindent();

                        ImGui::TableSetColumnIndex(1);
                        ImGui::TextUnformatted(pdi.proc_name.c_str());

                        ImGui::TableSetColumnIndex(2);
                        ImGui::PushID((const void*)(intptr_t)pdi.proc_hash);
                        if (ImGui::Button(get_gui_string(gui_string_id::btn_add_path, lang)))
                        {
                            adam::string_hash conn_hash = g_target_connection.get_hash();
                            adam::string_hash proc_hash = pdi.proc_hash;
                            ctrl.enqueue_commander_action([&ctrl, conn_hash, proc_hash]() { ctrl.commander().request_connection_processor_add(conn_hash, proc_hash); });
                        }
                        if (pdi.is_unavailable && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                        {
                            ImGui::SetTooltip(get_gui_string(gui_string_id::tt_module_missing, lang), pdi.module_name.c_str());
                        }
                        ImGui::PopID();
                        
                        if (is_used || pdi.is_unavailable)
                        {
                            ImGui::EndDisabled();
                        }
                    }
                }
                ImGui::EndTable();
            }
        };

        // Helper lambda for new tables content
        auto draw_new_table_content = [&](const char* title, const std::map<std::string, std::vector<processor_display_info>>& grouped_data, float width, float height) {
            ImGui::TextUnformatted(title);
            ImGui::Separator();

            if (ImGui::BeginTable(title, 3, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(width, height)))
            {
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::lbl_processor_type, lang), ImGuiTableColumnFlags_WidthStretch, 0.4f);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_name, lang), ImGuiTableColumnFlags_WidthStretch, 0.6f);
                ImGui::TableSetupColumn("##Action", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableHeadersRow();

                for (const auto& [mod_name, procs] : grouped_data)
                {
                    ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextColored(get_gui_color(gui_color_id::log_info), "[%s]", mod_name.c_str());

                    for (const auto& pdi : procs)
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Indent();
                        ImGui::TextUnformatted(pdi.type_name.c_str());
                        ImGui::Unindent();

                        ImGui::TableSetColumnIndex(1);
                        ImGui::PushID((const void*)(intptr_t)pdi.proc_hash);
                        auto& name_buffer = new_proc_names[pdi.proc_hash];
                        ImGui::SetNextItemWidth(-1.0f);
                        ImGui::InputText("##NewName", name_buffer.data(), name_buffer.size());

                        ImGui::TableSetColumnIndex(2);
                        bool has_name = name_buffer[0] != '\0';
                        
                        if (has_name)
                        {
                            adam::string_hash proposed_hash = adam::string_hashed(name_buffer.data()).get_hash();
                            if (ctrl.commander().registry().get_processors().find(proposed_hash) != ctrl.commander().registry().get_processors().end())
                            {
                                has_name = false;
                            }
                        }
                        
                        if (!has_name)
                        {
                            ImGui::BeginDisabled();
                        }
                        
                        if (ImGui::Button(get_gui_string(gui_string_id::btn_create, lang)))
                        {
                            adam::string_hashed new_name(name_buffer.data());
                            adam::string_hash type_hash = pdi.proc_hash;
                            adam::string_hash mod_hash = pdi.module_hash;
                            adam::string_hash conn_hash = g_target_connection.get_hash();
                            adam::string_hash new_proc_hash = adam::string_hashed(name_buffer.data()).get_hash();
                            bool is_filter = pdi.is_filter;
                            
                            ctrl.enqueue_commander_action([&ctrl, new_name, type_hash, mod_hash, conn_hash, new_proc_hash, is_filter]() { ctrl.commander().request_processor_create(new_name, type_hash, mod_hash, is_filter); ctrl.commander().request_connection_processor_add(conn_hash, new_proc_hash); });

                            name_buffer[0] = '\0';
                        }
                        
                        if (!has_name)
                        {
                            ImGui::EndDisabled();
                        }
                        ImGui::PopID();
                    }
                }
                ImGui::EndTable();
            }
        };

        float spacing = ImGui::GetStyle().ItemSpacing.x;
        float half_h = (child_h - ImGui::GetStyle().ItemSpacing.y * 2.0f) * 0.5f;

        // Draw Filters (same type, same child)
        if (ImGui::BeginChild("FiltersChild", ImVec2(0.0f, half_h), true, ImGuiWindowFlags_NoScrollbar))
        {
            float avail_w = ImGui::GetContentRegionAvail().x;
            float col_w = (avail_w - spacing * 2.0f - 12.0f * dpi_scale) * 0.5f;
            float title_h = ImGui::GetTextLineHeightWithSpacing();
            float table_h = ImGui::GetContentRegionAvail().y - title_h - ImGui::GetStyle().ItemSpacing.y * 2.0f - 16.0f * dpi_scale;

            // Left: Existing Filters
            ImGui::BeginGroup();
            draw_existing_table_content(get_gui_string(gui_string_id::lbl_existing_filters, lang), existing_filters, col_w, table_h);
            ImGui::EndGroup();

            ImGui::SameLine();

            // Vertical Separator
            ImVec2 p = ImGui::GetCursorScreenPos();
            ImGui::GetWindowDrawList()->AddLine(p, ImVec2(p.x, p.y + table_h + title_h), ImGui::GetColorU32(ImGuiCol_Separator));
            ImGui::Dummy(ImVec2(spacing, 0.0f));
            ImGui::SameLine();

            // Right: New Filters
            ImGui::BeginGroup();
            draw_new_table_content(get_gui_string(gui_string_id::lbl_new_filters, lang), new_filters, col_w, table_h);
            ImGui::EndGroup();
        }
        ImGui::EndChild();

        ImGui::Spacing();

        // Draw Converters (same type, same child)
        if (ImGui::BeginChild("ConvertersChild", ImVec2(0.0f, half_h), true, ImGuiWindowFlags_NoScrollbar))
        {
            float avail_w = ImGui::GetContentRegionAvail().x;
            float col_w = (avail_w - spacing * 2.0f - 12.0f * dpi_scale) * 0.5f;
            float title_h = ImGui::GetTextLineHeightWithSpacing();
            float table_h = ImGui::GetContentRegionAvail().y - title_h - ImGui::GetStyle().ItemSpacing.y * 2.0f - 16.0f * dpi_scale;

            // Left: Existing Converters
            ImGui::BeginGroup();
            draw_existing_table_content(get_gui_string(gui_string_id::lbl_existing_converters, lang), existing_converters, col_w, table_h);
            ImGui::EndGroup();

            ImGui::SameLine();

            // Vertical Separator
            ImVec2 p = ImGui::GetCursorScreenPos();
            ImGui::GetWindowDrawList()->AddLine(p, ImVec2(p.x, p.y + table_h + title_h), ImGui::GetColorU32(ImGuiCol_Separator));
            ImGui::Dummy(ImVec2(spacing, 0.0f));
            ImGui::SameLine();

            // Right: New Converters
            ImGui::BeginGroup();
            draw_new_table_content(get_gui_string(gui_string_id::lbl_new_converters, lang), new_converters, col_w, table_h);
            ImGui::EndGroup();
        }
        ImGui::EndChild();

        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        if (ImGui::Button(get_gui_string(gui_string_id::btn_cancel, lang), ImVec2(-1.0f, 0.0f)))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}
