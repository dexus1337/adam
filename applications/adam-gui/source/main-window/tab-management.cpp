#include "tab-management.hpp"
#include "main-window.hpp"

#include <imgui.h>
#include <algorithm>
#include <vector>
#include <mutex>
#include <string>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <chrono>

#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "configuration/parameters/configuration-parameter-double.hpp"
#include "configuration/parameters/configuration-parameter-string.hpp"
#include "configuration/parameters/configuration-parameter-list.hpp"
#include "controller/controller.hpp"
#include "module/module.hpp"
#include "data/port/port-input-replay.hpp"

namespace adam::gui 
{
    namespace
    {
        adam::string_hashed g_connection_to_delete("");
        bool g_request_delete_popup = false;
        ImVec2 g_connection_drag_offset(0, 0);

        adam::string_hashed g_target_connection("");
        adam::port::direction g_target_direction = adam::port::direction_invalid;
        bool g_request_port_popup = false;
        char g_port_popup_title[max_name_length] = "Add Port###PortPopup";

        adam::string_hash g_active_drag_hash = 0;
        size_t g_active_drag_target_index = 0;

        std::unordered_set<uint64_t> g_expanded_nodes;
        
        std::unordered_set<uint64_t> g_expanded_inject_nodes;

        std::unordered_set<uint64_t> g_expanded_param_nodes;
        std::unordered_set<adam::string_hash> g_expanded_inspector_ports;
        std::unordered_set<adam::string_hash> g_pending_inspector_ports;

        std::unordered_set<adam::string_hash> g_expanded_inspector_connections_input;
        std::unordered_set<adam::string_hash> g_pending_inspector_connections_input;
        
        std::unordered_set<adam::string_hash> g_expanded_inspector_connections_output;
        std::unordered_set<adam::string_hash> g_pending_inspector_connections_output;

        std::unordered_map<uint64_t, float> g_expanded_node_heights;
        
        bool g_request_open_inspector = false;
        adam::string_hash g_port_to_expand_in_inspector = 0;
        adam::string_hash g_connection_input_to_expand_in_inspector = 0;
        adam::string_hash g_connection_output_to_expand_in_inspector = 0;        static inline uint64_t get_unique_node_id(uint64_t port_hash, uint64_t conn_hash, int stage)
        {
            uint64_t id = port_hash;
            id ^= (conn_hash + 0x9e3779b97f4a7c15ULL + (id << 6) + (id >> 2));
            id ^= (static_cast<uint64_t>(stage) + 0x9e3779b97f4a7c15ULL + (id << 6) + (id >> 2));
            if (static_cast<ImGuiID>(id) == 0)
                id ^= 0x1337;
            return id;
        }

        struct inject_buffer_state
        {
            std::string text_buffer;
            std::string last_parsed_text;
            std::vector<uint8_t> parsed_bytes;
            bool is_valid;

            inject_buffer_state() : is_valid(true)
            {
                text_buffer.resize(1024, '\0');
            }
        };

        static int inject_text_resize_callback(ImGuiInputTextCallbackData* data)
        {
            if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
            {
                std::string* str = static_cast<std::string*>(data->UserData);
                str->resize(data->BufSize);
                data->Buf = str->data();
            }
            return 0;
        }

        std::map<uint64_t, inject_buffer_state> g_inject_data_buffers;

        struct connection_pin_data
        {
            ImVec2 pos;
            ImColor col;
            std::string format_name;
        };

        std::vector<std::vector<connection_pin_data>> g_stage_pins_in_normal, g_stage_pins_out_normal;
        std::vector<std::vector<connection_pin_data>> g_stage_pins_in_preview, g_stage_pins_out_preview;

        enum node_type : uint8_t
        {
            node_type_input,
            node_type_output,
            node_type_processor,
            node_type_converter
        };

        struct expanded_port_render_info
        {
            node_type type;
            int stage;
            adam::string_hash port_hash;
            uint64_t unique_node_id;
            ImVec2 p_max;
            float current_node_w;
            ImColor captured_color;
            float this_expanded_h;
            adam::string_hash hash;
        };

        std::pair<bool, std::vector<uint8_t>> parse_hex_bytes(const std::string& input)
        {
            std::vector<uint8_t> bytes;
            std::string current_token;
            bool is_valid = true;

            auto process_token = [&bytes, &is_valid](const std::string& token)
            {
                if (!is_valid)
                {
                    return;
                }

                std::string hex_str;
                size_t start = 0;
                if (token.size() >= 2 && token[0] == '0' && (token[1] == 'x' || token[1] == 'X'))
                {
                    start = 2;
                }

                for (size_t i = start; i < token.size(); ++i)
                {
                    char c = token[i];
                    if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))
                    {
                        hex_str += c;
                    }
                    else
                    {
                        is_valid = false;
                        return;
                    }
                }

                if (hex_str.empty())
                {
                    is_valid = false;
                    return;
                }

                if (hex_str.size() % 2 != 0)
                {
                    hex_str = "0" + hex_str;
                }

                for (size_t i = 0; i < hex_str.size(); i += 2)
                {
                    auto from_hex = [](char c) -> uint8_t
                    {
                        if (c >= '0' && c <= '9') return static_cast<uint8_t>(c - '0');
                        if (c >= 'a' && c <= 'f') return static_cast<uint8_t>(c - 'a' + 10);
                        if (c >= 'A' && c <= 'F') return static_cast<uint8_t>(c - 'A' + 10);
                        return 0;
                    };
                    bytes.push_back(static_cast<uint8_t>((from_hex(hex_str[i]) << 4) | from_hex(hex_str[i + 1])));
                }
            };

            for (char c : input)
            {
                if (std::isspace(static_cast<unsigned char>(c)) || c == ',')
                {
                    if (!current_token.empty())
                    {
                        process_token(current_token);
                        current_token.clear();
                    }
                }
                else
                {
                    current_token += c;
                }
            }

            if (!current_token.empty())
            {
                process_token(current_token);
            }

