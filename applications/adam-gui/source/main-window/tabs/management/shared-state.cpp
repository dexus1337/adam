/**
 * @file    shared-state.cpp
 * @author  dexus1337
 * @brief   Implementation of shared globals, parsing, and telemetry callback helpers.
 * @version 1.0
 * @date    12.06.2026
 */

#include "shared-state.hpp"
#include "../../main-window.hpp"
#include "controller/controller.hpp"
#include <mutex>
#include <chrono>
#include <cctype>

namespace adam::gui
{
    // Define shared globals
    adam::string_hashed g_connection_to_delete("");
    bool g_request_delete_popup = false;
    ImVec2 g_connection_drag_offset(0, 0);

    adam::string_hashed g_target_connection("");
    adam::port::direction g_target_direction = adam::port::direction_invalid;
    bool g_request_port_popup = false;
    char g_port_popup_title[adam::max_name_length] = "Add Port###PortPopup";

    bool g_request_processor_popup = false;
    char g_processor_popup_title[adam::max_name_length] = "Add Processor###ProcessorPopup";

    adam::string_hash g_active_drag_hash = 0;
    size_t g_active_drag_target_index = 0;

    bool g_is_dragging_processor = false;
    adam::string_hash g_dragged_processor_conn_hash = 0;
    adam::string_hash g_dragged_processor_hash = 0;
    int g_active_processor_drag_target_index = -1;
    ImVec2 g_processor_drag_offset(0, 0);

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
    adam::string_hash g_connection_output_to_expand_in_inspector = 0;

    std::map<uint64_t, inject_buffer_state> g_inject_data_buffers;

    std::vector<std::vector<connection_pin_data>> g_stage_pins_in_normal, g_stage_pins_out_normal;
    std::vector<std::vector<connection_pin_data>> g_stage_pins_in_preview, g_stage_pins_out_preview;

    // Helper function implementations
    int inject_text_resize_callback(ImGuiInputTextCallbackData* data)
    {
        if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
        {
            std::string* str = static_cast<std::string*>(data->UserData);
            str->resize(data->BufSize);
            data->Buf = str->data();
        }
        return 0;
    }

    std::pair<bool, std::vector<uint8_t>> parse_hex_bytes(const std::string& input)
    {
        std::vector<uint8_t> bytes;
        std::string current_token;
        bool is_valid = true;

        auto process_token = [&bytes, &is_valid](const std::string& token)
        {
            if (!is_valid) return;

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

    void fill_hex_preview
    (
        const uint8_t* data,
        size_t data_len,
        size_t max_bytes,
        char* hex_buf,
        size_t hex_buf_size,
        char* ascii_buf,
        size_t ascii_buf_size
    )
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

    void draw_direction_badge(adam::port::direction dir, bool is_used, adam::language lang)
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

    std::function<void(adam::buffer*)> make_inspector_buffer_callback(adam::string_hash port_hash)
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

    std::function<void(adam::buffer*)> make_inspector_connection_input_buffer_callback(adam::string_hash conn_hash)
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

    std::function<void(adam::buffer*)> make_inspector_connection_output_buffer_callback(adam::string_hash conn_hash)
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
}
