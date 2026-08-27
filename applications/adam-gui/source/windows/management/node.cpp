/**
 * @file    node.cpp
 * @author  dexus1337
 * @brief   Implementation of drawing functions for port and processor connection nodes.
 * @version 1.0
 * @date    12.06.2026
 */

#include <adam-core.hpp>
#include <adam-recrep.hpp>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <chrono>


#include "node.hpp"
#include "shared-state.hpp"
#include "../main-window.hpp"
#include "controller/controller.hpp"

namespace adam::gui
{
    void draw_configuration_parameter
    (
        gui_controller& ctrl,
        adam::language lang,
        adam::string_hash item_hash,
        const adam::string_hashed& param_name,
        adam::configuration_parameter* param_ptr,
        param_target_type target,
        float available_width
    )
    {
        auto dispatch_param_set = [&](auto val)
        {
            switch (target)
            {
                case param_target_type::port:
                    ctrl.enqueue_commander_action([&ctrl, item_hash, param_name, val]() { ctrl.commander().request_port_parameter_set(item_hash, param_name, val); });
                    break;
                case param_target_type::processor:
                    ctrl.enqueue_commander_action([&ctrl, item_hash, param_name, val]() { ctrl.commander().request_processor_parameter_set(item_hash, param_name, val); });
                    break;
                case param_target_type::connection_input_format:
                    ctrl.enqueue_commander_action([&ctrl, item_hash, param_name, val]() { ctrl.commander().request_connection_input_format_parameter_set(item_hash, param_name, val); });
                    break;
                case param_target_type::connection_output_format:
                    ctrl.enqueue_commander_action([&ctrl, item_hash, param_name, val]() { ctrl.commander().request_connection_output_format_parameter_set(item_hash, param_name, val); });
                    break;
            }
        };

        ImGui::PushID(reinterpret_cast<const void*>(static_cast<intptr_t>(param_name.get_hash())));
        ImGui::TextUnformatted(param_name.c_str());

        bool has_range = false;
        std::string range_str;
        switch (param_ptr->get_type())
        {
            case adam::configuration_parameter::type_integer:
            {
                auto* c_int = static_cast<adam::configuration_parameter_integer*>(param_ptr);
                if (c_int->get_mode() == adam::configuration_parameter_integer::value_mode_range)
                {
                    has_range = true;
                    if (lang == adam::language_german)
                    {
                        range_str = "Erlaubter Bereich: [" + std::to_string(c_int->get_min_value()) + " - " + std::to_string(c_int->get_max_value()) + "]";
                    }
                    else
                    {
                        range_str = "Allowed Range: [" + std::to_string(c_int->get_min_value()) + " - " + std::to_string(c_int->get_max_value()) + "]";
                    }
                }
                break;
            }
            case adam::configuration_parameter::type_double:
            {
                auto* c_dbl = static_cast<adam::configuration_parameter_double*>(param_ptr);
                if (c_dbl->get_mode() == adam::configuration_parameter_double::value_mode_range)
                {
                    has_range = true;
                    char min_buf[32], max_buf[32];
                    snprintf(min_buf, sizeof(min_buf), "%g", c_dbl->get_min_value());
                    snprintf(max_buf, sizeof(max_buf), "%g", c_dbl->get_max_value());
                    if (lang == adam::language_german)
                    {
                        range_str = std::string("Erlaubter Bereich: [") + min_buf + " - " + max_buf + "]";
                    }
                    else
                    {
                        range_str = std::string("Allowed Range: [") + min_buf + " - " + max_buf + "]";
                    }
                }
                break;
            }
            default:
                break;
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

        ImGui::SetNextItemWidth(available_width);
        switch (param_ptr->get_type())
        {
            case adam::configuration_parameter::type_integer:
            {
                auto* c_int = static_cast<adam::configuration_parameter_integer*>(param_ptr);
                int64_t current_val = c_int->get_value();
                
                if (c_int->get_mode() == adam::configuration_parameter_integer::value_mode_preset)
                {
                    char preview_buf[32];
                    snprintf(preview_buf, sizeof(preview_buf), "%lld", static_cast<long long>(current_val));
                    
                    if (ImGui::BeginCombo("##combo", preview_buf))
                    {
                        for (int64_t preset : c_int->get_presets())
                        {
                            char item_buf[32];
                            snprintf(item_buf, sizeof(item_buf), "%lld", static_cast<long long>(preset));
                            bool is_selected = (preset == current_val);
                            if (ImGui::Selectable(item_buf, is_selected))
                            {
                                if (!is_selected)
                                {
                                    c_int->set_value(preset);
                                    dispatch_param_set(preset);
                                }
                            }
                        }
                        ImGui::EndCombo();
                    }
                }
                else
                {
                    int64_t val = current_val;
                    if (ImGui::InputScalar("##input", ImGuiDataType_S64, &val, nullptr, nullptr, "%lld"))
                    {
                        c_int->set_value(val);
                    }
                    if (ImGui::IsItemDeactivatedAfterEdit())
                    {
                        int64_t new_val = c_int->get_value();
                        dispatch_param_set(new_val);
                    }
                }
                break;
            }
            case adam::configuration_parameter::type_double:
            {
                auto* c_dbl = static_cast<adam::configuration_parameter_double*>(param_ptr);
                double current_val = c_dbl->get_value();
                
                if (c_dbl->get_mode() == adam::configuration_parameter_double::value_mode_preset)
                {
                    char preview_buf[32];
                    snprintf(preview_buf, sizeof(preview_buf), "%g", current_val);
                    
                    if (ImGui::BeginCombo("##combo", preview_buf))
                    {
                        for (double preset : c_dbl->get_presets())
                        {
                            char item_buf[32];
                            snprintf(item_buf, sizeof(item_buf), "%g", preset);
                            bool is_selected = (std::abs(preset - current_val) < 1e-9);
                            if (ImGui::Selectable(item_buf, is_selected))
                            {
                                if (!is_selected)
                                {
                                    c_dbl->set_value(preset);
                                    dispatch_param_set(preset);
                                }
                            }
                        }
                        ImGui::EndCombo();
                    }
                }
                else
                {
                    double val = current_val;
                    if (ImGui::InputScalar("##input", ImGuiDataType_Double, &val, nullptr, nullptr, "%.3f"))
                    {
                        c_dbl->set_value(val);
                    }
                    if (ImGui::IsItemDeactivatedAfterEdit())
                    {
                        double new_val = c_dbl->get_value();
                        dispatch_param_set(new_val);
                    }
                }
                break;
            }
            case adam::configuration_parameter::type_boolean:
            {
                auto* c_bool = static_cast<adam::configuration_parameter_boolean*>(param_ptr);
                bool current_val = c_bool->get_value();
                const char* preview = current_val ? "enabled" : "disabled";
                if (ImGui::BeginCombo("##combo", preview))
                {
                    const char* options[] = { "disabled", "enabled" };
                    for (int i = 0; i < 2; ++i)
                    {
                        bool val = (i == 1);
                        bool is_selected = (val == current_val);
                        if (ImGui::Selectable(options[i], is_selected))
                        {
                            if (!is_selected)
                            {
                                c_bool->set_value(val);
                                dispatch_param_set(val);
                            }
                        }
                    }
                    ImGui::EndCombo();
                }
                break;
            }
            case adam::configuration_parameter::type_string:
            {
                auto* c_str = static_cast<adam::configuration_parameter_string*>(param_ptr);
                adam::string_hashed current_val = c_str->get_value();
                
                if (c_str->get_mode() == adam::configuration_parameter_string::value_mode_preset)
                {
                    const char* preview = current_val.c_str();
                    if (ImGui::BeginCombo("##combo", preview))
                    {
                        std::vector<std::string> sorted_presets;
                        for (const auto& [preset_name, preset_param] : c_str->get_presets())
                        {
                            sorted_presets.push_back(preset_name.c_str());
                        }
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
                                    dispatch_param_set(new_v);
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
                            dispatch_param_set(new_v);
                        }
                    }
                }
                break;
            }
            default:
                break;
        }
        ImGui::PopID();
    }

