/**
 * @file    main-window.cpp
 * @author  dexus1337
 * @brief   Operator interface main window implementation for adam-cop
 * @version 1.0
 * @date    05.08.2026
 */

#include "main-window.hpp"
#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui-tools.hpp>
#include <cstdio>
#include <SDL3/SDL_opengl.h>
#include "../setup.hpp"
#include <vector>
#include <array>
#include <os/os.hpp>
#include <version/version.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace adam::string_hashed_ct_literals;

namespace adam::cop
{
    const ImVec4& get_cop_color(cop_color_id id)
    {
        static const std::array<ImVec4, static_cast<size_t>(cop_color_id::count)> colors =
        {
            ImColor(0x26, 0xA6, 0x26), // #26A626 commander_connected
            ImColor(0xD9, 0x33, 0x33)  // #D93333 commander_disconnected
        };
        return colors[static_cast<size_t>(id)];
    }

    main_window::main_window(cop_controller& ctrl, SDL_Window* window)
        : m_ctrl(ctrl)
        , m_window(window)
        , m_last_lang(static_cast<adam::language>(255))
    {
        auto& params = m_ctrl.get_parameters();
        m_p_base_provider  = dynamic_cast<adam::configuration_parameter_integer*>(params.get("base_provider"_ct));
        m_p_map_opacity    = dynamic_cast<adam::configuration_parameter_double*>(params.get("map_opacity"_ct));
        m_p_map_projection = dynamic_cast<adam::configuration_parameter_integer*>(params.get("map_projection"_ct));
        m_p_show_grid      = dynamic_cast<adam::configuration_parameter_boolean*>(params.get("show_grid"_ct));
        m_p_show_coastlines = dynamic_cast<adam::configuration_parameter_boolean*>(params.get("show_coastlines"_ct));
        m_p_show_land_fill  = dynamic_cast<adam::configuration_parameter_boolean*>(params.get("show_land_fill"_ct));
        m_p_show_scale_bar  = dynamic_cast<adam::configuration_parameter_boolean*>(params.get("show_scale_bar"_ct));
        m_p_show_performance = dynamic_cast<adam::configuration_parameter_boolean*>(params.get("show_performance"_ct));
        m_p_perf_ovly_location = dynamic_cast<adam::configuration_parameter_integer*>(params.get("perf_ovly_location"_ct));
        m_p_perf_ovly_x      = dynamic_cast<adam::configuration_parameter_double*>(params.get("perf_ovly_x"_ct));
        m_p_perf_ovly_y      = dynamic_cast<adam::configuration_parameter_double*>(params.get("perf_ovly_y"_ct));
        m_p_perf_ovly_content= dynamic_cast<adam::configuration_parameter_integer*>(params.get("perf_ovly_content"_ct));
        m_p_fps_limit       = dynamic_cast<adam::configuration_parameter_integer*>(params.get("fps_limit"_ct));
        m_p_language        = dynamic_cast<adam::configuration_parameter_integer*>(params.get("language"_ct));
        m_p_theme          = dynamic_cast<adam::configuration_parameter_string*>(params.get("theme"_ct));
        m_p_gui_mode       = dynamic_cast<adam::configuration_parameter_integer*>(params.get("gui_mode"_ct));
        m_p_font_scale     = dynamic_cast<adam::configuration_parameter_double*>(params.get("font_scale"_ct));
        m_p_docking_layout = dynamic_cast<adam::configuration_parameter_string*>(params.get("docking_layout"_ct));
        
        if (m_p_docking_layout && !m_p_docking_layout->get_value().empty())
        {
            std::string layout_data = m_p_docking_layout->get_value().data();
            ImGui::LoadIniSettingsFromMemory(layout_data.c_str(), layout_data.size());
        }

        int x = static_cast<adam::configuration_parameter_integer*>(params.get("window_x"_ct))->get_value_as<int>();
        int y = static_cast<adam::configuration_parameter_integer*>(params.get("window_y"_ct))->get_value_as<int>();
        int w = static_cast<adam::configuration_parameter_integer*>(params.get("window_w"_ct))->get_value_as<int>();
        int h = static_cast<adam::configuration_parameter_integer*>(params.get("window_h"_ct))->get_value_as<int>();
        bool maximized = static_cast<adam::configuration_parameter_boolean*>(params.get("window_maximized"_ct))->get_value();
        if (x != -1 && y != -1)
        {
            bool pos_valid = false;
            int num_displays = 0;
            SDL_DisplayID* displays = SDL_GetDisplays(&num_displays);
            if (displays)
            {
                for (int i = 0; i < num_displays; ++i)
                {
                    SDL_Rect bounds;
                    if (SDL_GetDisplayBounds(displays[i], &bounds) == 0)
                    {
                        int center_x = x + w / 2;
                        int center_y = y + h / 2;
                        if (center_x >= bounds.x && center_x < bounds.x + bounds.w &&
                            center_y >= bounds.y && center_y < bounds.y + bounds.h)
                        {
                            pos_valid = true;
                            break;
                        }
                    }
                }
                SDL_free(displays);
            }
            
            if (!pos_valid)
            {
                x = SDL_WINDOWPOS_CENTERED;
                y = SDL_WINDOWPOS_CENTERED;
            }
            
            SDL_SetWindowPosition(m_window, x, y);
        }
            
        if (w > 0 && h > 0)
        {
            SDL_SetWindowSize(m_window, w, h);
        }
            
        if (maximized)
        {
            SDL_MaximizeWindow(m_window);
        }
        
        if (m_p_gui_mode && m_p_gui_mode->get_value() == 1)
        {
            SDL_GL_SetSwapInterval((m_p_fps_limit && m_p_fps_limit->get_value() == 4) ? 1 : 0);
        }
        else
        {
            SDL_GL_SetSwapInterval(1);
        }
    }

