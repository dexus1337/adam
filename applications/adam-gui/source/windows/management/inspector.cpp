/**
 * @file    inspector.cpp
 * @author  dexus1337
 * @brief   Source containing diagnostic data telemetry inspector drawing implementations.
 * @version 1.0
 * @date    12.06.2026
 */

#include "inspector.hpp"
#include "imgui.h"
#include "shared-state.hpp"
#include "../main-window.hpp"
#include "controller/controller.hpp"
#include "module/module.hpp"
#include "data/format.hpp"
#include "configuration/parameters/configuration-parameter-string.hpp"
#include <algorithm>
#include <vector>
#include <map>
#include <cmath>
#include <mutex>
#include <string>
#include <unordered_set>
#include <set>
#include <cctype>

namespace adam::gui
{
    static inline void draw_inspector_table_row
    (
        const adam::gui::inspected_buffer& ib,
        int actual_index,
        const std::vector<uint8_t>& data_pool,
        std::set<size_t>& expanded_nodes
    )
    {
        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        bool node_open = ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<intptr_t>(actual_index)), ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_OpenOnArrow | (expanded_nodes.count(actual_index) ? ImGuiTreeNodeFlags_DefaultOpen : 0), "%s", "");

        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", actual_index);

        ImGui::TableSetColumnIndex(2);
        ImGui::TextUnformatted(adam::get_log_time_string(ib.timestamp).c_str());

        ImGui::TableSetColumnIndex(3);
        ImGui::Text("%zu B", static_cast<size_t>(ib.size));

        ImGui::TableSetColumnIndex(4);
        if (ImGui::GetIO().Fonts->Fonts[1])
        {
            ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
        }

        char preview_hex[64];
        char preview_ascii[32];
        size_t preview_len = std::min(static_cast<size_t>(ib.size), static_cast<size_t>(16));
        fill_hex_preview(data_pool.data() + ib.offset, preview_len, 16, preview_hex, sizeof(preview_hex), preview_ascii, sizeof(preview_ascii));

        ImGui::TextUnformatted(preview_hex);
        
        ImGui::TableSetColumnIndex(5);
        ImGui::TextUnformatted(preview_ascii);
        
        if (ImGui::GetIO().Fonts->Fonts[1])
        {
            ImGui::PopFont();
        }

        if (node_open)
        {
            expanded_nodes.insert(actual_index);
            ImGui::TreePop();
        }
        else
        {
            expanded_nodes.erase(actual_index);
        }
    }

    struct hex_selection_state
    {
        int    buffer_id   = -1;
        int    start_idx   = -1;
        int    end_idx     = -1;
        bool   is_dragging = false;

        bool has_selection(int buf_id) const
        {
            return buf_id == buffer_id && start_idx >= 0 && end_idx >= 0;
        }

        void get_range(size_t& out_min, size_t& out_max) const
        {
            if (start_idx <= end_idx)
            {
                out_min = static_cast<size_t>(start_idx);
                out_max = static_cast<size_t>(end_idx);
            }
            else
            {
                out_min = static_cast<size_t>(end_idx);
                out_max = static_cast<size_t>(start_idx);
            }
        }

        bool is_selected(int buf_id, size_t idx) const
        {
            if (!has_selection(buf_id)) return false;
            size_t min_idx, max_idx;
            get_range(min_idx, max_idx);
            return idx >= min_idx && idx <= max_idx;
        }
    };
    static hex_selection_state s_hex_sel;

    static void draw_inspector_hex_dump
    (
        const uint8_t* data,
        size_t size,
        int actual_index,
        float dpi_scale,
        adam::language lang,
        float target_width
    )
    {
        size_t display_len = size;
        size_t num_rows = (display_len + 15) / 16;

        if (ImGui::GetIO().Fonts->Fonts[1])
        {
            ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
        }
        float line_h = ImGui::GetTextLineHeightWithSpacing();
        if (ImGui::GetIO().Fonts->Fonts[1])
        {
            ImGui::PopFont();
        }

        float window_border_size = ImGui::GetStyle().WindowBorderSize;
        float spacing_h = ImGui::GetStyle().ItemSpacing.y;
        float text_content_h = num_rows > 0 ? (line_h * num_rows) : 0.0f;
        float child_padding_h = (4.0f * dpi_scale) * 2.0f;
        float child_border_h = window_border_size * 2.0f;
        
        float calc_h = text_content_h + child_padding_h + child_border_h;
        float button_h = ImGui::GetFrameHeight();
        float container_padding_h = (6.0f * dpi_scale) * 2.0f;
        float container_border_h = window_border_size * 2.0f;
        float reserved_container_elements_h = button_h + spacing_h + container_padding_h + container_border_h;

        float child_h = calc_h;
        float container_h = child_h + reserved_container_elements_h;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(ImGui::GetStyle().WindowPadding.x, 6.0f * dpi_scale));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f * dpi_scale);

        ImGui::PushID(actual_index);
        if (ImGui::BeginChild("##hex_container", ImVec2(target_width, container_h), true))
        {
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f * dpi_scale, 2.0f * dpi_scale));

            float avail_w = ImGui::GetContentRegionAvail().x;
            float button_w = (avail_w - ImGui::GetStyle().ItemSpacing.x * 2.0f) / 3.0f;

            bool has_sel = s_hex_sel.has_selection(actual_index);
            size_t sel_min_idx = 0;
            size_t sel_max_idx = 0;
            if (has_sel)
            {
                s_hex_sel.get_range(sel_min_idx, sel_max_idx);
                if (sel_max_idx >= display_len) sel_max_idx = display_len > 0 ? display_len - 1 : 0;
            }

            size_t copy_start = has_sel ? sel_min_idx : 0;
            size_t copy_len   = has_sel ? (sel_max_idx - sel_min_idx + 1) : display_len;

            if (ImGui::Button(get_gui_string(gui_string_id::btn_copy_hex, lang), ImVec2(button_w, button_h)))
            {
                static std::string s_copy_str;
                s_copy_str.clear();
                s_copy_str.reserve(copy_len * 3);
                for (size_t j = 0; j < copy_len; ++j)
                {
                    char hex[4];
                    snprintf(hex, sizeof(hex), "%02X ", data[copy_start + j]);
                    s_copy_str += hex;
                }
                if (!s_copy_str.empty())
                {
                    s_copy_str.pop_back();
                }
                ImGui::SetClipboardText(s_copy_str.c_str());
            }
            ImGui::SameLine();

            if (ImGui::Button(get_gui_string(gui_string_id::btn_copy_ascii, lang), ImVec2(button_w, button_h)))
            {
                static std::string s_copy_str;
                s_copy_str.clear();
                s_copy_str.reserve(copy_len);
                for (size_t j = 0; j < copy_len; ++j)
                {
                    char c = data[copy_start + j];
                    s_copy_str += (c >= 32 && c <= 126) ? c : '.';
                }
                ImGui::SetClipboardText(s_copy_str.c_str());
            }
            ImGui::SameLine();

            if (ImGui::Button(get_gui_string(gui_string_id::btn_copy_hex_dump, lang), ImVec2(button_w, button_h)))
            {
                static std::string s_copy_str;
                s_copy_str.clear();
                size_t num_copy_rows = (copy_len + 15) / 16;
                s_copy_str.reserve(num_copy_rows * 80);
                for (size_t offset = 0; offset < copy_len; offset += 16)
                {
                    char line_buf[256];
                    int printed = snprintf(line_buf, sizeof(line_buf), "%04X:  ", static_cast<unsigned int>(copy_start + offset));
                    size_t chunk = std::min(static_cast<size_t>(16), copy_len - offset);
                    for (size_t j = 0; j < 16; ++j)
                    {
                        if (j == 8)
                        {
                            line_buf[printed++] = ' ';
                        }
                        if (j < chunk)
                        {
                            printed += snprintf(line_buf + printed, sizeof(line_buf) - printed, "%02X ", data[copy_start + offset + j]);
                        }
                        else
                        {
                            printed += snprintf(line_buf + printed, sizeof(line_buf) - printed, "   ");
                        }
                    }
                    line_buf[printed++] = ' ';
                    line_buf[printed++] = ' ';
                    line_buf[printed++] = '|';
                    for (size_t j = 0; j < chunk; ++j)
                    {
                        char c = data[copy_start + offset + j];
                        line_buf[printed++] = (c >= 32 && c <= 126) ? c : '.';
                    }
                    line_buf[printed++] = '|';
                    line_buf[printed++] = '\n';
                    line_buf[printed] = '\0';
                    s_copy_str += line_buf;
                }
                ImGui::SetClipboardText(s_copy_str.c_str());
            }

            ImGui::PopStyleVar();
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f * dpi_scale, 4.0f * dpi_scale));

            if (ImGui::BeginChild("##hex_child", ImVec2(-FLT_MIN, child_h), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
            {
                if (ImGui::GetIO().Fonts->Fonts[1])
                {
                    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
                }

                ImFont* font = ImGui::GetFont();
                float font_size = ImGui::GetFontSize();
                float char_w = ImGui::CalcTextSize("A").x;
                float text_h = ImGui::GetTextLineHeight();

                static constexpr const char* dummy_line = "0000:  00 11 22 33 44 55 66 77  88 99 AA BB CC DD EE FF   |0123456789ABCDEF|";
                float text_w = ImGui::CalcTextSize(dummy_line).x;
                float avail_w2 = ImGui::GetContentRegionAvail().x;
                float offset_x = (avail_w2 - text_w) / 2.0f;
                if (offset_x < 4.0f * dpi_scale)
                {
                    offset_x = 4.0f * dpi_scale;
                }

                float header_w = char_w * 7.0f;
                float byte_w = char_w * 3.0f;
                float ascii_start_x = header_w + 16.0f * byte_w + char_w * 2.0f;

                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                ImVec2 mouse_pos = ImGui::GetMousePos();
                bool is_window_hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
                bool is_mouse_down = ImGui::IsMouseDown(ImGuiMouseButton_Left);
                bool is_clicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);

                ImU32 col_text     = ImGui::GetColorU32(ImGuiCol_Text);
                ImU32 col_disabled = ImGui::GetColorU32(ImGuiCol_TextDisabled);
                ImU32 col_addr     = ImGui::GetColorU32(ImGuiCol_TextDisabled);
                ImU32 col_border   = ImGui::GetColorU32(ImGuiCol_Border);
                ImU32 col_sel_bg   = ImGui::GetColorU32(ImVec4(0.26f, 0.59f, 0.98f, 0.45f));
                ImU32 col_sel_text = col_text;

                int hovered_byte_idx = -1;

                ImGuiListClipper hex_clipper;
                hex_clipper.Begin(static_cast<int>(num_rows), line_h);
                while (hex_clipper.Step())
                {
                    for (int row = hex_clipper.DisplayStart; row < hex_clipper.DisplayEnd; ++row)
                    {
                        size_t offset = row * 16;
                        size_t chunk = std::min(static_cast<size_t>(16), display_len - offset);

                        ImVec2 cursor_screen_pos = ImGui::GetCursorScreenPos();
                        float row_x = cursor_screen_pos.x + offset_x;
                        float row_y = cursor_screen_pos.y;

                        // Per-row hit testing
                        if (is_window_hovered && mouse_pos.y >= row_y && mouse_pos.y < row_y + line_h)
                        {
                            float rel_x = mouse_pos.x - row_x;
                            if (rel_x >= header_w && rel_x < header_w + 16.0f * byte_w + char_w)
                            {
                                float hex_rel_x = rel_x - header_w;
                                if (hex_rel_x >= 8.0f * byte_w)
                                {
                                    hex_rel_x -= char_w;
                                }
                                if (hex_rel_x >= 0.0f)
                                {
                                    int byte_col = static_cast<int>(hex_rel_x / byte_w);
                                    if (byte_col >= 0 && byte_col < static_cast<int>(chunk))
                                    {
                                        hovered_byte_idx = static_cast<int>(offset + byte_col);
                                    }
                                }
                            }
                            else if (rel_x >= ascii_start_x && rel_x < ascii_start_x + 16.0f * char_w)
                            {
                                int byte_col = static_cast<int>((rel_x - ascii_start_x) / char_w);
                                if (byte_col >= 0 && byte_col < static_cast<int>(chunk))
                                {
                                    hovered_byte_idx = static_cast<int>(offset + byte_col);
                                }
                            }
                        }

                        // 1. Address column
                        char addr_buf[16];
                        snprintf(addr_buf, sizeof(addr_buf), "%04X:  ", static_cast<unsigned int>(offset));
                        draw_list->AddText(font, font_size, ImVec2(row_x, row_y), col_addr, addr_buf);

                        // 2. Hex bytes
                        for (size_t j = 0; j < 16; ++j)
                        {
                            float byte_pos_x = row_x + header_w + j * byte_w + (j >= 8 ? char_w : 0.0f);
                            if (j < chunk)
                            {
                                size_t byte_idx = offset + j;
                                uint8_t byte_val = data[byte_idx];
                                bool selected = s_hex_sel.is_selected(actual_index, byte_idx);

                                if (selected)
                                {
                                    ImVec2 b_min(byte_pos_x - char_w * 0.15f, row_y);
                                    ImVec2 b_max(byte_pos_x + char_w * 2.15f, row_y + text_h);
                                    draw_list->AddRectFilled(b_min, b_max, col_sel_bg, 2.0f * dpi_scale);
                                }

                                char byte_hex[4];
                                snprintf(byte_hex, sizeof(byte_hex), "%02X", byte_val);
                                ImU32 text_col = selected ? col_sel_text : (byte_val == 0 ? col_disabled : col_text);
                                draw_list->AddText(font, font_size, ImVec2(byte_pos_x, row_y), text_col, byte_hex);
                            }
                        }

                        // 3. ASCII column delimiters & characters
                        draw_list->AddText(font, font_size, ImVec2(row_x + ascii_start_x - char_w, row_y), col_border, "|");

                        for (size_t j = 0; j < chunk; ++j)
                        {
                            size_t byte_idx = offset + j;
                            uint8_t byte_val = data[byte_idx];
                            bool selected = s_hex_sel.is_selected(actual_index, byte_idx);
                            float ascii_pos_x = row_x + ascii_start_x + j * char_w;

                            if (selected)
                            {
                                ImVec2 a_min(ascii_pos_x, row_y);
                                ImVec2 a_max(ascii_pos_x + char_w, row_y + text_h);
                                draw_list->AddRectFilled(a_min, a_max, col_sel_bg, 2.0f * dpi_scale);
                            }

                            char c_str[2] = { (byte_val >= 32 && byte_val <= 126) ? static_cast<char>(byte_val) : '.', '\0' };
                            ImU32 ascii_col = selected ? col_sel_text : (byte_val == 0 ? col_disabled : (byte_val >= 32 && byte_val <= 126 ? col_text : col_disabled));
                            draw_list->AddText(font, font_size, ImVec2(ascii_pos_x, row_y), ascii_col, c_str);
                        }

                        draw_list->AddText(font, font_size, ImVec2(row_x + ascii_start_x + 16.0f * char_w, row_y), col_border, "|");

                        ImGui::SetCursorPosX(offset_x);
                        ImGui::Dummy(ImVec2(text_w, text_h));
                    }
                }

                if (ImGui::GetIO().Fonts->Fonts[1])
                {
                    ImGui::PopFont();
                }

                // Selection state machine
                if (is_clicked && is_window_hovered)
                {
                    if (hovered_byte_idx >= 0)
                    {
                        if (ImGui::GetIO().KeyShift && s_hex_sel.has_selection(actual_index))
                        {
                            s_hex_sel.end_idx = hovered_byte_idx;
                        }
                        else
                        {
                            s_hex_sel.buffer_id = actual_index;
                            s_hex_sel.start_idx = hovered_byte_idx;
                            s_hex_sel.end_idx   = hovered_byte_idx;
                            s_hex_sel.is_dragging = true;
                        }
                    }
                    else
                    {
                        s_hex_sel.buffer_id = -1;
                        s_hex_sel.start_idx = -1;
                        s_hex_sel.end_idx   = -1;
                        s_hex_sel.is_dragging = false;
                    }
                }
                else if (is_mouse_down && s_hex_sel.is_dragging && s_hex_sel.buffer_id == actual_index)
                {
                    if (hovered_byte_idx >= 0)
                    {
                        s_hex_sel.end_idx = hovered_byte_idx;
                    }
                }
                else if (!is_mouse_down)
                {
                    s_hex_sel.is_dragging = false;
                }

                // Context menu & tooltip setup
                struct context_range
                {
                    size_t offset = 0;
                    size_t len    = 0;
                };
                static context_range s_ctx;

                if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && is_window_hovered)
                {
                    if (s_hex_sel.has_selection(actual_index))
                    {
                        size_t s_min, s_max;
                        s_hex_sel.get_range(s_min, s_max);
                        s_ctx.offset = s_min;
                        s_ctx.len    = s_max - s_min + 1;
                    }
                    else if (hovered_byte_idx >= 0)
                    {
                        s_ctx.offset = static_cast<size_t>(hovered_byte_idx);
                        s_ctx.len    = 1;
                    }
                    else
                    {
                        s_ctx.offset = 0;
                        s_ctx.len    = display_len;
                    }
                    ImGui::OpenPopup("##hex_context_menu");
                }

                bool is_popup_open = ImGui::IsPopupOpen("##hex_context_menu");

                auto push_mono = []()
                {
                    if (ImGui::GetIO().Fonts->Fonts[1])
                    {
                        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
                    }
                };
                auto pop_mono = []()
                {
                    if (ImGui::GetIO().Fonts->Fonts[1])
                    {
                        ImGui::PopFont();
                    }
                };

                // Context menu popup
                if (ImGui::BeginPopup("##hex_context_menu"))
                {
                    size_t c_offset = s_ctx.offset;
                    size_t c_len    = s_ctx.len;
                    if (c_offset + c_len > display_len) c_len = display_len > c_offset ? display_len - c_offset : 0;

                    if (c_len == 1)
                    {
                        uint8_t b_val = data[c_offset];
                        ImGui::TextDisabled("Byte at 0x%04X (%zu)", static_cast<unsigned int>(c_offset), c_offset);
                        ImGui::Separator();

                        char h_str[8]; snprintf(h_str, sizeof(h_str), "%02X", b_val);
                        if (ImGui::MenuItem("Copy Hex", h_str)) ImGui::SetClipboardText(h_str);

                        char d_str[16]; snprintf(d_str, sizeof(d_str), "%u", b_val);
                        if (ImGui::MenuItem("Copy Decimal (UInt8)", d_str)) ImGui::SetClipboardText(d_str);

                        char s_str[16]; snprintf(s_str, sizeof(s_str), "%d", static_cast<int8_t>(b_val));
                        if (ImGui::MenuItem("Copy Signed (Int8)", s_str)) ImGui::SetClipboardText(s_str);

                        char bin_str[16];
                        for (int b = 7; b >= 0; --b) bin_str[7 - b] = ((b_val >> b) & 1) ? '1' : '0';
                        bin_str[8] = '\0';
                        char bin_disp[20]; snprintf(bin_disp, sizeof(bin_disp), "0b%s", bin_str);
                        if (ImGui::MenuItem("Copy Binary", bin_disp)) ImGui::SetClipboardText(bin_disp);

                        char a_str[4]; snprintf(a_str, sizeof(a_str), "%c", (b_val >= 32 && b_val <= 126) ? static_cast<char>(b_val) : '.');
                        if (ImGui::MenuItem("Copy ASCII", a_str)) ImGui::SetClipboardText(a_str);
                    }
                    else if (c_len > 1)
                    {
                        ImGui::TextDisabled("Selection: 0x%04X - 0x%04X (%zu bytes)", static_cast<unsigned int>(c_offset), static_cast<unsigned int>(c_offset + c_len - 1), c_len);
                        ImGui::Separator();

                        std::string h_copy;
                        for (size_t k = 0; k < c_len; ++k)
                        {
                            char h[4]; snprintf(h, sizeof(h), "%02X ", data[c_offset + k]); h_copy += h;
                        }
                        if (!h_copy.empty()) h_copy.pop_back();

                        std::string h_preview;
                        size_t max_prev_h = std::min(c_len, size_t(8));
                        for (size_t k = 0; k < max_prev_h; ++k)
                        {
                            char h[4]; snprintf(h, sizeof(h), "%02X ", data[c_offset + k]); h_preview += h;
                        }
                        if (c_len > 8) h_preview += "...";
                        else if (!h_preview.empty()) h_preview.pop_back();
                        if (ImGui::MenuItem("Copy Hex", h_preview.c_str())) ImGui::SetClipboardText(h_copy.c_str());

                        std::string a_copy;
                        for (size_t k = 0; k < c_len; ++k)
                        {
                            char c = data[c_offset + k]; a_copy += (c >= 32 && c <= 126) ? c : '.';
                        }
                        std::string a_preview = "\"" + (c_len > 16 ? a_copy.substr(0, 16) + "..." : a_copy) + "\"";
                        if (ImGui::MenuItem("Copy ASCII", a_preview.c_str())) ImGui::SetClipboardText(a_copy.c_str());

                        std::string b_copy;
                        for (size_t k = 0; k < c_len; ++k)
                        {
                            uint8_t bv = data[c_offset + k];
                            for (int b = 7; b >= 0; --b) b_copy += ((bv >> b) & 1) ? '1' : '0';
                            b_copy += ' ';
                        }
                        if (!b_copy.empty()) b_copy.pop_back();

                        std::string b_preview;
                        size_t max_prev_b = std::min(c_len, size_t(4));
                        for (size_t k = 0; k < max_prev_b; ++k)
                        {
                            uint8_t bv = data[c_offset + k];
                            for (int b = 7; b >= 0; --b) b_preview += ((bv >> b) & 1) ? '1' : '0';
                            b_preview += ' ';
                        }
                        if (c_len > 4) b_preview += "...";
                        else if (!b_preview.empty()) b_preview.pop_back();
                        if (ImGui::MenuItem("Copy Binary", b_preview.c_str())) ImGui::SetClipboardText(b_copy.c_str());

                        if (c_len == 2)
                        {
                            uint16_t v_le; std::memcpy(&v_le, data + c_offset, 2);
                            uint16_t v_be = (static_cast<uint16_t>(data[c_offset]) << 8) | data[c_offset + 1];

                            ImGui::Separator();
                            std::string s_le = std::to_string(static_cast<int16_t>(v_le));
                            std::string s_be = std::to_string(static_cast<int16_t>(v_be));
                            if (ImGui::MenuItem("Copy Int16 (LE)", s_le.c_str())) ImGui::SetClipboardText(s_le.c_str());
                            if (ImGui::MenuItem("Copy Int16 (BE)", s_be.c_str())) ImGui::SetClipboardText(s_be.c_str());

                            std::string u_le = std::to_string(v_le);
                            std::string u_be = std::to_string(v_be);
                            if (ImGui::MenuItem("Copy UInt16 (LE)", u_le.c_str())) ImGui::SetClipboardText(u_le.c_str());
                            if (ImGui::MenuItem("Copy UInt16 (BE)", u_be.c_str())) ImGui::SetClipboardText(u_be.c_str());
                        }
                        else if (c_len == 4)
                        {
                            uint32_t v_le; float vf_le;
                            std::memcpy(&v_le, data + c_offset, 4);
                            std::memcpy(&vf_le, data + c_offset, 4);

                            uint32_t v_be = (static_cast<uint32_t>(data[c_offset]) << 24) |
                                            (static_cast<uint32_t>(data[c_offset + 1]) << 16) |
                                            (static_cast<uint32_t>(data[c_offset + 2]) << 8) |
                                            static_cast<uint32_t>(data[c_offset + 3]);
                            uint32_t swapped_be = adam::swap_4(reinterpret_cast<const uint8_t*>(&v_le));
                            float vf_be; std::memcpy(&vf_be, &swapped_be, 4);

                            ImGui::Separator();
                            std::string s_le = std::to_string(static_cast<int32_t>(v_le));
                            std::string s_be = std::to_string(static_cast<int32_t>(v_be));
                            if (ImGui::MenuItem("Copy Int32 (LE)", s_le.c_str())) ImGui::SetClipboardText(s_le.c_str());
                            if (ImGui::MenuItem("Copy Int32 (BE)", s_be.c_str())) ImGui::SetClipboardText(s_be.c_str());

                            std::string u_le = std::to_string(v_le);
                            std::string u_be = std::to_string(v_be);
                            if (ImGui::MenuItem("Copy UInt32 (LE)", u_le.c_str())) ImGui::SetClipboardText(u_le.c_str());
                            if (ImGui::MenuItem("Copy UInt32 (BE)", u_be.c_str())) ImGui::SetClipboardText(u_be.c_str());

                            char f_le_buf[32]; snprintf(f_le_buf, sizeof(f_le_buf), "%g", vf_le);
                            char f_be_buf[32]; snprintf(f_be_buf, sizeof(f_be_buf), "%g", vf_be);
                            if (ImGui::MenuItem("Copy Float (LE)", f_le_buf)) ImGui::SetClipboardText(f_le_buf);
                            if (ImGui::MenuItem("Copy Float (BE)", f_be_buf)) ImGui::SetClipboardText(f_be_buf);
                        }
                        else if (c_len >= 8)
                        {
                            uint64_t v_le; double vd_le;
                            std::memcpy(&v_le, data + c_offset, 8);
                            std::memcpy(&vd_le, data + c_offset, 8);

                            uint64_t v_be = 0;
                            for (int k = 0; k < 8; ++k) v_be = (v_be << 8) | data[c_offset + k];
                            uint64_t swapped_be = adam::swap_8(reinterpret_cast<const uint8_t*>(&v_le));
                            double vd_be; std::memcpy(&vd_be, &swapped_be, 8);

                            ImGui::Separator();
                            std::string s_le = std::to_string(static_cast<long long>(v_le));
                            std::string s_be = std::to_string(static_cast<long long>(v_be));
                            if (ImGui::MenuItem("Copy Int64 (LE)", s_le.c_str())) ImGui::SetClipboardText(s_le.c_str());
                            if (ImGui::MenuItem("Copy Int64 (BE)", s_be.c_str())) ImGui::SetClipboardText(s_be.c_str());

                            std::string u_le = std::to_string(v_le);
                            std::string u_be = std::to_string(v_be);
                            if (ImGui::MenuItem("Copy UInt64 (LE)", u_le.c_str())) ImGui::SetClipboardText(u_le.c_str());
                            if (ImGui::MenuItem("Copy UInt64 (BE)", u_be.c_str())) ImGui::SetClipboardText(u_be.c_str());

                            char d_le_buf[32]; snprintf(d_le_buf, sizeof(d_le_buf), "%g", vd_le);
                            char d_be_buf[32]; snprintf(d_be_buf, sizeof(d_be_buf), "%g", vd_be);
                            if (ImGui::MenuItem("Copy Double (LE)", d_le_buf)) ImGui::SetClipboardText(d_le_buf);
                            if (ImGui::MenuItem("Copy Double (BE)", d_be_buf)) ImGui::SetClipboardText(d_be_buf);
                        }

                        ImGui::Separator();
                        if (ImGui::MenuItem("Copy Formatted Hex Dump"))
                        {
                            std::string s_dump;
                            size_t num_copy_rows = (c_len + 15) / 16;
                            s_dump.reserve(num_copy_rows * 80);
                            for (size_t offset = 0; offset < c_len; offset += 16)
                            {
                                char line_buf[256];
                                int printed = snprintf(line_buf, sizeof(line_buf), "%04X:  ", static_cast<unsigned int>(c_offset + offset));
                                size_t chunk = std::min(static_cast<size_t>(16), c_len - offset);
                                for (size_t j = 0; j < 16; ++j)
                                {
                                    if (j == 8) line_buf[printed++] = ' ';
                                    if (j < chunk) printed += snprintf(line_buf + printed, sizeof(line_buf) - printed, "%02X ", data[c_offset + offset + j]);
                                    else printed += snprintf(line_buf + printed, sizeof(line_buf) - printed, "   ");
                                }
                                line_buf[printed++] = ' ';
                                line_buf[printed++] = ' ';
                                line_buf[printed++] = '|';
                                for (size_t j = 0; j < chunk; ++j)
                                {
                                    char c = data[c_offset + offset + j];
                                    line_buf[printed++] = (c >= 32 && c <= 126) ? c : '.';
                                }
                                line_buf[printed++] = '|';
                                line_buf[printed++] = '\n';
                                line_buf[printed] = '\0';
                                s_dump += line_buf;
                            }
                            ImGui::SetClipboardText(s_dump.c_str());
                        }
                    }
                    ImGui::EndPopup();
                }
            }
            ImGui::EndChild();
            ImGui::PopStyleVar();
        }
        ImGui::EndChild();
        ImGui::PopID();
        ImGui::PopStyleVar(2);
    }

    static void calculate_custom_clipper
    (
        int num_total_rows,
        float row_height,
        const std::set<size_t>& expanded_nodes,
        float scroll_y,
        float window_h,
        int& out_display_start,
        int& out_display_end,
        float& out_top_dummy_h,
        float& out_bottom_dummy_h,
        const std::function<float(size_t)>& get_expanded_height
    )
    {
        out_top_dummy_h = 0.0f;
        out_bottom_dummy_h = 0.0f;
        out_display_start = -1;
        out_display_end = -1;

        if (num_total_rows <= 0)
        {
            out_display_start = 0;
            out_display_end = 0;
            return;
        }

        float current_y = 0.0f;
        int current_idx = 0;

        for (size_t exp_idx : expanded_nodes)
        {
            if (static_cast<int>(exp_idx) >= num_total_rows)
            {
                continue;
            }
            int unexpanded_count = static_cast<int>(exp_idx) - current_idx;
            if (unexpanded_count > 0)
            {
                float unexpanded_h = unexpanded_count * row_height;
                if (out_display_start == -1 && current_y + unexpanded_h > scroll_y)
                {
                    float overshoot = scroll_y - current_y;
                    int rows_in = static_cast<int>(overshoot / row_height);
                    if (rows_in < 0) rows_in = 0;
                    out_display_start = current_idx + rows_in;
                    out_top_dummy_h = current_y + rows_in * row_height;
                }
                if (out_display_end == -1 && current_y + unexpanded_h >= scroll_y + window_h)
                {
                    float overshoot = (scroll_y + window_h) - current_y;
                    int rows_in = static_cast<int>(overshoot / row_height) + 1;
                    if (rows_in > unexpanded_count) rows_in = unexpanded_count;
                    out_display_end = current_idx + rows_in;
                }
                current_y += unexpanded_h;
                current_idx = static_cast<int>(exp_idx);
            }

            float exp_h = get_expanded_height(exp_idx);
            float item_h = row_height + exp_h;

            if (out_display_start == -1 && current_y + item_h > scroll_y)
            {
                out_display_start = current_idx;
                out_top_dummy_h = current_y;
            }
            if (out_display_end == -1 && current_y + item_h >= scroll_y + window_h)
            {
                out_display_end = current_idx + 1;
            }

            current_y += item_h;
            current_idx++;
        }

        int unexpanded_count = num_total_rows - current_idx;
        if (unexpanded_count > 0)
        {
            float unexpanded_h = unexpanded_count * row_height;
            if (out_display_start == -1 && current_y + unexpanded_h > scroll_y)
            {
                float overshoot = scroll_y - current_y;
                int rows_in = static_cast<int>(overshoot / row_height);
                if (rows_in < 0) rows_in = 0;
                out_display_start = current_idx + rows_in;
                out_top_dummy_h = current_y + rows_in * row_height;
            }
            if (out_display_end == -1 && current_y + unexpanded_h >= scroll_y + window_h)
            {
                float overshoot = (scroll_y + window_h) - current_y;
                int rows_in = static_cast<int>(overshoot / row_height) + 1;
                if (rows_in > unexpanded_count) rows_in = unexpanded_count;
                out_display_end = current_idx + rows_in;
            }
            current_y += unexpanded_h;
        }

        if (out_display_start == -1) out_display_start = num_total_rows;
        if (out_display_end == -1) out_display_end = num_total_rows;

        out_display_start -= 3;
        if (out_display_start < 0) out_display_start = 0;
        out_display_end += 3;
        if (out_display_end > num_total_rows) out_display_end = num_total_rows;

        out_top_dummy_h = out_display_start * row_height;
        for (auto it = expanded_nodes.begin(); it != expanded_nodes.end(); ++it)
        {
            if (static_cast<int>(*it) < out_display_start) out_top_dummy_h += get_expanded_height(*it);
            else break;
        }

        out_bottom_dummy_h = (num_total_rows - out_display_end) * row_height;
        for (auto it = expanded_nodes.rbegin(); it != expanded_nodes.rend(); ++it)
        {
            if (static_cast<int>(*it) >= out_display_end && static_cast<int>(*it) < num_total_rows) out_bottom_dummy_h += get_expanded_height(*it);
            else if (static_cast<int>(*it) < out_display_end) break;
        }
    }

    static void draw_analyzer_expanded_region
    (
        inspection_port_data& port_data,
        size_t b_idx,
        size_t r_idx,
        size_t i,
        float inner_avail_w,
        float dpi_scale,
        adam::language lang
    )
    {
        auto& row_obj = port_data.parsed_data[b_idx][r_idx];

        if (!row_obj.expansions_fetched)
        {
            if (port_data.analyzer_ptr)
            {
                const uint8_t* ref_data = nullptr;
                if (port_data.buffers[b_idx].ref_size > 0)
                {
                    ref_data = port_data.data_pool.data() + port_data.buffers[b_idx].ref_offset;
                }
                
                port_data.analyzer_ptr->analyze_expanded(port_data.data_pool.data() + port_data.buffers[b_idx].offset, port_data.buffers[b_idx].size, ref_data, port_data.buffers[b_idx].ref_size, r_idx, row_obj.expansions);
            }
            row_obj.expansions_fetched = true;
        }

        ImGui::PushID(static_cast<int>(i));
        float sub_table_w = inner_avail_w;

        if (ImGui::BeginChild("##ExpandedRegion", ImVec2(sub_table_w, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
        {
            if (ImGui::BeginChild("##HexDumpRegion", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
            {
                if (ImGui::TreeNodeEx("Hex Dump"))
                {
                    ImGui::Unindent();
                    const uint8_t* hex_data = port_data.data_pool.data() + port_data.buffers[b_idx].offset;
                    size_t hex_size = port_data.buffers[b_idx].size;
                    if (port_data.buffers[b_idx].ref_size > 0)
                    {
                        hex_data = port_data.data_pool.data() + port_data.buffers[b_idx].ref_offset;
                        hex_size = port_data.buffers[b_idx].ref_size;
                    }

                    draw_inspector_hex_dump
                    (
                        hex_data,
                        hex_size,
                        static_cast<int>(i),
                        dpi_scale,
                        lang,
                        ImGui::GetContentRegionAvail().x
                    );
                        
                    ImGui::TreePop();
                }
            }
            ImGui::EndChild();

            if (!row_obj.expansions.empty())
            {
                for (size_t exp_idx = 0; exp_idx < row_obj.expansions.size(); ++exp_idx)
                {
                    auto exp_table_w = ImGui::GetContentRegionAvail().x;
                    const auto& exp = row_obj.expansions[exp_idx];
                    ImGui::PushID(static_cast<int>(exp_idx));
                    if (exp.data_type == adam::analyzer::expanded_data::type_text)
                    {
                        if (ImGui::GetIO().Fonts->Fonts[1]) ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
                        ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + exp_table_w);
                        ImGui::TextWrapped("%s", exp.text_content.c_str());
                        ImGui::PopTextWrapPos();
                        if (ImGui::GetIO().Fonts->Fonts[1]) ImGui::PopFont();
                    }
                    else if (exp.data_type == adam::analyzer::expanded_data::type_table)
                    {
                        const auto& ext_cols            = port_data.analyzer_ptr->get_expandable_columns();
                        const auto& ext_cols_fonts      = port_data.analyzer_ptr->get_expandable_columns_fonts();
                        const auto& ext_cols_weights    = port_data.analyzer_ptr->get_expandable_columns_weights();

                        if (ImGui::BeginTable("##sub_table", static_cast<int>(ext_cols.size()),
                            ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter |
                            ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable |
                            ImGuiTableFlags_SizingStretchSame,
                            ImVec2(exp_table_w, 0), 0.0f))
                        {
                            for (size_t c = 0; c < ext_cols.size(); c++) 
                            {
                                ImGui::TableSetupColumn(ext_cols[c].c_str(), ImGuiTableColumnFlags_WidthStretch, ext_cols_weights[c]);
                            }

                            ImGui::TableHeadersRow();
                            for (const auto& row_v : exp.table_rows)
                            {
                                ImGui::TableNextRow();
                                for (size_t c = 0; c < row_v.columns.size() && c < ext_cols.size(); ++c)
                                {
                                    ImGui::TableSetColumnIndex(static_cast<int>(c));
                                    
                                    bool usemonofont = (ext_cols_fonts[c] == adam::analyzer::column_font_mono) && ImGui::GetIO().Fonts->Fonts[1];

                                    if (usemonofont) ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
                                    ImGui::TextUnformatted(row_v.columns[c].c_str());
                                    if (usemonofont) ImGui::PopFont();
                                }
                            }
                            ImGui::EndTable();
                        }
                    }
                    ImGui::PopID();
                }
            }
            else
            {
                ImGui::TextDisabled("No expanded data available.");
            }
        }
        ImGui::EndChild();
        ImGui::PopID();
    }

    static void draw_inspector_frames_table
    (
        const char* name,
        adam::string_hash hash,
        std::map<adam::string_hash, adam::gui::inspection_port_data>& data_map,
        float initial_height,
        float dpi_scale,
        adam::language lang,
        uint64_t id_modifier,
        bool* is_detached = nullptr
    )
    {
        std::lock_guard<std::mutex> buffer_lock(adam::gui::g_inspection_data.mtx);
        auto& port_data = data_map[hash];
        const auto& buffers = port_data.buffers;
        const auto& data_pool = port_data.data_pool;
        auto& expanded_nodes = port_data.expanded_nodes;

        uint64_t unique_id = hash ^ id_modifier;
        if (g_expanded_inspector_heights.find(unique_id) == g_expanded_inspector_heights.end())
        {
            g_expanded_inspector_heights[unique_id] = initial_height;
        }
        float& current_height = g_expanded_inspector_heights[unique_id];

        ImGui::PushID(reinterpret_cast<const void*>(static_cast<intptr_t>(unique_id)));

        float outer_child_h = (is_detached && *is_detached) ? 0.0f : current_height;
        ImGui::BeginChild("##outer_child", ImVec2(0, outer_child_h), true);

        bool has_analyzer = !port_data.analyzer_columns.empty();

        if (has_analyzer)
        {
            bool is_expandable = port_data.analyzer_ptr && port_data.analyzer_ptr->is_row_expandable();
            int num_cols = static_cast<int>(port_data.analyzer_columns.size()) + (is_expandable ? 1 : 0);

            auto setup_analyzer_columns = [&]()
            {
                float expandable_colum_width = 0.0f;
                if (is_expandable) 
                {
                    expandable_colum_width = ImGui::GetTextLineHeight() + ImGui::GetStyle().FramePadding.x * 2.0f;
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoClip, expandable_colum_width);
                }

                for (size_t i = 0; i < port_data.analyzer_columns.size(); i++)
                {
                    ImGui::TableSetupColumn(port_data.analyzer_columns[i].c_str(), ImGuiTableColumnFlags_WidthStretch, port_data.analyzer_column_weights[i]);
                }
            };

            float inner_scroll_h = -(ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y);
            ImGui::BeginChild("##inner_child", ImVec2(0, inner_scroll_h), false, 0);
            
            if (port_data.force_scroll_top)
            {
                ImGui::SetScrollY(0.0f);
                port_data.force_scroll_top = false;
            }
            
            float inner_avail_w = ImGui::GetContentRegionAvail().x;

            bool active_user_scrolling = ImGui::GetIO().MouseWheel != 0.0f || 
                                         (ImGui::IsMouseDown(ImGuiMouseButton_Left) && 
                                          (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) || 
                                           ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)));

            if (active_user_scrolling && ImGui::GetScrollMaxY() > 0.0f && ImGui::GetScrollY() < ImGui::GetScrollMaxY() - 5.0f * dpi_scale)
            {
                port_data.was_at_bottom = false;
            }
            bool auto_scroll = port_data.was_at_bottom;

            ImGuiTableFlags common_flags = ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable;

            ImGui::GetWindowDrawList()->ChannelsSplit(3);

            // 1. Draw Floating Header on Foreground (Channel 2)
            ImGui::GetWindowDrawList()->ChannelsSetCurrent(2);
            float header_scroll_y = ImGui::GetScrollY();
            float header_start_y = ImGui::GetCursorPosY();
            ImGui::SetCursorPosY(header_start_y + header_scroll_y);

            ImVec2 header_pos = ImGui::GetCursorScreenPos();

            bool header_table_begun = ImGui::BeginTable("InspectorAnalyzerMain", num_cols, common_flags);
            if (header_table_begun)
            {
                setup_analyzer_columns();
                ImGui::TableHeadersRow();
                ImGui::EndTable();
            }
            float actual_header_h = ImGui::GetCursorPosY() - (header_start_y + header_scroll_y) - ImGui::GetStyle().ItemSpacing.y;

            // 2. Draw Header Background on Middleground (Channel 1)
            ImGui::GetWindowDrawList()->ChannelsSetCurrent(1);
            ImGui::GetWindowDrawList()->AddRectFilled(header_pos, ImVec2(header_pos.x + inner_avail_w, header_pos.y + actual_header_h), ImGui::GetColorU32(ImGuiCol_WindowBg));

            // 3. Draw Body Tables on Background (Channel 0)
            ImGui::GetWindowDrawList()->ChannelsSetCurrent(0);
            ImGui::SetCursorPosY(header_start_y + actual_header_h);

            ImVec2 clip_min(-FLT_MAX, header_pos.y + actual_header_h);
            ImVec2 clip_max(FLT_MAX, FLT_MAX);
            ImGui::PushClipRect(clip_min, clip_max, true);

            bool table_open = ImGui::BeginTable("InspectorAnalyzerMain", num_cols, common_flags);
            if (table_open) setup_analyzer_columns();

            const auto& flat_rows = port_data.parsed_flat_rows;
            std::erase_if(port_data.expanded_nodes, [&](size_t idx) { return idx >= flat_rows.size(); });

            float row_height = ImGui::GetTextLineHeight() + ImGui::GetStyle().CellPadding.y * 2.0f;
            static std::map<std::pair<size_t, size_t>, float> expanded_heights;
            float scroll_y = ImGui::GetScrollY();
            float window_h = ImGui::GetWindowHeight();
            float top_dummy_h = 0.0f, bottom_dummy_h = 0.0f;
            int display_start = 0, display_end = 0;

            calculate_custom_clipper
            (
                static_cast<int>(flat_rows.size()), row_height, port_data.expanded_nodes, scroll_y, window_h,
                display_start, display_end, top_dummy_h, bottom_dummy_h,
                [&](size_t idx) -> float
                {
                    auto it = expanded_heights.find(flat_rows[idx]);
                    return (it != expanded_heights.end()) ? it->second : 100.0f;
                }
            );
            
            if (top_dummy_h > 0.0f && table_open)
            {
                ImGui::TableNextRow(ImGuiTableRowFlags_None, top_dummy_h);
            }
            
            float missed_height = 0.0f;
            for (int i = display_start; i < display_end; ++i)
            {
                size_t b_idx = flat_rows[i].first;
                size_t r_idx = flat_rows[i].second;

                if (!table_open)
                {
                    missed_height += row_height;
                    if (port_data.expanded_nodes.count(i))
                    {
                        auto it = expanded_heights.find(flat_rows[i]);
                        missed_height += (it != expanded_heights.end()) ? it->second : 100.0f;
                    }
                    continue;
                }

                if (table_open)
                {
                    ImGui::TableNextRow();
                    int current_col = 0;

                    bool is_open = false;
                    if (is_expandable)
                    {
                        ImGui::TableSetColumnIndex(0);
                        ImGui::PushID(i);
                        ImGui::GetWindowDrawList()->ChannelsSetCurrent(0);
                        is_open = ImGui::TreeNodeEx("##row_node", ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_OpenOnArrow | (port_data.expanded_nodes.count(i) ? ImGuiTreeNodeFlags_DefaultOpen : 0));
                        if (is_open)
                        {
                            if (!port_data.expanded_nodes.count(i)) port_data.expanded_nodes.insert(i);
                            ImGui::TreePop();
                        }
                        else
                        {
                            port_data.expanded_nodes.erase(i);
                        }
                        ImGui::PopID();
                        current_col = 1;
                    }

                    if (r_idx == SIZE_MAX)
                    {
                        ImGui::TableSetColumnIndex(current_col);
                        char unparsed_buf[128];
                        snprintf(unparsed_buf, sizeof(unparsed_buf), "%s (Size: %zu)", get_gui_string(gui_string_id::lbl_no_data, lang), static_cast<size_t>(port_data.buffers[b_idx].size));
                        ImGui::TextDisabled("%s", unparsed_buf);
                    }
                    else
                    {
                        const auto& row = port_data.parsed_data[b_idx][r_idx];
                        size_t text_idx = 0;
                        for (size_t c = 0; c < port_data.analyzer_columns.size(); ++c)
                        {
                            ImGui::TableSetColumnIndex(current_col + static_cast<int>(c));
                            adam::analyzer::column_type c_type = adam::analyzer::column_text;
                            adam::analyzer::column_font c_font = adam::analyzer::column_font_normal;

                            if (c < port_data.analyzer_column_types.size())
                                c_type = port_data.analyzer_column_types[c];
                            if (c < port_data.analyzer_column_fonts.size())
                                c_font = port_data.analyzer_column_fonts[c];

                            if (c_font == adam::analyzer::column_font_mono && ImGui::GetIO().Fonts->Fonts[1])
                                ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);

                            if (c_type == adam::analyzer::column_frame_id)
                            {
                                ImGui::Text("%zu", b_idx);
                            }
                            else if (c_type == adam::analyzer::column_timestamp)
                            {
                                ImGui::TextUnformatted(adam::get_log_time_string(port_data.buffers[b_idx].timestamp).c_str());
                            }
                            else
                            {
                                if (text_idx < row.columns.size())
                                {
                                    ImGui::TextUnformatted(row.columns[text_idx].c_str());
                                }

                                text_idx++;
                            }
                            
                            if (c_font == adam::analyzer::column_font_mono && ImGui::GetIO().Fonts->Fonts[1])
                                ImGui::PopFont();
                        }
                    }
                }

                if (is_expandable && port_data.expanded_nodes.count(i) && r_idx != SIZE_MAX)
                {
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

                    if (table_open)
                    {
                        ImGui::EndTable();
                        table_open = false;
                    }

                    float start_y = ImGui::GetCursorPosY();
                    ImGui::PopStyleVar();

                    draw_analyzer_expanded_region(port_data, b_idx, r_idx, i, inner_avail_w, dpi_scale, lang);

                    float end_y = ImGui::GetCursorPosY();
                    expanded_heights[flat_rows[i]] = end_y - start_y;

                    table_open = ImGui::BeginTable("InspectorAnalyzerMain", num_cols, common_flags);
                    if (table_open) setup_analyzer_columns();
                }
            }

            if (bottom_dummy_h > 0.0f && table_open)
            {
                ImGui::TableNextRow(ImGuiTableRowFlags_None, bottom_dummy_h);
            }

            if (table_open) ImGui::EndTable();

            float total_dummy_outside = missed_height;
            if (!table_open && bottom_dummy_h > 0.0f)
            {
                total_dummy_outside += bottom_dummy_h;
            }
            
            if (total_dummy_outside > 0.0f)
            {
                ImGui::Dummy(ImVec2(0, total_dummy_outside));
            }

            if (auto_scroll && ImGui::GetScrollMaxY() > 0.0f)
            {
                ImGui::SetScrollY(ImGui::GetScrollMaxY());
            }

            if (!auto_scroll)
            {
                port_data.was_at_bottom = (ImGui::GetScrollMaxY() == 0.0f || ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 5.0f * dpi_scale);
            }

            ImGui::PopClipRect();
            ImGui::GetWindowDrawList()->ChannelsMerge();
            ImGui::EndChild();
        }
        else
        {
            auto setup_inner_columns = [&]()
            {
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoClip,
                    ImGui::GetTextLineHeight() + ImGui::GetStyle().FramePadding.x * 2.0f);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_index, lang), ImGuiTableColumnFlags_WidthFixed, 55.0f * dpi_scale);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_timestamp, lang), ImGuiTableColumnFlags_WidthFixed, 120.0f * dpi_scale);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_size, lang), ImGuiTableColumnFlags_WidthFixed, 75.0f * dpi_scale);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_preview_hex, lang), ImGuiTableColumnFlags_WidthStretch, 0.75f);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::col_preview_ascii, lang), ImGuiTableColumnFlags_WidthStretch, 0.25f);
            };

            float inner_scroll_h = -(ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y);
            ImGui::BeginChild("##inner_child", ImVec2(0, inner_scroll_h), false, 0);
            
            if (port_data.force_scroll_top)
            {
                ImGui::SetScrollY(0.0f);
                port_data.force_scroll_top = false;
            }
            
            float inner_avail_w = ImGui::GetContentRegionAvail().x;

            bool active_user_scrolling = ImGui::GetIO().MouseWheel != 0.0f || 
                                         (ImGui::IsMouseDown(ImGuiMouseButton_Left) && 
                                          (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) || 
                                           ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)));

            if (active_user_scrolling && ImGui::GetScrollMaxY() > 0.0f && ImGui::GetScrollY() < ImGui::GetScrollMaxY() - 5.0f * dpi_scale)
            {
                port_data.was_at_bottom = false;
            }
            bool auto_scroll = port_data.was_at_bottom;

            ImGuiTableFlags common_flags = ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable;

            ImGui::GetWindowDrawList()->ChannelsSplit(3);

            // 1. Draw Floating Header on Foreground (Channel 2)
            ImGui::GetWindowDrawList()->ChannelsSetCurrent(2);
            float header_scroll_y = ImGui::GetScrollY();
            float header_start_y = ImGui::GetCursorPosY();
            ImGui::SetCursorPosY(header_start_y + header_scroll_y);

            ImVec2 header_pos = ImGui::GetCursorScreenPos();

            bool header_table_begun = ImGui::BeginTable("InspectorTableMain", 6, common_flags);
            if (header_table_begun)
            {
                setup_inner_columns();
                ImGui::TableHeadersRow();
                ImGui::EndTable();
            }
            float actual_header_h = ImGui::GetCursorPosY() - (header_start_y + header_scroll_y) - ImGui::GetStyle().ItemSpacing.y;

            // 2. Draw Header Background on Middleground (Channel 1)
            ImGui::GetWindowDrawList()->ChannelsSetCurrent(1);
            ImGui::GetWindowDrawList()->AddRectFilled(header_pos, ImVec2(header_pos.x + inner_avail_w, header_pos.y + actual_header_h), ImGui::GetColorU32(ImGuiCol_WindowBg));

            // 3. Draw Body Tables on Background (Channel 0)
            ImGui::GetWindowDrawList()->ChannelsSetCurrent(0);
            ImGui::SetCursorPosY(header_start_y + actual_header_h);

            ImVec2 clip_min(-FLT_MAX, header_pos.y + actual_header_h);
            ImVec2 clip_max(FLT_MAX, FLT_MAX);
            ImGui::PushClipRect(clip_min, clip_max, true);

            bool table_open = ImGui::BeginTable("InspectorTableMain", 6, common_flags);
            if (table_open) setup_inner_columns();

            std::erase_if(expanded_nodes, [&](size_t idx) { return idx >= buffers.size(); });

            float row_height = ImGui::GetTextLineHeight() + ImGui::GetStyle().CellPadding.y * 2.0f;
            static std::map<uint64_t, float> normal_expanded_heights;
            float scroll_y = ImGui::GetScrollY();
            float window_h = ImGui::GetWindowHeight();
            float top_dummy_h = 0.0f, bottom_dummy_h = 0.0f;
            int display_start = 0, display_end = 0;

            calculate_custom_clipper(static_cast<int>(buffers.size()), row_height, expanded_nodes, scroll_y, window_h, display_start, display_end, top_dummy_h, bottom_dummy_h, [&](size_t idx) -> float
            {
                uint64_t dump_id = unique_id ^ (idx * 0x9e3779b9);
                auto it = normal_expanded_heights.find(dump_id);
                return (it != normal_expanded_heights.end()) ? it->second : 100.0f;
            });

            if (top_dummy_h > 0.0f && table_open)
            {
                ImGui::TableNextRow(ImGuiTableRowFlags_None, top_dummy_h);
            }

            float missed_height = 0.0f;
            for (int row_idx = display_start; row_idx < display_end; ++row_idx)
            {
                const auto& ib = buffers[row_idx];

                if (!table_open)
                {
                    missed_height += row_height;
                    if (expanded_nodes.count(row_idx))
                    {
                        uint64_t dump_id = unique_id ^ (row_idx * 0x9e3779b9);
                        auto it = normal_expanded_heights.find(dump_id);
                        missed_height += (it != normal_expanded_heights.end()) ? it->second : 100.0f;
                    }
                    continue;
                }

                if (table_open)
                {
                    draw_inspector_table_row(ib, row_idx, data_pool, (std::set<size_t>&)expanded_nodes);
                }

                if (expanded_nodes.count(row_idx))
                {
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
                    if (table_open)
                    {
                        ImGui::EndTable();
                        table_open = false;
                    }

                    float start_y = ImGui::GetCursorPosY();

                    ImGui::PushID(row_idx);
                    
                    const uint8_t* hex_data = data_pool.data() + ib.offset;
                    size_t hex_size = ib.size;
                    if (ib.ref_size > 0)
                    {
                        hex_data = data_pool.data() + ib.ref_offset;
                        hex_size = ib.ref_size;
                    }
                    ImGui::PopStyleVar();

                    draw_inspector_hex_dump(hex_data, hex_size, row_idx, dpi_scale, lang, inner_avail_w);
                    ImGui::PopID();

                    float end_y = ImGui::GetCursorPosY();
                    uint64_t dump_id = unique_id ^ (row_idx * 0x9e3779b9);
                    normal_expanded_heights[dump_id] = end_y - start_y;

                    table_open = ImGui::BeginTable("InspectorTableMain", 6, common_flags);
                    if (table_open) setup_inner_columns();
                }
            }

            if (bottom_dummy_h > 0.0f && table_open)
            {
                ImGui::TableNextRow(ImGuiTableRowFlags_None, bottom_dummy_h);
            }

            if (table_open) ImGui::EndTable();

            float total_dummy_outside = missed_height;
            if (!table_open && bottom_dummy_h > 0.0f)
            {
                total_dummy_outside += bottom_dummy_h;
            }
            
            if (total_dummy_outside > 0.0f)
            {
                ImGui::Dummy(ImVec2(0, total_dummy_outside));
            }

            if (auto_scroll && ImGui::GetScrollMaxY() > 0.0f)
            {
                ImGui::SetScrollY(ImGui::GetScrollMaxY());
            }

            if (!auto_scroll)
            {
                port_data.was_at_bottom = (ImGui::GetScrollMaxY() == 0.0f || ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 5.0f * dpi_scale);
            }

            ImGui::PopClipRect();
            ImGui::GetWindowDrawList()->ChannelsMerge();
            ImGui::EndChild();
        }

        char clear_btn_text[512];
        snprintf(clear_btn_text, sizeof(clear_btn_text), get_gui_string(gui_string_id::btn_clear_data_for, lang), name);

        auto clear_data_action = [&]() 
        {
            port_data.buffers.clear();
            port_data.data_pool.clear();
            port_data.data_pool.shrink_to_fit();
            port_data.expanded_nodes.clear();
            port_data.parsed_data.clear();
            port_data.parsed_flat_rows.clear();
            port_data.force_scroll_top = true;
        };

        if (is_detached)
        {
            float spacing = ImGui::GetStyle().ItemSpacing.x;
            float btn_width = (ImGui::GetContentRegionAvail().x - spacing) / 2.0f;
            
            const char* detach_btn_text = *is_detached ? get_gui_string(gui_string_id::btn_reattach, lang) : get_gui_string(gui_string_id::btn_detach, lang);
            if (ImGui::Button(detach_btn_text, ImVec2(btn_width, 0.0f)))
            {
                *is_detached = !(*is_detached);
            }
            ImGui::SameLine();
            if (ImGui::Button(clear_btn_text, ImVec2(btn_width, 0.0f)))
            {
                clear_data_action();
            }
        }
        else
        {
            if (ImGui::Button(clear_btn_text, ImVec2(-1.0f, 0.0f)))
            {
                clear_data_action();
            }
        }

        ImGui::EndChild();

        if (!is_detached || !(*is_detached))
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_Separator));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_SeparatorHovered));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_SeparatorActive));
            ImGui::Button("##hsplitter", ImVec2(-1.0f, 4.0f * dpi_scale));
            ImGui::PopStyleColor(3);

            if (ImGui::IsItemActive())
            {
                current_height += ImGui::GetIO().MouseDelta.y;
                if (current_height < 100.0f * dpi_scale) current_height = 100.0f * dpi_scale;
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
            }
            ImGui::Spacing();
        }

        ImGui::PopID();
    }

    void draw_inspector_view(gui_controller& ctrl, adam::language lang)
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
        const bool s_is_light_theme = s_theme_param && s_theme_param->get_value() == "light"_ct;

        auto format_bytes_to_buf = [](uint64_t bytes, char* buf, size_t buf_size) 
        {
            if (bytes < 1024) snprintf(buf, buf_size, "%llu B", static_cast<unsigned long long>(bytes));
            else if (bytes < 1024 * 1024) snprintf(buf, buf_size, "%.2f KB", bytes / 1024.0);
            else if (bytes < 1024 * 1024 * 1024) snprintf(buf, buf_size, "%.2f MB", bytes / (1024.0 * 1024.0));
            else snprintf(buf, buf_size, "%.2f GB", bytes / (1024.0 * 1024.0 * 1024.0));
        };

        const auto& connections = reg.get_connections();
        auto& connection_input_inspectors = ctrl.commander().get_connection_input_inspectors();
        auto& connection_output_inspectors = ctrl.commander().get_connection_output_inspectors();

        struct entry_preview
        {
            bool      has_data   = false;
            size_t    msg_count  = 0;
            size_t    total_size = 0;
            uint64_t  last_ts   = 0;
            char      preview_hex[32] = "";
        };
        std::map<adam::string_hash, entry_preview> conn_input_previews;
        std::map<adam::string_hash, entry_preview> conn_output_previews;
        std::map<adam::string_hash, entry_preview> port_previews;
        {
            std::lock_guard<std::mutex> preview_lock(adam::gui::g_inspection_data.mtx);
            auto snapshot_entry = [&](const std::map<adam::string_hash, adam::gui::inspection_port_data>& src,
                                      std::map<adam::string_hash, entry_preview>& dst)
            {
                for (const auto& [h, pd] : src)
                {
                    entry_preview ep;
                    ep.msg_count = pd.buffers.size();
                    if (ep.msg_count > 0)
                    {
                        ep.has_data   = true;
                        ep.total_size = pd.data_pool.size();
                        const auto& last_b = pd.buffers.back();
                        ep.last_ts   = last_b.timestamp;
                        size_t preview_len = std::min(static_cast<size_t>(last_b.size), static_cast<size_t>(8));
                        char preview_ascii_dummy[16];
                        fill_hex_preview(pd.data_pool.data() + last_b.offset, preview_len, 8,
                                         ep.preview_hex, sizeof(ep.preview_hex),
                                         preview_ascii_dummy, sizeof(preview_ascii_dummy));
                    }
                    dst[h] = ep;
                }
            };
            snapshot_entry(adam::gui::g_inspection_data.connections_input,  conn_input_previews);
            snapshot_entry(adam::gui::g_inspection_data.connections_output, conn_output_previews);
            snapshot_entry(adam::gui::g_inspection_data.ports,              port_previews);
        }

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

        static adam::configuration_parameter_integer* sort_mode_param = dynamic_cast<adam::configuration_parameter_integer*>(ctrl.get_parameters().get("analysis_sort_mode"_ct));
        int sort_mode = sort_mode_param ? static_cast<int>(sort_mode_param->get_value()) : 4;

        static adam::configuration_parameter_string* search_param = dynamic_cast<adam::configuration_parameter_string*>(ctrl.get_parameters().get("analysis_search"_ct));
        static std::string search_str = search_param ? std::string(search_param->get_value().c_str()) : "";

        // --- Top Control Bar ---
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_sort_by, lang));
        ImGui::SameLine();
        const char* sort_options[] = 
        {
            get_gui_string(gui_string_id::sort_analysis_active, lang),
            get_gui_string(gui_string_id::sort_analysis_inactive, lang),
            get_gui_string(gui_string_id::sort_analysis_name_asc, lang),
            get_gui_string(gui_string_id::sort_analysis_name_desc, lang),
            get_gui_string(gui_string_id::sort_analysis_mirror, lang)
        };
        ImGui::SetNextItemWidth(200.0f * dpi_scale);
        if (ImGui::Combo("##AnalysisSortMode", &sort_mode, sort_options, IM_ARRAYSIZE(sort_options)))
        {
            if (sort_mode_param)
            {
                sort_mode_param->set_value(static_cast<int64_t>(sort_mode));
            }
        }

        float center_pos = 0.0f;
        float search_width = 0.0f;
        get_search_bar_layout(lang, ImGui::GetContentRegionAvail().x, center_pos, search_width);
        
        if (center_pos < ImGui::GetCursorPosX())
        {
            center_pos = ImGui::GetCursorPosX() + ImGui::GetStyle().ItemSpacing.x;
        }

        ImGui::SameLine(center_pos);
        ImGui::SetNextItemWidth(search_width);
        char search_buf[256];
        strncpy(search_buf, search_str.c_str(), sizeof(search_buf) - 1);
        search_buf[sizeof(search_buf) - 1] = '\0';
        if (ImGui::InputTextWithHint("##AnalysisSearch", get_gui_string(gui_string_id::search_hint, lang), search_buf, sizeof(search_buf)))
        {
            search_str = search_buf;
            if (search_param)
            {
                search_param->set_value(adam::string_hashed(search_str));
            }
        }
        ImGui::Spacing();

        // --- Filtering and Sorting ---
        std::string search_lower = search_str;
        std::transform(search_lower.begin(), search_lower.end(), search_lower.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

        std::vector<std::pair<adam::string_hash, const adam::connection_view*>> sorted_connections;
        for (const auto& [hash, conn] : connections)
        {
            if (!search_lower.empty())
            {
                std::string name_lower = conn->name.c_str();
                std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                if (name_lower.find(search_lower) == std::string::npos)
                {
                    continue;
                }
            }
            sorted_connections.push_back({hash, conn.get()});
        }

        std::vector<std::pair<adam::string_hash, const adam::port_view*>> sorted_ports;
        for (const auto& [hash, p_view] : ports)
        {
            if (!search_lower.empty())
            {
                std::string name_lower = p_view->name.c_str();
                std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                if (name_lower.find(search_lower) == std::string::npos)
                {
                    continue;
                }
            }
            sorted_ports.push_back({hash, p_view.get()});
        }

        auto conn_sort_func = [sort_mode](const auto& a, const auto& b) 
        {
            if (sort_mode == 0) 
            {
                if (a.second->started != b.second->started) return a.second->started > b.second->started;
                return std::strcmp(a.second->name.c_str(), b.second->name.c_str()) < 0;
            }
            if (sort_mode == 1) 
            {
                if (a.second->started != b.second->started) return a.second->started < b.second->started;
                return std::strcmp(a.second->name.c_str(), b.second->name.c_str()) < 0;
            }
            if (sort_mode == 2) return std::strcmp(a.second->name.c_str(), b.second->name.c_str()) < 0;
            if (sort_mode == 3) return std::strcmp(a.second->name.c_str(), b.second->name.c_str()) > 0;
            if (sort_mode == 4) return a.second->sorting_index < b.second->sorting_index;
            return false;
        };
        std::stable_sort(sorted_connections.begin(), sorted_connections.end(), conn_sort_func);

        auto port_sort_func = [sort_mode](const auto& a, const auto& b) 
        {
            bool a_active = a.second->started;
            bool b_active = b.second->started;
            if (sort_mode == 0) 
            {
                if (a_active != b_active) return a_active > b_active;
                return std::strcmp(a.second->name.c_str(), b.second->name.c_str()) < 0;
            }
            if (sort_mode == 1) 
            {
                if (a_active != b_active) return a_active < b_active;
                return std::strcmp(a.second->name.c_str(), b.second->name.c_str()) < 0;
            }
            if (sort_mode == 3) return std::strcmp(a.second->name.c_str(), b.second->name.c_str()) > 0;
            return std::strcmp(a.second->name.c_str(), b.second->name.c_str()) < 0;
        };
        std::stable_sort(sorted_ports.begin(), sorted_ports.end(), port_sort_func);

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
                bool has_data = port_previews.count(hash) && port_previews.at(hash).has_data;
                bool has_inspector = inspectors.find(hash) != inspectors.end();
                if (has_inspector || has_data)
                {
                    num_expanded++;
                    g_pending_inspector_ports.erase(hash);
                }
                else
                {
                    if (g_pending_inspector_ports.count(hash))
                    {
                        num_expanded++;
                    }
                    else
                    {
                        g_expanded_inspector_ports.erase(hash);
                    }
                }
            }
        }

        for (const auto& [hash, conn] : connections)
        {
            if (g_expanded_inspector_connections_input.count(hash))
            {
                bool has_data = conn_input_previews.count(hash) && conn_input_previews.at(hash).has_data;
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
                bool has_data = conn_output_previews.count(hash) && conn_output_previews.at(hash).has_data;
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

        float spacing_h = ImGui::GetStyle().ItemSpacing.y;
        float sticky_button_h = ImGui::GetFrameHeight() * 1.5f + spacing_h;
        float initial_inspector_height = ImGui::GetContentRegionAvail().y * 0.5f;
        if (initial_inspector_height < 250.0f * dpi_scale)
        {
            initial_inspector_height = 250.0f * dpi_scale;
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::BeginChild("InspectorScrollContent", ImVec2(0, -sticky_button_h), false);
        ImGui::PopStyleVar();

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

            size_t connection_idx = 0;
            size_t total_connections = sorted_connections.size();
            for (const auto& [conn_hash, c_view] : sorted_connections)
            {
                connection_idx++;
                bool is_last_connection = (connection_idx == total_connections);
                
                // Input Row
                {
                    bool has_inspector = connection_input_inspectors.find(conn_hash) != connection_input_inspectors.end();
                    const auto& ep_in = conn_input_previews[conn_hash];
                    bool has_data   = ep_in.has_data;
                    size_t msg_count  = ep_in.msg_count;
                    size_t total_size = ep_in.total_size;
                    uint64_t last_ts  = ep_in.last_ts;
                    const char* preview_hex = ep_in.preview_hex;

                    bool node_open = (has_inspector || has_data) && (g_expanded_inspector_connections_input.count(conn_hash) > 0);
                    char name_buf[256];
                    snprintf(name_buf, sizeof(name_buf), "%s [Input]", c_view->name.c_str());

                    if (conn_table_open)
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        bool pushed_id = false;
                        
                        if (has_inspector || has_data)
                        {
                            ImGui::PushID(reinterpret_cast<const void*>(static_cast<intptr_t>(conn_hash ^ 0x9990)));
                            pushed_id = true;
                            
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered));
                            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
                            if (ImGui::ArrowButton("##node_in", node_open ? ImGuiDir_Down : ImGuiDir_Right))
                            {
                                if (node_open)
                                    g_expanded_inspector_connections_input.erase(conn_hash);
                                else
                                    g_expanded_inspector_connections_input.insert(conn_hash);
                                node_open = !node_open;
                            }
                            ImGui::PopStyleColor(3);
                        }
                        
                        ImGui::TableSetColumnIndex(1);
                        ImColor pin_col = s_is_light_theme ? get_gui_color(gui_color_id::node_connection_line_light) : get_gui_color(gui_color_id::node_connection_line);
                        if (c_view->started)
                        {
                            pin_col = get_gui_color(gui_color_id::node_pin_active);
                        }
                        ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
                        float dot_radius = ImGui::GetTextLineHeight() / 2 - ImGui::GetStyle().CellPadding.y;
                        ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(cursor_pos.x + dot_radius + ImGui::GetStyle().CellPadding.x, cursor_pos.y + ImGui::GetFrameHeight() * 0.5f), dot_radius, pin_col);
                        
                        ImGui::TableSetColumnIndex(2);
                        bool inspect_val = has_inspector;
                        ImGui::PushID(reinterpret_cast<const void*>(static_cast<intptr_t>(conn_hash ^ 0x9991)));
                        if (ImGui::Checkbox("##inspect_in", &inspect_val))
                        {
                            toggle_connection_inspector(ctrl, conn_hash, true, inspect_val);
                        }
                        ImGui::PopID();
                        
                        ImGui::TableSetColumnIndex(3);
                        ImGui::TextUnformatted(name_buf);
                        
                        ImGui::TableSetColumnIndex(4);
                        if (has_inspector || has_data) ImGui::Text("%zu", msg_count);
                        
                        ImGui::TableSetColumnIndex(5);
                        char total_size_buf[64] = "";
                        if (has_inspector || has_data)
                        {
                            format_bytes_to_buf(total_size, total_size_buf, sizeof(total_size_buf));
                            ImGui::TextUnformatted(total_size_buf);
                        }
                        
                        ImGui::TableSetColumnIndex(6);
                        if (has_inspector || has_data)
                        {
                            if (msg_count > 0)
                                ImGui::Text("%s | %s", adam::get_log_time_string(last_ts).c_str(), preview_hex);
                            else
                                ImGui::TextDisabled("%s", get_gui_string(gui_string_id::lbl_no_data, lang));
                        }
                            
                        if (pushed_id) ImGui::PopID();
                    }
                        
                    if (node_open)
                    {
                        if (conn_table_open) ImGui::EndTable();
                        
                        bool is_detached = g_detached_inspector_connections_input.count(conn_hash) > 0;
                        if (!is_detached)
                        {
                            bool detached_var = false;
                            draw_inspector_frames_table(name_buf, conn_hash, adam::gui::g_inspection_data.connections_input, initial_inspector_height, dpi_scale, lang, 0x1111111111111111ULL, &detached_var);
                            if (detached_var) g_detached_inspector_connections_input.insert(conn_hash);
                        }
                        else
                        {
                            ImGui::TextDisabled("Inspector is detached (see separate window)");
                            ImGui::Spacing();
                        }
                        
                        conn_table_open = ImGui::BeginTable("InspectorConnectionsTable", 7, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable);
                        if (conn_table_open) setup_columns();
                    }
                }

                // Output Row
                {
                    bool has_inspector = connection_output_inspectors.find(conn_hash) != connection_output_inspectors.end();
                    const auto& ep_out = conn_output_previews[conn_hash];
                    bool has_data   = ep_out.has_data;
                    size_t msg_count  = ep_out.msg_count;
                    size_t total_size = ep_out.total_size;
                    uint64_t last_ts  = ep_out.last_ts;
                    const char* preview_hex = ep_out.preview_hex;

                    bool node_open = (has_inspector || has_data) && (g_expanded_inspector_connections_output.count(conn_hash) > 0);
                    char name_buf[256];
                    snprintf(name_buf, sizeof(name_buf), "%s [Output]", c_view->name.c_str());

                    if (conn_table_open)
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        bool pushed_id = false;
                        
                        if (has_inspector || has_data)
                        {
                            ImGui::PushID(reinterpret_cast<const void*>(static_cast<intptr_t>(conn_hash ^ 0x9992)));
                            pushed_id = true;
                            
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered));
                            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
                            if (ImGui::ArrowButton("##node_out", node_open ? ImGuiDir_Down : ImGuiDir_Right))
                            {
                                if (node_open)
                                    g_expanded_inspector_connections_output.erase(conn_hash);
                                else
                                    g_expanded_inspector_connections_output.insert(conn_hash);
                                node_open = !node_open;
                            }
                            ImGui::PopStyleColor(3);
                        }
                        
                        ImGui::TableSetColumnIndex(1);
                        ImColor pin_col = s_is_light_theme ? get_gui_color(gui_color_id::node_connection_line_light) : get_gui_color(gui_color_id::node_connection_line);
                        if (c_view->started)
                        {
                            pin_col = get_gui_color(gui_color_id::node_pin_active);
                        }
                        ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
                        float dot_radius = ImGui::GetTextLineHeight() / 2 - ImGui::GetStyle().CellPadding.y;
                        ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(cursor_pos.x + dot_radius + ImGui::GetStyle().CellPadding.x, cursor_pos.y + ImGui::GetFrameHeight() * 0.5f), dot_radius, pin_col);
                        
                        ImGui::TableSetColumnIndex(2);
                        bool inspect_val = has_inspector;
                        ImGui::PushID(reinterpret_cast<const void*>(static_cast<intptr_t>(conn_hash ^ 0x9993)));
                        if (ImGui::Checkbox("##inspect_out", &inspect_val))
                        {
                            toggle_connection_inspector(ctrl, conn_hash, false, inspect_val);
                        }
                        ImGui::PopID();
                        
                        ImGui::TableSetColumnIndex(3);
                        ImGui::TextUnformatted(name_buf);
                        
                        ImGui::TableSetColumnIndex(4);
                        if (has_inspector || has_data) ImGui::Text("%zu", msg_count);
                        
                        ImGui::TableSetColumnIndex(5);
                        char total_size_buf[64] = "";
                        if (has_inspector || has_data)
                        {
                            format_bytes_to_buf(total_size, total_size_buf, sizeof(total_size_buf));
                            ImGui::TextUnformatted(total_size_buf);
                        }
                        
                        ImGui::TableSetColumnIndex(6);
                        if (has_inspector || has_data)
                        {
                            if (msg_count > 0)
                                ImGui::Text("%s | %s", adam::get_log_time_string(last_ts).c_str(), preview_hex);
                            else
                                ImGui::TextDisabled("%s", get_gui_string(gui_string_id::lbl_no_data, lang));
                        }
                            
                        if (pushed_id) ImGui::PopID();
                    }
                        
                    if (node_open)
                    {
                        if (conn_table_open) ImGui::EndTable();
                        
                        bool is_detached = g_detached_inspector_connections_output.count(conn_hash) > 0;
                        if (!is_detached)
                        {
                            bool detached_var = false;
                            draw_inspector_frames_table(name_buf, conn_hash, adam::gui::g_inspection_data.connections_output, initial_inspector_height, dpi_scale, lang, 0x2222222222222222ULL, &detached_var);
                            if (detached_var) g_detached_inspector_connections_output.insert(conn_hash);
                        }
                        else
                        {
                            ImGui::TextDisabled("Inspector is detached (see separate window)");
                            ImGui::Spacing();
                        }
                        
                        if (!is_last_connection)
                        {
                            conn_table_open = ImGui::BeginTable("InspectorConnectionsTable", 7, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable);
                            if (conn_table_open) setup_columns();
                        }
                        else
                        {
                            conn_table_open = false;
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

            size_t port_idx = 0;
            size_t total_ports = sorted_ports.size();
            for (const auto& [port_hash, p_view] : sorted_ports)
            {
                port_idx++;
                bool is_last_port = (port_idx == total_ports);
                bool has_inspector = inspectors.find(port_hash) != inspectors.end();
                const auto& ep_port = port_previews[port_hash];
                bool has_data   = ep_port.has_data;
                size_t msg_count  = ep_port.msg_count;
                size_t total_size = ep_port.total_size;
                uint64_t last_ts  = ep_port.last_ts;
                const char* preview_hex = ep_port.preview_hex;

                bool node_open = (has_inspector || has_data) && (g_expanded_inspector_ports.count(port_hash) > 0);

                if (table_open)
                {
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    bool pushed_id = false;
                    if (has_inspector || has_data)
                    {
                        ImGui::PushID(reinterpret_cast<const void*>(static_cast<intptr_t>(port_hash ^ 0x9999)));
                        pushed_id = true;
                        
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
                        if (ImGui::ArrowButton("##node", node_open ? ImGuiDir_Down : ImGuiDir_Right))
                        {
                            if (node_open)
                                g_expanded_inspector_ports.erase(port_hash);
                            else
                                g_expanded_inspector_ports.insert(port_hash);
                            node_open = !node_open;
                        }
                        ImGui::PopStyleColor(3);
                    }
                
                    ImGui::TableSetColumnIndex(1);
                    ImColor pin_col = s_is_light_theme ? get_gui_color(gui_color_id::node_connection_line_light) : get_gui_color(gui_color_id::node_connection_line);
                    if (p_view->started)
                    {
                        if (p_view->statistic_buffer)
                        {
                            auto* stats = p_view->statistic_buffer->data_as<adam::port::state_buffer_data>();
                            if (stats->cur_state == adam::port::state_running)
                            {
                                pin_col = get_gui_color(gui_color_id::node_pin_active);
                            }
                            else if (stats->cur_state == adam::port::state_inactive)
                            {
                                pin_col = get_gui_color(gui_color_id::log_warning);
                            }
                        }
                        else
                        {
                            pin_col = get_gui_color(gui_color_id::node_pin_active);
                        }
                    }

                    ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
                    float dot_radius = ImGui::GetTextLineHeight() / 2 - ImGui::GetStyle().CellPadding.y;
                    ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(cursor_pos.x + dot_radius + ImGui::GetStyle().CellPadding.x, cursor_pos.y + ImGui::GetFrameHeight() * 0.5f), dot_radius, pin_col);
                    
                    ImGui::TableSetColumnIndex(2);
                    bool inspect_val = has_inspector;
                    ImGui::PushID(reinterpret_cast<const void*>(static_cast<intptr_t>(port_hash)));
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

                    ImGui::TableSetColumnIndex(3);
                    ImGui::TextUnformatted(p_view->name.c_str());

                    if (has_inspector || has_data)
                    {
                        ImGui::TableSetColumnIndex(4);
                        ImGui::Text("%zu", msg_count);

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
                }

                if (node_open)
                {
                    if (table_open) ImGui::EndTable();
                    
                    bool is_detached = g_detached_inspector_ports.count(port_hash) > 0;
                    if (!is_detached)
                    {
                        bool detached_var = false;
                        draw_inspector_frames_table(p_view->name.c_str(), port_hash, adam::gui::g_inspection_data.ports, initial_inspector_height, dpi_scale, lang, 0x3333333333333333ULL, &detached_var);
                        if (detached_var) g_detached_inspector_ports.insert(port_hash);
                    }
                    else
                    {
                        ImGui::TextDisabled("Inspector is detached (see separate window)");
                        ImGui::Spacing();
                    }

                    if (!is_last_port)
                    {
                        table_open = ImGui::BeginTable("InspectorPortsTable", 7, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable);
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
            }

            if (table_open)
            {
                ImGui::EndTable();
            }
        }

        ImGui::EndChild();

        bool has_any_buffered_data = false;
        for (const auto& [h, ep] : port_previews)
        {
            if (ep.has_data)
            {
                has_any_buffered_data = true;
                break;
            }
        }
        if (!has_any_buffered_data)
        {
            for (const auto& [h, ep] : conn_input_previews)
            {
                if (ep.has_data)
                {
                    has_any_buffered_data = true;
                    break;
                }
            }
        }
        if (!has_any_buffered_data)
        {
            for (const auto& [h, ep] : conn_output_previews)
            {
                if (ep.has_data)
                {
                    has_any_buffered_data = true;
                    break;
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

    void draw_inspector_subwindow
    (
        gui_controller& ctrl,
        adam::language lang,
        float& left_w,
        float avail_w,
        float content_h
    )
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
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
        }

        ImGui::SameLine();

        ImGui::BeginChild("InspectorRegion", ImVec2(0, content_h), false);
        draw_inspector_view(ctrl, lang);
        ImGui::EndChild();
    }

    void draw_dockable_inspector_windows(gui_controller& ctrl, adam::language lang)
    {
        if (!ctrl.is_commander_active())
        {
            return;
        }

        auto& cmdr = ctrl.commander();
        float dpi_scale = ImGui::GetStyle()._MainScale;

        static std::vector<adam::data_inspector*> s_conn_in_to_destroy;
        static std::vector<adam::data_inspector*> s_conn_out_to_destroy;
        static std::vector<adam::data_inspector*> s_port_to_destroy;

        for (auto p : s_conn_in_to_destroy) cmdr.request_connection_input_inspector_destroy(p);
        s_conn_in_to_destroy.clear();

        for (auto p : s_conn_out_to_destroy) cmdr.request_connection_output_inspector_destroy(p);
        s_conn_out_to_destroy.clear();

        for (auto p : s_port_to_destroy) cmdr.request_inspector_destroy(p);
        s_port_to_destroy.clear();

        // 1. Connection Input Inspectors
        const auto& conn_in_inspectors = cmdr.get_connection_input_inspectors();

        for (const auto& [conn_hash, p_inspector] : conn_in_inspectors)
        {
            if (g_detached_inspector_connections_input.count(conn_hash) == 0) continue;

            const char* conn_name = "Unknown";
            auto view_it = cmdr.registry().get_connections().find(conn_hash);
            if (view_it != cmdr.registry().get_connections().end())
            {
                conn_name = view_it->second->name.c_str();
            }

            std::string title = std::string(conn_name) + " (Input)###ConnInInsp_" + std::to_string(conn_hash);
            bool open = true;
            ImGui::SetNextWindowSize(ImVec2(600 * dpi_scale, 400 * dpi_scale), ImGuiCond_FirstUseEver);

            if (ImGui::Begin(title.c_str(), &open))
            {
                bool detached_var = true;
                draw_inspector_frames_table(conn_name, conn_hash, adam::gui::g_inspection_data.connections_input, 400 * dpi_scale, dpi_scale, lang, 0x1111111111111111ULL, &detached_var);
                if (!detached_var) g_detached_inspector_connections_input.erase(conn_hash);
            }
            ImGui::End();

            if (!open) 
            {
                s_conn_in_to_destroy.push_back(p_inspector);
                g_detached_inspector_connections_input.erase(conn_hash);
            }
        }

        // 2. Connection Output Inspectors
        const auto& conn_out_inspectors = cmdr.get_connection_output_inspectors();

        for (const auto& [conn_hash, p_inspector] : conn_out_inspectors)
        {
            if (g_detached_inspector_connections_output.count(conn_hash) == 0) continue;

            const char* conn_name = "Unknown";
            auto view_it = cmdr.registry().get_connections().find(conn_hash);
            if (view_it != cmdr.registry().get_connections().end())
            {
                conn_name = view_it->second->name.c_str();
            }

            std::string title = std::string(conn_name) + " (Output)###ConnOutInsp_" + std::to_string(conn_hash);
            bool open = true;
            ImGui::SetNextWindowSize(ImVec2(600 * dpi_scale, 400 * dpi_scale), ImGuiCond_FirstUseEver);

            if (ImGui::Begin(title.c_str(), &open))
            {
                bool detached_var = true;
                draw_inspector_frames_table(conn_name, conn_hash, adam::gui::g_inspection_data.connections_output, 400 * dpi_scale, dpi_scale, lang, 0x2222222222222222ULL, &detached_var);
                if (!detached_var) g_detached_inspector_connections_output.erase(conn_hash);
            }
            ImGui::End();

            if (!open) 
            {
                s_conn_out_to_destroy.push_back(p_inspector);
                g_detached_inspector_connections_output.erase(conn_hash);
            }
        }

        // 3. Port Inspectors
        const auto& port_inspectors = cmdr.get_inspectors();

        for (const auto& [port_hash, p_inspector] : port_inspectors)
        {
            if (g_detached_inspector_ports.count(port_hash) == 0) continue;

            const char* port_name = "Unknown";
            auto view_it = cmdr.registry().get_ports().find(port_hash);
            if (view_it != cmdr.registry().get_ports().end())
            {
                port_name = view_it->second->name.c_str();
            }

            std::string title = std::string(port_name) + "###PortInsp_" + std::to_string(port_hash);
            bool open = true;
            ImGui::SetNextWindowSize(ImVec2(600 * dpi_scale, 400 * dpi_scale), ImGuiCond_FirstUseEver);

            if (ImGui::Begin(title.c_str(), &open))
            {
                bool detached_var = true;
                draw_inspector_frames_table(port_name, port_hash, adam::gui::g_inspection_data.ports, 400 * dpi_scale, dpi_scale, lang, 0x3333333333333333ULL, &detached_var);
                if (!detached_var) g_detached_inspector_ports.erase(port_hash);
            }
            ImGui::End();

            if (!open) 
            {
                s_port_to_destroy.push_back(p_inspector);
                g_detached_inspector_ports.erase(port_hash);
            }
        }
    }
}