    float get_expanded_node_height(uint64_t uid, float dpi_scale)
    {
        auto it = g_expanded_node_heights.find(uid);
        if (it != g_expanded_node_heights.end())
        {
            return it->second;
        }
        return 200.0f * dpi_scale;
    }

    static void draw_compact_processor_header
    (
        gui_controller& ctrl,
        const adam::registry_view& registry,
        const expanded_port_draw_info& info,
        float exp_pad,
        float dpi_scale
    )
    {
        ImVec2 header_min = ImGui::GetCursorScreenPos();
        float h_height = ImGui::GetFrameHeight() + 4.0f * dpi_scale;
        ImVec2 header_max(header_min.x + (info.current_node_w - exp_pad * 2.0f), header_min.y + h_height);

        ImGui::GetWindowDrawList()->AddRectFilled(header_min, header_max, info.captured_color, 4.0f * dpi_scale);
        ImGui::GetWindowDrawList()->AddRect(header_min, header_max, ImColor(info.captured_color.Value.x * 1.2f, info.captured_color.Value.y * 1.2f, info.captured_color.Value.z * 1.2f), 4.0f * dpi_scale, 0, 1.0f * dpi_scale);

        char name_buf[max_name_length];
        const char* proc_name = "Unknown Processor";
        auto proc_it = registry.get_processors().find(info.port_hash);
        if (proc_it != registry.get_processors().end())
        {
            proc_name = proc_it->second->name.c_str();
        }
        std::strncpy(name_buf, proc_name, sizeof(name_buf));
        name_buf[sizeof(name_buf) - 1] = '\0';

        float text_w = ImGui::CalcTextSize(name_buf).x;
        float field_w = text_w + ImGui::GetStyle().FramePadding.x * 2.0f + 8.0f * dpi_scale;
        float max_field_w = header_max.x - header_min.x - 24.0f * dpi_scale;
        if (field_w > max_field_w) field_w = max_field_w;
        if (field_w < 32.0f * dpi_scale) field_w = 32.0f * dpi_scale;

        float field_x = header_min.x + (header_max.x - header_min.x - field_w) * 0.5f + 4.0f * dpi_scale;
        ImGui::SetCursorScreenPos(ImVec2(field_x, header_min.y + (h_height - ImGui::GetFrameHeight()) * 0.5f));
        ImGui::PushItemWidth(field_w);

        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

        ImGui::PushID("CompactProcRename");
        bool enter_pressed = ImGui::InputText("##proc_edit", name_buf, sizeof(name_buf), ImGuiInputTextFlags_EnterReturnsTrue);
        bool deactivated = ImGui::IsItemDeactivatedAfterEdit();
        ImGui::PopID();

        ImGui::PopStyleColor(3);
        ImGui::PopItemWidth();

        if ((enter_pressed || deactivated) && name_buf[0] != '\0' && std::strcmp(proc_name, name_buf) != 0)
        {
            adam::string_hash proposed_hash = adam::string_hashed(&name_buf[0]).get_hash();
            if (registry.get_processors().find(proposed_hash) == registry.get_processors().end())
            {
                adam::string_hashed new_name(&name_buf[0]);
                ctrl.enqueue_commander_action([&ctrl, port_hash = info.port_hash, new_name]() 
                { 
                    ctrl.commander().request_processor_rename(port_hash, new_name); 
                });
            }
        }

        ImGui::SetCursorScreenPos(ImVec2(header_min.x, header_max.y + 8.0f * dpi_scale));
    }

