/**
 * @file    node.cpp
 * @author  dexus1337
 * @brief   Implementation of drawing functions for port and processor connection nodes.
 * @version 1.0
 * @date    12.06.2026
 */

#include "node.hpp"
#include "shared-state.hpp"
#include "../../main-window.hpp"
#include "controller/controller.hpp"
#include <data/port-types/port-input-replay.hpp>
#include <algorithm>
#include <cmath>
#include <cstring>

namespace adam::gui
{
    float get_expanded_node_height(uint64_t uid, float dpi_scale)
    {
        // 1. Check if we have an exact pixel-perfect cached height from the actual layout engine
        auto it = g_expanded_node_heights.find(uid);
        if (it != g_expanded_node_heights.end())
        {
            return it->second;
        }

        // 2. Fallback estimation for the very first frame the node is expanded before ImGui can cache it
        float h = 200.0f * dpi_scale;

        if (g_expanded_inject_nodes.count(uid))
            h += 150.0f * dpi_scale;

        if (g_expanded_param_nodes.count(uid))
            h += 150.0f * dpi_scale;

        return h;
    }

    void draw_expanded_port_node
    (
        gui_controller& ctrl,
        adam::language lang,
        const adam::registry_view& registry,
        float dpi_scale,
        ImDrawList* draw_list,
        const expanded_port_draw_info& info
    )
    {
        float exp_pad = 8.0f * dpi_scale;
        ImVec2 exp_min(info.p_max.x - info.current_node_w, info.p_max.y - 1.5f * dpi_scale);
        ImVec2 exp_max(info.p_max.x, info.p_max.y + info.this_expanded_h);

        ImVec4 bg_col = ImGui::GetStyleColorVec4(ImGuiCol_PopupBg);
        bg_col.w = 1.0f;
        draw_list->AddRectFilled(exp_min, exp_max, ImColor(bg_col), 6.0f * dpi_scale, ImDrawFlags_RoundCornersBottom);
        draw_list->AddRect(exp_min, exp_max, ImColor(info.captured_color.Value.x * 1.2f, info.captured_color.Value.y * 1.2f, info.captured_color.Value.z * 1.2f), 6.0f * dpi_scale, ImDrawFlags_RoundCornersBottom, 1.5f * dpi_scale);

        // Draw type and module
        bool p_started = false;
        const char* p_type = "Unknown";
        const char* p_module = "Unknown";
        adam::port::state_buffer_data* stats = nullptr;
        bool has_stats = false;
        bool is_port = (info.type == node_type_input || info.type == node_type_output);
        auto p_it = registry.get_ports().find(info.port_hash);
        auto proc_it = registry.get_processors().find(info.port_hash);
        const adam::configuration_parameter_list_sorted* user_params = nullptr;

        if (is_port && p_it != registry.get_ports().end())
        {
            p_started = p_it->second->started;
            user_params = &p_it->second->user_params;
            if (p_it->second->statistic_buffer)
            {
                stats = p_it->second->statistic_buffer->data_as<adam::port::state_buffer_data>();
                has_stats = true;
            }
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
        else if (!is_port && proc_it != registry.get_processors().end())
        {
            user_params = &proc_it->second->user_params;
            if (proc_it->second->state_buffer)
            {
                stats = proc_it->second->state_buffer->data_as<adam::port::state_buffer_data>();
                has_stats = true;
            }
            p_type = proc_it->second->type.c_str();
            if (proc_it->second->module_name.get_hash() != 0)
                p_module = proc_it->second->module_name.c_str();
            else
                p_module = "Unknown Module";
        }

        ImGui::SetCursorScreenPos(ImVec2(exp_min.x + exp_pad, exp_min.y + exp_pad));
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        bool is_child_visible = ImGui::BeginChild(static_cast<ImGuiID>(info.unique_node_id), ImVec2(info.current_node_w - exp_pad * 2.0f, info.this_expanded_h - exp_pad * 2.0f), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
        
        if (!is_child_visible)
        {
            return;
        }

        ImGui::TextColored(get_gui_color(gui_color_id::log_info), "%s [%s]", p_type, p_module);
        ImGui::Separator();

        // Draw stats
        if (has_stats && stats)
        {
            auto format_bytes_to_buf = [](uint64_t bytes, char* buf, size_t buf_size) 
            {
                if (bytes < 1024) snprintf(buf, buf_size, "%llu B", (unsigned long long)bytes);
                else if (bytes < 1024 * 1024) snprintf(buf, buf_size, "%.2f KB", bytes / 1024.0);
                else if (bytes < 1024 * 1024 * 1024) snprintf(buf, buf_size, "%.2f MB", bytes / (1024.0 * 1024.0));
                else snprintf(buf, buf_size, "%.2f GB", bytes / (1024.0 * 1024.0 * 1024.0));
            };
            
            if (ImGui::BeginTable("PortStatsTable", 3, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg))
            {
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::lbl_statistics, lang), ImGuiTableColumnFlags_WidthFixed, 100.0f * dpi_scale);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_messages, lang), ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_size, lang), ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_handled, lang));
                ImGui::TableNextColumn();
                ImGui::Text("%llu", (unsigned long long)stats->total_buffers_handled);
                ImGui::TableNextColumn();
                char buf_handled[64];
                format_bytes_to_buf(stats->total_bytes_handled, buf_handled, sizeof(buf_handled));
                ImGui::TextUnformatted(buf_handled);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_discarded, lang));
                ImGui::TableNextColumn();
                ImGui::Text("%llu", (unsigned long long)stats->total_buffers_discarded);
                ImGui::TableNextColumn();
                char buf_discarded[64];
                format_bytes_to_buf(stats->total_bytes_discarded, buf_discarded, sizeof(buf_discarded));
                ImGui::TextUnformatted(buf_discarded);

                ImGui::EndTable();
            }

            // Replay specific stats
            bool is_replay = (is_port && p_it != registry.get_ports().end() && p_it->second->type == "replay"_ct && p_it->second->type_module == "recrep"_ct);
            if (is_replay && p_started)
            {
                auto* replay_stats = p_it->second->statistic_buffer->data_as<adam::modules::recrep::port_input_replay::replay_state_buffer_data>();
                
                std::string full_path = replay_stats->file_name;
                size_t last_slash = full_path.find_last_of("/\\");
                std::string display_name = (last_slash != std::string::npos) ? full_path.substr(last_slash + 1) : full_path;

                // Display File name
                ImGui::Spacing();
                ImGui::Text("%s: %s", get_gui_string(gui_string_id::lbl_replay_file, lang), display_name.c_str());
                if (ImGui::IsItemHovered() && !full_path.empty())
                {
                    ImGui::SetTooltip("%s", full_path.c_str());
                }

                // Total play time calculations
                uint64_t duration_ns = replay_stats->file_time_end > replay_stats->file_time_start ? (replay_stats->file_time_end - replay_stats->file_time_start) : 0;
                double speed = 1.0;
                auto* speed_param = p_it->second->user_params.get<adam::configuration_parameter_double>("speed"_ct);
                if (speed_param)
                {
                    speed = speed_param->get_value();
                }

                double total_play_time_sec = 0.0;
                if (speed > 0.0)
                {
                    total_play_time_sec = (static_cast<double>(duration_ns) / 1e9) / speed;
                }

                // Elapsed time calculations
                double elapsed_real_sec = 0.0;
                if (p_started)
                {
                    uint64_t now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
                    if (now_ns > replay_stats->replay_start_time)
                    {
                        elapsed_real_sec = static_cast<double>(now_ns - replay_stats->replay_start_time) / 1e9;
                    }
                }

                if (elapsed_real_sec > total_play_time_sec)
                {
                    elapsed_real_sec = total_play_time_sec;
                }

                double progress_fraction = 0.0;
                double duration_sec = static_cast<double>(duration_ns) / 1e9;
                if (duration_sec > 0.0)
                {
                    if (speed > 0.0)
                    {
                        progress_fraction = elapsed_real_sec / total_play_time_sec;
                    }
                    else
                    {
                        progress_fraction = p_started ? 1.0 : 0.0;
                    }
                }
                if (progress_fraction > 1.0) progress_fraction = 1.0;
                if (progress_fraction < 0.0) progress_fraction = 0.0;

                // Format elapsed time and total play time
                auto format_time = [](double total_seconds, char* buf, size_t buf_size)
                {
                    if (total_seconds < 0.0) total_seconds = 0.0;
                    int hours = static_cast<int>(total_seconds / 3600.0);
                    int minutes = static_cast<int>((total_seconds - hours * 3600.0) / 60.0);
                    double seconds = total_seconds - hours * 3600.0 - minutes * 60.0;
                    if (hours > 0)
                    {
                        snprintf(buf, buf_size, "%dh %dm %.2fs", hours, minutes, seconds);
                    }
                    else if (minutes > 0)
                    {
                        snprintf(buf, buf_size, "%dm %.2fs", minutes, seconds);
                    }
                    else
                    {
                        snprintf(buf, buf_size, "%.2fs", seconds);
                    }
                };

                char elapsed_buf[64];
                char total_buf[64];
                format_time(elapsed_real_sec, elapsed_buf, sizeof(elapsed_buf));
                if (speed > 0.0)
                {
                    format_time(total_play_time_sec, total_buf, sizeof(total_buf));
                }
                else
                {
                    snprintf(total_buf, sizeof(total_buf), "%s", get_gui_string(gui_string_id::lbl_replay_instant, lang));
                }

                char overlay_buf[128];
                snprintf(overlay_buf, sizeof(overlay_buf), "%s / %s", elapsed_buf, total_buf);

                ImGui::ProgressBar(static_cast<float>(progress_fraction), ImVec2(-FLT_MIN, 0.0f), overlay_buf);
            }
        }
        else
        {
            ImGui::TextDisabled("%s", get_gui_string(gui_string_id::lbl_stat_unavailable, lang));
        }
        ImGui::Separator();

        float btn_w = (ImGui::GetContentRegionAvail().x - exp_pad) * 0.5f;

        // Start/Stop
        if (is_port)
        {
            ImGui::PushID((const void*)(intptr_t)(info.port_hash ^ 0x1111));
            if (p_started) ImGui::BeginDisabled();
            if (ImGui::Button(get_gui_string(gui_string_id::btn_start, lang), ImVec2(btn_w, 0)))
                ctrl.enqueue_commander_action([&ctrl, port_hash = info.port_hash]() { ctrl.commander().request_port_start(port_hash); });
            if (p_started) ImGui::EndDisabled();
            ImGui::PopID();

            ImGui::SameLine(0.0f, exp_pad);

            ImGui::PushID((const void*)(intptr_t)(info.port_hash ^ 0x2222));
            if (!p_started) ImGui::BeginDisabled();
            if (ImGui::Button(get_gui_string(gui_string_id::btn_stop, lang), ImVec2(btn_w, 0)))
                ctrl.enqueue_commander_action([&ctrl, port_hash = info.port_hash]() { ctrl.commander().request_port_stop(port_hash); });
            if (!p_started) ImGui::EndDisabled();
            ImGui::PopID();
            
            ImGui::Separator();
        }

        // Parameters
        if (user_params && !user_params->get_children().empty() && ((is_port && p_it != registry.get_ports().end() && !p_it->second->is_unavailable) || (!is_port && proc_it != registry.get_processors().end() && !proc_it->second->is_unavailable)))
        {
            ImGui::PushID((const void*)(intptr_t)(info.unique_node_id ^ 0x5555));
            bool param_expanded = g_expanded_param_nodes.count(info.unique_node_id) > 0;
            
            bool tree_open = ImGui::TreeNodeEx("Parameters", param_expanded ? ImGuiTreeNodeFlags_DefaultOpen : 0);
            if (tree_open != param_expanded)
            {
                if (tree_open)
                    g_expanded_param_nodes.insert(info.unique_node_id);
                else
                    g_expanded_param_nodes.erase(info.unique_node_id);

                g_expanded_node_heights.erase(info.unique_node_id); // Invalidate cache to force smooth layout recalculation
            }

            if (tree_open)
            {
                ImGui::Unindent();
                
                bool disable_params = p_started;
                if (disable_params) ImGui::BeginDisabled();
                
                float avail_w = ImGui::GetContentRegionAvail().x;
                
                auto* sorted_list = dynamic_cast<const adam::configuration_parameter_list_sorted*>(user_params);

                // Draw each param via a lambda to avoid building an intermediate vector every frame
                auto draw_param = [&](const adam::string_hashed& param_name, adam::configuration_parameter* param_ptr)
                {
                    auto param_type = param_ptr->get_type();
                    
                    ImGui::PushID((const void*)(intptr_t)param_name.get_hash());
                    ImGui::TextUnformatted(param_name.c_str());

                    bool has_range = false;
                    std::string range_str;
                    if (param_type == adam::configuration_parameter::type_integer)
                    {
                        auto* c_int = static_cast<adam::configuration_parameter_integer*>(param_ptr);
                        if (c_int->get_mode() == adam::configuration_parameter_integer::value_mode_range)
                        {
                            has_range = true;
                            if (lang == adam::language_german)
                                range_str = "Erlaubter Bereich: [" + std::to_string(c_int->get_min_value()) + " - " + std::to_string(c_int->get_max_value()) + "]";
                            else
                                range_str = "Allowed Range: [" + std::to_string(c_int->get_min_value()) + " - " + std::to_string(c_int->get_max_value()) + "]";
                        }
                    }
                    else if (param_type == adam::configuration_parameter::type_double)
                    {
                        auto* c_dbl = static_cast<adam::configuration_parameter_double*>(param_ptr);
                        if (c_dbl->get_mode() == adam::configuration_parameter_double::value_mode_range)
                        {
                            has_range = true;
                            char min_buf[32], max_buf[32];
                            snprintf(min_buf, sizeof(min_buf), "%g", c_dbl->get_min_value());
                            snprintf(max_buf, sizeof(max_buf), "%g", c_dbl->get_max_value());
                            if (lang == adam::language_german)
                                range_str = std::string("Erlaubter Bereich: [") + min_buf + " - " + max_buf + "]";
                            else
                                range_str = std::string("Allowed Range: [") + min_buf + " - " + max_buf + "]";
                        }
                    }

                    const adam::string_hashed& desc = param_ptr->get_description(lang);
                    if ((!desc.empty() || has_range) && ImGui::IsItemHovered())
                    {
                        ImGui::BeginTooltip();
                        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                        if (!desc.empty())
                        {
                            ImGui::TextUnformatted(desc.c_str());
                            if (has_range)
                            {
                                ImGui::Spacing();
                                ImGui::Separator();
                                ImGui::Spacing();
                            }
                        }
                        if (has_range)
                        {
                            ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "%s", range_str.c_str());
                        }
                        ImGui::PopTextWrapPos();
                        ImGui::EndTooltip();
                    }
                    
                    if (param_type == adam::configuration_parameter::type_integer)
                    {
                        auto* c_int = static_cast<adam::configuration_parameter_integer*>(param_ptr);
                        int64_t current_val = c_int->get_value();
                        ImGui::SetNextItemWidth(avail_w);
                        if (c_int->get_mode() == adam::configuration_parameter_integer::value_mode_preset)
                        {
                            char preview[64] = "";
                            bool found = false;
                            for (int64_t preset : c_int->get_presets())
                            {
                                if (preset == current_val)
                                {
                                    snprintf(preview, sizeof(preview), "%lld", (long long)preset);
                                    found = true;
                                    break;
                                }
                            }
                            if (!found) snprintf(preview, sizeof(preview), "%lld", (long long)current_val);
                            
                            if (ImGui::BeginCombo("##combo", preview))
                            {
                                std::vector<int64_t> sorted_presets(c_int->get_presets().begin(), c_int->get_presets().end());
                                std::sort(sorted_presets.begin(), sorted_presets.end());
                                
                                for (int64_t preset : sorted_presets)
                                {
                                    char preset_str[64];
                                    snprintf(preset_str, sizeof(preset_str), "%lld", (long long)preset);
                                    bool is_selected = (preset == current_val);
                                    if (ImGui::Selectable(preset_str, is_selected))
                                    {
                                        if (!is_selected)
                                        {
                                            c_int->set_value(preset);
                                            if (is_port)
                                                ctrl.enqueue_commander_action([&ctrl, h=info.port_hash, p=param_name, v=preset]() { ctrl.commander().request_port_parameter_set(h, p, v); });
                                            else
                                                ctrl.enqueue_commander_action([&ctrl, h=info.port_hash, p=param_name, v=preset]() { ctrl.commander().request_processor_parameter_set(h, p, v); });
                                        }
                                    }
                                }
                                ImGui::EndCombo();
                            }
                        }
                        else
                        {
                            char buf[64];
                            snprintf(buf, sizeof(buf), "%lld", (long long)current_val);
                            if (ImGui::InputText("##int", buf, sizeof(buf), ImGuiInputTextFlags_EnterReturnsTrue))
                            {
                                try 
                                {
                                    int64_t new_v = std::stoll(buf);
                                    if (c_int->set_value(new_v))
                                    {
                                        if (is_port)
                                            ctrl.enqueue_commander_action([&ctrl, h=info.port_hash, p=param_name, v=new_v]() { ctrl.commander().request_port_parameter_set(h, p, v); });
                                        else
                                            ctrl.enqueue_commander_action([&ctrl, h=info.port_hash, p=param_name, v=new_v]() { ctrl.commander().request_processor_parameter_set(h, p, v); });
                                    }
                                } catch(...) {}
                            }
                        }
                    }
                    else if (param_type == adam::configuration_parameter::type_double)
                    {
                        auto* c_dbl = static_cast<adam::configuration_parameter_double*>(param_ptr);
                        double current_val = c_dbl->get_value();
                        ImGui::SetNextItemWidth(avail_w);
                        if (c_dbl->get_mode() == adam::configuration_parameter_double::value_mode_preset)
                        {
                            char preview[64] = "";
                            bool found = false;
                            for (double preset : c_dbl->get_presets())
                            {
                                if (std::abs(preset - current_val) < 1e-9)
                                {
                                    snprintf(preview, sizeof(preview), "%f", preset);
                                    found = true;
                                    break;
                                }
                            }
                            if (!found) snprintf(preview, sizeof(preview), "%f", current_val);
                            
                            if (ImGui::BeginCombo("##combo", preview))
                            {
                                std::vector<double> sorted_presets(c_dbl->get_presets().begin(), c_dbl->get_presets().end());
                                std::sort(sorted_presets.begin(), sorted_presets.end());
                                
                                for (double preset : sorted_presets)
                                {
                                    char preset_str[64];
                                    snprintf(preset_str, sizeof(preset_str), "%f", preset);
                                    bool is_selected = (std::abs(preset - current_val) < 1e-9);
                                    if (ImGui::Selectable(preset_str, is_selected))
                                    {
                                        if (!is_selected)
                                        {
                                            c_dbl->set_value(preset);
                                            if (is_port)
                                                ctrl.enqueue_commander_action([&ctrl, h=info.port_hash, p=param_name, v=preset]() { ctrl.commander().request_port_parameter_set(h, p, v); });
                                            else
                                                ctrl.enqueue_commander_action([&ctrl, h=info.port_hash, p=param_name, v=preset]() { ctrl.commander().request_processor_parameter_set(h, p, v); });
                                        }
                                    }
                                }
                                ImGui::EndCombo();
                            }
                        }
                        else
                        {
                            char buf[64];
                            snprintf(buf, sizeof(buf), "%f", current_val);
                            if (ImGui::InputText("##double", buf, sizeof(buf), ImGuiInputTextFlags_EnterReturnsTrue))
                            {
                                try 
                                {
                                    double new_v = std::stod(buf);
                                    if (c_dbl->set_value(new_v))
                                    {
                                        if (is_port)
                                            ctrl.enqueue_commander_action([&ctrl, h=info.port_hash, p=param_name, v=new_v]() { ctrl.commander().request_port_parameter_set(h, p, v); });
                                        else
                                            ctrl.enqueue_commander_action([&ctrl, h=info.port_hash, p=param_name, v=new_v]() { ctrl.commander().request_processor_parameter_set(h, p, v); });
                                    }
                                } catch(...) {}
                            }
                        }
                    }
                    else if (param_type == adam::configuration_parameter::type_string)
                    {
                        auto* c_str = static_cast<adam::configuration_parameter_string*>(param_ptr);
                        adam::string_hashed current_val = c_str->get_value();
                        ImGui::SetNextItemWidth(avail_w);
                        
                        if (c_str->get_mode() == adam::configuration_parameter_string::value_mode_preset)
                        {
                            const char* preview = current_val.c_str();
                            if (ImGui::BeginCombo("##combo", preview))
                            {
                                std::vector<std::string> sorted_presets;
                                for (const auto& [preset_val, preset_param] : c_str->get_presets())
                                    sorted_presets.push_back(preset_param->get_value().c_str());
                                std::sort(sorted_presets.begin(), sorted_presets.end());
                                
                                for (const auto& preset_str : sorted_presets)
                                {
                                    bool is_selected = (preset_str == preview);
                                    if (ImGui::Selectable(preset_str.c_str(), is_selected))
                                    {
                                        if (!is_selected)
                                        {
                                            adam::string_hashed new_v(preset_str.c_str());
                                            c_str->set_value(new_v);
                                            if (is_port)
                                                ctrl.enqueue_commander_action([&ctrl, h=info.port_hash, p=param_name, v=new_v]() { ctrl.commander().request_port_parameter_set(h, p, v); });
                                            else
                                                ctrl.enqueue_commander_action([&ctrl, h=info.port_hash, p=param_name, v=new_v]() { ctrl.commander().request_processor_parameter_set(h, p, v); });
                                        }
                                    }
                                }
                                ImGui::EndCombo();
                            }
                        }
                        else
                        {
                            char buf[512];
                            strncpy(buf, current_val.c_str(), sizeof(buf));
                            buf[sizeof(buf) - 1] = '\0';
                            
                            if (ImGui::InputText("##str", buf, sizeof(buf), ImGuiInputTextFlags_EnterReturnsTrue))
                            {
                                adam::string_hashed new_v(&buf[0]);
                                if (c_str->set_value(new_v))
                                {
                                    if (is_port)
                                        ctrl.enqueue_commander_action([&ctrl, h=info.port_hash, p=param_name, v=new_v]() { ctrl.commander().request_port_parameter_set(h, p, v); });
                                    else
                                        ctrl.enqueue_commander_action([&ctrl, h=info.port_hash, p=param_name, v=new_v]() { ctrl.commander().request_processor_parameter_set(h, p, v); });
                                }
                            }
                        }
                    }
                    
                    ImGui::PopID();
                };

                if (sorted_list)
                {
                    for (auto hash : sorted_list->get_order())
                    {
                        auto* param_ptr = sorted_list->get(hash);
                        if (param_ptr)
                            draw_param(param_ptr->get_name(), param_ptr);
                    }
                }
                else
                {
                    for (const auto& [param_name, param_ptr] : user_params->get_children())
                        draw_param(param_name, param_ptr.get());
                }

                if (disable_params) ImGui::EndDisabled();
                
                ImGui::Indent();
                ImGui::TreePop();
            }
            ImGui::PopID();
            ImGui::Separator();
        }

        // Inject Data
        if (is_port)
        {
            ImGui::PushID((const void*)(intptr_t)(info.unique_node_id ^ 0x3333));
            bool inject_expanded = g_expanded_inject_nodes.count(info.unique_node_id) > 0;
            
            bool tree_open = ImGui::TreeNodeEx(get_gui_string(gui_string_id::lbl_inject_data, lang), inject_expanded ? ImGuiTreeNodeFlags_DefaultOpen : 0);
            if (tree_open != inject_expanded)
            {
                if (tree_open)
                    g_expanded_inject_nodes.insert(info.unique_node_id);
                else
                    g_expanded_inject_nodes.erase(info.unique_node_id);

                g_expanded_node_heights.erase(info.unique_node_id); // Invalidate cache to force smooth layout recalculation
            }

            if (tree_open)
            {
                ImGui::Unindent();

                auto& inject_state = g_inject_data_buffers[info.unique_node_id];
                if (inject_state.text_buffer.size() < 1024) inject_state.text_buffer.resize(1024, '\0');
                
                std::string current_input = inject_state.text_buffer.c_str();
                if (current_input != inject_state.last_parsed_text)
                {
                    auto parse_result = parse_hex_bytes(current_input);
                    inject_state.is_valid = parse_result.first;
                    inject_state.parsed_bytes = parse_result.second;
                    inject_state.last_parsed_text = current_input;
                }

                bool has_invalid_input = !inject_state.is_valid && !current_input.empty();

                if (has_invalid_input)
                {
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f * dpi_scale);
                    ImGui::PushStyleColor(ImGuiCol_Border, get_gui_color(gui_color_id::log_error));
                }

                float avail_w = ImGui::GetContentRegionAvail().x;
                
                ImGui::InputTextMultiline("##inject_data", inject_state.text_buffer.data(), inject_state.text_buffer.size(), ImVec2(avail_w, 100.0f * dpi_scale), ImGuiInputTextFlags_WordWrap | ImGuiInputTextFlags_CallbackResize, inject_text_resize_callback, &inject_state.text_buffer);
                
                if (has_invalid_input)
                {
                    ImGui::PopStyleColor();
                    ImGui::PopStyleVar();
                }

                bool disable_inject = !p_started || !inject_state.is_valid;
                if (disable_inject) ImGui::BeginDisabled();

                if (ImGui::Button(get_gui_string(gui_string_id::btn_inject, lang), ImVec2(avail_w, 0)))
                {
                    if (inject_state.is_valid && !inject_state.parsed_bytes.empty())
                    {
                        std::vector<uint8_t> data_to_inject = inject_state.parsed_bytes;
                        ctrl.enqueue_commander_action([&ctrl, type = info.type, port_hash = info.port_hash, data_to_inject]() 
                        { 
                            switch(type)
                            {
                                case node_type_input:
                                    ctrl.commander().request_port_inject_data(port_hash, data_to_inject.data(), data_to_inject.size(), data_direction_in); 
                                    break;
                                case node_type_output:
                                    ctrl.commander().request_port_inject_data(port_hash, data_to_inject.data(), data_to_inject.size(), data_direction_out); 
                                    break;
                                default:
                                    break;
                            }
                        });
                    }
                }

                if (disable_inject) ImGui::EndDisabled();

                ImGui::Indent();
                ImGui::TreePop();
            }
            ImGui::PopID();

            ImGui::Separator();
        }

        // Remove
        ImGui::PushID((const void*)(intptr_t)(info.port_hash ^ 0x4444));
        if (is_port)
        {
            if (ImGui::Button(get_gui_string(gui_string_id::btn_remove_port, lang), ImVec2(info.current_node_w - exp_pad * 2.0f, 0)))
            {
                ctrl.enqueue_commander_action([&ctrl, conn_hash = info.hash, port_hash = info.port_hash, is_input = (info.type == node_type_input)]() 
                { 
                    ctrl.commander().request_connection_port_remove(conn_hash, port_hash, is_input); 
                });
            }
        }
        else
        {
            if (ImGui::Button(get_gui_string(gui_string_id::btn_remove_processor, lang), ImVec2(info.current_node_w - exp_pad * 2.0f, 0)))
            {
                ctrl.enqueue_commander_action([&ctrl, conn_hash = info.hash, proc_hash = info.port_hash]() 
                { 
                    ctrl.commander().request_connection_processor_remove(conn_hash, proc_hash);
                });
            }
        }
        ImGui::PopID();

        // Cache height
        float exact_inner_h = ImGui::GetCursorPosY() - ImGui::GetStyle().ItemSpacing.y;
        g_expanded_node_heights[info.unique_node_id] = exact_inner_h + exp_pad * 2.0f;

        ImGui::EndChild();
    }

    bool draw_connection_node
    (
        gui_controller& ctrl,
        adam::language lang,
        adam::connection_view* conn,
        adam::string_hash hash,
        float dpi_scale,
        ImDrawList* draw_list,
        ImVec2 cur_pos,
        float port_w,
        float gap,
        float proc_w,
        float node_h,
        float row_height,
        int total_stages,
        float avail_x,
        bool is_drag_preview,
        const char* name,
        node_type type,
        int stage,
        float row,
        ImColor color,
        connection_pin_data& out_pin_in,
        connection_pin_data& out_pin_out,
        bool is_unavail,
        const char* unavail_module,
        adam::string_hash port_hash,
        float extra_y,
        std::vector<expanded_port_draw_info>& deferred_expansions
    )
    {
        bool is_light_theme = false;
        auto* theme_param = dynamic_cast<adam::configuration_parameter_string*>(ctrl.get_parameters().get("theme"_ct));
        if (theme_param && theme_param->get_value() == "default-light"_ct)
        {
            is_light_theme = true;
        }

        const auto& ports = ctrl.commander().registry().get_ports();

        float current_node_w = port_w;
        if (total_stages > 2 && stage > 0 && stage < total_stages - 1)
        {
            current_node_w = proc_w;
        }
            
        float node_x;
        if (total_stages == 1) node_x = cur_pos.x;
        else if (stage == 0) node_x = cur_pos.x;
        else if (stage == total_stages - 1) node_x = cur_pos.x + avail_x - current_node_w;
        else node_x = cur_pos.x + port_w + gap + static_cast<float>(stage - 1) * (proc_w + gap);
            
        float node_y = cur_pos.y + row * row_height + (row_height - node_h) * 0.5f + extra_y;
            
        ImVec2 p_min(node_x, node_y);
        ImVec2 p_max(node_x + current_node_w, node_y + node_h);
            
        draw_list->AddRectFilled(p_min, p_max, color, 6.0f * dpi_scale);
        draw_list->AddRect(p_min, p_max, ImColor(color.Value.x * 1.2f, color.Value.y * 1.2f, color.Value.z * 1.2f, color.Value.w), 6.0f * dpi_scale, 0, 1.5f * dpi_scale);

        ImGui::SetCursorScreenPos(p_min);
        ImGui::PushID((const void*)(intptr_t)(get_unique_node_id(port_hash, hash, stage) ^ 0xABCD));
        ImGui::SetNextItemAllowOverlap();
        bool clicked = ImGui::InvisibleButton("##node_btn", ImVec2(current_node_w, node_h));

        if (type == node_type_processor && !is_drag_preview)
        {
            if (ImGui::IsItemActivated())
            {
                ImVec2 mouse_pos = ImGui::GetMousePos();
                g_processor_drag_offset = ImVec2(mouse_pos.x - p_min.x, mouse_pos.y - p_min.y);
            }

            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID | ImGuiDragDropFlags_SourceNoPreviewTooltip))
            {
                DragProcessorPayload payload { hash, port_hash };
                ImGui::SetDragDropPayload("DND_PROCESSOR", &payload, sizeof(payload));
                ImGui::EndDragDropSource();
            }
        }

        bool is_expanded = false;
        if (port_hash != 0)
        {
            uint64_t unique_node_id = get_unique_node_id(port_hash, hash, stage);
            is_expanded = g_expanded_nodes.count(unique_node_id) > 0;

            // Handle Left Click -> Expand
            if (clicked)
            {
                if (is_expanded)
                    g_expanded_nodes.erase(unique_node_id);
                else
                    g_expanded_nodes.insert(unique_node_id);
                is_expanded = !is_expanded;
            }

            // Handle Right Click -> Inspect
            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
            {
                if (!is_drag_preview && ctrl.is_commander_active())
                {
                    auto it = ctrl.commander().get_inspectors().find(port_hash);
                    if (it == ctrl.commander().get_inspectors().end())
                    {
                        g_request_open_inspector = true;
                        g_port_to_expand_in_inspector = port_hash;
                        g_pending_inspector_ports.insert(port_hash);
                    }
                    else
                    {
                        g_expanded_inspector_ports.erase(port_hash);
                        g_pending_inspector_ports.erase(port_hash);
                    }

                    ctrl.enqueue_commander_action([&ctrl, port_hash]() 
                    {
                        auto& cmdr = ctrl.commander();
                        auto it = cmdr.inspectors().find(port_hash);
                        if (it == cmdr.inspectors().end())
                        {
                            adam::data_inspector* new_inspector = nullptr;
                            cmdr.request_inspector_create(port_hash, make_inspector_buffer_callback(port_hash), new_inspector);
                        }
                        else
                        {
                            cmdr.request_inspector_destroy(it->second);
                        }
                    });
                }
            }
        }
        ImGui::PopID();
        
        if (is_expanded && !is_drag_preview)
        {
            ImColor captured_color = color;
            uint64_t unique_node_id = get_unique_node_id(port_hash, hash, stage);
            float this_expanded_h = get_expanded_node_height(unique_node_id, dpi_scale);
            deferred_expansions.push_back
            ({
                type,
                stage,
                port_hash,
                unique_node_id,
                p_max,
                current_node_w,
                captured_color,
                this_expanded_h,
                hash
            });
        }
            
        // Draw Port Name
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
                
            ImGui::PushID((const void*)(intptr_t)get_unique_node_id(port_hash, hash, stage));
            bool enter_pressed = ImGui::InputText("##port_edit", name_buf, sizeof(name_buf), ImGuiInputTextFlags_EnterReturnsTrue);
            bool deactivated = ImGui::IsItemDeactivatedAfterEdit();
            ImGui::PopID();
                
            ImGui::PopItemWidth();
            ImGui::PopStyleColor(3);
                
            if ((enter_pressed || deactivated) && name_buf[0] != '\0' && std::strcmp(name, name_buf) != 0)
            {
                adam::string_hash proposed_hash = adam::string_hashed(&name_buf[0]).get_hash();
                bool is_node_port = (type == node_type_input || type == node_type_output);
                if (is_node_port)
                {
                    if (ports.find(proposed_hash) == ports.end())
                    {
                        adam::string_hashed new_name(&name_buf[0]);
                        ctrl.enqueue_commander_action([&ctrl, port_hash, new_name]() { ctrl.commander().request_port_rename(port_hash, new_name); });
                    }
                }
                else
                {
                    if (ctrl.commander().registry().get_processors().find(proposed_hash) == ctrl.commander().registry().get_processors().end())
                    {
                        adam::string_hashed new_name(&name_buf[0]);
                        ctrl.enqueue_commander_action([&ctrl, port_hash, new_name]() { ctrl.commander().request_processor_rename(port_hash, new_name); });
                    }
                }
            }
        }
        else
        {
            float text_width = ImGui::CalcTextSize(name).x;
            float text_x = p_min.x + (current_node_w - text_width) * 0.5f;
            if (text_x < p_min.x + 8.0f * dpi_scale) text_x = p_min.x + 8.0f * dpi_scale;
            ImVec2 text_pos(text_x, p_min.y + (node_h - ImGui::GetTextLineHeight()) * 0.5f);
            draw_list->PushClipRect(p_min, p_max, true);
            draw_list->AddText(text_pos, ImColor(1.0f, 1.0f, 1.0f, color.Value.w), name);
            draw_list->PopClipRect();
        }

        out_pin_in.pos = ImVec2(p_min.x, p_min.y + node_h * 0.5f);
        out_pin_out.pos = ImVec2(p_max.x, p_min.y + node_h * 0.5f);

        // Draw Port Status Dot
        ImColor pin_col = is_light_theme ? get_gui_color(gui_color_id::node_connection_line_light) : get_gui_color(gui_color_id::node_connection_line);
        if (port_hash != 0)
        {
            auto p_it = ports.find(port_hash);
            if (p_it != ports.end())
            {
                if (p_it->second->started)
                {
                    if (p_it->second->statistic_buffer)
                    {
                        auto* stats = p_it->second->statistic_buffer->data_as<adam::port::state_buffer_data>();
                        if (stats->cur_state == adam::port::state_running)
                            pin_col = get_gui_color(gui_color_id::node_pin_active);
                        else if (stats->cur_state == adam::port::state_inactive)
                            pin_col = get_gui_color(gui_color_id::log_warning);
                    }
                    else
                    {
                        pin_col = get_gui_color(gui_color_id::node_pin_active);
                    }
                }
            }
        }

        out_pin_in.col = pin_col;
        out_pin_out.col = pin_col;
                
        if (is_unavail && !is_drag_preview && ImGui::IsMouseHoveringRect(p_min, p_max))
        {
            ImGui::BeginTooltip();
            ImGui::Text(get_gui_string(gui_string_id::tt_module_missing, lang), unavail_module ? unavail_module : "Unknown");
            ImGui::EndTooltip();
        }

        return is_expanded;
    }
}