    void main_window::save_window_state()
    {
        if (m_window)
        {
            int x, y, w, h;
            SDL_GetWindowPosition(m_window, &x, &y);
            SDL_GetWindowSize(m_window, &w, &h);
            SDL_WindowFlags flags = SDL_GetWindowFlags(m_window);
            auto& params = m_ctrl.get_parameters();
            bool is_maximized = (flags & SDL_WINDOW_MAXIMIZED) != 0;
            bool is_minimized = (flags & SDL_WINDOW_MINIMIZED) != 0;
            bool is_fullscreen = (flags & SDL_WINDOW_FULLSCREEN) != 0;

            if (!is_maximized && !is_minimized && !is_fullscreen)
            {
                static_cast<adam::configuration_parameter_integer*>(params.get("window_x"_ct))->set_value(static_cast<int64_t>(x));
                static_cast<adam::configuration_parameter_integer*>(params.get("window_y"_ct))->set_value(static_cast<int64_t>(y));
                static_cast<adam::configuration_parameter_integer*>(params.get("window_w"_ct))->set_value(static_cast<int64_t>(w));
                static_cast<adam::configuration_parameter_integer*>(params.get("window_h"_ct))->set_value(static_cast<int64_t>(h));
            }
            
            if (!is_minimized)
            {
                static_cast<adam::configuration_parameter_boolean*>(params.get("window_maximized"_ct))->set_value(is_maximized);
            }

            size_t ini_size = 0;
            const char* ini_data = ImGui::SaveIniSettingsToMemory(&ini_size);
            if (ini_data && ini_size > 0 && m_p_docking_layout)
            {
                m_p_docking_layout->set_value(adam::string_hashed(std::string_view(ini_data, ini_size)));
            }
        }
    }