    static void draw_replay_statistics
    (
        const adam::registry_view& registry,
        const expanded_port_draw_info& info,
        adam::language lang,
        bool p_started
    )
    {
        auto p_it = registry.get_ports().find(info.port_hash);
        if (p_it == registry.get_ports().end() || !p_it->second->statistic_buffer)
        {
            return;
        }

        auto* replay_stats = p_it->second->statistic_buffer->data_as<adam::modules::recrep::port_input_replay::replay_state_buffer_data>();
        
        std::string full_path = replay_stats->file_name;
        size_t last_slash = full_path.find_last_of("/\\");
        std::string display_name = (last_slash != std::string::npos) ? full_path.substr(last_slash + 1) : full_path;

        ImGui::Spacing();
        ImGui::Text("%s: %s", get_gui_string(gui_string_id::lbl_replay_file, lang), display_name.c_str());
        if (ImGui::IsItemHovered() && !full_path.empty())
        {
            ImGui::SetTooltip("%s", full_path.c_str());
        }

        uint64_t duration_ns = replay_stats->file_time_end > replay_stats->file_time_start ? (replay_stats->file_time_end - replay_stats->file_time_start) : 0;
        double speed = 1.0;
        auto* speed_param = p_it->second->user_params.get<adam::configuration_parameter_double>("speed"_ct);
        if (speed_param)
        {
            speed = speed_param->get_value();
        }

        double total_play_time_sec = (speed > 0.0) ? ((static_cast<double>(duration_ns) / 1e9) / speed) : 0.0;
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
            progress_fraction = (speed > 0.0) ? (elapsed_real_sec / total_play_time_sec) : (p_started ? 1.0 : 0.0);
        }
        if (progress_fraction > 1.0) progress_fraction = 1.0;
        if (progress_fraction < 0.0) progress_fraction = 0.0;

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

        char overlay_buf[131];
        snprintf(overlay_buf, sizeof(overlay_buf), "%s / %s", elapsed_buf, total_buf);

