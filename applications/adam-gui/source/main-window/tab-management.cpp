#include "tab-management.hpp"
#include "main-window.hpp"

#include <imgui.h>
#include <algorithm>
#include <vector>
#include <mutex>
#include <string>
#include <map>
#include <functional>
#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "configuration/parameters/configuration-parameter-string.hpp"

namespace adam::gui 
{
    namespace
    {
        adam::string_hashed g_connection_to_delete("");
        bool g_request_delete_popup = false;
        ImVec2 g_connection_drag_offset(0, 0);

        adam::string_hashed g_target_connection("");
        adam::port_direction g_target_direction = adam::port_direction::none;
        bool g_request_port_popup = false;
        char g_port_popup_title[128] = "Add Port###PortPopup";

        adam::string_hash g_active_drag_hash = 0;
        size_t g_active_drag_target_index = 0;

        std::vector<uint64_t> g_expanded_nodes;
        
        std::vector<std::function<void()>> g_deferred_actions;

        struct connection_pin_data {
            ImVec2 pos;
            ImColor col;
        };

        std::vector<std::vector<connection_pin_data>> g_stage_pins_in_normal, g_stage_pins_out_normal;
        std::vector<std::vector<connection_pin_data>> g_stage_pins_in_preview, g_stage_pins_out_preview;
    }

