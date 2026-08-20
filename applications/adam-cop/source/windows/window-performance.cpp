/**
 * @file    window-performance.cpp
 * @author  dexus1337
 * @brief   Implementation of the performance overlay HUD for adam-cop.
 * @version 1.0
 * @date    20.08.2026
 */

#include "window-performance.hpp"
#include "../cop-strings.hpp"
#include <imgui.h>
#include <string>

namespace adam::cop
{
    void draw_performance_overlay(perf_overlay_params& params, adam::language lang)
    {
        if (!params.p_show_performance || !params.p_show_performance->get_value())
        {
            return;
        }

        int location = params.p_perf_ovly_location ? static_cast<int>(params.p_perf_ovly_location->get_value()) : 0;
        static bool custom_pos_initialized = false;
        
        ImGuiWindowFlags overlay_flags = ImGuiWindowFlags_NoDecoration | 
                                        ImGuiWindowFlags_AlwaysAutoResize | 
                                        ImGuiWindowFlags_NoSavedSettings | 
                                        ImGuiWindowFlags_NoFocusOnAppearing | 
                                        ImGuiWindowFlags_NoNav | 
                                        ImGuiWindowFlags_NoMove;
        
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        
        if (location >= 0)
        {
            float pad = 10.0f * ImGui::GetStyle()._MainScale;
            ImVec2 work_pos = viewport->WorkPos;
            ImVec2 work_size = viewport->WorkSize;
            ImVec2 window_pos, window_pos_pivot;
            float status_bar_height = ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y * 2.0f;
            
            window_pos.x = (location & 1) ? (work_pos.x + work_size.x - pad) : (work_pos.x + pad);
            window_pos.y = (location & 2) ? (work_pos.y + work_size.y - pad - status_bar_height) : (work_pos.y + ImGui::GetFrameHeight() + pad);
            window_pos_pivot.x = (location & 1) ? 1.0f : 0.0f;
            window_pos_pivot.y = (location & 2) ? 1.0f : 0.0f;
            
            ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
            custom_pos_initialized = false;
        }
        else if (params.p_perf_ovly_x && params.p_perf_ovly_y)
        {
            float custom_x = static_cast<float>(params.p_perf_ovly_x->get_value());
            float custom_y = static_cast<float>(params.p_perf_ovly_y->get_value());
            
            if (!custom_pos_initialized && custom_x >= 0.0f && custom_y >= 0.0f)
            {
                ImGui::SetNextWindowPos(ImVec2(custom_x, custom_y), ImGuiCond_Always);
                custom_pos_initialized = true;
            }
        }
        
        ImGui::SetNextWindowBgAlpha(0.35f);
        
        std::string title_perf = std::string(get_cop_string(lbl_performance_overlay, lang)) + "###PerfOverlay";
        if (ImGui::Begin(title_perf.c_str(), nullptr, overlay_flags))
        {
            if (location == -1 && params.p_perf_ovly_x && params.p_perf_ovly_y)
            {
                static bool is_dragging = false;
                if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                {
                    is_dragging = true;
                }
                if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
                {
                    is_dragging = false;
                }

                if (is_dragging)
                {
                    ImVec2 pos = ImGui::GetWindowPos();
                    ImVec2 delta = ImGui::GetIO().MouseDelta;
                    ImGui::SetWindowPos(ImVec2(pos.x + delta.x, pos.y + delta.y));
                }

                ImVec2 pos = ImGui::GetWindowPos();
                if (pos.x != static_cast<float>(params.p_perf_ovly_x->get_value()) || 
                    pos.y != static_cast<float>(params.p_perf_ovly_y->get_value()))
                {
                    params.p_perf_ovly_x->set_value(static_cast<double>(pos.x));
                    params.p_perf_ovly_y->set_value(static_cast<double>(pos.y));
                }
            }

            int content = params.p_perf_ovly_content ? static_cast<int>(params.p_perf_ovly_content->get_value()) : 7;
            if ((content & 1) && params.p_gui_mode && params.p_gui_mode->get_value() == 1)
            {
                ImGui::Text("%s %.1f (%.3f ms)", get_cop_string(lbl_fps, lang), ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
            }
            if (content & 2)
            {
                ImGui::Text(get_cop_string(lbl_cpu, lang), adam::os::get_cpu_usage());
            }
            if (content & 4)
            {
                float used_mb, avail_mb;
                adam::os::get_ram_usage_mb(used_mb, avail_mb);
                float used_gb = used_mb / 1024.0f;
                float total_gb = (used_mb + avail_mb) / 1024.0f;
                float percentage = (used_mb + avail_mb > 0.0f) ? (used_mb / (used_mb + avail_mb) * 100.0f) : 0.0f;
                ImGui::Text(get_cop_string(lbl_ram, lang), used_gb, total_gb, percentage);
            }

            if (ImGui::BeginPopupContextWindow("PerformanceOverlayPopup"))
            {
                if (params.p_perf_ovly_location && ImGui::BeginMenu(get_cop_string(menu_overlay_position, lang)))
                {
                    auto update_loc = [&](int loc) 
                    { 
                        params.p_perf_ovly_location->set_value(static_cast<int64_t>(loc)); 
                    };
                    if (ImGui::MenuItem(get_cop_string(menu_overlay_custom, lang),       nullptr, location == -1)) update_loc(-1);
                    if (ImGui::MenuItem(get_cop_string(menu_overlay_top_left, lang),     nullptr, location == 0)) update_loc(0);
                    if (ImGui::MenuItem(get_cop_string(menu_overlay_top_right, lang),    nullptr, location == 1)) update_loc(1);
                    if (ImGui::MenuItem(get_cop_string(menu_overlay_bottom_left, lang),  nullptr, location == 2)) update_loc(2);
                    if (ImGui::MenuItem(get_cop_string(menu_overlay_bottom_right, lang), nullptr, location == 3)) update_loc(3);
                    ImGui::EndMenu();
                }
                if (params.p_perf_ovly_content && ImGui::BeginMenu(get_cop_string(menu_overlay_content, lang)))
                {
                    bool show_fps = (content & 1) != 0;
                    bool show_cpu = (content & 2) != 0;
                    bool show_ram = (content & 4) != 0;

                    if (ImGui::MenuItem(get_cop_string(menu_overlay_show_fps, lang), nullptr, &show_fps))
                    {
                        params.p_perf_ovly_content->set_value((content & ~1) | (show_fps ? 1 : 0));
                    }
                    if (ImGui::MenuItem(get_cop_string(menu_overlay_show_cpu, lang), nullptr, &show_cpu))
                    {
                        params.p_perf_ovly_content->set_value((content & ~2) | (show_cpu ? 2 : 0));
                    }
                    if (ImGui::MenuItem(get_cop_string(menu_overlay_show_ram, lang), nullptr, &show_ram))
                    {
                        params.p_perf_ovly_content->set_value((content & ~4) | (show_ram ? 4 : 0));
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndPopup();
            }
        }
        ImGui::End();
    }
}