        ImGui::ProgressBar(static_cast<float>(progress_fraction), ImVec2(-FLT_MIN, 0.0f), overlay_buf);
    }

    static void draw_expanded_node_statistics
    (
        const adam::registry_view& registry,
        const expanded_port_draw_info& info,
        adam::language lang,
        bool is_port,
        bool p_started,
        bool has_stats,
        adam::port::state_buffer_data* port_stats,
        adam::processor::state_buffer_data* proc_stats
    )
    {
        ImGui::PushID(reinterpret_cast<const void*>(static_cast<intptr_t>(info.unique_node_id ^ 0x6666)));
        bool stats_expanded = g_expanded_stats_nodes.count(info.unique_node_id) > 0;
        
        bool stats_tree_open = ImGui::TreeNodeEx(get_gui_string(gui_string_id::lbl_statistics, lang), stats_expanded ? ImGuiTreeNodeFlags_DefaultOpen : 0);
        if (stats_tree_open != stats_expanded)
        {
            if (stats_tree_open)
            {
                g_expanded_stats_nodes.insert(info.unique_node_id);
            }
            else
            {
                g_expanded_stats_nodes.erase(info.unique_node_id);
            }

            g_expanded_node_heights.erase(info.unique_node_id);
        }

        if (stats_tree_open)
        {
            ImGui::Unindent();
            
            if (has_stats && (port_stats || proc_stats))
            {
                auto format_bytes_to_buf = [](uint64_t bytes, char* buf, size_t buf_size) 
                {
                    if (bytes < 1024) snprintf(buf, buf_size, "%llu B", static_cast<unsigned long long>(bytes));
                    else if (bytes < 1024 * 1024) snprintf(buf, buf_size, "%.2f KB", bytes / 1024.0);
                    else if (bytes < 1024 * 1024 * 1024) snprintf(buf, buf_size, "%.2f MB", bytes / (1024.0 * 1024.0));
                    else snprintf(buf, buf_size, "%.2f GB", bytes / (1024.0 * 1024.0 * 1024.0));
                };
                
                if (ImGui::BeginTable("PortStatsTable", 3, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg))
                {
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, ImGui::GetTextLineHeight() * 6.f);
                    ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_messages, lang), ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_size, lang), ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableHeadersRow();

                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    char buf_in[64];
                    switch (info.type)
                    {
                        case node_type_input:
                            ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_read, lang));
                            ImGui::TableNextColumn();
                            ImGui::Text("%llu", static_cast<unsigned long long>(port_stats->total_buffers_read));
                            ImGui::TableNextColumn();
                            format_bytes_to_buf(port_stats->total_bytes_read, buf_in, sizeof(buf_in));
                            ImGui::TextUnformatted(buf_in);
                            break;
                        case node_type_output:
                            ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_received, lang));
                            ImGui::TableNextColumn();
                            ImGui::Text("%llu", static_cast<unsigned long long>(port_stats->total_buffers_recieved));
                            ImGui::TableNextColumn();
                            format_bytes_to_buf(port_stats->total_bytes_recieved, buf_in, sizeof(buf_in));
                            ImGui::TextUnformatted(buf_in);
                            break;
                        case node_type_filter:
                        case node_type_converter:
                            ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_received, lang));
                            ImGui::TableNextColumn();
                            ImGui::Text("%llu", static_cast<unsigned long long>(proc_stats->total_buffers_recieved));
                            ImGui::TableNextColumn();
                            format_bytes_to_buf(proc_stats->total_bytes_recieved, buf_in, sizeof(buf_in));
                            ImGui::TextUnformatted(buf_in);
                            break;
                    }
                    
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    char buf_forwarded[64];
                    switch (info.type)
                    {
                        case node_type_input:
                        case node_type_output:
                            ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_discarded, lang));
                            ImGui::TableNextColumn();
                            ImGui::Text("%llu", static_cast<unsigned long long>(port_stats->total_buffers_discarded));
                            ImGui::TableNextColumn();
                            format_bytes_to_buf(port_stats->total_bytes_discarded, buf_forwarded, sizeof(buf_forwarded));
                            ImGui::TextUnformatted(buf_forwarded);
                            break;
                        case node_type_filter:
                            ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_filtered, lang));
                            ImGui::TableNextColumn();
                            ImGui::Text("%llu", static_cast<unsigned long long>(proc_stats->total_buffers_discarded));
                            ImGui::TableNextColumn();
                            format_bytes_to_buf(proc_stats->total_bytes_discarded, buf_forwarded, sizeof(buf_forwarded));
                            ImGui::TextUnformatted(buf_forwarded);
                            break;
                        case node_type_converter:
                            ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_discarded, lang));
                            ImGui::TableNextColumn();
                            ImGui::Text("%llu", static_cast<unsigned long long>(proc_stats->total_buffers_discarded));
                            ImGui::TableNextColumn();
                            format_bytes_to_buf(proc_stats->total_bytes_discarded, buf_forwarded, sizeof(buf_forwarded));
                            ImGui::TextUnformatted(buf_forwarded);
                            break;
                    }
                    
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    char buf_out[64];
                    switch (info.type)
                    {
                        case node_type_output:
                            ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_written, lang));
                            ImGui::TableNextColumn();
                            ImGui::Text("%llu", static_cast<unsigned long long>(port_stats->total_buffers_written));
                            ImGui::TableNextColumn();
                            format_bytes_to_buf(port_stats->total_bytes_written, buf_out, sizeof(buf_out));
                            ImGui::TextUnformatted(buf_out);
                            break;
                        case node_type_input:
                            ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_forwarded, lang));
                            ImGui::TableNextColumn();
                            ImGui::Text("%llu", static_cast<unsigned long long>(port_stats->total_buffers_forwarded));
                            ImGui::TableNextColumn();
                            format_bytes_to_buf(port_stats->total_bytes_forwarded, buf_out, sizeof(buf_out));
                            ImGui::TextUnformatted(buf_out);
                            break;
                        case node_type_filter:
                            ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_forwarded, lang));
                            ImGui::TableNextColumn();
                            ImGui::Text("%llu", static_cast<unsigned long long>(proc_stats->total_buffers_forwarded));
                            ImGui::TableNextColumn();
                            format_bytes_to_buf(proc_stats->total_bytes_forwarded, buf_out, sizeof(buf_out));
                            ImGui::TextUnformatted(buf_out);
                            break;
                        case node_type_converter:
                            ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_converted, lang));
                            ImGui::TableNextColumn();
                            ImGui::Text("%llu", static_cast<unsigned long long>(proc_stats->total_buffers_forwarded));
                            ImGui::TableNextColumn();
                            format_bytes_to_buf(proc_stats->total_bytes_forwarded, buf_out, sizeof(buf_out));
                            ImGui::TextUnformatted(buf_out);
                            break;
                    }
                    
                    ImGui::EndTable();
                }

                auto p_it = registry.get_ports().find(info.port_hash);
                bool is_replay = (is_port && p_it != registry.get_ports().end() && p_it->second->type == "replay"_ct && p_it->second->type_module == "recrep"_ct);
                if (is_replay && p_started)
                {
                    draw_replay_statistics(registry, info, lang, p_started);
                }
            }
            else
            {
                ImGui::TextDisabled("%s", get_gui_string(gui_string_id::lbl_stat_unavailable, lang));
            }
            
            ImGui::Indent();
            ImGui::TreePop();
        }
        ImGui::PopID();
        ImGui::Separator();
    }

    static void draw_expanded_node_parameters
    (
        gui_controller& ctrl,
        adam::language lang,
        const expanded_port_draw_info& info,
        bool is_port,
        bool disable_params,
        const adam::configuration_parameter_list_sorted* user_params
    )
    {
        if (!user_params || user_params->get_children().empty())
        {
            return;
        }

        ImGui::PushID(reinterpret_cast<const void*>(static_cast<intptr_t>(info.unique_node_id ^ 0x5555)));
        bool param_expanded = g_expanded_param_nodes.count(info.unique_node_id) > 0;
        
        bool tree_open = ImGui::TreeNodeEx(get_gui_string(gui_string_id::lbl_parameters, lang), param_expanded ? ImGuiTreeNodeFlags_DefaultOpen : 0);
        if (tree_open != param_expanded)
        {
            if (tree_open)
            {
                g_expanded_param_nodes.insert(info.unique_node_id);
            }
            else
            {
                g_expanded_param_nodes.erase(info.unique_node_id);
            }

            g_expanded_node_heights.erase(info.unique_node_id);
        }

        if (tree_open)
        {
            ImGui::Unindent();

            if (disable_params)
            {
                ImGui::BeginDisabled();
            }

            float avail_w = ImGui::GetContentRegionAvail().x;
            for (auto hash : user_params->get_order())
            { 
                if (auto* param_ptr = user_params->get(hash))
                {
                    draw_configuration_parameter(ctrl, lang, info.port_hash, param_ptr->get_name(), param_ptr, is_port ? param_target_type::port : param_target_type::processor, avail_w);
                }
            }

            if (disable_params)
            {
                ImGui::EndDisabled();
            }
            
            ImGui::Indent();
            ImGui::TreePop();
        }
        ImGui::PopID();
        ImGui::Separator();
    }

    static void draw_expanded_node_inject_data
    (
        gui_controller& ctrl,
        adam::language lang,
        const expanded_port_draw_info& info,
        float dpi_scale,
        bool disable_inject
    )
    {
        ImGui::PushID(reinterpret_cast<const void*>(static_cast<intptr_t>(info.unique_node_id ^ 0x3333)));
        bool inject_expanded = g_expanded_inject_nodes.count(info.unique_node_id) > 0;
        
        bool tree_open = ImGui::TreeNodeEx(get_gui_string(gui_string_id::lbl_inject_data, lang), inject_expanded ? ImGuiTreeNodeFlags_DefaultOpen : 0);
        if (tree_open != inject_expanded)
        {
            if (tree_open)
            {
                g_expanded_inject_nodes.insert(info.unique_node_id);
            }
            else
            {
                g_expanded_inject_nodes.erase(info.unique_node_id);
            }

            g_expanded_node_heights.erase(info.unique_node_id);
        }

        if (tree_open)
        {
            ImGui::Unindent();

            auto& inject_state = g_inject_data_buffers[info.unique_node_id];
            if (inject_state.text_buffer.size() < 1024)
            {
                inject_state.text_buffer.resize(1024, '\0');
            }
            
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

            bool should_disable_inject = disable_inject || !inject_state.is_valid;
            if (should_disable_inject)
            {
                ImGui::BeginDisabled();
            }

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

            if (should_disable_inject)
            {
                ImGui::EndDisabled();
            }

            ImGui::Indent();
            ImGui::TreePop();
        }
        ImGui::PopID();
        ImGui::Separator();
    }

    static void draw_expanded_node_remove_action
    (
        gui_controller& ctrl,
        adam::language lang,
        const expanded_port_draw_info& info,
        float exp_pad,
        bool is_port
    )
    {
        ImGui::PushID(reinterpret_cast<const void*>(static_cast<intptr_t>(info.port_hash ^ 0x4444)));
        if (is_port)
        {
            if (ImGui::Button(get_gui_string(gui_string_id::btn_remove_port, lang), ImVec2(info.current_node_w - exp_pad * 2.0f, 0)))
            {
                bool isolated = true;
                for (const auto& [ch, c] : ctrl.commander().registry().get_connections())
                {
                    if (ch != info.hash)
                    {
                        if (std::find(c->inputs.begin(), c->inputs.end(), info.port_hash) != c->inputs.end() ||
                            std::find(c->outputs.begin(), c->outputs.end(), info.port_hash) != c->outputs.end())
                        {
                            isolated = false;
                            break;
                        }
                    }
                }

                if (isolated)
                {
                    g_port_to_delete_hash = info.port_hash;
                    g_port_to_delete_conn_hash = info.hash;
                    g_port_to_delete_is_input = (info.type == node_type_input);
                    g_request_delete_port_popup = true;
                }
                else
                {
                    ctrl.enqueue_commander_action([&ctrl, conn_hash = info.hash, port_hash = info.port_hash, is_input = (info.type == node_type_input)]() 
                    { 
                        ctrl.commander().request_connection_port_remove(conn_hash, port_hash, is_input); 
                    });
                }
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
        float exp_w = info.current_node_w;
        float header_w = info.header_w;

        float left_x = (info.p_max.x - header_w * 0.5f) - exp_w * 0.5f;
        float right_x = left_x + exp_w;

        float win_min_x = ImGui::GetWindowPos().x;
        float win_max_x = win_min_x + ImGui::GetWindowWidth();
        float margin = 4.0f * dpi_scale;
        if (left_x < win_min_x + margin)
        {
            left_x = win_min_x + margin;
            right_x = left_x + exp_w;
        }
        else if (right_x > win_max_x - margin)
        {
            right_x = win_max_x - margin;
            left_x = right_x - exp_w;
        }

        float gap = -1.5f * dpi_scale;
        if (info.type == node_type_filter && info.header_w < info.current_node_w)
        {
            gap = 6.0f * dpi_scale;
        }
        ImVec2 exp_min(left_x, info.p_max.y + gap);
        ImVec2 exp_max(right_x, info.p_max.y + info.this_expanded_h);

        ImVec4 bg_col = ImGui::GetStyleColorVec4(ImGuiCol_PopupBg);
        bg_col.w = 1.0f;
        
        ImDrawFlags corners = (gap > 0.0f) ? ImDrawFlags_RoundCornersAll : ImDrawFlags_RoundCornersBottom;
        draw_list->AddRectFilled(exp_min, exp_max, ImColor(bg_col), 6.0f * dpi_scale, corners);
        draw_list->AddRect(exp_min, exp_max, ImColor(info.captured_color.Value.x * 1.2f, info.captured_color.Value.y * 1.2f, info.captured_color.Value.z * 1.2f), 6.0f * dpi_scale, corners, 1.5f * dpi_scale);

        if (info.type == node_type_filter && info.header_w < info.current_node_w)
        {
            float center_x = info.p_max.x - info.header_w * 0.5f;
            draw_list->AddLine(
                ImVec2(center_x, info.p_max.y), 
                ImVec2(center_x, info.p_max.y + gap), 
                info.captured_color, 
                2.0f * dpi_scale
            );
        }

        bool p_started = false;
        const char* p_type = "Unknown";
        const char* p_module = "Unknown";
        adam::port::state_buffer_data* port_stats = nullptr;
        adam::processor::state_buffer_data* proc_stats = nullptr;
        bool has_stats = false;
        bool is_port = (info.type == node_type_input || info.type == node_type_output);
        auto p_it = registry.get_ports().find(info.port_hash);
        auto proc_it = registry.get_processors().find(info.port_hash);
        const adam::configuration_parameter_list_sorted* user_params = nullptr;

        bool is_unavailable = false;
        bool module_missing = false;

        if (is_port && p_it != registry.get_ports().end())
        {
            p_started = p_it->second->started;
            user_params = &p_it->second->user_params;
            is_unavailable = p_it->second->is_unavailable;
            module_missing = !p_it->second->type_module.empty() && !ctrl.commander().get_modules().is_module_loaded(p_it->second->type_module);
            if (p_it->second->statistic_buffer)
            {
                port_stats = p_it->second->statistic_buffer->data_as<adam::port::state_buffer_data>();
                has_stats = true;
            }
            p_type = p_it->second->type.c_str();
            p_module = p_it->second->type_module.c_str();
        }
        else if (!is_port && proc_it != registry.get_processors().end())
        {
            user_params = &proc_it->second->user_params;
            is_unavailable = proc_it->second->is_unavailable;
            module_missing = !proc_it->second->module_name.empty() && !ctrl.commander().get_modules().is_module_loaded(proc_it->second->module_name);
            if (proc_it->second->state_buffer)
            {
                proc_stats = proc_it->second->state_buffer->data_as<adam::processor::state_buffer_data>();
                has_stats = true;
            }
            p_type = proc_it->second->type.c_str();
            p_module = proc_it->second->module_name.c_str();
        }

        ImGui::SetCursorScreenPos(ImVec2(exp_min.x + exp_pad, exp_min.y + exp_pad));
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        bool is_child_visible = ImGui::BeginChild(static_cast<ImGuiID>(info.unique_node_id), ImVec2(info.current_node_w - exp_pad * 2.0f, info.this_expanded_h - gap - exp_pad * 2.0f), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
        
        if (!is_child_visible)
        {
            ImGui::EndChild();
            return;
        }

        if (info.type == node_type_filter && info.header_w < info.current_node_w)
        {
            draw_compact_processor_header(ctrl, registry, info, exp_pad, dpi_scale);
        }

        ImGui::TextColored(get_gui_color(gui_color_id::log_info), "%s [%s]", p_type, p_module);
        ImGui::Separator();

        bool disable_controls = p_started || is_unavailable || module_missing;

        if (is_port)
        {
            float btn_w = (ImGui::GetContentRegionAvail().x - exp_pad) * 0.5f;
            ImGui::PushID(reinterpret_cast<const void*>(static_cast<intptr_t>(info.port_hash ^ 0x1111)));
            if (disable_controls)
            {
                ImGui::BeginDisabled();
            }
            if (ImGui::Button(get_gui_string(gui_string_id::btn_start, lang), ImVec2(btn_w, 0)))
            {
                ctrl.enqueue_commander_action([&ctrl, port_hash = info.port_hash]() { ctrl.commander().request_port_start(port_hash); });
            }
            if (disable_controls)
            {
                ImGui::EndDisabled();
            }
            ImGui::PopID();

            ImGui::SameLine(0.0f, exp_pad);

            ImGui::PushID(reinterpret_cast<const void*>(static_cast<intptr_t>(info.port_hash ^ 0x2222)));
            if (!p_started || is_unavailable || module_missing)
            {
                ImGui::BeginDisabled();
            }
            if (ImGui::Button(get_gui_string(gui_string_id::btn_stop, lang), ImVec2(btn_w, 0)))
            {
                ctrl.enqueue_commander_action([&ctrl, port_hash = info.port_hash]() { ctrl.commander().request_port_stop(port_hash); });
            }
            if (!p_started || is_unavailable || module_missing)
            {
                ImGui::EndDisabled();
            }
            ImGui::PopID();
            
            ImGui::Separator();
        }

        draw_expanded_node_statistics(registry, info, lang, is_port, p_started, has_stats, port_stats, proc_stats);
        draw_expanded_node_parameters(ctrl, lang, info, is_port, disable_controls, user_params);

        if (is_port)
        {
            draw_expanded_node_inject_data(ctrl, lang, info, dpi_scale, !p_started || is_unavailable || module_missing);
        }

        draw_expanded_node_remove_action(ctrl, lang, info, exp_pad, is_port);

        float exact_inner_h = ImGui::GetCursorPosY() - ImGui::GetStyle().ItemSpacing.y;
        g_expanded_node_heights[info.unique_node_id] = exact_inner_h + exp_pad * 2.0f + gap;

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
        (void)conn;
        bool is_light_theme = false;
        auto* theme_param = dynamic_cast<adam::configuration_parameter_string*>(ctrl.get_parameters().get("theme"_ct));
        if (theme_param && theme_param->get_value() == "light"_ct)
        {
            is_light_theme = true;
        }

        const auto& ports = ctrl.commander().registry().get_ports();

        float current_node_w = port_w;
        if (total_stages > 2 && stage > 0 && stage < total_stages - 1)
        {
            current_node_w = proc_w;
        }
            
        bool compact_processors = false;
        if (total_stages > 2)
        {
            float gap_if_large = (avail_x - static_cast<float>(total_stages) * port_w) / static_cast<float>(total_stages - 1);
            if (gap_if_large < 10.0f * dpi_scale)
            {
                compact_processors = true;
            }
        }
            
        float node_x;
        if (total_stages == 1) node_x = cur_pos.x;
        else if (stage == 0) node_x = cur_pos.x;
        else if (stage == total_stages - 1) node_x = cur_pos.x + avail_x - current_node_w;
        else node_x = cur_pos.x + port_w + gap + static_cast<float>(stage - 1) * (proc_w + gap);
            
        float node_y = cur_pos.y + row * row_height + (row_height - node_h) * 0.5f + extra_y;
            
        ImVec2 p_min(node_x, node_y);
        ImVec2 p_max(node_x + current_node_w, node_y + node_h);

        bool is_expanded = false;
        uint64_t unique_node_id = 0;
        if (port_hash != 0)
        {
            unique_node_id = get_unique_node_id(port_hash, hash, stage, type);
            if (is_unavail)
            {
                g_expanded_nodes.erase(unique_node_id);
            }
            else
            {
                is_expanded = g_expanded_nodes.count(unique_node_id) > 0;
            }
        }

        ImDrawFlags header_corners = ImDrawFlags_RoundCornersAll;
        if (is_expanded && !(type == node_type_filter && compact_processors))
        {
            header_corners = ImDrawFlags_RoundCornersTop;
        }
            
        draw_list->AddRectFilled(p_min, p_max, color, 6.0f * dpi_scale, header_corners);
        draw_list->AddRect(p_min, p_max, ImColor(color.Value.x * 1.2f, color.Value.y * 1.2f, color.Value.z * 1.2f, color.Value.w), 6.0f * dpi_scale, header_corners, 1.5f * dpi_scale);

        ImGui::SetCursorScreenPos(p_min);
        ImGui::PushID(reinterpret_cast<const void*>(static_cast<intptr_t>(get_unique_node_id(port_hash, hash, stage, type) ^ 0xABCD)));
        ImGui::SetNextItemAllowOverlap();
        bool clicked = ImGui::InvisibleButton("##node_btn", ImVec2(current_node_w, node_h));

        if (type == node_type_filter && !is_drag_preview)
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

        if (port_hash != 0 && !is_unavail)
        {
            if (clicked)
            {
                if (is_expanded)
                {
                    g_expanded_nodes.erase(unique_node_id);
                }
                else
                {
                    if (compact_processors)
                    {
                        g_expanded_nodes.clear();
                    }
                    g_expanded_nodes.insert(unique_node_id);
                }
                is_expanded = !is_expanded;
            }
        }
        ImGui::PopID();
        
        if (is_expanded && !is_drag_preview)
        {
            ImColor captured_color = color;
            unique_node_id = get_unique_node_id(port_hash, hash, stage, type);
            float this_expanded_h = get_expanded_node_height(unique_node_id, dpi_scale);
            
            float exp_w = (type == node_type_filter) ? port_w : current_node_w;

            deferred_expansions.push_back
            ({
                type,
                stage,
                port_hash,
                unique_node_id,
                p_max,
                exp_w,
                captured_color,
                this_expanded_h,
                hash,
                current_node_w
            });
        }
            
        bool allow_edit = (port_hash != 0 && !is_drag_preview);
        if (type == node_type_filter && compact_processors)
        {
            allow_edit = false;
        }

        if (allow_edit)
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
            {
                input_w = current_node_w - 16.0f * dpi_scale;
            }
            if (input_w < 32.0f * dpi_scale)
            {
                input_w = 32.0f * dpi_scale;
            }
                    
            float input_x = p_min.x + (current_node_w - input_w) * 0.5f + 4.0f * dpi_scale;
            float input_h = ImGui::GetFrameHeight();
                
            ImGui::SetCursorScreenPos(ImVec2(input_x, p_min.y + (node_h - input_h) * 0.5f));
            ImGui::PushItemWidth(input_w);
                
            ImGui::PushID(reinterpret_cast<const void*>(static_cast<intptr_t>(get_unique_node_id(port_hash, hash, stage, type))));
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
            float min_margin = (type == node_type_filter && compact_processors) ? 2.0f * dpi_scale : 8.0f * dpi_scale;
            if (text_x < p_min.x + min_margin)
            {
                text_x = p_min.x + min_margin;
            }
            ImVec2 text_pos(text_x, p_min.y + (node_h - ImGui::GetTextLineHeight()) * 0.5f);
            draw_list->PushClipRect(p_min, p_max, true);
            draw_list->AddText(text_pos, ImColor(1.0f, 1.0f, 1.0f, color.Value.w), name);
            draw_list->PopClipRect();
        }

        out_pin_in.pos = ImVec2(p_min.x, p_min.y + node_h * 0.5f);
        out_pin_out.pos = ImVec2(p_max.x, p_min.y + node_h * 0.5f);

        ImColor default_line_col = is_light_theme ? get_gui_color(gui_color_id::node_connection_line_light) : get_gui_color(gui_color_id::node_connection_line);
        ImColor status_col = default_line_col;
        if (port_hash != 0)
        {
            auto p_it = ports.find(port_hash);
            if (p_it != ports.end())
            {
                if (p_it->second->statistic_buffer)
                {
                    auto* stats = p_it->second->statistic_buffer->data_as<adam::port::state_buffer_data>();
                    switch (stats->cur_state)
                    {
                        case adam::port::state_running:
                        case adam::port::state_started:
                            if (p_it->second->started)
                            {
                                status_col = get_gui_color(gui_color_id::node_pin_active);
                            }
                            break;
                        case adam::port::state_inactive:
                        case adam::port::state_starting:
                        case adam::port::state_stopping:
                            if (p_it->second->started)
                            {
                                status_col = get_gui_color(gui_color_id::log_warning);
                            }
                            break;
                        case adam::port::state_error:
                            status_col = get_gui_color(gui_color_id::log_error);
                            break;
                        default:
                            break;
                    }
                }
                else if (p_it->second->started)
                {
                    status_col = get_gui_color(gui_color_id::node_pin_active);
                }
            }
        }

        if (type == node_type_input)
        {
            out_pin_in.col = status_col;
            out_pin_out.col = default_line_col;
        }
        else if (type == node_type_output)
        {
            out_pin_in.col = default_line_col;
            out_pin_out.col = status_col;
        }
        else
        {
            out_pin_in.col = default_line_col;
            out_pin_out.col = default_line_col;
        }
                
        if (is_unavail && !is_drag_preview && ImGui::IsMouseHoveringRect(p_min, p_max))
        {
            ImGui::BeginTooltip();
            ImGui::Text(get_gui_string(gui_string_id::tt_module_missing, lang), unavail_module ? unavail_module : "Unknown");
            ImGui::EndTooltip();
        }

        return is_expanded;
    }
}
