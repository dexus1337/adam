/**
 * @file    window-about.cpp
 * @author  dexus1337
 * @brief   Implementation of the about dialog for adam-cop.
 * @version 1.0
 * @date    20.08.2026
 */

#include "window-about.hpp"
#include "../cop-strings.hpp"
#include "../cop-controller.hpp"
#include <imgui.h>
#include <textures.hpp>
#include <vector>
#include <string>
#include <cstring>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace adam::cop
{
#ifdef _WIN32
    static ImTextureID g_logo_texture = (ImTextureID)0;
    static int g_logo_width = 0;
    static int g_logo_height = 0;

    static void load_logo_texture_once()
    {
        if (g_logo_texture != (ImTextureID)0)
        {
            return;
        }

        HICON hIcon = (HICON)LoadImageA(GetModuleHandleA(NULL), MAKEINTRESOURCEA(101), IMAGE_ICON, 256, 256, LR_CREATEDIBSECTION);
        if (!hIcon)
        {
            hIcon = (HICON)LoadImageA(GetModuleHandleA(NULL), MAKEINTRESOURCEA(101), IMAGE_ICON, 0, 0, LR_CREATEDIBSECTION | LR_DEFAULTSIZE);
        }
        if (!hIcon)
        {
            return;
        }

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

        for (int i = 0; i < g_logo_width * g_logo_height; ++i)
        {
            uint8_t b = pixels[i * 4 + 0];
            uint8_t r = pixels[i * 4 + 2];
            pixels[i * 4 + 0] = r;
            pixels[i * 4 + 2] = b;
        }

        g_logo_texture = adam::lib::imgui::create_texture_rgba(g_logo_width, g_logo_height, pixels.data());

        DeleteObject(ii.hbmColor);
        DeleteObject(ii.hbmMask);
        ReleaseDC(NULL, hdc);
        DestroyIcon(hIcon);
    }
#endif

    void draw_about_dialog(cop_controller& ctrl, adam::language lang, bool& show_about)
    {
        (void)ctrl;
        if (!show_about)
        {
            return;
        }

#ifdef _WIN32
        load_logo_texture_once();
#endif

        const char* title_text = "ADAM COP";
        
        char version_text[128];
        auto ver = adam::decode_version(adam::sdk_version);
        snprintf(version_text, sizeof(version_text), "v%d.%d.%d", ver.major, ver.minor, ver.patch);

        const char* desc_text = get_cop_string(msg_cop_description, lang);
        const char* cpy1_text = "© 2026 dexus1337.";
        const char* cpy2_text = "All rights reserved.";

        std::string title_about = std::string(get_cop_string(wnd_about, lang)) + "###AboutDialog";
        ImGui::OpenPopup(title_about.c_str());

        ImGui::SetNextWindowSize(ImVec2(600, 480), ImGuiCond_FirstUseEver);
        if (ImGui::BeginPopupModal(title_about.c_str(), &show_about, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking))
        {
            ImVec2 avail = ImGui::GetContentRegionAvail();
            float wrap_width = avail.x * 0.8f;
            if (wrap_width < 300.0f)
            {
                wrap_width = avail.x;
            }

            float item_spacing = ImGui::GetStyle().ItemSpacing.y;
            float large_spacing = item_spacing * 2.0f;

            float total_height = 0.0f;
            
            float logo_display_size = 96.0f * ImGui::GetStyle()._MainScale;
#ifdef _WIN32
            if (g_logo_texture)
            {
                total_height += logo_display_size + item_spacing;
            }
#endif

            ImVec2 title_size = ImVec2(ImGui::CalcTextSize(title_text).x * 2.0f, ImGui::CalcTextSize(title_text).y * 2.0f);
            ImVec2 version_size = ImGui::CalcTextSize(version_text);

            auto measure_or_draw_centered_text = [&](const char* text, float max_w, bool do_draw) -> ImVec2
            {
                ImVec2 total_size(0.0f, 0.0f);
                const char* text_end = text + strlen(text);
                const char* s = text;
                while (s < text_end)
                {
                    const char* line_end = s;
                    while (line_end < text_end && *line_end != '\n')
                    {
                        line_end++;
                    }
                    
                    const char* line_p = s;
                    while (line_p < line_end)
                    {
                        const char* word_end = line_p;
                        while (word_end < line_end && *word_end != ' ')
                        {
                            word_end++;
                        }
                        
                        const char* next_word = word_end;
                        while (next_word < line_end && *next_word == ' ')
                        {
                            next_word++;
                        }
                        
                        float line_w = ImGui::CalcTextSize(s, word_end).x;
                        
                        if (line_w > max_w && line_p > s)
                        {
                            if (do_draw)
                            {
                                ImVec2 line_sz = ImGui::CalcTextSize(s, line_p - 1);
                                float x = ImGui::GetCursorPosX() + (avail.x - line_sz.x) * 0.5f;
                                ImGui::SetCursorPosX(x);
                                ImGui::TextUnformatted(s, line_p - 1);
                            }
                            else
                            {
                                total_size.y += ImGui::GetTextLineHeightWithSpacing();
                            }
                            s = line_p;
                        }
                        line_p = next_word;
                    }
                    
                    if (do_draw)
                    {
                        ImVec2 line_sz = ImGui::CalcTextSize(s, line_end);
                        float x = ImGui::GetCursorPosX() + (avail.x - line_sz.x) * 0.5f;
                        ImGui::SetCursorPosX(x);
                        ImGui::TextUnformatted(s, line_end);
                    }
                    else
                    {
                        total_size.y += ImGui::GetTextLineHeightWithSpacing();
                    }

                    s = line_end + 1;
                }
                return total_size;
            };

            ImVec2 desc_size = measure_or_draw_centered_text(desc_text, wrap_width, false);
            ImVec2 cpy1_size = ImGui::CalcTextSize(cpy1_text);
            ImVec2 cpy2_size = ImGui::CalcTextSize(cpy2_text);

            float start_cursor_y = ImGui::GetCursorPosY();
            float start_cursor_x = ImGui::GetCursorPosX();

            total_height += title_size.y + large_spacing;
            total_height += item_spacing;
            total_height += version_size.y + large_spacing;
            total_height += desc_size.y + large_spacing;
            total_height += cpy1_size.y + item_spacing;
            total_height += cpy2_size.y;

            float start_y = (avail.y - total_height) * 0.5f - ImGui::GetFrameHeight() * 1.5f;
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
            if (g_logo_texture != (ImTextureID)0) 
            {
                center_x(logo_display_size);
                ImGui::Image(g_logo_texture, ImVec2(logo_display_size, logo_display_size));
                ImGui::Spacing();
            }
#endif

            center_x(title_size.x);
            ImGui::SetWindowFontScale(2.0f);
            ImGui::TextUnformatted(title_text);
            ImGui::SetWindowFontScale(1.0f);

            ImGui::Spacing();
            
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

            ImGui::Spacing();
            ImGui::Spacing();

            float btn_w = 120.0f;
            ImGui::SetCursorPosX((avail.x - btn_w) * 0.5f);
            if (ImGui::Button("OK", ImVec2(btn_w, 0)))
            {
                show_about = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }
}