            return {is_valid, bytes};
        }

        // ---------------------------------------------------------------------------
        // fill_hex_preview
        // Fills hex_buf (up to max_bytes hex pairs) and ascii_buf with a printable
        // preview of the given byte range. Both buffers must be sized by caller.
        // ---------------------------------------------------------------------------
        static void fill_hex_preview(
            const uint8_t* data, size_t data_len, size_t max_bytes,
            char* hex_buf,  size_t hex_buf_size,
            char* ascii_buf, size_t ascii_buf_size)
        {
            char* p_hex   = hex_buf;
            char* p_ascii = ascii_buf;
            size_t n = std::min(data_len, max_bytes);
            for (size_t k = 0; k < n; ++k)
            {
                int written = snprintf(p_hex, hex_buf_size - (p_hex - hex_buf), "%02X ", data[k]);
                if (written > 0) p_hex += written;
                *p_ascii++ = (data[k] >= 32 && data[k] <= 126) ? static_cast<char>(data[k]) : '.';
            }
            if (data_len > max_bytes)
            {
                if (p_hex > hex_buf) *(p_hex - 1) = '\0';
                snprintf(p_hex,   hex_buf_size   - (p_hex   - hex_buf),   "...");
                snprintf(p_ascii, ascii_buf_size - (p_ascii - ascii_buf), "...");
            }
            else
            {
                if (p_hex > hex_buf) *(p_hex - 1) = '\0';
                *p_ascii = '\0';
            }
        }

        // ---------------------------------------------------------------------------
        // draw_direction_badge
        // Renders a coloured In/Out/In+Out label for a port direction.
        // is_used dims the colours by half alpha.
        // ---------------------------------------------------------------------------
        static void draw_direction_badge(adam::port::direction dir, bool is_used, adam::language lang)
        {
            const bool has_in  = (dir & adam::port::direction_in)  != adam::port::direction_invalid;
            const bool has_out = (dir & adam::port::direction_out) != adam::port::direction_invalid;

            if (has_in && has_out)
            {
                ImVec4 in_col  = get_gui_color(gui_color_id::node_input);
                ImVec4 out_col = get_gui_color(gui_color_id::node_output);
                if (is_used) { in_col.w *= 0.5f; out_col.w *= 0.5f; }
                ImGui::TextColored(in_col,  "%s", get_gui_string(gui_string_id::lbl_badge_in_short,  lang));
                ImGui::SameLine(0, 0);
                ImGui::TextUnformatted("/");
                ImGui::SameLine(0, 0);
                ImGui::TextColored(out_col, "%s", get_gui_string(gui_string_id::lbl_badge_out_short, lang));
            }
            else if (has_in)
            {
                ImVec4 col = get_gui_color(gui_color_id::node_input);
                if (is_used) col.w *= 0.5f;
                ImGui::TextColored(col, "%s", get_gui_string(gui_string_id::lbl_badge_input, lang));
            }
            else if (has_out)
            {
                ImVec4 col = get_gui_color(gui_color_id::node_output);
                if (is_used) col.w *= 0.5f;
                ImGui::TextColored(col, "%s", get_gui_string(gui_string_id::lbl_badge_output, lang));
            }
            else
            {
                ImGui::TextDisabled("Unknown");
            }
        }

        // ---------------------------------------------------------------------------
        // make_inspector_buffer_callback
        // Returns the std::function used when creating a data inspector for a port.
        // Both the draw_node right-click path and the inspector-tab checkbox share
        // the exact same logic.
        // ---------------------------------------------------------------------------
        static std::function<void(adam::buffer*)> make_inspector_buffer_callback(adam::string_hash port_hash)
        {
            return [port_hash](adam::buffer* buf)
            {
                if (!buf) return;
                
                std::lock_guard<std::mutex> lock(adam::gui::g_inspection_data.mtx);
                auto& port_data = adam::gui::g_inspection_data.ports[port_hash];
                
                adam::gui::inspected_buffer ib;
                ib.timestamp = buf->get_timestamp();
                if (ib.timestamp == 0)
                    ib.timestamp = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
                
                ib.size = buf->get_size();
                ib.offset = static_cast<uint32_t>(port_data.data_pool.size());
                
                if (ib.size > 0 && buf->get_data())
                {
                    const uint8_t* ptr = buf->get_data_as<uint8_t>();
                    port_data.data_pool.insert(port_data.data_pool.end(), ptr, ptr + ib.size);
                }
                
                port_data.buffers.push_back(ib);
            };
        }

        static std::function<void(adam::buffer*)> make_inspector_connection_input_buffer_callback(adam::string_hash conn_hash)
        {
            return [conn_hash](adam::buffer* buf)
            {
                if (!buf) return;
                
                std::lock_guard<std::mutex> lock(adam::gui::g_inspection_data.mtx);
                auto& port_data = adam::gui::g_inspection_data.connections_input[conn_hash];
                
                adam::gui::inspected_buffer ib;
                ib.timestamp = buf->get_timestamp();
                if (ib.timestamp == 0)
                    ib.timestamp = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
                
                ib.size = buf->get_size();
                ib.offset = static_cast<uint32_t>(port_data.data_pool.size());
                
                if (ib.size > 0 && buf->get_data())
                {
                    const uint8_t* ptr = buf->get_data_as<uint8_t>();
                    port_data.data_pool.insert(port_data.data_pool.end(), ptr, ptr + ib.size);
                }
                
                port_data.buffers.push_back(ib);
            };
        }

        static std::function<void(adam::buffer*)> make_inspector_connection_output_buffer_callback(adam::string_hash conn_hash)
        {
            return [conn_hash](adam::buffer* buf)
            {
                if (!buf) return;
                
                std::lock_guard<std::mutex> lock(adam::gui::g_inspection_data.mtx);
                auto& port_data = adam::gui::g_inspection_data.connections_output[conn_hash];
                
                adam::gui::inspected_buffer ib;
                ib.timestamp = buf->get_timestamp();
                if (ib.timestamp == 0)
                    ib.timestamp = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
                
                ib.size = buf->get_size();
                ib.offset = static_cast<uint32_t>(port_data.data_pool.size());
                
                if (ib.size > 0 && buf->get_data())
                {
                    const uint8_t* ptr = buf->get_data_as<uint8_t>();
                    port_data.data_pool.insert(port_data.data_pool.end(), ptr, ptr + ib.size);
                }
                
                port_data.buffers.push_back(ib);
            };
        }

        void draw_expanded_port_node
        (
            gui_controller& ctrl,
            adam::language lang,
            const adam::registry_view::port_map& ports,
            float dpi_scale,
            ImDrawList* draw_list,
            const expanded_port_render_info& info)
        {
            float exp_pad = 8.0f * dpi_scale;
            ImVec2 exp_min(info.p_max.x - info.current_node_w, info.p_max.y - 1.5f * dpi_scale);
            ImVec2 exp_max(info.p_max.x, info.p_max.y + info.this_expanded_h);

            ImVec4 bg_col = ImGui::GetStyleColorVec4(ImGuiCol_PopupBg);
            bg_col.w = 1.0f;
            draw_list->AddRectFilled(exp_min, exp_max, ImColor(bg_col), 6.0f * dpi_scale, ImDrawFlags_RoundCornersBottom);
            draw_list->AddRect(exp_min, exp_max, ImColor(info.captured_color.Value.x * 1.2f, info.captured_color.Value.y * 1.2f, info.captured_color.Value.z * 1.2f), 6.0f * dpi_scale, ImDrawFlags_RoundCornersBottom, 1.5f * dpi_scale);

            // Draw type and module
            bool p_is_active = false;
            const char* p_type = "Unknown";
            const char* p_module = "Unknown";
            adam::port::state_buffer_data* stats;
            bool has_stats = false;
            auto p_it = ports.find(info.port_hash);
            if (p_it != ports.end())
            {
                p_is_active = p_it->second->is_active;
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

            ImGui::SetCursorScreenPos(ImVec2(exp_min.x + exp_pad, exp_min.y + exp_pad));
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::BeginChild(static_cast<ImGuiID>(info.unique_node_id), ImVec2(info.current_node_w - exp_pad * 2.0f, info.this_expanded_h - exp_pad * 2.0f), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
            
            ImGui::TextColored(get_gui_color(gui_color_id::log_info), "%s [%s]", p_type, p_module);
            ImGui::Separator();

            // Draw stats
            if (has_stats)
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
                bool is_replay = (p_it != ports.end() && p_it->second->type == "replay"_ct && p_it->second->type_module == "recrep"_ct);
                if (is_replay && p_is_active)
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
                    if (p_is_active)
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
                            progress_fraction = p_is_active ? 1.0 : 0.0;
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
            ImGui::PushID((const void*)(intptr_t)(info.port_hash ^ 0x1111));
            if (p_is_active) ImGui::BeginDisabled();
            if (ImGui::Button(get_gui_string(gui_string_id::btn_start, lang), ImVec2(btn_w, 0)))
                ctrl.enqueue_commander_action([&ctrl, port_hash = info.port_hash]() { ctrl.commander().request_port_start(port_hash); });
            if (p_is_active) ImGui::EndDisabled();
            ImGui::PopID();

            ImGui::SameLine(0.0f, exp_pad);

            ImGui::PushID((const void*)(intptr_t)(info.port_hash ^ 0x2222));
            if (!p_is_active) ImGui::BeginDisabled();
            if (ImGui::Button(get_gui_string(gui_string_id::btn_stop, lang), ImVec2(btn_w, 0)))
                ctrl.enqueue_commander_action([&ctrl, port_hash = info.port_hash]() { ctrl.commander().request_port_stop(port_hash); });
            if (!p_is_active) ImGui::EndDisabled();
            ImGui::PopID();
            
            ImGui::Separator();

            // Parameters
            if (p_it != ports.end() && !p_it->second->is_unavailable && !p_it->second->user_params.get_children().empty())
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
                    
                    bool disable_params = p_is_active;
                    if (disable_params) ImGui::BeginDisabled();
                    
                    float avail_w = ImGui::GetContentRegionAvail().x;
                    
                    auto* sorted_list = dynamic_cast<const adam::configuration_parameter_list_sorted*>(&p_it->second->user_params);

                    // Render each param via a lambda to avoid building an intermediate vector every frame
                    auto render_param = [&](const adam::string_hashed& param_name, adam::configuration_parameter* param_ptr)
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
                                                ctrl.enqueue_commander_action([&ctrl, h=info.port_hash, p=param_name, v=preset]() { ctrl.commander().request_port_parameter_set(h, p, v); });
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
                                            ctrl.enqueue_commander_action([&ctrl, h=info.port_hash, p=param_name, v=new_v]() { ctrl.commander().request_port_parameter_set(h, p, v); });
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
                                                ctrl.enqueue_commander_action([&ctrl, h=info.port_hash, p=param_name, v=preset]() { ctrl.commander().request_port_parameter_set(h, p, v); });
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
                                            ctrl.enqueue_commander_action([&ctrl, h=info.port_hash, p=param_name, v=new_v]() { ctrl.commander().request_port_parameter_set(h, p, v); });
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
                                                ctrl.enqueue_commander_action([&ctrl, h=info.port_hash, p=param_name, v=new_v]() { ctrl.commander().request_port_parameter_set(h, p, v); });
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
                                
                                // Use set_value to determine if the string is inherently matching preset validity or regex configuration parameters
                                if (ImGui::InputText("##str", buf, sizeof(buf), ImGuiInputTextFlags_EnterReturnsTrue))
                                {
                                    adam::string_hashed new_v(&buf[0]);
                                    if (c_str->set_value(new_v))
                                    {
                                        ctrl.enqueue_commander_action([&ctrl, h=info.port_hash, p=param_name, v=new_v]() { ctrl.commander().request_port_parameter_set(h, p, v); });
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
                                render_param(param_ptr->get_name(), param_ptr);
                        }
                    }
                    else
                    {
                        for (const auto& [param_name, param_ptr] : p_it->second->user_params.get_children())
                            render_param(param_name, param_ptr.get());
                    }

                    if (disable_params) ImGui::EndDisabled();
                    
                    ImGui::Indent();
                    ImGui::TreePop();
                }
                ImGui::PopID();
                ImGui::Separator();
            }

            // Inject Data
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

                bool disable_inject = !p_is_active || !inject_state.is_valid;
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

            // Remove
            ImGui::PushID((const void*)(intptr_t)(info.port_hash ^ 0x4444));
            if (ImGui::Button(get_gui_string(gui_string_id::btn_remove_port, lang), ImVec2(info.current_node_w - exp_pad * 2.0f, 0)))
            {
                ctrl.enqueue_commander_action([&ctrl, conn_hash = info.hash, port_hash = info.port_hash, is_input = (info.type == node_type_input)]() { 
                    ctrl.commander().request_connection_port_remove(conn_hash, port_hash, is_input); 
                });
            }
            ImGui::PopID();

            // Cache the exact 100% accurate pixel height generated by ImGui's layout engine for flawless container sizing next frame
            float exact_inner_h = ImGui::GetCursorPosY() - ImGui::GetStyle().ItemSpacing.y;
            g_expanded_node_heights[info.unique_node_id] = exact_inner_h + exp_pad * 2.0f;

            ImGui::EndChild();
        }
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
    }

    void render_create_connection_modal(gui_controller& ctrl, adam::language lang)
    {
        float dpi_scale = ImGui::GetStyle()._MainScale;
        if (ImGui::BeginPopupModal(get_gui_string(gui_string_id::dlg_create_connection, lang), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
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
            adam::port::direction direction;
            std::string type_name;
            bool is_unavailable = false;
        };

        static std::map<std::string, std::vector<port_display_info>> existing_grouped;
        static std::map<std::string, std::vector<port_display_info>> new_grouped;
        static std::map<adam::string_hash, port_display_info> known_port_types;
        static std::map<adam::string_hash, std::array<char, max_name_length>> new_port_names;
        static std::vector<adam::string_hash> used_ports;

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
                    if (g_target_direction == adam::port::direction_in)
                    {
                        used_ports = conn_it->second->inputs;
                    }
                    else if (g_target_direction == adam::port::direction_out)
                    {
                        used_ports = conn_it->second->outputs;
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
        if (ImGui::BeginPopupModal(g_port_popup_title, nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
        {
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
                            draw_direction_badge(pdi.direction, is_used, lang);

                            ImGui::TableSetColumnIndex(2);
                            ImGui::TextUnformatted(pdi.port_name.c_str());

                            ImGui::TableSetColumnIndex(3);
                            ImGui::PushID((const void*)(intptr_t)pdi.port_hash);
                            if (ImGui::Button(get_gui_string(gui_string_id::btn_add_path, lang)))
                            {
                                adam::string_hash conn_hash = g_target_connection.get_hash();
                                adam::string_hash port_hash = pdi.port_hash;
                                bool is_input = g_target_direction == adam::port::direction_in;
                                ctrl.enqueue_commander_action([&ctrl, conn_hash, port_hash, is_input]() { ctrl.commander().request_connection_port_add(conn_hash, port_hash, is_input); });
                                
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
                                bool is_input = g_target_direction == adam::port::direction_in;
                                adam::string_hash new_port_hash = adam::string_hashed(name_buffer.data()).get_hash();
                                
                                ctrl.enqueue_commander_action([&ctrl, new_name, type_hash, mod_hash, conn_hash, new_port_hash, is_input]() { ctrl.commander().request_port_create(new_name, type_hash, mod_hash); ctrl.commander().request_connection_port_add(conn_hash, new_port_hash, is_input); });

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

    void render_top_control_bar(gui_controller& ctrl, adam::language lang, int& sort_mode, bool& show_inspector)
    {
        float dpi_scale = ImGui::GetStyle()._MainScale;
        // --- START: Top Control Bar (Sort) ---
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_sort_by, lang));
        ImGui::SameLine();
        const char* sort_options[] = 
        {
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

        const char* inspector_lbl = get_gui_string(gui_string_id::lbl_data_inspector, lang);
        float right_align = ImGui::GetWindowWidth() - ImGui::CalcTextSize(inspector_lbl).x - ImGui::GetFrameHeight() - ImGui::GetStyle().WindowPadding.x * 2.0f;
        if (right_align > ImGui::GetCursorPosX())
        {
            ImGui::SameLine(right_align);
        }
        else
        {
            ImGui::SameLine();
        }
        ImGui::Checkbox(inspector_lbl, &show_inspector);
    }

    /**
     * @brief Renders the header section of a connection card, including rename inputs, 
     *        color picker, state action buttons, and drag-and-drop handles.
     */
    void draw_connection_card_header
    (
        gui_controller& ctrl,
        adam::language lang,
        int sort_mode,
        adam::string_hash hash,
        adam::connection_view* conn,
        bool is_drag_preview,
        float dpi_scale,
        float port_w,
        bool is_unavailable
    )
    {
        bool commander_active = ctrl.is_commander_active();
        const auto& connections = ctrl.commander().registry().get_connections();

        ImGui::AlignTextToFramePadding();

        char name_buf[max_name_length];
        std::strncpy(name_buf, conn->name.c_str(), sizeof(name_buf));
        name_buf[sizeof(name_buf) - 1] = '\0';

        // Direct editing of the connection name via InputText
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

        // Enqueue rename request on edit commit
        if (enter_pressed || deactivated)
        {
            if (name_buf[0] != '\0' && conn->name != string_hashed(&name_buf[0]))
            {
                adam::string_hash proposed_hash = adam::string_hashed(&name_buf[0]).get_hash();
                if (connections.find(proposed_hash) == connections.end())
                {
                    adam::string_hash old_hash = conn->name.get_hash();
                    adam::string_hashed new_name(&name_buf[0]);
                    ctrl.enqueue_commander_action([&ctrl, old_hash, new_name]() { ctrl.commander().request_connection_rename(old_hash, new_name); });
                }
            }
        }

        ImGui::SameLine();

        // Custom card coloring button and picker dialog
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
                    ctrl.enqueue_commander_action([&ctrl, h]() { ctrl.commander().request_connection_color_change(h, 0); });
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
                        ctrl.enqueue_commander_action([&ctrl, h, c]() { ctrl.commander().request_connection_color_change(h, c); });
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

        if (!is_unavailable)
        {
            const char* btn_start_str = get_gui_string(gui_string_id::btn_start, lang);
            const char* btn_stop_str  = get_gui_string(gui_string_id::btn_stop, lang);
                
            ImGui::SameLine();

            // Start/Stop execution controls for the entire connection chain
            bool is_active = conn->is_active;
            bool can_start = !is_active && conn->valid_chain;

            if (!can_start) ImGui::BeginDisabled();
            if (ImGui::Button(btn_start_str) && !is_drag_preview)
            {
                adam::string_hash h = conn->name.get_hash();
                ctrl.enqueue_commander_action([&ctrl, h]() { ctrl.commander().request_connection_start(h); });
            }
            if (!can_start) ImGui::EndDisabled();
                
            ImGui::SameLine();
            if (!is_active) ImGui::BeginDisabled();
            if (ImGui::Button(btn_stop_str) && !is_drag_preview)
            {
                adam::string_hash h = conn->name.get_hash();
                ctrl.enqueue_commander_action([&ctrl, h]() { ctrl.commander().request_connection_stop(h); });
            }
            if (!is_active) ImGui::EndDisabled();
        }
        else
        {
            ImGui::SameLine();
            ImGui::TextColored(get_gui_color(gui_color_id::log_warning), "%s", get_gui_string(gui_string_id::stat_unavailable, lang));
        }

        const char* btn_delete_str = get_gui_string(gui_string_id::btn_delete, lang);
        float btn_delete_w = ImGui::CalcTextSize(btn_delete_str).x + ImGui::GetStyle().FramePadding.x * 2.0f;
            
        ImGui::SameLine(ImGui::GetWindowWidth() - btn_delete_w - ImGui::GetStyle().WindowPadding.x);
            
        // Delete button (prompts modal if populated, destroys directly if empty)
        if (ImGui::Button(btn_delete_str) && !is_drag_preview)
        {
            if (conn->inputs.empty() && conn->outputs.empty() && conn->filters.empty())
            {
                adam::string_hash h = conn->name.get_hash();
                ctrl.enqueue_commander_action([&ctrl, h]() { ctrl.commander().request_connection_destroy(h); });
            }
            else
            {
                g_connection_to_delete = conn->name;
                g_request_delete_popup = true;
            }
        }

        // Drag handle for custom manual re-ordering (sort mode = 6)
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
                ImVec2 win_pos = ImGui::GetWindowPos();
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
    }

    /**
     * @brief Draws the bezier curve lines and pin connection dots between the different port nodes.
     */
    void draw_connection_lines
    (
        ImDrawList* draw_list,
        float dpi_scale,
        bool is_light_theme,
        int total_stages,
        size_t max_rows,
        float row_height,
        float avail_x,
        const ImVec2& cur_pos,
        const std::vector<std::vector<connection_pin_data>>& stage_pins_in,
        const std::vector<std::vector<connection_pin_data>>& stage_pins_out,
        adam::connection_view* conn
    )
    {
        ImColor line_col = is_light_theme ? get_gui_color(gui_color_id::node_connection_line_light) : get_gui_color(gui_color_id::node_connection_line);
        ImColor grey_col = is_light_theme ? ImColor(210, 210, 210, 80) : ImColor(70, 70, 70, 80);
        float line_thickness = 5.f * dpi_scale;
        size_t num_processors = conn->filters.size() + conn->converters.size();

        // Special layout case: straight bypass through center when no processor nodes are present
        if (num_processors == 0 && !conn->inputs.empty() && !conn->outputs.empty() && (conn->inputs.size() > 1 || conn->outputs.size() > 1))
        {
            float center_y = cur_pos.y + static_cast<float>(max_rows) * row_height * 0.5f;
            float mid_x = cur_pos.x + avail_x * 0.5f;
                
            ImVec2 p_mid(mid_x, center_y);
                
            for (const auto& pin_out_data : stage_pins_out[0])
            {
                ImVec2 pin_out = pin_out_data.pos;
                float b_strength = (mid_x - pin_out.x) * 0.5f;
                
                ImColor cur_line_col = conn->valid_chain ? line_col : grey_col;

                draw_list->AddBezierCubic(
                    pin_out, ImVec2(pin_out.x + b_strength, pin_out.y), ImVec2(mid_x - b_strength, center_y), p_mid,
                    cur_line_col, line_thickness);
            }
                
            for (const auto& pin_in_data : stage_pins_in[1])
            {
                ImVec2 pin_in = pin_in_data.pos;
                float b_strength = (pin_in.x - mid_x) * 0.5f;

                ImColor cur_line_col = conn->valid_chain ? line_col : grey_col;

                draw_list->AddBezierCubic(
                    p_mid, ImVec2(mid_x + b_strength, center_y), ImVec2(pin_in.x - b_strength, pin_in.y), pin_in,
                    cur_line_col, line_thickness);
            }
        }
        else
        {
            // Standard multi-stage bezier wiring
            for (int s = 0; s < total_stages - 1; ++s)
            {
                for (const auto& pin_out_data : stage_pins_out[s])
                {
                    ImVec2 pin_out = pin_out_data.pos;
                    for (const auto& pin_in_data : stage_pins_in[s + 1])
                    {
                        ImVec2 pin_in = pin_in_data.pos;
                        float b_strength = (pin_in.x - pin_out.x) * 0.5f;

                        ImColor cur_line_col = conn->valid_chain ? line_col : grey_col;

                        draw_list->AddBezierCubic
                        (
                            pin_out, ImVec2(pin_out.x + b_strength, pin_out.y), ImVec2(pin_in.x - b_strength, pin_in.y), pin_in,
                            cur_line_col, line_thickness
                        );
                    }
                }
            }
        }

        // Draw circular connection dots (pins) on active nodes
        auto draw_pin_dot = [&](const connection_pin_data& pin)
        {
            float dot_radius = ImGui::GetTextLineHeight() / 2 - ImGui::GetStyle().CellPadding.y;
            draw_list->AddCircleFilled(pin.pos, dot_radius, pin.col);
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
    }

    /**
     * @brief Renders the node addition buttons ("Add Input", "Add Output", "Add Processor") 
     *        at the bottom of a connection card.
     */
    void draw_connection_card_footer
    (
        gui_controller& ctrl,
        adam::language lang,
        adam::connection_view* conn,
        bool is_drag_preview,
        float port_w,
        float current_y,
        float start_x,
        float avail_x,
        float avail_w
    )
    {
        (void)ctrl;
        const char* btn_add_input_str = get_gui_string(gui_string_id::btn_add_input, lang);
        const char* btn_add_processor_str = get_gui_string(gui_string_id::btn_add_processor, lang);
        const char* btn_add_output_str = get_gui_string(gui_string_id::btn_add_output, lang);

        float add_in_w = ImGui::CalcTextSize(btn_add_input_str).x + ImGui::GetStyle().FramePadding.x * 2.0f;
        float in_x = start_x + port_w * 0.5f - add_in_w * 0.5f;
        ImGui::SetCursorPos(ImVec2(std::max(start_x, in_x), current_y));
        if (ImGui::Button(btn_add_input_str) && !is_drag_preview)
        {
            g_target_connection = conn->name;
            g_target_direction = adam::port::direction_in;
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
                g_target_direction = adam::port::direction_out;
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
    }

    void render_connection_card(gui_controller& ctrl, adam::language lang, int sort_mode, adam::string_hash hash, adam::connection_view* conn, bool is_drag_preview, float card_w)
    {
        float dpi_scale = ImGui::GetStyle()._MainScale;
        auto& reg_view = ctrl.commander().registry();
        const auto& ports = reg_view.get_ports();

        bool is_unavailable = conn->is_unavailable;
        bool input_missing = false;
        bool output_missing = false;

        auto* theme_param = dynamic_cast<adam::configuration_parameter_string*>(ctrl.get_parameters().get("theme"_ct));
        bool is_light_theme = theme_param && theme_param->get_value() == "default-light"_ct;

        auto& stage_pins_in = is_drag_preview ? g_stage_pins_in_preview : g_stage_pins_in_normal;
        auto& stage_pins_out = is_drag_preview ? g_stage_pins_out_preview : g_stage_pins_out_normal;

        ImGui::PushID((const void*)(intptr_t)(is_drag_preview ? hash + 0x1000000 : hash));
            
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
            
        if (conn->color != 0)
        {
            bg.x = ((conn->color >> 16) & 0xFF) / 255.0f * 0.35f + bg.x * 0.65f;
            bg.y = ((conn->color >> 8) & 0xFF) / 255.0f * 0.35f + bg.y * 0.65f;
            bg.z = (conn->color & 0xFF) / 255.0f * 0.35f + bg.z * 0.65f;
        }
            
        if (is_being_dragged || is_unavailable)
        {
            bg.w *= 0.4f;
        }
            
        ImGui::PushStyleColor(ImGuiCol_ChildBg, bg);

        size_t max_rows = std::max(conn->inputs.size(), conn->outputs.size());
        size_t num_processors = conn->filters.size() + conn->converters.size();
        if (max_rows == 0 && num_processors > 0) max_rows = 1;
            
        float node_h = ImGui::GetTextLineHeight() * 2.0f;
        float row_height = node_h + 10.0f * dpi_scale;
            
        float base_height = ImGui::GetStyle().WindowPadding.y * 2.0f;
        base_height += ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y;
        base_height += (sort_mode == 6 && !is_drag_preview ? 4.0f * dpi_scale : 1.0f) + ImGui::GetStyle().ItemSpacing.y;

        if (!is_drag_preview)
            base_height += ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y;

        if (max_rows > 0 || num_processors > 0)
            base_height += ImGui::GetStyle().ItemSpacing.y * 2.0f + 1.0f;
        
        base_height += ImGui::GetFrameHeight();

        int total_stages = 2 + static_cast<int>(num_processors);

        auto get_expanded_h = [&](uint64_t uid)
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
        };

        float in_offset = (static_cast<float>(max_rows) - static_cast<float>(conn->inputs.size())) * 0.5f;
        float out_offset = (static_cast<float>(max_rows) - static_cast<float>(conn->outputs.size())) * 0.5f;

        float in_col_bottom = (in_offset + static_cast<float>(conn->inputs.size())) * row_height;
        float out_col_bottom = (out_offset + static_cast<float>(conn->outputs.size())) * row_height;

        if (!is_drag_preview)
        {
            for (auto pid : conn->inputs)
            {
                uint64_t uid = get_unique_node_id(pid, hash, 0);
                if (g_expanded_nodes.count(uid)) in_col_bottom += get_expanded_h(uid);
            }

            for (auto pid : conn->outputs)
            {
                uint64_t uid = get_unique_node_id(pid, hash, total_stages - 1);
                if (g_expanded_nodes.count(uid)) out_col_bottom += get_expanded_h(uid);
            }
        }
        
        float max_col_h = std::max({in_col_bottom, out_col_bottom, static_cast<float>(max_rows) * row_height});
        float child_height = base_height + max_col_h;

        if (!is_drag_preview) ImGui::BeginGroup();

        static std::vector<expanded_port_render_info> deferred_expansions;
        deferred_expansions.clear();

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

            draw_connection_card_header(ctrl, lang, sort_mode, hash, conn, is_drag_preview, dpi_scale, port_w, is_unavailable);

            ImVec2 cur_pos = ImGui::GetCursorScreenPos();

            if (!is_drag_preview)
            {
                // Available formats — rebuilt once per frame across all connection cards, not once per card
                static std::vector<std::pair<adam::string_hashed, adam::string_hashed>> available_formats;
                static int s_formats_frame = -1;
                {
                    std::lock_guard<const adam::module_view> mod_lg(ctrl.commander().modules());
                    const auto& loaded_modules = ctrl.commander().get_modules().get_loaded();

                    if (ImGui::GetFrameCount() != s_formats_frame)
                    {
                        s_formats_frame = ImGui::GetFrameCount();
                        available_formats.clear();
                        available_formats.push_back({ "transparent"_ct, ""_ct });
                        for (const auto& [mod_name, mod_info] : ctrl.commander().get_modules().database())
                        {
                            if (loaded_modules.find(mod_name) != loaded_modules.end())
                            {
                                for (const auto& fmt : mod_info.data_formats)
                                    available_formats.push_back({ fmt, mod_name });
                            }
                        }
                    }

                    if (is_unavailable)
                    {
                        bool is_in_transparent = conn->input_format.empty() || conn->input_format.get_hash() == ("transparent"_ct).get_hash();
                        if (!is_in_transparent && conn->input_format_module.get_hash() != 0 && loaded_modules.find(conn->input_format_module.get_hash()) == loaded_modules.end())
                        {
                            input_missing = true;
                        }
                        
                        bool is_out_transparent = conn->output_format.empty() || conn->output_format.get_hash() == ("transparent"_ct).get_hash();
                        if (!is_out_transparent && conn->output_format_module.get_hash() != 0 && loaded_modules.find(conn->output_format_module.get_hash()) == loaded_modules.end())
                        {
                            output_missing = true;
                        }
                    }
                }

                // 1. Input Format Dropdown above the first column (input ports)
                ImGui::SetCursorScreenPos(ImVec2(cur_pos.x, cur_pos.y));
                char in_fmt_str[256];
                if (conn->input_format.empty() || conn->input_format == "transparent"_ct)
                {
                    snprintf(in_fmt_str, sizeof(in_fmt_str), "%s", get_gui_string(gui_string_id::lbl_data_format_transparent_none, lang));
                }
                else
                {
                    snprintf(in_fmt_str, sizeof(in_fmt_str), "%s", conn->input_format.c_str());
                }
                if (conn->input_format_module.c_str()[0] != '\0')
                {
                    size_t len = strlen(in_fmt_str);
                    snprintf(in_fmt_str + len, sizeof(in_fmt_str) - len, " [%s]", conn->input_format_module.c_str());
                }

                if (input_missing)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, get_gui_color(gui_color_id::log_warning));
                }

                float cb_size = ImGui::GetFrameHeight();
                const char* inspect_lbl = get_gui_string(gui_string_id::col_inspect, lang);
                float inspect_w = ImGui::CalcTextSize(inspect_lbl).x + cb_size + ImGui::GetStyle().ItemInnerSpacing.x;

                ImGui::SetNextItemWidth(port_w - inspect_w - ImGui::GetStyle().ItemSpacing.x);
                ImGui::PushID("conn_input_format_combo");
                bool in_combo_open = ImGui::BeginCombo("##InputFormatCombo", in_fmt_str);
                
                if (input_missing)
                {
                    ImGui::PopStyleColor();
                    if (ImGui::IsItemHovered())
                    {
                        ImGui::BeginTooltip();
                        ImGui::Text(get_gui_string(gui_string_id::tt_module_missing, lang), conn->input_format_module.c_str());
                        ImGui::EndTooltip();
                    }
                }

                if (in_combo_open)
                {
                    for (const auto& [fmt, mod] : available_formats)
                    {
                        char item_str[256];
                        if (fmt == "transparent"_ct)
                            snprintf(item_str, sizeof(item_str), "%s", get_gui_string(gui_string_id::lbl_data_format_transparent_none, lang));
                        else
                            snprintf(item_str, sizeof(item_str), "%s", fmt.c_str());
                        if (mod.c_str()[0] != '\0')
                        {
                            size_t len = strlen(item_str);
                            snprintf(item_str + len, sizeof(item_str) - len, " [%s]", mod.c_str());
                        }
                        
                        bool is_selected = (fmt == conn->input_format && mod == conn->input_format_module);
                        if (ImGui::Selectable(item_str, is_selected))
                        {
                            if (!is_selected)
                            {
                                ctrl.enqueue_commander_action([&ctrl, hash, fmt, mod]()
                                {
                                    ctrl.commander().request_connection_set_input_data_format(hash, fmt.get_hash(), mod.get_hash());
                                });
                            }
                        }
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
                ImGui::PopID();

                ImGui::SameLine();
                bool input_inspect_val = ctrl.commander().get_connection_input_inspectors().find(hash) != ctrl.commander().get_connection_input_inspectors().end();
                ImGui::PushID("conn_input_inspect");
                if (ImGui::Checkbox(inspect_lbl, &input_inspect_val))
                {
                    if (input_inspect_val)
                    {
                        ctrl.enqueue_commander_action([&ctrl, hash]() 
                        {
                            auto& cmdr = ctrl.commander();
                            if (cmdr.get_connection_input_inspectors().find(hash) == cmdr.get_connection_input_inspectors().end())
                            {
                                adam::data_inspector* new_inspector = nullptr;
                                cmdr.request_connection_input_inspector_create(hash, make_inspector_connection_input_buffer_callback(hash), new_inspector);
                            }
                        });
                        g_request_open_inspector = true;
                        g_connection_input_to_expand_in_inspector = hash;
                        g_pending_inspector_connections_input.insert(hash);
                    }
                    else
                    {
                        g_expanded_inspector_connections_input.erase(hash);
                        g_pending_inspector_connections_input.erase(hash);
                        ctrl.enqueue_commander_action([&ctrl, hash]() 
                        {
                            auto& cmdr = ctrl.commander();
                            auto it = cmdr.connection_input_inspectors().find(hash);
                            if (it != cmdr.connection_input_inspectors().end())
                            {
                                cmdr.request_connection_input_inspector_destroy(it->second);
                            }
                        });
                    }
                }
                ImGui::PopID();

                // 2. Output Format Dropdown above the last column (output ports)
                ImGui::SetCursorScreenPos(ImVec2(cur_pos.x + avail_x - port_w, cur_pos.y));
                char out_fmt_str[256];
                if (conn->output_format.empty() || conn->output_format == "transparent"_ct)
                {
                    snprintf(out_fmt_str, sizeof(out_fmt_str), "%s", get_gui_string(gui_string_id::lbl_data_format_transparent_none, lang));
                }
                else
                {
                    snprintf(out_fmt_str, sizeof(out_fmt_str), "%s", conn->output_format.c_str());
                }
                if (conn->output_format_module.c_str()[0] != '\0')
                {
                    size_t len = strlen(out_fmt_str);
                    snprintf(out_fmt_str + len, sizeof(out_fmt_str) - len, " [%s]", conn->output_format_module.c_str());
                }

                if (output_missing)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, get_gui_color(gui_color_id::log_warning));
                }

                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted(inspect_lbl);
                ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
                bool output_inspect_val = ctrl.commander().get_connection_output_inspectors().find(hash) != ctrl.commander().get_connection_output_inspectors().end();
                ImGui::PushID("conn_output_inspect");
                if (ImGui::Checkbox("##inspect_out", &output_inspect_val))
                {
                    if (output_inspect_val)
                    {
                        ctrl.enqueue_commander_action([&ctrl, hash]() 
                        {
                            auto& cmdr = ctrl.commander();
                            if (cmdr.get_connection_output_inspectors().find(hash) == cmdr.get_connection_output_inspectors().end())
                            {
                                adam::data_inspector* new_inspector = nullptr;
                                cmdr.request_connection_output_inspector_create(hash, make_inspector_connection_output_buffer_callback(hash), new_inspector);
                            }
                        });
                        g_request_open_inspector = true;
                        g_connection_output_to_expand_in_inspector = hash;
                        g_pending_inspector_connections_output.insert(hash);
                    }
                    else
                    {
                        g_expanded_inspector_connections_output.erase(hash);
                        g_pending_inspector_connections_output.erase(hash);
                        ctrl.enqueue_commander_action([&ctrl, hash]() 
                        {
                            auto& cmdr = ctrl.commander();
                            auto it = cmdr.connection_output_inspectors().find(hash);
                            if (it != cmdr.connection_output_inspectors().end())
                            {
                                cmdr.request_connection_output_inspector_destroy(it->second);
                            }
                        });
                    }
                }
                ImGui::PopID();

                ImGui::SameLine();
                ImGui::SetNextItemWidth(port_w - inspect_w - ImGui::GetStyle().ItemSpacing.x);
                ImGui::PushID("conn_output_format_combo");
                bool out_combo_open = ImGui::BeginCombo("##OutputFormatCombo", out_fmt_str);

                if (output_missing)
                {
                    ImGui::PopStyleColor();
                    if (ImGui::IsItemHovered())
                    {
                        ImGui::BeginTooltip();
                        ImGui::Text(get_gui_string(gui_string_id::tt_module_missing, lang), conn->output_format_module.c_str());
                        ImGui::EndTooltip();
                    }
                }

                if (out_combo_open)
                {
                    for (const auto& [fmt, mod] : available_formats)
                    {
                        char item_str[256];
                        if (fmt == "transparent"_ct)
                            snprintf(item_str, sizeof(item_str), "%s", get_gui_string(gui_string_id::lbl_data_format_transparent_none, lang));
                        else
                            snprintf(item_str, sizeof(item_str), "%s", fmt.c_str());
                        if (mod.c_str()[0] != '\0')
                        {
                            size_t len = strlen(item_str);
                            snprintf(item_str + len, sizeof(item_str) - len, " [%s]", mod.c_str());
                        }
                        
                        bool is_selected = (fmt == conn->output_format && mod == conn->output_format_module);
                        if (ImGui::Selectable(item_str, is_selected))
                        {
                            if (!is_selected)
                            {
                                ctrl.enqueue_commander_action([&ctrl, hash, fmt, mod]()
                                {
                                    ctrl.commander().request_connection_set_output_data_format(hash, fmt.get_hash(), mod.get_hash());
                                });
                            }
                        }
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
                ImGui::PopID();

                // Shift everything down for ports
                cur_pos.y += ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y;
            }
                
            float total_node_widths = 2.0f * port_w + static_cast<float>(std::max(0, total_stages - 2)) * proc_w;
            float gap = (total_stages > 1) ? (avail_x - total_node_widths) / static_cast<float>(total_stages - 1) : 0.0f;
                
            // --- START: Node Renderer ---
            auto draw_node = [&](const char* name, node_type type, int stage, float row, ImColor color, connection_pin_data& out_pin_in, connection_pin_data& out_pin_out, bool is_unavail = false, const char* unavail_module = nullptr, adam::string_hash port_hash = 0, float extra_y = 0.0f)
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
                ImGui::PushID((const void*)(intptr_t)(get_unique_node_id(port_hash, hash, stage) ^ 0xABCD));
                ImGui::SetNextItemAllowOverlap();
                ImGui::InvisibleButton("##node_btn", ImVec2(current_node_w, node_h));
                
                bool is_expanded = false;
                if (port_hash != 0 && !is_drag_preview)
                {
                    uint64_t unique_node_id = get_unique_node_id(port_hash, hash, stage);
                    is_expanded = g_expanded_nodes.count(unique_node_id) > 0;

                    // Handle Left Click -> Expand
                    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
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
                    float this_expanded_h = get_expanded_h(unique_node_id);
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
                        if (ports.find(proposed_hash) == ports.end())
                        {
                            adam::string_hashed new_name(&name_buf[0]);
                            ctrl.enqueue_commander_action([&ctrl, port_hash, new_name]() { ctrl.commander().request_port_rename(port_hash, new_name); });
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
                    draw_list->AddText(text_pos, ImColor(255, 255, 255), name);
                    draw_list->PopClipRect();
                }

                out_pin_in.pos = ImVec2(p_min.x, p_min.y + node_h * 0.5f);
                out_pin_out.pos = ImVec2(p_max.x, p_min.y + node_h * 0.5f);

                // Draw Port Status Dot
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

            float current_in_y = in_offset * row_height;
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

                draw_node(it != ports.end() ? it->second->name.c_str() : "Unknown Input", node_type_input, 0, 0.0f, col, p_in, p_out, is_unavail, mod_name, pid, current_in_y);
                if (!conn->input_format.empty())
                    p_out.format_name = conn->input_format.c_str();
                else
                    p_out.format_name = "transparent";
                stage_pins_out[0].push_back(p_out);
                current_in_y += row_height;

                uint64_t uid = get_unique_node_id(pid, hash, 0);
                if (g_expanded_nodes.count(uid))
                    current_in_y += get_expanded_h(uid);
            }

            int current_stage = 1;
            int processor_idx = 1;
            float proc_offset = (static_cast<float>(max_rows) - 1.0f) * 0.5f;
            float proc_extra_y = proc_offset * row_height;
            for (auto fid : conn->filters)
            {
                (void)fid;
                connection_pin_data p_in, p_out;
                if (compact_processors)
                {
                    char short_name[16];
                    snprintf(short_name, sizeof(short_name), "%02d   ", processor_idx);
                    draw_node(short_name, node_type_processor, current_stage, 0.0f, proc_col, p_in, p_out, false, nullptr, 0, proc_extra_y);
                }
                else
                {
                    draw_node("Filter", node_type_processor, current_stage, 0.0f, proc_col, p_in, p_out, false, nullptr, 0, proc_extra_y);
                }
                p_in.format_name = "transparent";
                p_out.format_name = "transparent";
                stage_pins_in[current_stage].push_back(p_in);
                stage_pins_out[current_stage].push_back(p_out);
                current_stage++;
                processor_idx++;
            }
            for (auto cid : conn->converters)
            {
                (void)cid;
                connection_pin_data p_in, p_out;
                if (compact_processors)
                {
                    char short_name[16];
                    snprintf(short_name, sizeof(short_name), "%02d   ", processor_idx);
                    draw_node(short_name, node_type_converter, current_stage, 0.0f, proc_col, p_in, p_out, false, nullptr, 0, proc_extra_y);
                }
                else
                {
                    draw_node("Converter", node_type_converter, current_stage, 0.0f, proc_col, p_in, p_out, false, nullptr, 0, proc_extra_y);
                }
                p_in.format_name = "transparent";
                p_out.format_name = "transparent";
                stage_pins_in[current_stage].push_back(p_in);
                stage_pins_out[current_stage].push_back(p_out);
                current_stage++;
                processor_idx++;
            }

            float current_out_y = out_offset * row_height;
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

                draw_node(it != ports.end() ? it->second->name.c_str() : "Unknown Output", node_type_output, total_stages - 1, 0.0f, col, p_in, p_out, is_unavail, mod_name, pid, current_out_y);
                if (!conn->output_format.empty())
                    p_in.format_name = conn->output_format.c_str();
                else
                    p_in.format_name = "transparent";
                stage_pins_in[total_stages - 1].push_back(p_in);
                current_out_y += row_height;

                uint64_t uid = get_unique_node_id(pid, hash, total_stages - 1);
                if (g_expanded_nodes.count(uid))
                    current_out_y += get_expanded_h(uid);
            }

            // Draw connection bezier cables
            draw_connection_lines(draw_list, dpi_scale, is_light_theme, total_stages, max_rows, row_height, avail_x, cur_pos, stage_pins_in, stage_pins_out, conn);

            for (const auto& info : deferred_expansions)
            {
                draw_expanded_port_node(ctrl, lang, ports, dpi_scale, draw_list, info);
            }

            ImGui::SetCursorScreenPos(ImVec2(cur_pos.x, cur_pos.y + max_col_h));
                
            if (max_rows > 0 || num_processors > 0)
            {
                ImGui::Spacing();
                ImGui::Separator();
            }
                
            float avail_w = ImGui::GetWindowWidth();
                
            float current_y = ImGui::GetCursorPosY();
            float start_x = ImGui::GetStyle().WindowPadding.x;

            // Draw card footer node-adding buttons
            draw_connection_card_footer(ctrl, lang, conn, is_drag_preview, port_w, current_y, start_x, avail_x, avail_w);
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

    static void render_inspector_hex_dump(const uint8_t* data, size_t size, int actual_index, float inspector_height, float dpi_scale, adam::language lang)
    {
        size_t display_len = size;
        size_t num_rows = (display_len + 15) / 16;

        if (g_mono_font) ImGui::PushFont(g_mono_font);
        float line_h = ImGui::GetTextLineHeight();
        if (g_mono_font) ImGui::PopFont();

        float item_spacing_y = ImGui::GetStyle().ItemSpacing.y;
        float window_border_size = ImGui::GetStyle().WindowBorderSize;

        // Calculate text content height with item spacing between lines
        float text_content_h = num_rows > 0 ? (line_h * num_rows + item_spacing_y * (num_rows - 1)) : 0.0f;
        // child padding is 4.0f * dpi_scale on top and bottom
        float child_padding_h = (4.0f * dpi_scale) * 2.0f;
        // child border height (top + bottom)
        float child_border_h = window_border_size * 2.0f;
        
        float calc_h = text_content_h + child_padding_h + child_border_h;

        float button_h = ImGui::GetFrameHeight();
        float spacing_h = item_spacing_y;
        // container padding is 6.0f * dpi_scale on top and bottom
        float container_padding_h = (6.0f * dpi_scale) * 2.0f;
        // container border height
        float container_border_h = window_border_size * 2.0f;
        float reserved_container_elements_h = button_h + spacing_h + container_padding_h + container_border_h;

        // Capped container height: at most 50% of the inspector height, or at least 110 pixels (to fit buttons and some hex lines)
        float max_container_h = inspector_height * 0.5f;
        if (max_container_h < 110.0f * dpi_scale) max_container_h = 110.0f * dpi_scale;
        
        // Ensure max_container_h doesn't exceed inspector_height - 35 (to avoid overflowing the outer child)
        float absolute_max_h = inspector_height - (button_h + spacing_h + 8.0f * dpi_scale);
        if (max_container_h > absolute_max_h) max_container_h = absolute_max_h;
        if (max_container_h < 80.0f * dpi_scale) max_container_h = 80.0f * dpi_scale;

        // Calculate child_h based on our capped container_h
        float max_child_h = max_container_h - reserved_container_elements_h;
        if (max_child_h < 30.0f * dpi_scale) max_child_h = 30.0f * dpi_scale;

        float child_h = std::min(calc_h, max_child_h);
        float container_h = child_h + reserved_container_elements_h;

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.0f * dpi_scale, 6.0f * dpi_scale));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f * dpi_scale);

        ImGui::PushID(actual_index);
        if (ImGui::BeginChild("##hex_container", ImVec2(-FLT_MIN, container_h), true))
        {
            // Add Copy Buttons
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f * dpi_scale, 2.0f * dpi_scale));

            float avail_w = ImGui::GetContentRegionAvail().x;
            float button_w = (avail_w - ImGui::GetStyle().ItemSpacing.x * 2.0f) / 3.0f;

            if (ImGui::Button(get_gui_string(gui_string_id::btn_copy_hex, lang), ImVec2(button_w, button_h)))
            {
                std::string copy_str;
                copy_str.reserve(size * 3);
                for (size_t j = 0; j < size; ++j)
                {
                    char hex[4];
                    snprintf(hex, sizeof(hex), "%02X ", data[j]);
                    copy_str += hex;
                }
                if (!copy_str.empty()) copy_str.pop_back(); // Remove trailing space
                ImGui::SetClipboardText(copy_str.c_str());
            }
            ImGui::SameLine();

            if (ImGui::Button(get_gui_string(gui_string_id::btn_copy_ascii, lang), ImVec2(button_w, button_h)))
            {
                std::string copy_str;
                copy_str.reserve(size);
                for (size_t j = 0; j < size; ++j)
                {
                    char c = data[j];
                    if (c >= 32 && c <= 126) copy_str += c;
                    else copy_str += '.';
                }
                ImGui::SetClipboardText(copy_str.c_str());
            }
            ImGui::SameLine();

            if (ImGui::Button(get_gui_string(gui_string_id::btn_copy_hex_dump, lang), ImVec2(button_w, button_h)))
            {
                std::string copy_str;
                copy_str.reserve(num_rows * 80);
                for (size_t offset = 0; offset < display_len; offset += 16)
                {
                    char line_buf[256];
                    int printed = snprintf(line_buf, sizeof(line_buf), "%04X:  ", static_cast<unsigned int>(offset));
                    size_t chunk = std::min((size_t)16, display_len - offset);
                    for (size_t j = 0; j < 16; ++j)
                    {
                        if (j == 8) line_buf[printed++] = ' ';
                        if (j < chunk) printed += snprintf(line_buf + printed, sizeof(line_buf) - printed, "%02X ", data[offset + j]);
                        else printed += snprintf(line_buf + printed, sizeof(line_buf) - printed, "   ");
                    }
                    line_buf[printed++] = ' ';
                    line_buf[printed++] = ' ';
                    line_buf[printed++] = '|';
                    for (size_t j = 0; j < chunk; ++j)
                    {
                        char c = data[offset + j];
                        line_buf[printed++] = (c >= 32 && c <= 126) ? c : '.';
                    }
                    line_buf[printed++] = '|';
                    line_buf[printed++] = '\n';
                    line_buf[printed] = '\0';
                    copy_str += line_buf;
                }
                ImGui::SetClipboardText(copy_str.c_str());
            }

            ImGui::PopStyleVar();

            // Render Hex Dump Child with Clipper
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0.15f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f * dpi_scale, 4.0f * dpi_scale));

            if (ImGui::BeginChild("##hex_child", ImVec2(-FLT_MIN, child_h), true, ImGuiWindowFlags_HorizontalScrollbar))
            {
                if (g_mono_font) ImGui::PushFont(g_mono_font);

                static constexpr const char* dummy_line = "0000:  00 11 22 33 44 55 66 77  88 99 AA BB CC DD EE FF   |0123456789ABCDEF|";
                float text_w = ImGui::CalcTextSize(dummy_line).x;
                float avail_w = ImGui::GetContentRegionAvail().x;
                float offset_x = (avail_w - text_w) / 2.0f;
                if (offset_x < 4.0f * dpi_scale) offset_x = 4.0f * dpi_scale;

                ImGuiListClipper hex_clipper;
                hex_clipper.Begin(static_cast<int>(num_rows), line_h);
                while (hex_clipper.Step())
                {
                    for (int row = hex_clipper.DisplayStart; row < hex_clipper.DisplayEnd; ++row)
                    {
                        size_t offset = row * 16;
                        size_t chunk = std::min((size_t)16, display_len - offset);

                        char line_buf[256];
                        int printed = snprintf(line_buf, sizeof(line_buf), "%04X:  ", static_cast<unsigned int>(offset));

                        for (size_t j = 0; j < 16; ++j)
                        {
                            if (j == 8)
                            {
                                line_buf[printed++] = ' ';
                            }
                            if (j < chunk)
                            {
                                printed += snprintf(line_buf + printed, sizeof(line_buf) - printed, "%02X ", data[offset + j]);
                            }
                            else
                            {
                                printed += snprintf(line_buf + printed, sizeof(line_buf) - printed, "   ");
                            }
                        }

                        line_buf[printed++] = ' ';
                        line_buf[printed++] = ' ';
                        line_buf[printed++] = '|';

                        for (size_t j = 0; j < 16; ++j)
                        {
                            if (j < chunk)
                            {
                                char c = data[offset + j];
                                if (c >= 32 && c <= 126)
                                    line_buf[printed++] = c;
                                else
                                    line_buf[printed++] = '.';
                            }
                            else
                            {
                                line_buf[printed++] = ' ';
                            }
                        }
                        line_buf[printed++] = '|';
                        line_buf[printed] = '\0';

                        ImGui::SetCursorPosX(offset_x);
                        ImGui::TextUnformatted(line_buf);
                    }
                }

                if (g_mono_font) ImGui::PopFont();
            }
            ImGui::EndChild();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
        }
        ImGui::EndChild();
        ImGui::PopID();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor();
    }

    static void render_inspector_frames_table(const char* name, adam::string_hash hash, std::map<adam::string_hash, adam::gui::inspection_port_data>& data_map, float inspector_height, float dpi_scale, adam::language lang, uint64_t id_modifier)
    {
        std::lock_guard<std::mutex> buffer_lock(adam::gui::g_inspection_data.mtx);
        auto& port_data = data_map[hash];
        const auto& buffers = port_data.buffers;
        const auto& data_pool = port_data.data_pool;
        auto& expanded_nodes = port_data.expanded_nodes;

        ImGui::PushID((const void*)(intptr_t)(hash ^ id_modifier));
        
        ImGui::BeginChild("##outer_child", ImVec2(0, inspector_height), true);

        float inner_scroll_h = -(ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y);
        ImGui::BeginChild("##inner_child", ImVec2(0, inner_scroll_h), false);

        bool auto_scroll = ImGui::GetScrollY() >= ImGui::GetScrollMaxY();

        bool table_begun = ImGui::BeginTable("InspectorTableInner", 6, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable);
            
        auto setup_inner_columns = [&]() 
        {
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, ImGui::GetTextLineHeight() + ImGui::GetStyle().FramePadding.x * 2.0f);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_index, lang), ImGuiTableColumnFlags_WidthFixed, 55.0f * dpi_scale);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_timestamp, lang), ImGuiTableColumnFlags_WidthFixed, 120.0f * dpi_scale);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_size, lang), ImGuiTableColumnFlags_WidthFixed, 75.0f * dpi_scale);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_preview_hex, lang), ImGuiTableColumnFlags_WidthStretch, 0.75f);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_preview_ascii, lang), ImGuiTableColumnFlags_WidthStretch, 0.25f);
        };

        int current_pushed_id = -1;

        if (table_begun)
        {
            setup_inner_columns();
            ImGui::TableHeadersRow();
        }

        float row_height = ImGui::GetTextLineHeight() + ImGui::GetStyle().CellPadding.y * 2.0f;

        std::vector<size_t> expanded_list(expanded_nodes.begin(), expanded_nodes.end());
        while (!expanded_list.empty() && expanded_list.back() >= buffers.size()) {
            expanded_nodes.erase(expanded_list.back());
            expanded_list.pop_back();
        }

        int current_idx = 0;
        int expanded_list_idx = 0;

        while (current_idx < (int)buffers.size())
        {
            int chunk_end = (expanded_list_idx < (int)expanded_list.size()) ? static_cast<int>(expanded_list[expanded_list_idx]) : (int)buffers.size();

            int unexpanded_count = chunk_end - current_idx;
            if (unexpanded_count > 0 && table_begun)
            {
                ImGuiListClipper clipper;
                clipper.Begin(unexpanded_count, row_height);
                while (clipper.Step())
                {
                    for (int j = clipper.DisplayStart; j < clipper.DisplayEnd; ++j)
                    {
                        int actual_index = current_idx + j;
                        const auto& ib = buffers[actual_index];

                        ImGui::TableNextRow();

                        ImGui::TableSetColumnIndex(0);
                        bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)actual_index, ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_OpenOnArrow, "");

                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%zu", actual_index);

                        ImGui::TableSetColumnIndex(2);
                        ImGui::TextUnformatted(adam::get_log_time_string(ib.timestamp).c_str());

                        ImGui::TableSetColumnIndex(3);
                        ImGui::Text("%zu B", static_cast<size_t>(ib.size));

                        ImGui::TableSetColumnIndex(4);
                        if (g_mono_font) ImGui::PushFont(g_mono_font);

                        char preview_hex[64];
                        char preview_ascii[32];
                        size_t preview_len = std::min((size_t)ib.size, (size_t)16);
                        fill_hex_preview(data_pool.data() + ib.offset, preview_len, 16, preview_hex, sizeof(preview_hex), preview_ascii, sizeof(preview_ascii));

                        ImGui::TextUnformatted(preview_hex);
                        
                        ImGui::TableSetColumnIndex(5);
                        ImGui::TextUnformatted(preview_ascii);
                        
                        if (g_mono_font) ImGui::PopFont();

                        if (node_open)
                        {
                            expanded_nodes.insert(actual_index);
                            ImGui::TreePop();
                        }
                    }
                }
            }

            if (chunk_end < (int)buffers.size())
            {
                int actual_index = chunk_end;
                const auto& ib = buffers[actual_index];
                
                if (table_begun)
                {
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)actual_index, ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen, "");

                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%zu", actual_index);

                    ImGui::TableSetColumnIndex(2);
                    ImGui::TextUnformatted(adam::get_log_time_string(ib.timestamp).c_str());

                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("%zu B", static_cast<size_t>(ib.size));

                    ImGui::TableSetColumnIndex(4);
                    if (g_mono_font) ImGui::PushFont(g_mono_font);

                    char preview_hex[64];
                    char preview_ascii[32];
                    size_t preview_len = std::min((size_t)ib.size, (size_t)16);
                    fill_hex_preview(data_pool.data() + ib.offset, preview_len, 16, preview_hex, sizeof(preview_hex), preview_ascii, sizeof(preview_ascii));

                    ImGui::TextUnformatted(preview_hex);
                    
                    ImGui::TableSetColumnIndex(5);
                    ImGui::TextUnformatted(preview_ascii);
                    
                    if (g_mono_font) ImGui::PopFont();

                    if (!node_open)
                    {
                        expanded_nodes.erase(actual_index);
                    }
                    else
                    {
                        ImGui::TreePop();
                    }

                    ImGui::EndTable();
                }

                if (current_pushed_id != -1)
                {
                    ImGui::PopID();
                    current_pushed_id = -1;
                }

                if (expanded_nodes.count(actual_index) > 0)
                {
                    render_inspector_hex_dump(data_pool.data() + ib.offset, ib.size, actual_index, inspector_height, dpi_scale, lang);
                }

                current_pushed_id = actual_index;
                ImGui::PushID(current_pushed_id);
                table_begun = ImGui::BeginTable("InspectorTableInner", 6, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable);
                if (table_begun)
                {
                    setup_inner_columns();
                }

                expanded_list_idx++;
                current_idx = chunk_end + 1;
            }
            else
            {
                current_idx = chunk_end;
            }
        }

        if (table_begun)
        {
            ImGui::EndTable();
        }
        
        if (current_pushed_id != -1)
        {
            ImGui::PopID();
        }

        if (auto_scroll && ImGui::GetScrollMaxY() > 0.0f)
        {
            ImGui::SetScrollY(ImGui::GetScrollMaxY());
        }
        
        ImGui::EndChild(); // End of inner child

        char clear_btn_text[512];
        if (lang == adam::language_german)
        {
            snprintf(clear_btn_text, sizeof(clear_btn_text), "Daten löschen für \"%s\"", name);
        }
        else
        {
            snprintf(clear_btn_text, sizeof(clear_btn_text), "Clear Data for \"%s\"", name);
        }

        if (ImGui::Button(clear_btn_text, ImVec2(-1.0f, 0.0f)))
        {
            port_data.buffers.clear();
            port_data.data_pool.clear();
            port_data.data_pool.shrink_to_fit();
            port_data.expanded_nodes.clear();
        }
        
        ImGui::EndChild(); // End of outer_child
        
        ImGui::Spacing();
        ImGui::PopID();
    }

    void render_inspector_view(gui_controller& ctrl, adam::language lang)
    {
        if (!ctrl.is_commander_active())
        {
            return;
        }

        auto& reg = ctrl.commander().registry();

        auto& inspectors = ctrl.commander().get_inspectors();
        const auto& ports = reg.get_ports();
        
        float dpi_scale = ImGui::GetStyle()._MainScale;
        auto* s_theme_param = dynamic_cast<adam::configuration_parameter_string*>(ctrl.get_parameters().get("theme"_ct));
        const bool s_is_light_theme = s_theme_param && s_theme_param->get_value() == "default-light"_ct;

        auto format_bytes_to_buf = [](uint64_t bytes, char* buf, size_t buf_size) 
        {
            if (bytes < 1024) snprintf(buf, buf_size, "%llu B", (unsigned long long)bytes);
            else if (bytes < 1024 * 1024) snprintf(buf, buf_size, "%.2f KB", bytes / 1024.0);
            else if (bytes < 1024 * 1024 * 1024) snprintf(buf, buf_size, "%.2f MB", bytes / (1024.0 * 1024.0));
            else snprintf(buf, buf_size, "%.2f GB", bytes / (1024.0 * 1024.0 * 1024.0));
        };

        const auto& connections = reg.get_connections();
        auto& connection_input_inspectors = ctrl.commander().get_connection_input_inspectors();
        auto& connection_output_inspectors = ctrl.commander().get_connection_output_inspectors();

        for (auto it = g_expanded_inspector_connections_input.begin(); it != g_expanded_inspector_connections_input.end(); )
        {
            if (connections.find(*it) == connections.end()) it = g_expanded_inspector_connections_input.erase(it);
            else ++it;
        }
        for (auto it = g_pending_inspector_connections_input.begin(); it != g_pending_inspector_connections_input.end(); )
        {
            if (connections.find(*it) == connections.end()) it = g_pending_inspector_connections_input.erase(it);
            else ++it;
        }

        for (auto it = g_expanded_inspector_connections_output.begin(); it != g_expanded_inspector_connections_output.end(); )
        {
            if (connections.find(*it) == connections.end()) it = g_expanded_inspector_connections_output.erase(it);
            else ++it;
        }
        for (auto it = g_pending_inspector_connections_output.begin(); it != g_pending_inspector_connections_output.end(); )
        {
            if (connections.find(*it) == connections.end()) it = g_pending_inspector_connections_output.erase(it);
            else ++it;
        }

        // Clean up any expanded/pending ports that no longer exist in the registry
        for (auto it = g_expanded_inspector_ports.begin(); it != g_expanded_inspector_ports.end(); )
        {
            if (ports.find(*it) == ports.end()) it = g_expanded_inspector_ports.erase(it);
            else ++it;
        }

        for (auto it = g_pending_inspector_ports.begin(); it != g_pending_inspector_ports.end(); )
        {
            if (ports.find(*it) == ports.end()) it = g_pending_inspector_ports.erase(it);
            else ++it;
        }

        if (g_port_to_expand_in_inspector != 0)
        {
            g_expanded_inspector_ports.insert(g_port_to_expand_in_inspector);
            g_port_to_expand_in_inspector = 0;
        }

        if (g_connection_input_to_expand_in_inspector != 0)
        {
            g_expanded_inspector_connections_input.insert(g_connection_input_to_expand_in_inspector);
            g_connection_input_to_expand_in_inspector = 0;
        }

        if (g_connection_output_to_expand_in_inspector != 0)
        {
            g_expanded_inspector_connections_output.insert(g_connection_output_to_expand_in_inspector);
            g_connection_output_to_expand_in_inspector = 0;
        }

        size_t num_expanded = 0;
        for (const auto& [hash, p_view] : ports)
        {
            if (g_expanded_inspector_ports.count(hash))
            {
                bool has_data = false;
                {
                    std::lock_guard<std::mutex> buffer_lock(adam::gui::g_inspection_data.mtx);
                    auto it = adam::gui::g_inspection_data.ports.find(hash);
                    if (it != adam::gui::g_inspection_data.ports.end() && !it->second.buffers.empty()) has_data = true;
                }
                bool has_inspector = inspectors.find(hash) != inspectors.end();
                if (has_inspector || has_data)
                {
                    num_expanded++;
                    g_pending_inspector_ports.erase(hash);
                }
                else
                {
                    if (g_pending_inspector_ports.count(hash))
                        num_expanded++;
                    else
                        g_expanded_inspector_ports.erase(hash);
                }
            }
        }

        for (const auto& [hash, conn] : connections)
        {
            if (g_expanded_inspector_connections_input.count(hash))
            {
                bool has_data = false;
                {
                    std::lock_guard<std::mutex> buffer_lock(adam::gui::g_inspection_data.mtx);
                    auto it = adam::gui::g_inspection_data.connections_input.find(hash);
                    if (it != adam::gui::g_inspection_data.connections_input.end() && !it->second.buffers.empty()) has_data = true;
                }
                bool has_inspector = connection_input_inspectors.find(hash) != connection_input_inspectors.end();
                if (has_inspector || has_data)
                {
                    num_expanded++;
                    g_pending_inspector_connections_input.erase(hash);
                }
                else
                {
                    if (g_pending_inspector_connections_input.count(hash)) num_expanded++;
                    else g_expanded_inspector_connections_input.erase(hash);
                }
            }

            if (g_expanded_inspector_connections_output.count(hash))
            {
                bool has_data = false;
                {
                    std::lock_guard<std::mutex> buffer_lock(adam::gui::g_inspection_data.mtx);
                    auto it = adam::gui::g_inspection_data.connections_output.find(hash);
                    if (it != adam::gui::g_inspection_data.connections_output.end() && !it->second.buffers.empty()) has_data = true;
                }
                bool has_inspector = connection_output_inspectors.find(hash) != connection_output_inspectors.end();
                if (has_inspector || has_data)
                {
                    num_expanded++;
                    g_pending_inspector_connections_output.erase(hash);
                }
                else
                {
                    if (g_pending_inspector_connections_output.count(hash)) num_expanded++;
                    else g_expanded_inspector_connections_output.erase(hash);
                }
            }
        }

        float sticky_button_h = ImGui::GetFrameHeight() * 1.5f + ImGui::GetStyle().ItemSpacing.y;
        float base_outer_h = ImGui::GetFrameHeightWithSpacing() * (ports.size() + connections.size() + 4);
        float avail_for_inner = ImGui::GetWindowHeight() - base_outer_h - ImGui::GetStyle().FramePadding.y - sticky_button_h;
        float inspector_height = 150.0f * dpi_scale;
        if (num_expanded == 1)
        {
            inspector_height = avail_for_inner;
        }
        else if (num_expanded >= 2)
        {
            inspector_height = avail_for_inner / 2.0f;
        }
        if (inspector_height < 150.0f * dpi_scale) 
        {
            inspector_height = 150.0f * dpi_scale;
        }

        ImGui::BeginChild("InspectorScrollContent", ImVec2(0, -sticky_button_h), false);

        bool conn_table_open = ImGui::BeginTable("InspectorConnectionsTable", 7, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable);
        if (conn_table_open)
        {
            auto setup_columns = [dpi_scale, lang]() 
            {
                auto fixed_single_size = ImGui::GetTextLineHeight() + ImGui::GetStyle().CellPadding.x * 2.f;
                ImGui::TableSetupScrollFreeze(0, 3);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, fixed_single_size);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, ImGui::GetTextLineHeight() + ImGui::GetStyle().CellPadding.x);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_inspect, lang),       ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, fixed_single_size);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::lbl_connection, lang),    ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_messages, lang),      ImGuiTableColumnFlags_WidthFixed, 80.0f * dpi_scale);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_size, lang),          ImGuiTableColumnFlags_WidthFixed, 80.0f * dpi_scale);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_last_received, lang), ImGuiTableColumnFlags_WidthStretch);
            };

            setup_columns();
            ImGui::TableHeadersRow();

            for (const auto& [conn_hash, c_view] : connections)
            {
                // Input Row
                {
                    bool has_inspector = connection_input_inspectors.find(conn_hash) != connection_input_inspectors.end();
                    bool has_data = false;
                    size_t msg_count = 0;
                    size_t total_size = 0;
                    uint64_t last_ts = 0;
                    char preview_hex[32] = "";

                    {
                        std::lock_guard<std::mutex> buffer_lock(adam::gui::g_inspection_data.mtx);
                        auto it = adam::gui::g_inspection_data.connections_input.find(conn_hash);
                        if (it != adam::gui::g_inspection_data.connections_input.end())
                        {
                            const auto& port_data = it->second;
                            const auto& buffers = port_data.buffers;
                            msg_count = buffers.size();
                            if (msg_count > 0)
                            {
                                has_data = true;
                                total_size = port_data.data_pool.size();
                                const auto& last_b = buffers.back();
                                last_ts = last_b.timestamp;
                                
                                size_t preview_len = std::min((size_t)last_b.size, (size_t)8);
                                char preview_hex_buf[32];
                                char preview_ascii_dummy[16];
                                fill_hex_preview(port_data.data_pool.data() + last_b.offset, preview_len, 8, preview_hex_buf, sizeof(preview_hex_buf), preview_ascii_dummy, sizeof(preview_ascii_dummy));
                                snprintf(preview_hex, sizeof(preview_hex), "%s", preview_hex_buf);
                            }
                        }
                    }

                    if (true)
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        bool pushed_id = false;
                        ImGui::PushID((const void*)(intptr_t)(conn_hash ^ 0x9990));
                        pushed_id = true;
                        
                        bool is_expanded = g_expanded_inspector_connections_input.count(conn_hash) > 0;
                        
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
                        if (ImGui::ArrowButton("##node_in", is_expanded ? ImGuiDir_Down : ImGuiDir_Right))
                        {
                            if (is_expanded)
                                g_expanded_inspector_connections_input.erase(conn_hash);
                            else
                                g_expanded_inspector_connections_input.insert(conn_hash);
                            is_expanded = !is_expanded;
                        }
                        ImGui::PopStyleColor(3);
                        bool node_open = is_expanded;
                        
                        ImGui::TableSetColumnIndex(1);
                        ImColor pin_col = s_is_light_theme ? get_gui_color(gui_color_id::node_connection_line_light) : get_gui_color(gui_color_id::node_connection_line);
                        if (c_view->is_active)
                            pin_col = get_gui_color(gui_color_id::node_pin_active);
                        ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
                        float dot_radius = ImGui::GetTextLineHeight() / 2 - ImGui::GetStyle().CellPadding.y;
                        ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(cursor_pos.x + dot_radius + ImGui::GetStyle().CellPadding.x, cursor_pos.y + ImGui::GetFrameHeight() * 0.5f), dot_radius, pin_col);
                        
                        ImGui::TableSetColumnIndex(2);
                        bool inspect_val = has_inspector;
                        ImGui::PushID((const void*)(intptr_t)(conn_hash ^ 0x9991));
                        if (ImGui::Checkbox("##inspect_in", &inspect_val))
                        {
                            if (inspect_val)
                            {
                                ctrl.enqueue_commander_action([&ctrl, conn_hash]() 
                                {
                                    auto& cmdr = ctrl.commander();
                                    if (cmdr.get_connection_input_inspectors().find(conn_hash) == cmdr.get_connection_input_inspectors().end())
                                    {
                                        adam::data_inspector* new_inspector = nullptr;
                                        cmdr.request_connection_input_inspector_create(conn_hash, make_inspector_connection_input_buffer_callback(conn_hash), new_inspector);
                                    }
                                });
                            }
                            else
                            {
                                g_expanded_inspector_connections_input.erase(conn_hash);
                                g_pending_inspector_connections_input.erase(conn_hash);
                                ctrl.enqueue_commander_action([&ctrl, conn_hash]() 
                                {
                                    auto& cmdr = ctrl.commander();
                                    auto it = cmdr.connection_input_inspectors().find(conn_hash);
                                    if (it != cmdr.connection_input_inspectors().end())
                                        cmdr.request_connection_input_inspector_destroy(it->second);
                                });
                            }
                        }
                        ImGui::PopID();
                        
                        ImGui::TableSetColumnIndex(3);
                        char name_buf[256];
                        snprintf(name_buf, sizeof(name_buf), "%s [Input]", c_view->name.c_str());
                        ImGui::TextUnformatted(name_buf);
                        
                        ImGui::TableSetColumnIndex(4);
                        ImGui::Text("%zu", msg_count);
                        
                        ImGui::TableSetColumnIndex(5);
                        char total_size_buf[64];
                        format_bytes_to_buf(total_size, total_size_buf, sizeof(total_size_buf));
                        ImGui::TextUnformatted(total_size_buf);
                        
                        ImGui::TableSetColumnIndex(6);
                        if (msg_count > 0)
                            ImGui::Text("%s | %s", adam::get_log_time_string(last_ts).c_str(), preview_hex);
                        else
                            ImGui::TextDisabled("%s", get_gui_string(gui_string_id::lbl_no_data, lang));
                            
                        ImGui::PopID();
                        
                        if (node_open)
                        {
                            ImGui::EndTable();
                            render_inspector_frames_table(name_buf, conn_hash, adam::gui::g_inspection_data.connections_input, inspector_height, dpi_scale, lang, 0x1111111111111111ULL);
                            conn_table_open = ImGui::BeginTable("InspectorConnectionsTable", 7, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable);
                            if (conn_table_open) setup_columns(); else break;
                        }
                    }
                }

                // Output Row
                {
                    bool has_inspector = connection_output_inspectors.find(conn_hash) != connection_output_inspectors.end();
                    bool has_data = false;
                    size_t msg_count = 0;
                    size_t total_size = 0;
                    uint64_t last_ts = 0;
                    char preview_hex[32] = "";

                    {
                        std::lock_guard<std::mutex> buffer_lock(adam::gui::g_inspection_data.mtx);
                        auto it = adam::gui::g_inspection_data.connections_output.find(conn_hash);
                        if (it != adam::gui::g_inspection_data.connections_output.end())
                        {
                            const auto& port_data = it->second;
                            const auto& buffers = port_data.buffers;
                            msg_count = buffers.size();
                            if (msg_count > 0)
                            {
                                has_data = true;
                                total_size = port_data.data_pool.size();
                                const auto& last_b = buffers.back();
                                last_ts = last_b.timestamp;
                                
                                size_t preview_len = std::min((size_t)last_b.size, (size_t)8);
                                char preview_hex_buf[32];
                                char preview_ascii_dummy[16];
                                fill_hex_preview(port_data.data_pool.data() + last_b.offset, preview_len, 8, preview_hex_buf, sizeof(preview_hex_buf), preview_ascii_dummy, sizeof(preview_ascii_dummy));
                                snprintf(preview_hex, sizeof(preview_hex), "%s", preview_hex_buf);
                            }
                        }
                    }

                    if (true)
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        bool pushed_id = false;
                        ImGui::PushID((const void*)(intptr_t)(conn_hash ^ 0x9992));
                        pushed_id = true;
                        
                        bool is_expanded = g_expanded_inspector_connections_output.count(conn_hash) > 0;
                        
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
                        if (ImGui::ArrowButton("##node_out", is_expanded ? ImGuiDir_Down : ImGuiDir_Right))
                        {
                            if (is_expanded)
                                g_expanded_inspector_connections_output.erase(conn_hash);
                            else
                                g_expanded_inspector_connections_output.insert(conn_hash);
                            is_expanded = !is_expanded;
                        }
                        ImGui::PopStyleColor(3);
                        bool node_open = is_expanded;
                        
                        ImGui::TableSetColumnIndex(1);
                        ImColor pin_col = s_is_light_theme ? get_gui_color(gui_color_id::node_connection_line_light) : get_gui_color(gui_color_id::node_connection_line);
                        if (c_view->is_active)
                            pin_col = get_gui_color(gui_color_id::node_pin_active);
                        ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
                        float dot_radius = ImGui::GetTextLineHeight() / 2 - ImGui::GetStyle().CellPadding.y;
                        ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(cursor_pos.x + dot_radius + ImGui::GetStyle().CellPadding.x, cursor_pos.y + ImGui::GetFrameHeight() * 0.5f), dot_radius, pin_col);
                        
                        ImGui::TableSetColumnIndex(2);
                        bool inspect_val = has_inspector;
                        ImGui::PushID((const void*)(intptr_t)(conn_hash ^ 0x9993));
                        if (ImGui::Checkbox("##inspect_out", &inspect_val))
                        {
                            if (inspect_val)
                            {
                                ctrl.enqueue_commander_action([&ctrl, conn_hash]() 
                                {
                                    auto& cmdr = ctrl.commander();
                                    if (cmdr.get_connection_output_inspectors().find(conn_hash) == cmdr.get_connection_output_inspectors().end())
                                    {
                                        adam::data_inspector* new_inspector = nullptr;
                                        cmdr.request_connection_output_inspector_create(conn_hash, make_inspector_connection_output_buffer_callback(conn_hash), new_inspector);
                                    }
                                });
                            }
                            else
                            {
                                g_expanded_inspector_connections_output.erase(conn_hash);
                                g_pending_inspector_connections_output.erase(conn_hash);
                                ctrl.enqueue_commander_action([&ctrl, conn_hash]() 
                                {
                                    auto& cmdr = ctrl.commander();
                                    auto it = cmdr.connection_output_inspectors().find(conn_hash);
                                    if (it != cmdr.connection_output_inspectors().end())
                                        cmdr.request_connection_output_inspector_destroy(it->second);
                                });
                            }
                        }
                        ImGui::PopID();
                        
                        ImGui::TableSetColumnIndex(3);
                        char name_buf[256];
                        snprintf(name_buf, sizeof(name_buf), "%s [Output]", c_view->name.c_str());
                        ImGui::TextUnformatted(name_buf);
                        
                        ImGui::TableSetColumnIndex(4);
                        ImGui::Text("%zu", msg_count);
                        
                        ImGui::TableSetColumnIndex(5);
                        char total_size_buf[64];
                        format_bytes_to_buf(total_size, total_size_buf, sizeof(total_size_buf));
                        ImGui::TextUnformatted(total_size_buf);
                        
                        ImGui::TableSetColumnIndex(6);
                        if (msg_count > 0)
                            ImGui::Text("%s | %s", adam::get_log_time_string(last_ts).c_str(), preview_hex);
                        else
                            ImGui::TextDisabled("%s", get_gui_string(gui_string_id::lbl_no_data, lang));
                            
                        ImGui::PopID();
                        
                        if (node_open)
                        {
                            ImGui::EndTable();
                            render_inspector_frames_table(name_buf, conn_hash, adam::gui::g_inspection_data.connections_output, inspector_height, dpi_scale, lang, 0x2222222222222222ULL);
                            conn_table_open = ImGui::BeginTable("InspectorConnectionsTable", 7, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable);
                            if (conn_table_open) setup_columns(); else break;
                        }
                    }
                }
            }

            if (conn_table_open) ImGui::EndTable();
        }

        ImGui::Spacing();

        bool table_open = ImGui::BeginTable("InspectorPortsTable", 7, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable);
        if (table_open)
        {
            auto setup_columns = [dpi_scale, lang]() 
            {
                auto fixed_single_size = ImGui::GetTextLineHeight() + ImGui::GetStyle().CellPadding.x * 2.f;
                ImGui::TableSetupScrollFreeze(0, 3);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, fixed_single_size);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, ImGui::GetTextLineHeight() + ImGui::GetStyle().CellPadding.x);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_inspect, lang),       ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, fixed_single_size);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_port_name, lang),     ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_messages, lang),      ImGuiTableColumnFlags_WidthFixed, 80.0f * dpi_scale);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_size, lang),          ImGuiTableColumnFlags_WidthFixed, 80.0f * dpi_scale);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_last_received, lang), ImGuiTableColumnFlags_WidthStretch);
            };

            setup_columns();
            ImGui::TableHeadersRow();

            for (const auto& [port_hash, p_view] : ports)
            {
                bool has_inspector = inspectors.find(port_hash) != inspectors.end();
                bool node_open = false;

                ImGui::TableNextRow();

                bool has_data = false;
                size_t msg_count = 0;
                size_t total_size = 0;
                uint64_t last_ts = 0;
                char preview_hex[32] = "";

                {
                    std::lock_guard<std::mutex> buffer_lock(adam::gui::g_inspection_data.mtx);
                    auto it = adam::gui::g_inspection_data.ports.find(port_hash);
                    if (it != adam::gui::g_inspection_data.ports.end())
                    {
                        const auto& port_data = it->second;
                        const auto& buffers = port_data.buffers;
                        msg_count = buffers.size();
                        if (msg_count > 0)
                        {
                            has_data = true;
                            total_size = port_data.data_pool.size();
                            const auto& last_b = buffers.back();
                            last_ts = last_b.timestamp;
                            
                            size_t preview_len = std::min((size_t)last_b.size, (size_t)8);
                            char preview_hex_buf[32];
                            char preview_ascii_dummy[16];
                            fill_hex_preview(port_data.data_pool.data() + last_b.offset, preview_len, 8, preview_hex_buf, sizeof(preview_hex_buf), preview_ascii_dummy, sizeof(preview_ascii_dummy));
                            snprintf(preview_hex, sizeof(preview_hex), "%s", preview_hex_buf);
                        }
                    }
                }

                // ---- BEGIN Expandable  ----
                ImGui::TableSetColumnIndex(0);
                bool pushed_id = false;
                if (has_inspector || has_data)
                {
                    ImGui::PushID((const void*)(intptr_t)(port_hash ^ 0x9999));
                    pushed_id = true;
                    
                    bool is_expanded = g_expanded_inspector_ports.count(port_hash) > 0;
                    
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
                    if (ImGui::ArrowButton("##node", is_expanded ? ImGuiDir_Down : ImGuiDir_Right))
                    {
                        if (is_expanded)
                            g_expanded_inspector_ports.erase(port_hash);
                        else
                            g_expanded_inspector_ports.insert(port_hash);
                        is_expanded = !is_expanded;
                    }
                    ImGui::PopStyleColor(3);
                    node_open = is_expanded;
                }
                // ---- END Expandable  ----
                
                // ---- BEGIN Status Icon  ----
                ImGui::TableSetColumnIndex(1);
                ImColor pin_col = s_is_light_theme ? get_gui_color(gui_color_id::node_connection_line_light) : get_gui_color(gui_color_id::node_connection_line);
                if (p_view->is_active)
                    pin_col = get_gui_color(gui_color_id::node_pin_active);

                ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
                float dot_radius = ImGui::GetTextLineHeight() / 2 - ImGui::GetStyle().CellPadding.y;
                ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(cursor_pos.x + dot_radius + ImGui::GetStyle().CellPadding.x, cursor_pos.y + ImGui::GetFrameHeight() * 0.5f), dot_radius, pin_col);
                // ---- END Status Icon  ----
                
                // ---- BEGIN Inspect  ----
                ImGui::TableSetColumnIndex(2);
                
                bool inspect_val = has_inspector;
                ImGui::PushID((const void*)(intptr_t)port_hash);
                if (ImGui::Checkbox("##inspect", &inspect_val))
                {
                    if (inspect_val)
                    {
                        ctrl.enqueue_commander_action([&ctrl, port_hash]() 
                        {
                            auto& cmdr = ctrl.commander();
                            if (cmdr.inspectors().find(port_hash) == cmdr.inspectors().end())
                            {
                                adam::data_inspector* new_inspector = nullptr;
                                cmdr.request_inspector_create(port_hash, make_inspector_buffer_callback(port_hash), new_inspector);
                            }
                        });
                    }
                    else
                    {
                        g_expanded_inspector_ports.erase(port_hash);
                        g_pending_inspector_ports.erase(port_hash);

                        ctrl.enqueue_commander_action([&ctrl, port_hash]() 
                        {
                            auto& cmdr = ctrl.commander();
                            auto it = cmdr.inspectors().find(port_hash);
                            if (it != cmdr.inspectors().end())
                            {
                                cmdr.request_inspector_destroy(it->second);
                            }
                        });
                    }
                }
                ImGui::PopID();
                // ---- END Expandable  ----

                // ---- BEGIN Name  ----
                ImGui::TableSetColumnIndex(3);
                ImGui::TextUnformatted(p_view->name.c_str());
                // ---- END Name  ----

                if (has_inspector || has_data)
                {
                    // ---- BEGIN Messages  ----
                    ImGui::TableSetColumnIndex(4);
                    ImGui::Text("%zu", msg_count);
                    // ---- END Messages  ----

                    ImGui::TableSetColumnIndex(5);
                    char total_size_buf[64];
                    format_bytes_to_buf(total_size, total_size_buf, sizeof(total_size_buf));
                    ImGui::TextUnformatted(total_size_buf);

                    ImGui::TableSetColumnIndex(6);
                    if (msg_count > 0)
                    {
                        ImGui::Text("%s | %s", adam::get_log_time_string(last_ts).c_str(), preview_hex);
                    }
                    else
                    {
                        ImGui::TextDisabled("%s", get_gui_string(gui_string_id::lbl_no_data, lang));
                    }
                }

                if (pushed_id)
                {
                    ImGui::PopID();
                }

                if (node_open)
                {
                    ImGui::EndTable();
                    
                    render_inspector_frames_table(p_view->name.c_str(), port_hash, adam::gui::g_inspection_data.ports, inspector_height, dpi_scale, lang, 0x3333333333333333ULL);

                    table_open = ImGui::BeginTable("InspectorPortsTable", 7, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable);
                    if (table_open)
                    {
                        setup_columns();
                    }
                    else
                    {
                        break;
                    }
                }
            }

            if (table_open)
            {
                ImGui::EndTable();
            }
        }

        ImGui::EndChild(); // End of InspectorScrollContent

        bool has_any_buffered_data = false;
        {
            std::lock_guard<std::mutex> buffer_lock(adam::gui::g_inspection_data.mtx);
            for (const auto& [hash, port_data] : adam::gui::g_inspection_data.ports)
            {
                if (!port_data.buffers.empty())
                {
                    has_any_buffered_data = true;
                    break;
                }
            }
            if (!has_any_buffered_data)
            {
                for (const auto& [hash, port_data] : adam::gui::g_inspection_data.connections_input)
                {
                    if (!port_data.buffers.empty())
                    {
                        has_any_buffered_data = true;
                        break;
                    }
                }
            }
            if (!has_any_buffered_data)
            {
                for (const auto& [hash, port_data] : adam::gui::g_inspection_data.connections_output)
                {
                    if (!port_data.buffers.empty())
                    {
                        has_any_buffered_data = true;
                        break;
                    }
                }
            }
        }

        ImGui::BeginDisabled(!has_any_buffered_data);
        if (ImGui::Button(get_gui_string(gui_string_id::btn_clear_all_data, lang), ImVec2(-1.0f, ImGui::GetFrameHeight() * 1.5f)))
        {
            std::lock_guard<std::mutex> buffer_lock(adam::gui::g_inspection_data.mtx);
            adam::gui::g_inspection_data.ports.clear();
            adam::gui::g_inspection_data.connections_input.clear();
            adam::gui::g_inspection_data.connections_output.clear();
        }
        ImGui::EndDisabled();
    }

    void render_inspector_subwindow(gui_controller& ctrl, adam::language lang, float& left_w, float avail_w, float content_h)
    {
        ImGui::SameLine();

        float dpi_scale     = ImGui::GetStyle()._MainScale;
        float splitter_w    = 4.0f * dpi_scale;
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_Separator));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_SeparatorHovered));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_SeparatorActive));
        ImGui::Button("##vsplitter", ImVec2(splitter_w, content_h));
        ImGui::PopStyleColor(3);

        if (ImGui::IsItemActive())
        {
            left_w += ImGui::GetIO().MouseDelta.x;
            if (left_w < 100.0f * dpi_scale) left_w = 100.0f * dpi_scale;
            if (left_w > avail_w - 100.0f * dpi_scale) left_w = avail_w - 100.0f * dpi_scale;
        }
        if (ImGui::IsItemHovered())
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

        ImGui::SameLine();

        ImGui::BeginChild("InspectorRegion", ImVec2(0, content_h), false);
        render_inspector_view(ctrl, lang);
        ImGui::EndChild();
    }

    void render_tab_management(gui_controller& ctrl, adam::language lang)
    {
        static bool show_inspector = false;
        if (g_request_open_inspector)
        {
            show_inspector = true;
            g_request_open_inspector = false;
        }

        static size_t last_frame_inspector_count = 0;
        size_t current_inspector_count = 0;
        if (ctrl.is_commander_active())
        {
            current_inspector_count = ctrl.commander().get_inspectors().size() + 
                                      ctrl.commander().get_connection_input_inspectors().size() + 
                                      ctrl.commander().get_connection_output_inspectors().size();
        }

        if (last_frame_inspector_count > 0 && current_inspector_count == 0)
        {
            show_inspector = false;
        }
        last_frame_inspector_count = current_inspector_count;
        bool commander_active   = ctrl.is_commander_active();

        render_delete_connection_modal(ctrl, lang);
        render_create_connection_modal(ctrl, lang);
        render_add_create_port_modal(ctrl, lang);

        auto& reg_view = ctrl.commander().registry();
        std::lock_guard<const adam::registry_view> lock(reg_view);

        const auto& connections = reg_view.get_connections();
        
        static adam::configuration_parameter_integer* sort_mode_param = dynamic_cast<adam::configuration_parameter_integer*>(ctrl.get_parameters().get("connection_sort_mode"_ct));
        int sort_mode = static_cast<int>(sort_mode_param->get_value());
        
        render_top_control_bar(ctrl, lang, sort_mode, show_inspector);

        ImGui::Spacing();

        static std::vector<std::pair<adam::string_hash, adam::connection_view*>> sorted_connections;
        sorted_connections.clear();
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
                        [dragged_hash](const auto& pair)
                        {
                            return pair.first == dragged_hash;
                        });
                        if (it != sorted_connections.end())
                            g_active_drag_target_index = std::distance(sorted_connections.begin(), it);
                        else
                            g_active_drag_target_index = 0;
                    }

                    auto it = std::find_if(sorted_connections.begin(), sorted_connections.end(), 
                    [dragged_hash](const auto& pair)
                    {
                        return pair.first == dragged_hash;
                    });
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

        float avail_w = ImGui::GetContentRegionAvail().x;
        float content_h = ImGui::GetContentRegionAvail().y;

        static float left_ratio = 0.66f;
        float dpi_scale     = ImGui::GetStyle()._MainScale;
        float left_w        = avail_w * left_ratio;
        
        if (avail_w > 0.0f)
        {
            if (left_w < 100.0f * dpi_scale) left_w = 100.0f * dpi_scale;
            if (show_inspector && left_w > avail_w - 100.0f * dpi_scale) left_w = avail_w - 100.0f * dpi_scale;
            left_ratio = left_w / avail_w;
        }

        if (show_inspector) 
            ImGui::BeginChild("ConnectionsRegion", ImVec2(left_w, content_h), false);

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
                        ctrl.enqueue_commander_action([&ctrl, h, new_idx]() { ctrl.commander().request_connection_sorting_index_change(h, new_idx); });
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
                ctrl.enqueue_commander_action([&ctrl, new_conn_name]() { ctrl.commander().request_connection_create(new_conn_name); });
            }
        }
        if (!commander_active) ImGui::EndDisabled();

        if (show_inspector)
        {
            ImGui::EndChild();
            render_inspector_subwindow(ctrl, lang, left_w, avail_w, content_h);
            if (avail_w > 0.0f)
            {
                left_ratio = left_w / avail_w;
            }
        }
    }
}