    void main_window::draw()
    {
        adam::language lang = adam::language_english;
        if (m_p_language)
        {
            lang = static_cast<adam::language>(m_p_language->get_value());
        }
        if (m_last_lang != lang)
        {
            m_last_lang = lang;
            SDL_SetWindowTitle(m_window, get_cop_string(cop_main_title, lang));
        }
        
        adam::imgui_tools::gui_theme active_theme = adam::imgui_tools::parse_theme(m_p_theme->get_value().get_hash());
        adam::imgui_tools::apply_theme(active_theme);

        if (m_p_font_scale)
        {
            ImGui::GetIO().FontGlobalScale = static_cast<float>(m_p_font_scale->get_value()) * adam::cop::get_current_dpi_scale();
        }

        float status_bar_height = ImGui::GetFrameHeight() + ImGui::GetStyle().FramePadding.y * 2.0f;
        
        // Setup Main Dockspace Viewport
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, viewport->WorkSize.y - status_bar_height));
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiWindowFlags host_flags = ImGuiWindowFlags_MenuBar |
                                      ImGuiWindowFlags_NoTitleBar |
                                      ImGuiWindowFlags_NoCollapse |
                                      ImGuiWindowFlags_NoResize |
                                      ImGuiWindowFlags_NoMove |
                                      ImGuiWindowFlags_NoBringToFrontOnFocus |
                                      ImGuiWindowFlags_NoNavFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGui::Begin("COP_Dockspace_Host", nullptr, host_flags);
        ImGui::PopStyleVar(3);

        ImGuiID dockspace_id = ImGui::GetID("COP_Dockspace");
        
        if (ImGui::DockBuilderGetNode(dockspace_id) == nullptr)
        {
            ImGui::DockBuilderRemoveNode(dockspace_id);
            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, ImVec2(viewport->WorkSize.x, viewport->WorkSize.y - status_bar_height));

            ImGuiID dock_main_id = dockspace_id;
            ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, nullptr, &dock_main_id);

            // Need a dummy string before ### to dock correctly if we don't know the localized prefix
            ImGui::DockBuilderDockWindow("Settings###ControlPanel", dock_id_left);
            ImGui::DockBuilderDockWindow("World Map Overview###MapWindow", dock_main_id);
            ImGui::DockBuilderFinish(dockspace_id);
        }

        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

        draw_menu_bar(lang);

        if (m_show_control_panel)
        {
            draw_control_panel(lang);
        }

        draw_map_window(lang);

        if (m_show_about)
        {
            draw_about_dialog(lang);
        }

        ImGui::End();
        
        draw_status_bar(lang);
        
        if (m_p_show_performance && m_p_show_performance->get_value())
        {
            draw_performance_overlay(lang);
        }
    }

    void main_window::draw_menu_bar(adam::language lang)
    {
        if (!ImGui::BeginMenuBar())
        {
            return;
        }

        if (ImGui::BeginMenu(get_cop_string(menu_file, lang)))
        {
            if (ImGui::MenuItem(get_cop_string(menu_exit, lang), "Alt+F4"))
            {
                SDL_Event quit_ev;
                SDL_zerop(&quit_ev);
                quit_ev.type = SDL_EVENT_QUIT;
                SDL_PushEvent(&quit_ev);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(get_cop_string(menu_view, lang)))
        {
            ImGui::MenuItem(get_cop_string(lbl_layers_panel, lang), nullptr, &m_show_control_panel);

            if (m_p_show_performance)
            {
                bool show_perf = m_p_show_performance->get_value();
                if (ImGui::MenuItem("Performance Overlay", nullptr, &show_perf))
                {
                    m_p_show_performance->set_value(show_perf);
                }
            }
            
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(get_cop_string(menu_settings, lang)))
        {
            const char* preview_mode = (m_p_gui_mode && m_p_gui_mode->get_value() == 1) ? get_cop_string(gui_mode_immediate, lang) : get_cop_string(gui_mode_default, lang);
            if (ImGui::BeginCombo(get_cop_string(menu_gui_mode, lang), preview_mode))
            {
                if (ImGui::Selectable(get_cop_string(gui_mode_default, lang), m_p_gui_mode && m_p_gui_mode->get_value() == 0))
                {
                    if (m_p_gui_mode) m_p_gui_mode->set_value(0);
                    SDL_GL_SetSwapInterval(1);
                }
                if (ImGui::Selectable(get_cop_string(gui_mode_immediate, lang), m_p_gui_mode && m_p_gui_mode->get_value() == 1))
                {
                    if (m_p_gui_mode) m_p_gui_mode->set_value(1);
                    SDL_GL_SetSwapInterval((m_p_fps_limit && m_p_fps_limit->get_value() == 4) ? 1 : 0);
                    m_ctrl.request_redraw();
                }
                ImGui::EndCombo();
            }

            if (m_p_gui_mode && m_p_gui_mode->get_value() == 1 && m_p_fps_limit)
            {
                int current_fps_limit = static_cast<int>(m_p_fps_limit->get_value());
                const char* preview_fps = "";
                switch (current_fps_limit) {
                    case 0: preview_fps = get_cop_string(fps_10, lang); break;
                    case 1: preview_fps = get_cop_string(fps_30, lang); break;
                    case 2: preview_fps = get_cop_string(fps_60, lang); break;
                    case 3: preview_fps = get_cop_string(fps_120, lang); break;
                    case 4: preview_fps = get_cop_string(fps_vsync, lang); break;
                    case 5: preview_fps = get_cop_string(fps_unlimited, lang); break;
                }

                if (ImGui::BeginCombo(get_cop_string(menu_fps_limit, lang), preview_fps))
                {
                    auto do_selectable = [&](int val, cop_string_id id) {
                        if (ImGui::Selectable(get_cop_string(id, lang), current_fps_limit == val))
                        {
                            m_p_fps_limit->set_value(val);
                            SDL_GL_SetSwapInterval((val == 4) ? 1 : 0);
                        }
                    };
                    
                    do_selectable(0, fps_10);
                    do_selectable(1, fps_30);
                    do_selectable(2, fps_60);
                    do_selectable(3, fps_120);
                    do_selectable(4, fps_vsync);
                    do_selectable(5, fps_unlimited);
                    
                    ImGui::EndCombo();
                }
            }
            
            ImGui::Separator();
            if (m_p_language)
            {
                if (ImGui::BeginCombo(get_cop_string(combo_language, lang), adam::language_strings::language_name(lang, lang).data()))
                {
                    uint64_t available_langs = ((1ULL << static_cast<int>(adam::language_english)) | (1ULL << static_cast<int>(adam::language_german)));
                    
                    for (int i = 0; i < static_cast<int>(adam::languages_count); ++i)
                    {
                        if (!(available_langs & (1ULL << i)))
                            continue;
                            
                        adam::language avail_lang = static_cast<adam::language>(i);
                        bool is_selected = (lang == avail_lang);
                        if (ImGui::Selectable(adam::language_strings::language_name(avail_lang, lang).data(), is_selected))
                        {
                            m_p_language->set_value(static_cast<int64_t>(avail_lang));
                        }
                        if (is_selected)
                        {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }
            }

            if (m_p_font_scale)
            {
                float font_scale_val = static_cast<float>(m_p_font_scale->get_value());
                if (ImGui::SliderFloat(get_cop_string(slider_font_scale, lang), &font_scale_val, 0.5f, 3.0f, "%.2f"))
                {
                    m_p_font_scale->set_value(static_cast<double>(font_scale_val));
                }
                if (ImGui::IsItemDeactivatedAfterEdit())
                {
                    ImGui::GetIO().FontGlobalScale = static_cast<float>(m_p_font_scale->get_value()) * adam::cop::get_current_dpi_scale();
                }
            }

            ImGui::Separator();
            if (m_p_theme)
            {
                adam::string_hashed current_theme_str = m_p_theme->get_value();
                adam::imgui_tools::gui_theme current_theme = adam::imgui_tools::parse_theme(current_theme_str.get_hash());
                
                auto get_theme_str_id = [](adam::imgui_tools::gui_theme t) {
                    switch (t) {
                        case adam::imgui_tools::gui_theme::light: return theme_light;
                        case adam::imgui_tools::gui_theme::dark_navy: return theme_dark_navy;
                        case adam::imgui_tools::gui_theme::dark: default: return theme_dark;
                    }
                };
                
                const char* preview_value = get_cop_string(get_theme_str_id(current_theme), lang);
                
                if (ImGui::BeginCombo(get_cop_string(combo_theme, lang), preview_value))
                {
                    for (std::size_t i = 0; i < adam::imgui_tools::c_themes_count; ++i)
                    {
                        auto theme_val = static_cast<adam::imgui_tools::gui_theme>(i);
                        bool is_selected = (current_theme == theme_val);
                        
                        if (ImGui::Selectable(get_cop_string(get_theme_str_id(theme_val), lang), is_selected))
                        {
                            m_p_theme->set_value(adam::imgui_tools::theme_to_string(theme_val));
                            adam::imgui_tools::apply_theme(theme_val);
                        }
                        if (is_selected) ImGui::SetItemDefaultFocus();
                    }
                    
                    ImGui::EndCombo();
                }
            }
            
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(get_cop_string(menu_map, lang)))
        {
            if (ImGui::MenuItem(get_cop_string(menu_reset_view, lang)))
            {
                m_map.reset_view();
            }

            if (ImGui::MenuItem(get_cop_string(menu_center_origin, lang)))
            {
                m_map.set_center(0.0f, 0.0f);
            }

            if (ImGui::MenuItem(get_cop_string(btn_clear_markers, lang)))
            {
                m_map.clear_markers();
            }

            ImGui::Separator();

            if (m_p_map_projection)
            {
                int current_proj = static_cast<int>(m_p_map_projection->get_value());
                if (ImGui::RadioButton(get_cop_string(proj_equirectangular, lang), &current_proj, 0))
                {
                    m_p_map_projection->set_value(0);
                }
                if (ImGui::RadioButton(get_cop_string(proj_mercator, lang), &current_proj, 1))
                {
                    m_p_map_projection->set_value(1);
                }
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(get_cop_string(menu_help, lang)))
        {
            if (ImGui::MenuItem(get_cop_string(wnd_about, lang)))
            {
                m_show_about = true;
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    void main_window::draw_status_bar(adam::language lang)
    {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGuiWindowFlags status_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                                        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | 
                                        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | 
                                        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoDocking;

        float status_bar_height = ImGui::GetFrameHeight() + ImGui::GetStyle().FramePadding.y * 2.0f;
        ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + viewport->WorkSize.y - status_bar_height));
        ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, status_bar_height));

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(ImGui::GetStyle().WindowPadding.x, ImGui::GetStyle().FramePadding.y));

        if (ImGui::Begin("##MainStatusBar", nullptr, status_flags))
        {
            ImGui::Separator();
            if (m_ctrl.is_commander_active())
                ImGui::TextColored(get_cop_color(cop_color_id::commander_connected), "%s", get_cop_string(lbl_status_online, lang));
            else
                ImGui::TextColored(get_cop_color(cop_color_id::commander_disconnected), "%s", get_cop_string(lbl_status_offline, lang));

            ImGui::SameLine(0, 20.0f);
            if (m_map.is_hovered())
            {
                ImGui::Text("%s Lat: %.4f°, Lon: %.4f°",
                    get_cop_string(lbl_cursor_coords, lang),
                    m_map.get_hover_lat(),
                    m_map.get_hover_lon()
                );
            }
            else
            {
                ImGui::Text("%s --", get_cop_string(lbl_cursor_coords, lang));
            }

            ImGui::SameLine(0, 20.0f);
            ImGui::Text("%s Lat: %.4f°, Lon: %.4f°",
                get_cop_string(lbl_map_center, lang),
                m_map.get_center_lat(),
                m_map.get_center_lon()
            );

            ImGui::SameLine(0, 20.0f);
            ImGui::Text("%s %.1fx", get_cop_string(lbl_zoom_level, lang), m_map.get_zoom());
            
            const char* lang_short = (lang == adam::language_german) ? "DE" : "EN";
            float lang_width = ImGui::CalcTextSize(lang_short).x;
            ImGui::SameLine(ImGui::GetWindowWidth() - lang_width - ImGui::GetStyle().WindowPadding.x * 2.0f);
            ImGui::TextUnformatted(lang_short);
            if (ImGui::IsItemHovered())
                ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            if (ImGui::IsItemClicked())
            {
                adam::language new_lang = (lang == adam::language_english) ? adam::language_german : adam::language_english;
                if (m_ctrl.is_commander_active())
                    m_ctrl.commander().request_language_change(new_lang);
                m_p_language->set_value(static_cast<int64_t>(new_lang));
            }
        }
        ImGui::End();
        ImGui::PopStyleVar(3);
    }

    void main_window::draw_control_panel(adam::language lang)
    {
        ImGui::SetNextWindowSize(ImVec2(340.0f, 550.0f), ImGuiCond_FirstUseEver);

        std::string panel_title = std::string(get_cop_string(lbl_layers_panel, lang)) + "###ControlPanel";
        if (!ImGui::Begin(panel_title.c_str(), &m_show_control_panel))
        {
            ImGui::End();
            return;
        }

        if (ImGui::CollapsingHeader(get_cop_string(lbl_base_map_layer, lang), ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (m_p_base_provider)
            {
                int current_prov = static_cast<int>(m_p_base_provider->get_value());
                const char* items[] = {
                    get_cop_string(provider_cartodb, lang),
                    get_cop_string(provider_osm, lang),
                    get_cop_string(provider_esri, lang),
                    get_cop_string(provider_opentopo, lang),
                    get_cop_string(provider_vector, lang)
                };

                if (ImGui::Combo("Map Provider", &current_prov, items, IM_ARRAYSIZE(items)))
                {
                    m_p_base_provider->set_value(current_prov);
                }
            }

            if (m_p_map_opacity)
            {
                float opacity = static_cast<float>(m_p_map_opacity->get_value());
                if (ImGui::SliderFloat(get_cop_string(lbl_map_opacity, lang), &opacity, 0.1f, 1.0f, "%.2f"))
                {
                    m_p_map_opacity->set_value(static_cast<double>(opacity));
                }
            }
        }

        if (ImGui::CollapsingHeader(get_cop_string(lbl_projection, lang), ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (m_p_map_projection)
            {
                int current_proj = static_cast<int>(m_p_map_projection->get_value());
                if (ImGui::RadioButton(get_cop_string(proj_mercator, lang), current_proj == 1))
                {
                    m_p_map_projection->set_value(1);
                }
                if (ImGui::RadioButton(get_cop_string(proj_equirectangular, lang), current_proj == 0))
                {
                    m_p_map_projection->set_value(0);
                }
            }
        }

        if (ImGui::CollapsingHeader("Overlays & Tactical Graphics", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (m_p_show_grid)
            {
                bool v = m_p_show_grid->get_value();
                if (ImGui::Checkbox(get_cop_string(lbl_grid_toggle, lang), &v))
                {
                    m_p_show_grid->set_value(v);
                }
            }

            if (m_p_show_coastlines)
            {
                bool v = m_p_show_coastlines->get_value();
                if (ImGui::Checkbox(get_cop_string(lbl_coastlines_toggle, lang), &v))
                {
                    m_p_show_coastlines->set_value(v);
                }
            }

            if (m_p_show_scale_bar)
            {
                bool v = m_p_show_scale_bar->get_value();
                if (ImGui::Checkbox(get_cop_string(lbl_scale_bar_toggle, lang), &v))
                {
                    m_p_show_scale_bar->set_value(v);
                }
            }

            ImGui::Checkbox(get_cop_string(lbl_compass_toggle, lang), &m_map_options.show_compass);
        }

        if (ImGui::CollapsingHeader(get_cop_string(lbl_placed_markers, lang), ImGuiTreeNodeFlags_DefaultOpen))
        {
            const auto& markers = m_map.get_markers();
            if (markers.empty())
            {
                ImGui::TextDisabled("%s", get_cop_string(lbl_no_markers, lang));
            }
            else
            {
                for (const auto& m : markers)
                {
                    ImGui::PushID(static_cast<int>(m.id));
                    ImGui::BulletText("%s: Lat %.4f°, Lon %.4f°", m.label.c_str(), m.lat, m.lon);

                    ImGui::SameLine(ImGui::GetWindowWidth() - 70.0f);
                    if (ImGui::SmallButton("Jump"))
                    {
                        m_map.set_center(m.lat, m.lon);
                    }

                    ImGui::SameLine();
                    if (ImGui::SmallButton("X"))
                    {
                        m_map.remove_marker(m.id);
                        ImGui::PopID();
                        break;
                    }
                    ImGui::PopID();
                }

                ImGui::Spacing();
                if (ImGui::Button(get_cop_string(btn_clear_markers, lang), ImVec2(-1.0f, 0.0f)))
                {
                    m_map.clear_markers();
                }
            }
        }

        if (ImGui::CollapsingHeader(get_cop_string(lbl_cache_stats, lang)))
        {
            auto& engine = m_map.get_tile_engine();
            ImGui::Text("GPU Loaded Textures: %zu", engine.get_loaded_texture_count());
            ImGui::Text("Pending HTTP Requests: %zu", engine.get_pending_request_count());

            if (ImGui::Button(get_cop_string(btn_clear_tile_cache, lang), ImVec2(-1.0f, 0.0f)))
            {
                engine.clear_cache();
            }
        }

        if (ImGui::CollapsingHeader(get_cop_string(lbl_jump_to_coordinates, lang)))
        {
            ImGui::InputFloat(get_cop_string(lbl_lat, lang), &m_jump_lat, 1.0f, 5.0f, "%.4f");
            ImGui::InputFloat(get_cop_string(lbl_lon, lang), &m_jump_lon, 1.0f, 5.0f, "%.4f");

            if (ImGui::Button(get_cop_string(btn_jump, lang), ImVec2(-1.0f, 0.0f)))
            {
                m_map.set_center(m_jump_lat, m_jump_lon);
            }

            if (ImGui::Button(get_cop_string(btn_reset_camera, lang), ImVec2(-1.0f, 0.0f)))
            {
                m_map.reset_view();
            }
        }

        if (ImGui::CollapsingHeader(get_cop_string(wnd_asterix_connections, lang), ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::TextDisabled("No active ASTERIX data feeds connected.");
            ImGui::TextWrapped("Select ASTERIX radar sources in adam to begin multi-sensor air object tracking.");
        }

        ImGui::End();
    }

    void main_window::draw_map_window(adam::language lang)
    {
        (void)lang;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGui::Begin("World Map Overview###MapWindow", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        ImVec2 avail = ImGui::GetContentRegionAvail();

        m_map_options.base_provider = m_p_base_provider ? static_cast<tile_provider_type>(m_p_base_provider->get_value()) : tile_provider_type::cartodb_dark;
        m_map_options.map_opacity = m_p_map_opacity ? static_cast<float>(m_p_map_opacity->get_value()) : 1.0f;
        m_map_options.projection = (m_p_map_projection && m_p_map_projection->get_value() == 1)
                                   ? projection_type::mercator
                                   : projection_type::equirectangular;
        m_map_options.show_grid = m_p_show_grid ? m_p_show_grid->get_value() : true;
        m_map_options.show_coastlines = m_p_show_coastlines ? m_p_show_coastlines->get_value() : true;
        m_map_options.show_scale_bar = m_p_show_scale_bar ? m_p_show_scale_bar->get_value() : true;
        m_map_options.show_markers = true;

        m_map.draw(avail, m_map_options);

        ImGui::End();
        ImGui::PopStyleVar();
    }

#ifdef _WIN32
    static ImTextureID g_logo_texture = (ImTextureID)0;
    static int g_logo_width = 0;
    static int g_logo_height = 0;

    static void load_logo_texture_once()
    {
        if (g_logo_texture != (ImTextureID)0) return;

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

        for (int i = 0; i < g_logo_width * g_logo_height; i++)
        {
            uint8_t b = pixels[i * 4 + 0];
            uint8_t r = pixels[i * 4 + 2];
            pixels[i * 4 + 0] = r;
            pixels[i * 4 + 2] = b;
        }

        g_logo_texture = adam::imgui_tools::create_texture_rgba(g_logo_width, g_logo_height, pixels.data());

        DeleteObject(ii.hbmColor);
        DeleteObject(ii.hbmMask);
        ReleaseDC(NULL, hdc);
        DestroyIcon(hIcon);
    }
#endif

    void main_window::draw_about_dialog(adam::language lang)
    {
#ifdef _WIN32
        load_logo_texture_once();
#endif

        const char* title_text = "ADAM COP";
        
        char version_text[128];
        auto ver = adam::decode_version(adam::sdk_version);
        snprintf(version_text, sizeof(version_text), "v%d.%d.%d", 
                 ver.major, ver.minor, ver.patch);

        const char* desc_text = get_cop_string(msg_cop_description, lang);
        const char* cpy1_text = "© 2026 dexus1337.";
        const char* cpy2_text = "All rights reserved.";

        ImGui::OpenPopup(get_cop_string(wnd_about, lang));

        ImGui::SetNextWindowSize(ImVec2(600, 480), ImGuiCond_FirstUseEver);
        if (ImGui::BeginPopupModal(get_cop_string(wnd_about, lang), &m_show_about, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
        {
            ImVec2 avail = ImGui::GetContentRegionAvail();
            float wrap_width = avail.x * 0.8f;
            if (wrap_width < 300.0f) wrap_width = avail.x;

            float item_spacing = ImGui::GetStyle().ItemSpacing.y;
            float large_spacing = item_spacing * 2.0f;

            float total_height = 0.0f;
            
            float logo_display_size = 96.0f * ImGui::GetStyle()._MainScale;
#ifdef _WIN32
            if (g_logo_texture) total_height += logo_display_size + item_spacing;
#endif

            ImVec2 title_size = ImVec2(ImGui::CalcTextSize(title_text).x * 2.0f, ImGui::CalcTextSize(title_text).y * 2.0f);
            ImVec2 version_size = ImGui::CalcTextSize(version_text);

            auto measure_or_draw_centered_text = [&](const char* text, float max_w, bool do_draw)
            {
                ImVec2 total_size(0.0f, 0.0f);
                const char* text_end = text + strlen(text);
                const char* s = text;
                while (s < text_end)
                {
                    const char* line_end = s;
                    while (line_end < text_end && *line_end != '\n') line_end++;
                    
                    const char* line_p = s;
                    while (line_p < line_end)
                    {
                        const char* word_end = line_p;
                        while (word_end < line_end && *word_end != ' ') word_end++;
                        
                        const char* next_word = word_end;
                        while (next_word < line_end && *next_word == ' ') next_word++;
                        
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
                m_show_about = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }

    void main_window::draw_performance_overlay(adam::language lang)
    {
        int location = static_cast<int>(m_p_perf_ovly_location->get_value());
        static bool custom_pos_initialized = false;
        
        ImGuiWindowFlags overlay_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;
        
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
        else
        {
            float custom_x = static_cast<float>(m_p_perf_ovly_x->get_value());
            float custom_y = static_cast<float>(m_p_perf_ovly_y->get_value());
            
            if (!custom_pos_initialized && custom_x >= 0.0f && custom_y >= 0.0f)
            {
                ImGui::SetNextWindowPos(ImVec2(custom_x, custom_y), ImGuiCond_Always);
                custom_pos_initialized = true;
            }
        }
        
        ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
        
        if (ImGui::Begin(get_cop_string(lbl_performance_overlay, lang), nullptr, overlay_flags))
        {
            if (location == -1)
            {
                static bool is_dragging = false;
                if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                    is_dragging = true;
                if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
                    is_dragging = false;

                if (is_dragging)
                {
                    ImVec2 pos = ImGui::GetWindowPos();
                    ImVec2 delta = ImGui::GetIO().MouseDelta;
                    ImGui::SetWindowPos(ImVec2(pos.x + delta.x, pos.y + delta.y));
                }

                ImVec2 pos = ImGui::GetWindowPos();
                if (pos.x != static_cast<float>(m_p_perf_ovly_x->get_value()) || 
                    pos.y != static_cast<float>(m_p_perf_ovly_y->get_value()))
                {
                    m_p_perf_ovly_x->set_value(static_cast<double>(pos.x));
                    m_p_perf_ovly_y->set_value(static_cast<double>(pos.y));
                }
            }

            int content = static_cast<int>(m_p_perf_ovly_content->get_value());
            if ((content & 1) && m_p_gui_mode && m_p_gui_mode->get_value() == 1) ImGui::Text(get_cop_string(lbl_fps, lang), ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
            if (content & 2) ImGui::Text(get_cop_string(lbl_cpu, lang), adam::os::get_cpu_usage());
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
                if (ImGui::BeginMenu(get_cop_string(menu_overlay_position, lang)))
                {
                    auto update_loc = [&](int loc) { m_p_perf_ovly_location->set_value(static_cast<int64_t>(loc)); };
                    if (ImGui::MenuItem(get_cop_string(menu_overlay_custom, lang),       nullptr, location == -1)) update_loc(-1);
                    if (ImGui::MenuItem(get_cop_string(menu_overlay_top_left, lang),     nullptr, location == 0)) update_loc(0);
                    if (ImGui::MenuItem(get_cop_string(menu_overlay_top_right, lang),    nullptr, location == 1)) update_loc(1);
                    if (ImGui::MenuItem(get_cop_string(menu_overlay_bottom_left, lang),  nullptr, location == 2)) update_loc(2);
                    if (ImGui::MenuItem(get_cop_string(menu_overlay_bottom_right, lang), nullptr, location == 3)) update_loc(3);
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu(get_cop_string(menu_overlay_content, lang)))
                {
                    bool show_fps = (content & 1) != 0, show_cpu = (content & 2) != 0, show_ram = (content & 4) != 0;
                    if (ImGui::MenuItem(get_cop_string(menu_overlay_show_fps, lang), nullptr, &show_fps)) m_p_perf_ovly_content->set_value((content & ~1) | (show_fps ? 1 : 0));
                    if (ImGui::MenuItem(get_cop_string(menu_overlay_show_cpu, lang), nullptr, &show_cpu)) m_p_perf_ovly_content->set_value((content & ~2) | (show_cpu ? 2 : 0));
                    if (ImGui::MenuItem(get_cop_string(menu_overlay_show_ram, lang), nullptr, &show_ram)) m_p_perf_ovly_content->set_value((content & ~4) | (show_ram ? 4 : 0));
                    ImGui::EndMenu();
                }
                ImGui::EndPopup();
            }
        }
        ImGui::End();
    }
}
