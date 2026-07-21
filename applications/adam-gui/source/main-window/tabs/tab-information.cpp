/**
 * @file    tab-information.cpp
 * @author  dexus1337
 * @brief   Implementation of the information tab drawing functions.
 * @version 1.0
 * @date    12.06.2026
 */

#include "tab-information.hpp"
#include "../main-window.hpp"
#include <imgui.h>
#include <version/version.hpp>
#include <cstdio>
#include <string>

#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#include <vector>

static GLuint g_logo_texture = 0;
static int g_logo_width = 0;
static int g_logo_height = 0;

static void load_logo_texture_once()
{
    if (g_logo_texture != 0) return;

    HICON hIcon = (HICON)LoadImageA(GetModuleHandleA(NULL), MAKEINTRESOURCEA(101), IMAGE_ICON, 256, 256, LR_CREATEDIBSECTION);
    if (!hIcon)
    {
        hIcon = (HICON)LoadImageA(GetModuleHandleA(NULL), MAKEINTRESOURCEA(101), IMAGE_ICON, 0, 0, LR_CREATEDIBSECTION | LR_DEFAULTSIZE);
    }
    if (!hIcon) return;

    ICONINFO ii;
    if (!GetIconInfo(hIcon, &ii))
    {
        DestroyIcon(hIcon);
        return;
    }

    BITMAP bmp;
    GetObject(ii.hbmColor, sizeof(BITMAP), &bmp);

    g_logo_width = bmp.bmWidth;
    g_logo_height = bmp.bmHeight;

    HDC hdc = GetDC(NULL);
    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = g_logo_width;
    bmi.bmiHeader.biHeight = -g_logo_height; // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    std::vector<uint8_t> pixels(g_logo_width * g_logo_height * 4);
    GetDIBits(hdc, ii.hbmColor, 0, g_logo_height, pixels.data(), &bmi, DIB_RGB_COLORS);
    
    // BGRA to RGBA
    for (size_t i = 0; i < pixels.size(); i += 4)
    {
        uint8_t b = pixels[i];
        uint8_t r = pixels[i + 2];
        pixels[i] = r;
        pixels[i + 2] = b;
    }

    ReleaseDC(NULL, hdc);
    DeleteObject(ii.hbmColor);
    if (ii.hbmMask) DeleteObject(ii.hbmMask);
    DestroyIcon(hIcon);

    glGenTextures(1, &g_logo_texture);
    glBindTexture(GL_TEXTURE_2D, g_logo_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, g_logo_width, g_logo_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
}
#endif

namespace adam::gui 
{
    void draw_tab_information(gui_controller& ctrl, adam::language lang)
    {
        (void)ctrl;

#ifdef _WIN32
        load_logo_texture_once();
#endif

        ImVec2 avail = ImGui::GetContentRegionAvail();
        float start_cursor_y = ImGui::GetCursorPosY();
        float start_cursor_x = ImGui::GetCursorPosX();
        
        float item_spacing = ImGui::GetStyle().ItemSpacing.y;
        float large_spacing = item_spacing * 3.0f;
        
        const char* title_text = "ADAM";
        char version_text[64];
        std::snprintf(version_text, sizeof(version_text), "Version: %d.%d.%d", 
            adam::get_major(adam::sdk_version),
            adam::get_minor(adam::sdk_version),
            adam::get_patch(adam::sdk_version));
            
        const char* desc_text = get_gui_string(gui_string_id::msg_about_description, lang);
        const char* cpy1_text = get_gui_string(gui_string_id::msg_about_copyright1, lang);
        const char* cpy2_text = get_gui_string(gui_string_id::msg_about_copyright2, lang);

        ImGui::SetWindowFontScale(2.0f);
        ImVec2 title_size = ImGui::CalcTextSize(title_text);
        ImGui::SetWindowFontScale(1.0f);

        ImVec2 version_size = ImGui::CalcTextSize(version_text);
        
        float max_wrap = 45.0f * ImGui::GetFontSize();
        float wrap_width = (avail.x > max_wrap + 40.0f) ? max_wrap : (avail.x - 40.0f);
        if (wrap_width < 100.0f) wrap_width = 100.0f;
        
        auto measure_or_draw_centered_text = [&](const char* text, float wrap_w, bool draw) -> ImVec2
        {
            float total_h = 0.0f;
            float max_w = 0.0f;
            const char* word_start = text;
            const char* word_end = text;
            std::string current_line;
            float line_height = ImGui::GetTextLineHeight();
            ImVec2 cursor_pos = ImGui::GetCursorPos();
            
            auto flush_line = [&]() {
                if (!current_line.empty()) {
                    ImVec2 size = ImGui::CalcTextSize(current_line.c_str());
                    if (size.x > max_w) max_w = size.x;
                    if (draw) {
                        float cx = (avail.x - size.x) * 0.5f;
                        if (cx < 0.0f) cx = 0.0f;
                        ImGui::SetCursorPos(ImVec2(start_cursor_x + cx, cursor_pos.y + total_h));
                        ImGui::TextUnformatted(current_line.c_str());
                    }
                    total_h += line_height;
                    current_line.clear();
                }
            };

            while (*word_end != '\0') 
            {
                while (*word_end != '\0' && *word_end != ' ' && *word_end != '\n') {
                    word_end++;
                }
                
                bool is_newline = (*word_end == '\n');
                
                std::string word(word_start, word_end);
                std::string test_line = current_line.empty() ? word : current_line + " " + word;
                
                if (ImGui::CalcTextSize(test_line.c_str()).x > wrap_w && !current_line.empty()) 
                {
                    flush_line();
                    current_line = word;
                }
                else 
                {
                    current_line = test_line;
                }
                
                if (is_newline) {
                    flush_line();
                }
                
                if (*word_end != '\0') {
                    word_end++;
                    word_start = word_end;
                }
            }
            flush_line();
            
            if (draw && total_h > 0.0f) {
                // Advance cursor past the block so subsequent items are placed correctly
                ImGui::SetCursorPos(ImVec2(start_cursor_x, cursor_pos.y + total_h));
            }
            
            return ImVec2(max_w, total_h);
        };

        ImVec2 desc_size = measure_or_draw_centered_text(desc_text, wrap_width, false);
        ImVec2 cpy1_size = ImGui::CalcTextSize(cpy1_text);
        ImVec2 cpy2_size = ImGui::CalcTextSize(cpy2_text);

        float total_height = 0.0f;
        
        float logo_display_size = 8.0f * ImGui::GetFontSize();
        
#ifdef _WIN32
        if (g_logo_texture) 
        {
            total_height += logo_display_size + item_spacing;
        }
#endif

        total_height += title_size.y + large_spacing;
        total_height += item_spacing; // For separator
        total_height += version_size.y + large_spacing;
        total_height += desc_size.y + large_spacing;
        total_height += cpy1_size.y + item_spacing;
        total_height += cpy2_size.y;

        float start_y = (avail.y - total_height) * 0.5f;
        if (start_y > 0.0f) 
        {
            ImGui::SetCursorPosY(start_cursor_y + start_y);
        }

        auto center_x = [&](float width) 
        {
            float x = (avail.x - width) * 0.5f;
            if (x > 0.0f) 
            {
                ImGui::SetCursorPosX(start_cursor_x + x);
            }
        };

#ifdef _WIN32
        if (g_logo_texture) 
        {
            center_x(logo_display_size);
            ImGui::Image((void*)(intptr_t)g_logo_texture, ImVec2(logo_display_size, logo_display_size));
            ImGui::Spacing();
        }
#endif

        center_x(title_size.x);
        ImGui::SetWindowFontScale(2.0f);
        ImGui::TextUnformatted(title_text);
        ImGui::SetWindowFontScale(1.0f);

        ImGui::Spacing();
        
        // Custom centered separator
        ImVec2 p = ImGui::GetCursorScreenPos();
        float sep_width = wrap_width;
        float sep_start_x = p.x + (avail.x - sep_width) * 0.5f;
        ImGui::GetWindowDrawList()->AddLine(ImVec2(sep_start_x, p.y), ImVec2(sep_start_x + sep_width, p.y), ImGui::GetColorU32(ImGuiCol_Separator));
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + item_spacing);

        ImGui::Spacing();

        center_x(version_size.x);
        ImGui::TextUnformatted(version_text);
        
        ImGui::Dummy(ImVec2(0.0f, large_spacing - item_spacing));

        measure_or_draw_centered_text(desc_text, wrap_width, true);

        ImGui::Dummy(ImVec2(0.0f, large_spacing - item_spacing));

        center_x(cpy1_size.x);
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s", cpy1_text);
        center_x(cpy2_size.x);
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s", cpy2_text);
    }
}