    void render_delete_connection_modal(gui_controller& ctrl, adam::language lang)
    {
        float dpi_scale = ImGui::GetStyle()._MainScale;
        if (g_request_delete_popup)
        {
            ImGui::OpenPopup(get_gui_string(gui_string_id::dlg_delete_connection, lang));
            g_request_delete_popup = false;
        }

        if (ImGui::BeginPopupModal(get_gui_string(gui_string_id::dlg_delete_connection, lang), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::TextUnformatted(get_gui_string(gui_string_id::msg_delete_connection_confirm, lang));
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button(get_gui_string(gui_string_id::btn_ok, lang), ImVec2(120.0f * dpi_scale, 0.0f)))
            {
                if (g_connection_to_delete.get_hash() != 0)
                {
                    adam::string_hash h = g_connection_to_delete.get_hash();
                    g_deferred_actions.push_back([&ctrl, h]() { ctrl.commander().request_connection_destroy(h); });
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
    }

    void render_create_connection_modal(gui_controller& ctrl, adam::language lang)
    {
        float dpi_scale = ImGui::GetStyle()._MainScale;
        if (ImGui::BeginPopupModal(get_gui_string(gui_string_id::dlg_create_connection, lang), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            static char conn_name[256] = "";
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
                    g_deferred_actions.push_back([&ctrl, new_name]() { ctrl.commander().request_connection_create(new_name); });
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
    }

    void render_add_create_port_modal(gui_controller& ctrl, adam::language lang)
    {
        float dpi_scale = ImGui::GetStyle()._MainScale;
        struct port_display_info
        {
            std::string module_name;
            adam::string_hash module_hash;
            std::string port_name;
            adam::string_hash port_hash;
            adam::port_direction direction;
            std::string type_name;
            bool is_unavailable = false;
        };

        static std::map<std::string, std::vector<port_display_info>> existing_grouped;
        static std::map<std::string, std::vector<port_display_info>> new_grouped;
        static std::map<adam::string_hash, port_display_info> known_port_types;
        static std::map<adam::string_hash, std::array<char, 128>> new_port_names;
        static std::vector<adam::string_hash> used_ports;

        if (g_request_port_popup)
        {
            if (g_target_direction == adam::port_direction::input)
            {
                snprintf(g_port_popup_title, sizeof(g_port_popup_title), get_gui_string(gui_string_id::dlg_add_input_port_to, lang), g_target_connection.c_str());
            }
            else if (g_target_direction == adam::port_direction::output)
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
            used_ports.clear();

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
                    if (g_target_direction == adam::port_direction::input)
                    {
                        used_ports = conn_it->second->inputs;
                    }
                    else if (g_target_direction == adam::port_direction::output)
                    {
                        used_ports = conn_it->second->outputs;
                    }
                }

                for (const auto& [mod_hash, mod_info] : db)
                {
                    bool is_loaded = loaded_modules.find(mod_hash) != loaded_modules.end();

                    for (const auto& p : mod_info.ports)
                    {
                        port_display_info pdi;
                        pdi.module_hash = mod_hash.get_hash();
                        pdi.module_name = mod_info.name.c_str();
                        if (!p.type_name_str.empty())
                        {
                            pdi.port_name = p.type_name_str;
                            pdi.type_name = p.type_name_str;
                        }
                        else
                        {
                            char hash_buf[32];
                            snprintf(hash_buf, sizeof(hash_buf), "0x%llx", static_cast<unsigned long long>(p.name_hash));
                            pdi.port_name = std::string("Unknown (Hash: ") + hash_buf + ")";
                            pdi.type_name = pdi.port_name;
                        }
                        pdi.port_hash = p.name_hash;
                        pdi.direction = p.direction;
                        pdi.is_unavailable = false;

                        known_port_types[p.name_hash] = pdi;

                        if (is_loaded)
                        {
                            if (g_target_direction == adam::port_direction::none || (pdi.direction & g_target_direction) != adam::port_direction::none)
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
                    pdi.direction = adam::port_direction::in_out;
                    pdi.is_unavailable = false;

                    if (g_target_direction == adam::port_direction::none || (pdi.direction & g_target_direction) != adam::port_direction::none)
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

                    if (g_target_direction == adam::port_direction::none || pdi.direction == adam::port_direction::none || (pdi.direction & g_target_direction) != adam::port_direction::none)
                    {
                        existing_grouped[pdi.module_name].push_back(pdi);
                    }
                }
            }

            ImGui::OpenPopup(g_port_popup_title);
            g_request_port_popup = false;
        }

        ImGui::SetNextWindowSize(ImVec2(1200.0f * dpi_scale, 600.0f * dpi_scale), ImGuiCond_Appearing);
        if (ImGui::BeginPopupModal(g_port_popup_title, nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
        {
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
                            bool is_used = std::find(used_ports.begin(), used_ports.end(), pdi.port_hash) != used_ports.end();

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
                            if ((pdi.direction & adam::port_direction::input) != adam::port_direction::none && (pdi.direction & adam::port_direction::output) != adam::port_direction::none)
                            {
                                ImVec4 in_col = get_gui_color(gui_color_id::node_input);
                                ImVec4 out_col = get_gui_color(gui_color_id::node_output);
                                if (is_used) 
                                {
                                    in_col.w *= 0.5f;
                                    out_col.w *= 0.5f;
                                }
                                ImGui::TextColored(in_col, "%s", get_gui_string(gui_string_id::lbl_badge_in_short, lang));
                                ImGui::SameLine(0, 0);
                                ImGui::TextUnformatted("/");
                                ImGui::SameLine(0, 0);
                                ImGui::TextColored(out_col, "%s", get_gui_string(gui_string_id::lbl_badge_out_short, lang));
                            }
                            else if ((pdi.direction & adam::port_direction::input) != adam::port_direction::none)
                            {
                                ImVec4 col = get_gui_color(gui_color_id::node_input);
                                if (is_used) 
                                {
                                    col.w *= 0.5f;
                                }
                                ImGui::TextColored(col, "%s", get_gui_string(gui_string_id::lbl_badge_input, lang));
                            }
                            else if ((pdi.direction & adam::port_direction::output) != adam::port_direction::none)
                            {
                                ImVec4 col = get_gui_color(gui_color_id::node_output);
                                if (is_used) 
                                {
                                    col.w *= 0.5f;
                                }
                                ImGui::TextColored(col, "%s", get_gui_string(gui_string_id::lbl_badge_output, lang));
                            }
                            else
                            {
                                ImGui::TextDisabled("Unknown");
                            }

                            ImGui::TableSetColumnIndex(2);
                            ImGui::TextUnformatted(pdi.port_name.c_str());

                            ImGui::TableSetColumnIndex(3);
                            ImGui::PushID(static_cast<int>(pdi.port_hash));
                            if (ImGui::Button(get_gui_string(gui_string_id::btn_add_path, lang)))
                            {
                                adam::string_hash conn_hash = g_target_connection.get_hash();
                                adam::string_hash port_hash = pdi.port_hash;
                                bool is_input = g_target_direction == adam::port_direction::input;
                                g_deferred_actions.push_back([&ctrl, conn_hash, port_hash, is_input]() { ctrl.commander().request_connection_port_add(conn_hash, port_hash, is_input); });
                                
                                used_ports.push_back(pdi.port_hash);
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
                            ImGui::TextUnformatted(pdi.port_name.c_str());
                            ImGui::Unindent();

                            ImGui::TableSetColumnIndex(1);
                            if ((pdi.direction & adam::port_direction::input) != adam::port_direction::none && (pdi.direction & adam::port_direction::output) != adam::port_direction::none)
                            {
                                ImGui::TextColored(get_gui_color(gui_color_id::node_input), "%s", get_gui_string(gui_string_id::lbl_badge_in_short, lang));
                                ImGui::SameLine(0, 0);
                                ImGui::TextUnformatted("/");
                                ImGui::SameLine(0, 0);
                                ImGui::TextColored(get_gui_color(gui_color_id::node_output), "%s", get_gui_string(gui_string_id::lbl_badge_out_short, lang));
                            }
                            else if ((pdi.direction & adam::port_direction::input) != adam::port_direction::none)
                            {
                                ImGui::TextColored(get_gui_color(gui_color_id::node_input), "%s", get_gui_string(gui_string_id::lbl_badge_input, lang));
                            }
                            else if ((pdi.direction & adam::port_direction::output) != adam::port_direction::none)
                            {
                                ImGui::TextColored(get_gui_color(gui_color_id::node_output), "%s", get_gui_string(gui_string_id::lbl_badge_output, lang));
                            }
                            else
                            {
                                ImGui::TextDisabled("Unknown");
                            }

                            ImGui::TableSetColumnIndex(2);
                            ImGui::PushID(static_cast<int>(pdi.port_hash));
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
                                bool is_input = g_target_direction == adam::port_direction::input;
                                adam::string_hash new_port_hash = adam::string_hashed(name_buffer.data()).get_hash();
                                
                                g_deferred_actions.push_back([&ctrl, new_name, type_hash, mod_hash, conn_hash, new_port_hash, is_input]() { ctrl.commander().request_port_create(new_name, type_hash, mod_hash); ctrl.commander().request_connection_port_add(conn_hash, new_port_hash, is_input); });

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
        // --- END: Add/Create Port Modal ---
    }

    void render_top_control_bar(gui_controller& ctrl, adam::language lang, int& sort_mode)
    {
        float dpi_scale = ImGui::GetStyle()._MainScale;
        // --- START: Top Control Bar (Sort) ---
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_sort_by, lang));
        ImGui::SameLine();
        const char* sort_options[] = {
            get_gui_string(gui_string_id::sort_name_asc, lang),
            get_gui_string(gui_string_id::sort_name_desc, lang),
            get_gui_string(gui_string_id::sort_date_asc, lang),
            get_gui_string(gui_string_id::sort_date_desc, lang),
            get_gui_string(gui_string_id::sort_edited_asc, lang),
            get_gui_string(gui_string_id::sort_edited_desc, lang),
            get_gui_string(gui_string_id::sort_custom, lang)
        };
        ImGui::SetNextItemWidth(200.0f * dpi_scale);
        if (ImGui::Combo("##SortMode", &sort_mode, sort_options, IM_ARRAYSIZE(sort_options)))
        {
            auto* sort_mode_param = dynamic_cast<adam::configuration_parameter_integer*>(ctrl.get_parameters().get("connection_sort_mode"_ct));
            if (sort_mode_param)
                sort_mode_param->set_value(static_cast<int64_t>(sort_mode));
        }
    }

    void render_connection_card(gui_controller& ctrl, adam::language lang, int sort_mode, adam::string_hash hash, adam::connection_view* conn, bool is_drag_preview, float card_w)
    {
        bool commander_active = ctrl.is_commander_active();
        float dpi_scale = ImGui::GetStyle()._MainScale;
        auto& reg_view = ctrl.commander().registry();
        const auto& connections = reg_view.get_connections();
        const auto& ports = reg_view.get_ports();

        auto* theme_param = dynamic_cast<adam::configuration_parameter_string*>(ctrl.get_parameters().get("theme"_ct));
        bool is_light_theme = theme_param && theme_param->get_value() == "default-light"_ct;

        auto& stage_pins_in = is_drag_preview ? g_stage_pins_in_preview : g_stage_pins_in_normal;
        auto& stage_pins_out = is_drag_preview ? g_stage_pins_out_preview : g_stage_pins_out_normal;

        ImGui::PushID(is_drag_preview ? static_cast<int>(hash) + 0x1000000 : static_cast<int>(hash));
            
        bool is_being_dragged = false;
        if (!is_drag_preview && sort_mode == 6) 
        {
            if (const ImGuiPayload* payload = ImGui::GetDragDropPayload()) 
            {
                if (payload->IsDataType("DND_CONNECTION") && *(adam::string_hash*)payload->Data == hash) 
                {
                    is_being_dragged = true;
                }
            }
        }

        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f * dpi_scale);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f * dpi_scale, 8.0f * dpi_scale));
            
        ImVec4 bg = is_drag_preview ? ImVec4(0.2f, 0.2f, 0.2f, 0.9f) : ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
            
        if (conn->color != 0xFFFFFF && conn->color != 0)
        {
            bg.x = ((conn->color >> 16) & 0xFF) / 255.0f * 0.35f + bg.x * 0.65f;
            bg.y = ((conn->color >> 8) & 0xFF) / 255.0f * 0.35f + bg.y * 0.65f;
            bg.z = (conn->color & 0xFF) / 255.0f * 0.35f + bg.z * 0.65f;
        }
            
        if (is_being_dragged)
        {
            bg.w *= 0.4f; // Ghost transparency
        }
            
        ImGui::PushStyleColor(ImGuiCol_ChildBg, bg);

        size_t max_rows = std::max(conn->inputs.size(), conn->outputs.size());
        size_t num_processors = conn->filters.size() + conn->converters.size();
        if (max_rows == 0 && num_processors > 0) max_rows = 1;
            
        float node_h = ImGui::GetTextLineHeight() * 2.0f;
        float row_height = node_h + 10.0f * dpi_scale;
            
        float base_height = ImGui::GetFrameHeight() * 2.0f + ImGui::GetTextLineHeight() + ImGui::GetStyle().ItemSpacing.y * 2.0f + ImGui::GetStyle().WindowPadding.y * 2.0f;
        if (max_rows == 0)
            base_height -= ImGui::GetTextLineHeight();
        float child_height = base_height + static_cast<float>(max_rows) * row_height;

        int total_stages = 2 + static_cast<int>(num_processors);

        float expanded_h = ImGui::GetTextLineHeight() + ImGui::GetFrameHeight() + 24.0f * dpi_scale;
        float max_expansion_h = 0.0f;

        if (!is_drag_preview)
        {
            float current_in_exp = 0.0f;
            for (auto pid : conn->inputs) {
                uint64_t uid = static_cast<uint64_t>(pid ^ hash ^ (0 << 16));
                if (std::find(g_expanded_nodes.begin(), g_expanded_nodes.end(), uid) != g_expanded_nodes.end()) current_in_exp += expanded_h;
            }
            max_expansion_h = std::max(max_expansion_h, current_in_exp);

            float current_out_exp = 0.0f;
            for (auto pid : conn->outputs) {
                uint64_t uid = static_cast<uint64_t>(pid ^ hash ^ ((total_stages - 1) << 16));
                if (std::find(g_expanded_nodes.begin(), g_expanded_nodes.end(), uid) != g_expanded_nodes.end()) current_out_exp += expanded_h;
            }
            max_expansion_h = std::max(max_expansion_h, current_out_exp);
        }
        child_height += max_expansion_h;

        if (!is_drag_preview) ImGui::BeginGroup();

        std::vector<std::function<void()>> deferred_expansions;

        if (is_being_dragged) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.4f);

        if (ImGui::BeginChild(is_drag_preview ? "ConnCardPreview" : "ConnCard", ImVec2(card_w, child_height), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
        {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            float avail_x = ImGui::GetContentRegionAvail().x;
                
            float char_width = ImGui::CalcTextSize("x").x;
            float desired_node_w = char_width * 40.0f + ImGui::GetStyle().FramePadding.x * 4.0f;
                
            float port_w = std::min(desired_node_w, avail_x * 0.25f);
            if (port_w < char_width * 15.0f) port_w = char_width * 15.0f;
                
            float proc_w = port_w;
            bool compact_processors = false;
                
            if (total_stages > 2)
            {
                float gap_if_large = (avail_x - static_cast<float>(total_stages) * port_w) / static_cast<float>(total_stages - 1);
                if (gap_if_large < 10.0f * dpi_scale)
                {
                    proc_w = char_width * 5.0f + ImGui::GetStyle().FramePadding.x * 4.0f;
                    compact_processors = true;
                        
                    float gap_if_compact = (avail_x - 2.0f * port_w - static_cast<float>(num_processors) * proc_w) / static_cast<float>(total_stages - 1);
                    if (gap_if_compact < 10.0f * dpi_scale)
                    {
                        float available_for_procs = avail_x - 2.0f * port_w - static_cast<float>(total_stages - 1) * 10.0f * dpi_scale;
                        if (available_for_procs > 0.0f)
                            proc_w = available_for_procs / static_cast<float>(num_processors);
                        else
                            proc_w = 4.0f * dpi_scale; 
                    }
                }
            }

            ImGui::AlignTextToFramePadding();

            char name_buf[max_name_length];
            std::strncpy(name_buf, conn->name.c_str(), sizeof(name_buf));
            name_buf[sizeof(name_buf) - 1] = '\0';

            ImGui::PushID("conn_name_edit");
            bool enter_pressed = false;
            bool deactivated = false;
            if (!is_drag_preview)
            {
                ImGui::SetNextItemWidth(port_w);
                enter_pressed = ImGui::InputText("##edit", name_buf, sizeof(name_buf), ImGuiInputTextFlags_EnterReturnsTrue);
                deactivated = ImGui::IsItemDeactivatedAfterEdit();
            }
            else
            {
                ImGui::TextUnformatted(name_buf);
            }
            ImGui::PopID();

            if (enter_pressed || deactivated)
            {
                if (name_buf[0] != '\0' && conn->name != string_hashed(&name_buf[0]))
                {
                    adam::string_hash proposed_hash = adam::string_hashed(&name_buf[0]).get_hash();
                    if (connections.find(proposed_hash) == connections.end())
                    {
                        adam::string_hash old_hash = conn->name.get_hash();
                        adam::string_hashed new_name(&name_buf[0]);
                        g_deferred_actions.push_back([&ctrl, old_hash, new_name]() { ctrl.commander().request_connection_rename(old_hash, new_name); });
                    }
                }
            }

            ImGui::SameLine();

            ImVec4 conn_color_vec;
            if (conn->color == 0)
            {
                conn_color_vec = is_drag_preview ? ImVec4(0.2f, 0.2f, 0.2f, 1.0f) : ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
            }
            else
            {
                conn_color_vec.x = ((conn->color >> 16) & 0xFF) / 255.0f;
                conn_color_vec.y = ((conn->color >> 8) & 0xFF) / 255.0f;
                conn_color_vec.z = (conn->color & 0xFF) / 255.0f;
                conn_color_vec.w = 1.0f;
            }

            if (!is_drag_preview)
            {
                ImGui::PushID("conn_color_edit");
                    
                if (ImGui::ColorButton("##color_btn", conn_color_vec, ImGuiColorEditFlags_NoTooltip))
                {
                    ImGui::OpenPopup("ColorPickerPopup");
                }
                if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                {
                    conn->color = 0;
                    if (commander_active)
                    {
                        adam::string_hash h = conn->name.get_hash();
                        g_deferred_actions.push_back([&ctrl, h]() { ctrl.commander().request_connection_color_change(h, 0); });
                    }
                }
                    
                if (ImGui::BeginPopup("ColorPickerPopup"))
                {
                    float picker_col[3] = { conn_color_vec.x, conn_color_vec.y, conn_color_vec.z };
                        
                    if (conn->color == 0) 
                    {
                        picker_col[0] = 1.0f; picker_col[1] = 1.0f; picker_col[2] = 1.0f;
                    }

                    if (ImGui::ColorPicker3("##picker", picker_col, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview))
                    {
                        uint32_t new_color = (static_cast<uint32_t>(picker_col[0] * 255.0f + 0.5f) << 16) |
                                             (static_cast<uint32_t>(picker_col[1] * 255.0f + 0.5f) << 8) |
                                             (static_cast<uint32_t>(picker_col[2] * 255.0f + 0.5f));
                        if (new_color == 0) new_color = 0x000001; 
                        conn->color = new_color;
                    }
                        
                    if (ImGui::IsItemDeactivatedAfterEdit())
                    {
                        if (commander_active)
                        {
                            adam::string_hash h = conn->name.get_hash();
                            uint32_t c = conn->color;
                            g_deferred_actions.push_back([&ctrl, h, c]() { ctrl.commander().request_connection_color_change(h, c); });
                        }
                    }
                        
                    ImGui::EndPopup();
                }
                ImGui::PopID();
            }
            else
            {
                ImGui::ColorButton("##color_preview", conn_color_vec, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop);
            }

            const char* btn_start_str = get_gui_string(gui_string_id::btn_start, lang);
            const char* btn_stop_str  = get_gui_string(gui_string_id::btn_stop, lang);
                
            ImGui::SameLine();

            bool is_active = conn->is_active;
            if (is_active) ImGui::BeginDisabled();
            if (ImGui::Button(btn_start_str) && !is_drag_preview)
            {
                adam::string_hash h = conn->name.get_hash();
                g_deferred_actions.push_back([&ctrl, h]() { ctrl.commander().request_connection_start(h); });
            }
            if (is_active) ImGui::EndDisabled();
                
            ImGui::SameLine();
            if (!is_active) ImGui::BeginDisabled();
            if (ImGui::Button(btn_stop_str) && !is_drag_preview)
            {
                adam::string_hash h = conn->name.get_hash();
                g_deferred_actions.push_back([&ctrl, h]() { ctrl.commander().request_connection_stop(h); });
            }
            if (!is_active) ImGui::EndDisabled();

            const char* btn_delete_str = get_gui_string(gui_string_id::btn_delete, lang);
            float btn_delete_w = ImGui::CalcTextSize(btn_delete_str).x + ImGui::GetStyle().FramePadding.x * 2.0f;
                
            ImGui::SameLine(ImGui::GetWindowWidth() - btn_delete_w - ImGui::GetStyle().WindowPadding.x);
                
            if (ImGui::Button(btn_delete_str) && !is_drag_preview)
            {
                if (conn->inputs.empty() && conn->outputs.empty() && conn->filters.empty())
                {
                    adam::string_hash h = conn->name.get_hash();
                    g_deferred_actions.push_back([&ctrl, h]() { ctrl.commander().request_connection_destroy(h); });
                }
                else
                {
                    g_connection_to_delete = conn->name;
                    g_request_delete_popup = true;
                }
            }

            if (sort_mode == 6 && !is_drag_preview)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_Separator));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_SeparatorHovered));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_SeparatorActive));
                ImGui::Button("##drag_handle", ImVec2(-1.0f, 4.0f * dpi_scale));
                ImGui::PopStyleColor(3);

                if (ImGui::IsItemActivated())
                {
                    ImVec2 mouse_pos = ImGui::GetMousePos();
                    ImVec2 win_pos = ImGui::GetWindowPos(); // The top-left of the card child
                    g_connection_drag_offset = ImVec2(mouse_pos.x - win_pos.x, mouse_pos.y - win_pos.y);
                }

                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID | ImGuiDragDropFlags_SourceNoPreviewTooltip))
                {
                    ImGui::SetDragDropPayload("DND_CONNECTION", &hash, sizeof(adam::string_hash));
                    ImGui::EndDragDropSource();
                }
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
                }
            }
            else
            {
                ImGui::Separator();
            }

            ImVec2 cur_pos = ImGui::GetCursorScreenPos();
                
            float total_node_widths = 2.0f * port_w + static_cast<float>(std::max(0, total_stages - 2)) * proc_w;
            float gap = (total_stages > 1) ? (avail_x - total_node_widths) / static_cast<float>(total_stages - 1) : 0.0f;
                
            // --- START: Node Renderer ---
            auto draw_node = [&](const char* name, int stage, float row, ImColor color, connection_pin_data& out_pin_in, connection_pin_data& out_pin_out, bool is_unavail = false, const char* unavail_module = nullptr, adam::string_hash port_hash = 0, float extra_y = 0.0f)
            {
                float current_node_w = (stage == 0 || stage == total_stages - 1) ? port_w : proc_w;
                float node_x;
                if (total_stages == 1) node_x = cur_pos.x;
                else if (stage == 0) node_x = cur_pos.x;
                else if (stage == total_stages - 1) node_x = cur_pos.x + avail_x - current_node_w;
                else node_x = cur_pos.x + port_w + gap + static_cast<float>(stage - 1) * (proc_w + gap);
                    
                float node_y = cur_pos.y + row * row_height + (row_height - node_h) * 0.5f + extra_y;
                    
                ImVec2 p_min(node_x, node_y);
                ImVec2 p_max(node_x + current_node_w, node_y + node_h);
                    
                draw_list->AddRectFilled(p_min, p_max, color, 6.0f * dpi_scale);
                draw_list->AddRect(p_min, p_max, ImColor(color.Value.x * 1.2f, color.Value.y * 1.2f, color.Value.z * 1.2f), 6.0f * dpi_scale, 0, 1.5f * dpi_scale);

                ImGui::SetCursorScreenPos(p_min);
                ImGui::PushID(static_cast<int>(port_hash ^ hash ^ (stage << 16) ^ 0xABCD));
                ImGui::SetNextItemAllowOverlap();
                ImGui::InvisibleButton("##node_btn", ImVec2(current_node_w, node_h));
                
                bool is_expanded = false;
                if (port_hash != 0 && !is_drag_preview)
                {
                    uint64_t unique_node_id = static_cast<uint64_t>(port_hash ^ hash ^ (stage << 16));
                    auto exp_it = std::find(g_expanded_nodes.begin(), g_expanded_nodes.end(), unique_node_id);
                    is_expanded = (exp_it != g_expanded_nodes.end());

                    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                    {
                        if (is_expanded)
                            g_expanded_nodes.erase(exp_it);
                        else
                            g_expanded_nodes.push_back(unique_node_id);
                        is_expanded = !is_expanded;
                    }
                }
                ImGui::PopID();
                
                if (is_expanded && !is_drag_preview)
                {
                    ImColor captured_color = color;
                    deferred_expansions.push_back([&ctrl, lang, port_hash, p_max, current_node_w, captured_color, &ports, dpi_scale, draw_list]() 
                    {
                        float exp_pad = 8.0f * dpi_scale;
                        float expanded_h = ImGui::GetTextLineHeight() + ImGui::GetFrameHeight() + exp_pad * 3.0f;
                        ImVec2 exp_min(p_max.x - current_node_w, p_max.y - 1.5f * dpi_scale);
                        ImVec2 exp_max(p_max.x, p_max.y + expanded_h);

                        ImVec4 bg_col = ImGui::GetStyleColorVec4(ImGuiCol_PopupBg);
                        bg_col.w = 1.0f;
                        draw_list->AddRectFilled(exp_min, exp_max, ImColor(bg_col), 6.0f * dpi_scale, ImDrawFlags_RoundCornersBottom);
                        draw_list->AddRect(exp_min, exp_max, ImColor(captured_color.Value.x * 1.2f, captured_color.Value.y * 1.2f, captured_color.Value.z * 1.2f), 6.0f * dpi_scale, ImDrawFlags_RoundCornersBottom, 1.5f * dpi_scale);

                        bool p_is_active = false;
                        std::string p_type = "Unknown";
                        std::string p_module = "Unknown";
                        auto p_it = ports.find(port_hash);
                        if (p_it != ports.end())
                        {
                            p_is_active = p_it->second->is_active;
                            if (p_it->second->type.get_hash() == ("internal"_ct).get_hash())
                            {
                                p_type = "internal";
                                p_module = "Internal";
                            }
                            else
                            {
                                p_type = p_it->second->type.c_str();
                                if (p_it->second->type_module.get_hash() != 0)
                                    p_module = p_it->second->type_module.c_str();
                                else
                                    p_module = "Unknown Module";
                            }
                        }

                        ImGui::SetCursorScreenPos(ImVec2(exp_min.x + exp_pad, exp_min.y + exp_pad));
                        ImGui::BeginGroup();
                        
                        ImGui::TextColored(get_gui_color(gui_color_id::log_info), "%s [%s]", p_type.c_str(), p_module.c_str());

                        float buttons_y = exp_max.y - ImGui::GetFrameHeight() - exp_pad;
                        ImGui::SetCursorScreenPos(ImVec2(exp_min.x + exp_pad, buttons_y));
                        
                        float btn_w = (current_node_w - exp_pad * 3.0f) * 0.5f;

                        ImGui::PushID(static_cast<int>(port_hash ^ 0x1111));
                        if (p_is_active) ImGui::BeginDisabled();
                        if (ImGui::Button(get_gui_string(gui_string_id::btn_start, lang), ImVec2(btn_w, 0)))
                            g_deferred_actions.push_back([&ctrl, port_hash]() { ctrl.commander().request_port_start(port_hash); });
                        if (p_is_active) ImGui::EndDisabled();
                        ImGui::PopID();

                        ImGui::SameLine(0.0f, exp_pad);

                        ImGui::PushID(static_cast<int>(port_hash ^ 0x2222));
                        if (!p_is_active) ImGui::BeginDisabled();
                        if (ImGui::Button(get_gui_string(gui_string_id::btn_stop, lang), ImVec2(btn_w, 0)))
                            g_deferred_actions.push_back([&ctrl, port_hash]() { ctrl.commander().request_port_stop(port_hash); });
                        if (!p_is_active) ImGui::EndDisabled();
                        ImGui::PopID();

                        ImGui::EndGroup();
                    });
                }
                    
                // --- START: Node Name Editor ---
                if (port_hash != 0 && !is_drag_preview)
                {
                    char name_buf[max_name_length];
                    std::strncpy(name_buf, name, sizeof(name_buf));
                    name_buf[sizeof(name_buf) - 1] = '\0';
                        
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
                        
                    float text_width = ImGui::CalcTextSize(name_buf).x;
                    float input_w = text_width + ImGui::GetStyle().FramePadding.x * 2.0f + 8.0f * dpi_scale;
                    if (input_w > current_node_w - 16.0f * dpi_scale) 
                        input_w = current_node_w - 16.0f * dpi_scale;
                    if (input_w < 32.0f * dpi_scale)
                        input_w = 32.0f * dpi_scale;
                            
                    float input_x = p_min.x + (current_node_w - input_w) * 0.5f;
                    float input_h = ImGui::GetFrameHeight();
                        
                    ImGui::SetCursorScreenPos(ImVec2(input_x, p_min.y + (node_h - input_h) * 0.5f));
                    ImGui::PushItemWidth(input_w);
                        
                    ImGui::PushID(static_cast<int>(port_hash ^ hash ^ (stage << 16)));
                    bool enter_pressed = ImGui::InputText("##port_edit", name_buf, sizeof(name_buf), ImGuiInputTextFlags_EnterReturnsTrue);
                    bool deactivated = ImGui::IsItemDeactivatedAfterEdit();
                    ImGui::PopID();
                        
                    ImGui::PopItemWidth();
                    ImGui::PopStyleColor(3);
                        
                    if ((enter_pressed || deactivated) && name_buf[0] != '\0' && std::strcmp(name, name_buf) != 0)
                    {
                        adam::string_hash proposed_hash = adam::string_hashed(&name_buf[0]).get_hash();
                        if (ports.find(proposed_hash) == ports.end())
                        {
                            adam::string_hashed new_name(&name_buf[0]);
                            g_deferred_actions.push_back([&ctrl, port_hash, new_name]() { ctrl.commander().request_port_rename(port_hash, new_name); });
                        }
                    }
                }
                // --- END: Node Name Editor ---
                else
                {
                    float text_width = ImGui::CalcTextSize(name).x;
                    float text_x = p_min.x + (current_node_w - text_width) * 0.5f;
                    if (text_x < p_min.x + 8.0f * dpi_scale) text_x = p_min.x + 8.0f * dpi_scale;
                    ImVec2 text_pos(text_x, p_min.y + (node_h - ImGui::GetTextLineHeight()) * 0.5f);
                    draw_list->PushClipRect(p_min, p_max, true);
                    draw_list->AddText(text_pos, ImColor(255, 255, 255), name);
                    draw_list->PopClipRect();
                }

                out_pin_in.pos = ImVec2(p_min.x, p_min.y + node_h * 0.5f);
                out_pin_out.pos = ImVec2(p_max.x, p_min.y + node_h * 0.5f);

                bool is_port_active = false;
                if (port_hash != 0)
                {
                    auto p_it = ports.find(port_hash);
                    if (p_it != ports.end())
                        is_port_active = p_it->second->is_active;
                }

                ImColor pin_col = is_light_theme ? get_gui_color(gui_color_id::node_connection_line_light) : get_gui_color(gui_color_id::node_connection_line);
                if (is_port_active)
                {
                    pin_col = get_gui_color(gui_color_id::node_pin_active);
                }

                out_pin_in.col = pin_col;
                out_pin_out.col = pin_col;
                        
                if (is_unavail && !is_drag_preview && ImGui::IsMouseHoveringRect(p_min, p_max))
                {
                    ImGui::BeginTooltip();
                    ImGui::Text(get_gui_string(gui_string_id::tt_module_missing, lang), unavail_module ? unavail_module : "Unknown");
                    ImGui::EndTooltip();
                }
            };
            // --- END: Node Renderer ---

            if (stage_pins_in.size() < static_cast<size_t>(total_stages))
            {
                stage_pins_in.resize(total_stages);
                stage_pins_out.resize(total_stages);
            }
            for (int i = 0; i < total_stages; ++i)
            {
                stage_pins_in[i].clear();
                stage_pins_out[i].clear();
            }

            ImColor in_col = get_gui_color(gui_color_id::node_input);
            ImColor proc_col = get_gui_color(gui_color_id::node_processor);
            ImColor out_col = get_gui_color(gui_color_id::node_output);

            float in_offset = (static_cast<float>(max_rows) - static_cast<float>(conn->inputs.size())) * 0.5f;
            float out_offset = (static_cast<float>(max_rows) - static_cast<float>(conn->outputs.size())) * 0.5f;

            float row_val = in_offset;
            float current_expansion_y = 0.0f;
            for (auto pid : conn->inputs)
            {
                connection_pin_data p_in, p_out;
                auto it = ports.find(pid);
                bool is_unavail = (it != ports.end() && it->second->is_unavailable);
                ImColor col = in_col;
                if (is_unavail) col.Value.w *= 0.4f;

                const char* mod_name = "Unknown";
                if (is_unavail && it->second->type_module.get_hash() != 0)
                    mod_name = it->second->type_module.c_str();

                draw_node(it != ports.end() ? it->second->name.c_str() : "Unknown Input", 0, row_val, col, p_in, p_out, is_unavail, mod_name, pid, current_expansion_y);
                stage_pins_out[0].push_back(p_out);
                row_val += 1.0f;

                uint64_t uid = static_cast<uint64_t>(pid ^ hash ^ (0 << 16));
                if (std::find(g_expanded_nodes.begin(), g_expanded_nodes.end(), uid) != g_expanded_nodes.end())
                    current_expansion_y += expanded_h;
            }

            int current_stage = 1;
            int processor_idx = 1;
            float proc_row_val = (static_cast<float>(max_rows) - 1.0f) * 0.5f;
            float proc_extra_y = max_expansion_h * 0.5f;
            for (auto fid : conn->filters)
            {
                (void)fid; // Unused
                connection_pin_data p_in, p_out;
                if (compact_processors)
                {
                    char short_name[16];
                    snprintf(short_name, sizeof(short_name), "%02d   ", processor_idx);
                    draw_node(short_name, current_stage, proc_row_val, proc_col, p_in, p_out, false, nullptr, 0, proc_extra_y);
                }
                else
                {
                    draw_node("Filter", current_stage, proc_row_val, proc_col, p_in, p_out, false, nullptr, 0, proc_extra_y);
                }
                stage_pins_in[current_stage].push_back(p_in);
                stage_pins_out[current_stage].push_back(p_out);
                current_stage++;
                processor_idx++;
            }
            for (auto cid : conn->converters)
            {
                (void)cid; // Unused
                connection_pin_data p_in, p_out;
                if (compact_processors)
                {
                    char short_name[16];
                    snprintf(short_name, sizeof(short_name), "%02d   ", processor_idx);
                    draw_node(short_name, current_stage, proc_row_val, proc_col, p_in, p_out, false, nullptr, 0, proc_extra_y);
                }
                else
                {
                    draw_node("Converter", current_stage, proc_row_val, proc_col, p_in, p_out, false, nullptr, 0, proc_extra_y);
                }
                stage_pins_in[current_stage].push_back(p_in);
                stage_pins_out[current_stage].push_back(p_out);
                current_stage++;
                processor_idx++;
            }

            row_val = out_offset;
            current_expansion_y = 0.0f;
            for (auto pid : conn->outputs)
            {
                connection_pin_data p_in, p_out;
                auto it = ports.find(pid);
                bool is_unavail = (it != ports.end() && it->second->is_unavailable);
                ImColor col = out_col;
                if (is_unavail) col.Value.w *= 0.4f;

                const char* mod_name = "Unknown";
                if (is_unavail && it->second->type_module.get_hash() != 0)
                    mod_name = it->second->type_module.c_str();

                draw_node(it != ports.end() ? it->second->name.c_str() : "Unknown Output", total_stages - 1, row_val, col, p_in, p_out, is_unavail, mod_name, pid, current_expansion_y);
                stage_pins_in[total_stages - 1].push_back(p_in);
                row_val += 1.0f;

                uint64_t uid = static_cast<uint64_t>(pid ^ hash ^ ((total_stages - 1) << 16));
                if (std::find(g_expanded_nodes.begin(), g_expanded_nodes.end(), uid) != g_expanded_nodes.end())
                    current_expansion_y += expanded_h;
            }

            ImColor line_col = is_light_theme ? get_gui_color(gui_color_id::node_connection_line_light) : get_gui_color(gui_color_id::node_connection_line);
            float line_thickness = 5.f * dpi_scale;
                
            if (num_processors == 0 && !conn->inputs.empty() && !conn->outputs.empty() && (conn->inputs.size() > 1 || conn->outputs.size() > 1))
            {
                float center_y = cur_pos.y + static_cast<float>(max_rows) * row_height * 0.5f;
                float mid_x = cur_pos.x + avail_x * 0.5f;
                    
                ImVec2 p_mid(mid_x, center_y);
                    
                for (const auto& pin_out_data : stage_pins_out[0])
                {
                    ImVec2 pin_out = pin_out_data.pos;
                    float b_strength = (mid_x - pin_out.x) * 0.5f;
                    draw_list->AddBezierCubic(
                        pin_out, ImVec2(pin_out.x + b_strength, pin_out.y), ImVec2(mid_x - b_strength, center_y), p_mid,
                        line_col, line_thickness);
                }
                    
                for (const auto& pin_in_data : stage_pins_in[1])
                {
                    ImVec2 pin_in = pin_in_data.pos;
                    float b_strength = (pin_in.x - mid_x) * 0.5f;
                    draw_list->AddBezierCubic(
                        p_mid, ImVec2(mid_x + b_strength, center_y), ImVec2(pin_in.x - b_strength, pin_in.y), pin_in,
                        line_col, line_thickness);
                }
                    
                draw_list->AddCircleFilled(p_mid, line_thickness * 0.6f, line_col);
            }
            else
            {
                for (int s = 0; s < total_stages - 1; ++s)
                {
                    for (const auto& pin_out_data : stage_pins_out[s])
                    {
                        ImVec2 pin_out = pin_out_data.pos;
                        for (const auto& pin_in_data : stage_pins_in[s + 1])
                        {
                            ImVec2 pin_in = pin_in_data.pos;
                            float b_strength = (pin_in.x - pin_out.x) * 0.5f;
                            draw_list->AddBezierCubic(
                                pin_out, ImVec2(pin_out.x + b_strength, pin_out.y), ImVec2(pin_in.x - b_strength, pin_in.y), pin_in,
                                line_col, line_thickness);
                        }
                    }
                }
            }

            auto draw_pin_dot = [&](const connection_pin_data& pin) {
                draw_list->AddCircleFilled(pin.pos, 6.0f * dpi_scale, pin.col);
            };

            for (int s = 0; s < total_stages; ++s)
            {
                if (s > 0)
                {
                    for (const auto& pin_in : stage_pins_in[s]) draw_pin_dot(pin_in);
                }
                if (s < total_stages - 1)
                {
                    for (const auto& pin_out : stage_pins_out[s]) draw_pin_dot(pin_out);
                }
            }

            for (auto& fn : deferred_expansions)
            {
                fn();
            }

            ImGui::SetCursorScreenPos(ImVec2(cur_pos.x, cur_pos.y + static_cast<float>(max_rows) * row_height + max_expansion_h));
                
            if (max_rows > 0 || num_processors > 0)
            {
                ImGui::Spacing();
                ImGui::Separator();
            }
                
            float avail_w = ImGui::GetWindowWidth();
                
            float current_y = ImGui::GetCursorPosY();
            float start_x = ImGui::GetStyle().WindowPadding.x;

            // --- START: Add Node Buttons ---
            const char* btn_add_input_str = get_gui_string(gui_string_id::btn_add_input, lang);
            const char* btn_add_processor_str = get_gui_string(gui_string_id::btn_add_processor, lang);
            const char* btn_add_output_str = get_gui_string(gui_string_id::btn_add_output, lang);

            float add_in_w = ImGui::CalcTextSize(btn_add_input_str).x + ImGui::GetStyle().FramePadding.x * 2.0f;
            float in_x = start_x + port_w * 0.5f - add_in_w * 0.5f;
            ImGui::SetCursorPos(ImVec2(std::max(start_x, in_x), current_y));
            if (ImGui::Button(btn_add_input_str) && !is_drag_preview)
            {
                g_target_connection = conn->name;
                g_target_direction = adam::port_direction::input;
                g_request_port_popup = true;
            }

            if (!conn->inputs.empty())
            {
                float add_out_w = ImGui::CalcTextSize(btn_add_output_str).x + ImGui::GetStyle().FramePadding.x * 2.0f;
                float out_x = start_x + avail_x - port_w * 0.5f - add_out_w * 0.5f;
                ImGui::SetCursorPos(ImVec2(std::min(start_x + avail_x - add_out_w, out_x), current_y));
                if (ImGui::Button(btn_add_output_str) && !is_drag_preview)
                {
                    g_target_connection = conn->name;
                    g_target_direction = adam::port_direction::output;
                    g_request_port_popup = true;
                }

                if (!conn->outputs.empty())
                {
                    float add_proc_w = ImGui::CalcTextSize(btn_add_processor_str).x + ImGui::GetStyle().FramePadding.x * 2.0f;
                    float center_x = (avail_w - add_proc_w) * 0.5f;
                    ImGui::SetCursorPos(ImVec2(center_x, current_y));
                    ImGui::Button(btn_add_processor_str);
                }
            }
            // --- END: Add Node Buttons ---
        }
        ImGui::EndChild();

        if (is_being_dragged) ImGui::PopStyleVar();

        if (!is_drag_preview)
        {
            ImGui::Spacing();
            ImGui::EndGroup();
        }

        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
        ImGui::PopID();
    }

    void render_connections_list(gui_controller& ctrl, adam::language lang, int sort_mode, float& card_width, const std::vector<std::pair<adam::string_hash, adam::connection_view*>>& sorted_connections, bool is_dragging_connection)
    {
        if (ImGui::BeginChild("ConnectionsList", ImVec2(0, -(ImGui::GetFrameHeight() * 1.5f + ImGui::GetStyle().ItemSpacing.y)), false))
        {
            card_width = ImGui::GetContentRegionAvail().x;

            for (size_t i = 0; i < sorted_connections.size(); ++i)
            {
                auto hash = sorted_connections[i].first;
                auto* conn = sorted_connections[i].second;

                render_connection_card(ctrl, lang, sort_mode, hash, conn, false, card_width);

                if (sort_mode == 6 && is_dragging_connection)
                {
                    ImVec2 min = ImGui::GetItemRectMin();
                    ImVec2 max = ImGui::GetItemRectMax();
                    ImVec2 mouse_pos = ImGui::GetMousePos();
                    
                    if (mouse_pos.y >= min.y && mouse_pos.y <= max.y)
                    {
                        g_active_drag_target_index = i;
                    }
                }
            }
        }
        ImGui::EndChild();
    }

    void render_tab_management(gui_controller& ctrl, adam::language lang)
    {
        for (auto& action : g_deferred_actions)
        {
            action();
        }
        g_deferred_actions.clear();
        
        bool commander_active   = ctrl.is_commander_active();

        render_delete_connection_modal(ctrl, lang);
        render_create_connection_modal(ctrl, lang);
        render_add_create_port_modal(ctrl, lang);

        auto& reg_view = ctrl.commander().registry();
        std::lock_guard<const adam::registry_view> lock(reg_view);

        /*#ifdef ADAM_BUILD_DEBUG
        // Inject a test connection if it doesn't exist to test layout
        static bool test_injected = false;
        if (!test_injected)
        {
            auto conn = std::make_unique<adam::connection_view>();
            conn->name = adam::string_hashed("Debug_Connection");
            
            auto p_in1 = std::make_unique<adam::port_view>(); p_in1->name = adam::string_hashed("Video_In"); p_in1->type = ("internal"_ct);
            auto p_in2 = std::make_unique<adam::port_view>(); p_in2->name = adam::string_hashed("Audio_In"); p_in2->type = ("internal"_ct);
            
            auto p_out1 = std::make_unique<adam::port_view>(); p_out1->name = adam::string_hashed("Stream_Out"); p_out1->type = ("internal"_ct);
            auto p_out2 = std::make_unique<adam::port_view>(); p_out2->name = adam::string_hashed("Preview_Out"); p_out2->type = ("internal"_ct);

            conn->inputs.push_back(p_in1->name.get_hash());
            conn->inputs.push_back(p_in2->name.get_hash());
            conn->outputs.push_back(p_out1->name.get_hash());
            conn->outputs.push_back(p_out2->name.get_hash());

            // Mock processor
            conn->filters.push_back(1234);

            reg_view.ports()[p_in1->name.get_hash()] = std::move(p_in1);
            reg_view.ports()[p_in2->name.get_hash()] = std::move(p_in2);
            reg_view.ports()[p_out1->name.get_hash()] = std::move(p_out1);
            reg_view.ports()[p_out2->name.get_hash()] = std::move(p_out2);

            reg_view.connections()[conn->name.get_hash()] = std::move(conn);

            auto conn2 = std::make_unique<adam::connection_view>();
            conn2->name = adam::string_hashed("Converter_Connection");
            
            auto p2_in1 = std::make_unique<adam::port_view>(); p2_in1->name = adam::string_hashed("Raw_Data_In"); p2_in1->type = ("internal"_ct);
            auto p2_out1 = std::make_unique<adam::port_view>(); p2_out1->name = adam::string_hashed("Processed_Out"); p2_out1->type = ("internal"_ct);

            conn2->inputs.push_back(p2_in1->name.get_hash());
            conn2->outputs.push_back(p2_out1->name.get_hash());
            
            // Mock converter
            conn2->converters.push_back(5678);

            reg_view.ports()[p2_in1->name.get_hash()] = std::move(p2_in1);
            reg_view.ports()[p2_out1->name.get_hash()] = std::move(p2_out1);
            
            reg_view.connections()[conn2->name.get_hash()] = std::move(conn2);

            auto conn3 = std::make_unique<adam::connection_view>();
            conn3->name = adam::string_hashed("Splitter_Connection");
            
            auto p3_in1 = std::make_unique<adam::port_view>(); p3_in1->name = adam::string_hashed("Sensor_1"); p3_in1->type = ("internal"_ct);
            auto p3_in2 = std::make_unique<adam::port_view>(); p3_in2->name = adam::string_hashed("Sensor_2"); p3_in2->type = ("internal"_ct);
            
            auto p3_out1 = std::make_unique<adam::port_view>(); p3_out1->name = adam::string_hashed("Log_Out"); p3_out1->type = ("internal"_ct);
            auto p3_out2 = std::make_unique<adam::port_view>(); p3_out2->name = adam::string_hashed("Display_Out"); p3_out2->type = ("internal"_ct);
            auto p3_out3 = std::make_unique<adam::port_view>(); p3_out3->name = adam::string_hashed("Network_Out"); p3_out3->type = ("internal"_ct);

            conn3->inputs.push_back(p3_in1->name.get_hash());
            conn3->inputs.push_back(p3_in2->name.get_hash());
            conn3->outputs.push_back(p3_out1->name.get_hash());
            conn3->outputs.push_back(p3_out2->name.get_hash());
            conn3->outputs.push_back(p3_out3->name.get_hash());

            reg_view.ports()[p3_in1->name.get_hash()] = std::move(p3_in1);
            reg_view.ports()[p3_in2->name.get_hash()] = std::move(p3_in2);
            reg_view.ports()[p3_out1->name.get_hash()] = std::move(p3_out1);
            reg_view.ports()[p3_out2->name.get_hash()] = std::move(p3_out2);
            reg_view.ports()[p3_out3->name.get_hash()] = std::move(p3_out3);
            
            reg_view.connections()[conn3->name.get_hash()] = std::move(conn3);
            
            auto conn4 = std::make_unique<adam::connection_view>();
            conn4->name = adam::string_hashed("Complex_Filter_Connection");
            
            auto p4_in1 = std::make_unique<adam::port_view>(); p4_in1->name = adam::string_hashed("Mic_1"); p4_in1->type = ("internal"_ct);
            auto p4_in2 = std::make_unique<adam::port_view>(); p4_in2->name = adam::string_hashed("Mic_2"); p4_in2->type = ("internal"_ct);
            auto p4_in3 = std::make_unique<adam::port_view>(); p4_in3->name = adam::string_hashed("Mic_3"); p4_in3->type = ("internal"_ct);
            
            auto p4_out1 = std::make_unique<adam::port_view>(); p4_out1->name = adam::string_hashed("Mixed_Audio_Out"); p4_out1->type = ("internal"_ct);

            conn4->inputs.push_back(p4_in1->name.get_hash());
            conn4->inputs.push_back(p4_in2->name.get_hash());
            conn4->inputs.push_back(p4_in3->name.get_hash());
            conn4->outputs.push_back(p4_out1->name.get_hash());

            // Mock filters
            conn4->filters.push_back(1111);
            conn4->filters.push_back(2222);

            reg_view.ports()[p4_in1->name.get_hash()] = std::move(p4_in1);
            reg_view.ports()[p4_in2->name.get_hash()] = std::move(p4_in2);
            reg_view.ports()[p4_in3->name.get_hash()] = std::move(p4_in3);
            reg_view.ports()[p4_out1->name.get_hash()] = std::move(p4_out1);
            
            reg_view.connections()[conn4->name.get_hash()] = std::move(conn4);
            
            auto conn5 = std::make_unique<adam::connection_view>();
            conn5->name = adam::string_hashed("Massive_Connection");
            
            auto p5_in1 = std::make_unique<adam::port_view>(); p5_in1->name = adam::string_hashed("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"); p5_in1->type = ("internal"_ct);
            auto p5_in2 = std::make_unique<adam::port_view>(); p5_in2->name = adam::string_hashed("In_2"); p5_in2->type = ("internal"_ct);
            auto p5_in3 = std::make_unique<adam::port_view>(); p5_in3->name = adam::string_hashed("In_3"); p5_in3->type = ("internal"_ct);
            auto p5_in4 = std::make_unique<adam::port_view>(); p5_in4->name = adam::string_hashed("In_4"); p5_in4->type = ("internal"_ct);
            auto p5_in5 = std::make_unique<adam::port_view>(); p5_in5->name = adam::string_hashed("In_5"); p5_in5->type = ("internal"_ct);
            
            auto p5_out1 = std::make_unique<adam::port_view>(); p5_out1->name = adam::string_hashed("Out_1"); p5_out1->type = ("internal"_ct);
            auto p5_out2 = std::make_unique<adam::port_view>(); p5_out2->name = adam::string_hashed("Out_2"); p5_out2->type = ("internal"_ct);

            conn5->inputs.push_back(p5_in1->name.get_hash());
            conn5->inputs.push_back(p5_in2->name.get_hash());
            conn5->inputs.push_back(p5_in3->name.get_hash());
            conn5->inputs.push_back(p5_in4->name.get_hash());
            conn5->inputs.push_back(p5_in5->name.get_hash());

            conn5->outputs.push_back(p5_out1->name.get_hash());
            conn5->outputs.push_back(p5_out2->name.get_hash());

            for (int i = 0; i < 10; ++i)
                conn5->filters.push_back(10000 + i);

            reg_view.ports()[p5_in1->name.get_hash()] = std::move(p5_in1);
            reg_view.ports()[p5_in2->name.get_hash()] = std::move(p5_in2);
            reg_view.ports()[p5_in3->name.get_hash()] = std::move(p5_in3);
            reg_view.ports()[p5_in4->name.get_hash()] = std::move(p5_in4);
            reg_view.ports()[p5_in5->name.get_hash()] = std::move(p5_in5);
            reg_view.ports()[p5_out1->name.get_hash()] = std::move(p5_out1);
            reg_view.ports()[p5_out2->name.get_hash()] = std::move(p5_out2);
            
            reg_view.connections()[conn5->name.get_hash()] = std::move(conn5);
            
            test_injected = true;
        }
        #endif*/

        const auto& connections = reg_view.get_connections();
        
        static adam::configuration_parameter_integer* sort_mode_param = dynamic_cast<adam::configuration_parameter_integer*>(ctrl.get_parameters().get("connection_sort_mode"_ct));
        int sort_mode = static_cast<int>(sort_mode_param->get_value());
        
        render_top_control_bar(ctrl, lang, sort_mode);

        ImGui::Spacing();

        std::vector<std::pair<adam::string_hash, adam::connection_view*>> sorted_connections;
        for (const auto& [hash, conn] : connections)
            sorted_connections.push_back({hash, conn.get()});

        std::sort(sorted_connections.begin(), sorted_connections.end(), [sort_mode](const auto& a, const auto& b)
        {
            if (sort_mode == 0) // Name (Asc)
                return std::strcmp(a.second->name.c_str(), b.second->name.c_str()) < 0;
            else if (sort_mode == 1) // Name (Desc)
                return std::strcmp(a.second->name.c_str(), b.second->name.c_str()) > 0;
            else if (sort_mode == 2) // Date (Asc)
                return a.second->created < b.second->created;
            else if (sort_mode == 3) // Date (Desc)
                return a.second->created > b.second->created;
            else if (sort_mode == 4) // Edited (Asc)
                return a.second->edited < b.second->edited;
            else if (sort_mode == 5) // Edited (Desc)
                return a.second->edited > b.second->edited;
            else if (sort_mode == 6) // Custom
                return a.second->sorting_index < b.second->sorting_index;
            return false;
        });

        bool g_is_dragging_connection = false;

        if (sort_mode == 6)
        {
            if (const ImGuiPayload* payload = ImGui::GetDragDropPayload())
            {
                if (payload->IsDataType("DND_CONNECTION"))
                {
                    g_is_dragging_connection = true;
                    adam::string_hash dragged_hash = *(const adam::string_hash*)payload->Data;
                    
                    if (g_active_drag_hash != dragged_hash)
                    {
                        g_active_drag_hash = dragged_hash;
                        auto it = std::find_if(sorted_connections.begin(), sorted_connections.end(), 
                            [dragged_hash](const auto& pair){ return pair.first == dragged_hash; });
                        if (it != sorted_connections.end())
                            g_active_drag_target_index = std::distance(sorted_connections.begin(), it);
                        else
                            g_active_drag_target_index = 0;
                    }

                    auto it = std::find_if(sorted_connections.begin(), sorted_connections.end(), 
                        [dragged_hash](const auto& pair){ return pair.first == dragged_hash; });
                    if (it != sorted_connections.end())
                    {
                        auto item = *it;
                        sorted_connections.erase(it);
                        if (g_active_drag_target_index > sorted_connections.size())
                            g_active_drag_target_index = sorted_connections.size();
                        sorted_connections.insert(sorted_connections.begin() + g_active_drag_target_index, item);
                    }
                }
            }
            else
            {
                g_active_drag_hash = 0;
            }
        }

        static float card_width = ImGui::GetContentRegionAvail().x;
        
        render_connections_list(ctrl, lang, sort_mode, card_width, sorted_connections, g_is_dragging_connection);

        if (sort_mode == 6 && g_is_dragging_connection && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            for (uint32_t new_idx = 0; new_idx < sorted_connections.size(); ++new_idx)
            {
                if (sorted_connections[new_idx].second->sorting_index != new_idx)
                {
                    if (commander_active)
                    {
                        adam::string_hash h = sorted_connections[new_idx].first;
                        g_deferred_actions.push_back([&ctrl, h, new_idx]() { ctrl.commander().request_connection_sorting_index_change(h, new_idx); });
                    }
                    sorted_connections[new_idx].second->sorting_index = new_idx; 
                }
            }
            g_active_drag_hash = 0;
            g_is_dragging_connection = false;
        }

        if (const ImGuiPayload* payload = ImGui::GetDragDropPayload())
        {
            if (payload->IsDataType("DND_CONNECTION") && payload->Data)
            {
                adam::string_hash dragged_hash = *(const adam::string_hash*)payload->Data;
                auto it = reg_view.get_connections().find(dragged_hash);
                if (it != reg_view.get_connections().end())
                {
                    ImGui::SetNextWindowPos(ImVec2(ImGui::GetMousePos().x - g_connection_drag_offset.x, ImGui::GetMousePos().y - g_connection_drag_offset.y));
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
                    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.7f); // Restore native ImGui drag preview transparency
                    if (ImGui::Begin("##drag_preview", nullptr, ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize))
                    {
                        render_connection_card(ctrl, lang, sort_mode, dragged_hash, it->second.get(), true, card_width);
                    }
                    ImGui::End();
                    ImGui::PopStyleVar();
                    ImGui::PopStyleColor();
                    ImGui::PopStyleVar(2);
                }
            }
        }

        if (!commander_active) ImGui::BeginDisabled();
        if (ImGui::Button(get_gui_string(gui_string_id::btn_create_connection, lang), ImVec2(-1.0f, ImGui::GetFrameHeight() * 1.5f)))
        {
            if (ImGui::GetIO().KeyShift)
            {
                ImGui::OpenPopup(get_gui_string(gui_string_id::dlg_create_connection, lang));
            }
            else
            {
                std::string default_name = "connection_" + std::to_string(connections.size());
                size_t suffix = connections.size();
                while (connections.find(adam::string_hashed(default_name.c_str()).get_hash()) != connections.end())
                {
                    suffix++;
                    default_name = "connection_" + std::to_string(suffix);
                }
                adam::string_hashed new_conn_name(default_name.c_str());
                g_deferred_actions.push_back([&ctrl, new_conn_name]() { ctrl.commander().request_connection_create(new_conn_name); });
            }
        }
        if (!commander_active) ImGui::EndDisabled();
    }
}