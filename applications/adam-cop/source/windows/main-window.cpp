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
#include <cstdio>
#include <SDL3/SDL_opengl.h>
#include <renderer-setup.hpp>
#include <vector>
#include <array>
#include <algorithm>
#include <os/os.hpp>
#include <version/version.hpp>
#include <lib-imgui.hpp>

using namespace adam::string_hashed_ct_literals;

namespace adam::cop
{
    const ImVec4& get_cop_color(cop_color_id id)
    {
        static const std::array<ImVec4, static_cast<size_t>(cop_color_id::count)> colors =
        {
            ImColor(0x26, 0xA6, 0x26),       // #26A626 commander_connected
            ImColor(0xD9, 0x33, 0x33),       // #D93333 commander_disconnected
            ImColor(0xC8, 0xC8, 0xC8, 0xFF), // #C8C8C8 node_connection_line
            ImColor(0xFF, 0xFF, 0xFF, 0xFF), // #FFFFFF node_connection_line_light
            ImColor(0x28, 0xBC, 0x28, 0xFF)  // #28BC28 node_pin_active
        };
        return colors[static_cast<size_t>(id)];
    }

    main_window::main_window(cop_controller& ctrl, SDL_Window* window)
        : m_ctrl(ctrl)
        , m_window(window)
        , m_last_lang(static_cast<adam::language>(255))
    {
        auto& params = m_ctrl.get_parameters();

        m_p_map_projection     = params.get<adam::configuration_parameter_integer>("map_projection"_ct);
        m_p_show_grid          = params.get<adam::configuration_parameter_boolean>("show_grid"_ct);
        m_p_show_coastlines    = params.get<adam::configuration_parameter_boolean>("show_coastlines"_ct);
        m_p_show_land_fill     = params.get<adam::configuration_parameter_boolean>("show_land_fill"_ct);
        m_p_show_scale_bar     = params.get<adam::configuration_parameter_boolean>("show_scale_bar"_ct);
        m_p_show_performance   = params.get<adam::configuration_parameter_boolean>("show_performance"_ct);
        m_p_perf_ovly_location = params.get<adam::configuration_parameter_integer>("perf_ovly_location"_ct);
        m_p_perf_ovly_x        = params.get<adam::configuration_parameter_double>("perf_ovly_x"_ct);
        m_p_perf_ovly_y        = params.get<adam::configuration_parameter_double>("perf_ovly_y"_ct);
        m_p_perf_ovly_content  = params.get<adam::configuration_parameter_integer>("perf_ovly_content"_ct);
        m_p_fps_limit          = params.get<adam::configuration_parameter_integer>("fps_limit"_ct);
        m_p_language           = params.get<adam::configuration_parameter_integer>("language"_ct);
        m_p_theme              = params.get<adam::configuration_parameter_string>("theme"_ct);
        m_p_gui_mode           = params.get<adam::configuration_parameter_integer>("gui_mode"_ct);
        m_p_font_scale         = params.get<adam::configuration_parameter_double>("font_scale"_ct);
        m_p_docking_layout     = params.get<adam::configuration_parameter_string>("docking_layout"_ct);
        m_p_map_lat            = params.get<adam::configuration_parameter_double>("map_lat"_ct);
        m_p_map_lon            = params.get<adam::configuration_parameter_double>("map_lon"_ct);
        m_p_map_zoom           = params.get<adam::configuration_parameter_double>("map_zoom"_ct);

        m_p_show_control_panel = params.get<adam::configuration_parameter_boolean>("show_control_panel"_ct);
        m_p_show_waypoints     = params.get<adam::configuration_parameter_boolean>("show_waypoints"_ct);
        m_p_show_sites         = params.get<adam::configuration_parameter_boolean>("show_sites"_ct);
        m_p_show_cache_stats   = params.get<adam::configuration_parameter_boolean>("show_cache_stats"_ct);
        m_p_show_jump_coords   = params.get<adam::configuration_parameter_boolean>("show_jump_coords"_ct);
        m_p_show_data_sources  = params.get<adam::configuration_parameter_boolean>("show_asterix"_ct);

        m_p_window_x           = params.get<adam::configuration_parameter_integer>("window_x"_ct);
        m_p_window_y           = params.get<adam::configuration_parameter_integer>("window_y"_ct);
        m_p_window_w           = params.get<adam::configuration_parameter_integer>("window_w"_ct);
        m_p_window_h           = params.get<adam::configuration_parameter_integer>("window_h"_ct);
        m_p_window_maximized   = params.get<adam::configuration_parameter_boolean>("window_maximized"_ct);
        
        if (m_p_docking_layout && !m_p_docking_layout->get_value().empty())
        {
            std::string layout_data = m_p_docking_layout->get_value().data();
            ImGui::LoadIniSettingsFromMemory(layout_data.c_str(), layout_data.size());
        }

        const adam::string_hashed param_names_provider[4] = { "map_layer_0_provider"_ct, "map_layer_1_provider"_ct, "map_layer_2_provider"_ct, "map_layer_3_provider"_ct };
        const adam::string_hashed param_names_opacity[4]  = { "map_layer_0_opacity"_ct,  "map_layer_1_opacity"_ct,  "map_layer_2_opacity"_ct,  "map_layer_3_opacity"_ct };
        const adam::string_hashed param_names_visible[4]  = { "map_layer_0_visible"_ct,  "map_layer_1_visible"_ct,  "map_layer_2_visible"_ct,  "map_layer_3_visible"_ct };

        for (int i = 0; i < 4; ++i)
        {
            m_p_map_layer_params[i].provider = dynamic_cast<adam::configuration_parameter_integer*>(params.get(param_names_provider[i]));
            m_p_map_layer_params[i].opacity  = dynamic_cast<adam::configuration_parameter_double*>(params.get(param_names_opacity[i]));
            m_p_map_layer_params[i].visible  = dynamic_cast<adam::configuration_parameter_boolean*>(params.get(param_names_visible[i]));

            if (m_p_map_layer_params[i].provider && m_p_map_layer_params[i].opacity && m_p_map_layer_params[i].visible)
            {
                int64_t provider_val = m_p_map_layer_params[i].provider->get_value();
                if (provider_val < 0 || provider_val > 3)
                {
                    provider_val = i;
                }
                m_map_options.tile_layers[i].provider = static_cast<tile_provider_type>(provider_val);
                m_map_options.tile_layers[i].opacity  = static_cast<float>(m_p_map_layer_params[i].opacity->get_value());
                m_map_options.tile_layers[i].visible  = m_p_map_layer_params[i].visible->get_value();
            }
            else
            {
                m_map_options.tile_layers[i] = { static_cast<tile_provider_type>(i), 1.0f, (i == 0) };
            }
        }
        
        if (m_p_map_lat && m_p_map_lon)
        {
            m_map.set_center(static_cast<float>(m_p_map_lat->get_value()), static_cast<float>(m_p_map_lon->get_value()));
        }

        if (m_p_map_zoom)
        {
            m_map.set_zoom(static_cast<float>(m_p_map_zoom->get_value()));
        }

        int x = m_p_window_x ? m_p_window_x->get_value_as<int>() : -1;
        int y = m_p_window_y ? m_p_window_y->get_value_as<int>() : -1;
        int w = m_p_window_w ? m_p_window_w->get_value_as<int>() : 1280;
        int h = m_p_window_h ? m_p_window_h->get_value_as<int>() : 720;
        bool maximized = m_p_window_maximized ? m_p_window_maximized->get_value() : false;
        
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
            
            if (pos_valid)
            {
                SDL_SetWindowPosition(m_window, x, y);
            }
            else
            {
                SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
            }
        }
        else
        {
            SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        }

        SDL_SetWindowSize(m_window, w, h);
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

        if (m_p_font_scale)
        {
            ImGui::GetIO().FontGlobalScale = static_cast<float>(m_p_font_scale->get_value()) * adam::lib::imgui::get_current_dpi_scale();
        }
    }

    void main_window::save_window_state()
    {
        int x, y, w, h;
        SDL_GetWindowPosition(m_window, &x, &y);
        SDL_GetWindowSize(m_window, &w, &h);
        SDL_WindowFlags flags = SDL_GetWindowFlags(m_window);
        bool is_maximized = (flags & SDL_WINDOW_MAXIMIZED) != 0;
        bool is_minimized = (flags & SDL_WINDOW_MINIMIZED) != 0;

        if (!is_maximized && !is_minimized)
        {
            if (m_p_window_x) m_p_window_x->set_value(x);
            if (m_p_window_y) m_p_window_y->set_value(y);
            if (m_p_window_w) m_p_window_w->set_value(w);
            if (m_p_window_h) m_p_window_h->set_value(h);
        }

        if (!is_minimized && m_p_window_maximized)
        {
            m_p_window_maximized->set_value(is_maximized);
        }

        size_t ini_size = 0;
        const char* ini_data = ImGui::SaveIniSettingsToMemory(&ini_size);
        if (ini_data && ini_size > 0 && m_p_docking_layout)
        {
            m_p_docking_layout->set_value(adam::string_hashed(std::string_view(ini_data, ini_size)));
        }

        if (m_p_map_lat)  m_p_map_lat->set_value(static_cast<double>(m_map.get_center_lat()));
        if (m_p_map_lon)  m_p_map_lon->set_value(static_cast<double>(m_map.get_center_lon()));
        if (m_p_map_zoom) m_p_map_zoom->set_value(static_cast<double>(m_map.get_zoom()));
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
        
        adam::lib::imgui::gui_theme active_theme = adam::lib::imgui::parse_theme(m_p_theme->get_value().get_hash());
        adam::lib::imgui::apply_theme(active_theme);

        if (m_p_font_scale)
        {
            ImGui::GetIO().FontGlobalScale = static_cast<float>(m_p_font_scale->get_value()) * adam::lib::imgui::get_current_dpi_scale();
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

        ImGuiID dockspace_id = ImGui::GetID("COP_Dockspace_v5");
        
        if (ImGui::DockBuilderGetNode(dockspace_id) == nullptr)
        {
            ImGui::DockBuilderRemoveNode(dockspace_id);
            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, ImVec2(viewport->WorkSize.x, viewport->WorkSize.y - status_bar_height));

            ImGuiID dock_main_id = dockspace_id;
            ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.25f, nullptr, &dock_main_id);
            
            ImGuiID dock_left_bottom = ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Down, 0.15f, nullptr, &dock_id_left);
            ImGuiID dock_left_mid3   = ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Down, 0.20f, nullptr, &dock_id_left);
            ImGuiID dock_left_mid2   = ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Down, 0.20f, nullptr, &dock_id_left);
            ImGuiID dock_left_mid1   = ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Down, 0.40f, nullptr, &dock_id_left);

            ImGui::DockBuilderDockWindow("World Overview###MapWindow", dock_main_id);
            ImGui::DockBuilderDockWindow("Layers###ControlPanel", dock_id_left);
            ImGui::DockBuilderDockWindow("Waypoints###Waypoints", dock_left_mid1);
            ImGui::DockBuilderDockWindow("Radar Sites###RadarSites", dock_left_mid1);
            ImGui::DockBuilderDockWindow("Cache###CacheStats", dock_left_mid2);
            ImGui::DockBuilderDockWindow("Asterix###Asterix", dock_left_mid3);
            ImGui::DockBuilderDockWindow("Jump###JumpCoords", dock_left_bottom);
            ImGui::DockBuilderFinish(dockspace_id);
        }

        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

        draw_menu_bar(lang);

        // 1. Layers & Controls Window
        layers_window_context layers_ctx = {
            m_map_options,
            m_p_map_layer_params,
            m_p_show_control_panel,
            m_p_map_projection,
            m_p_show_grid,
            m_p_show_coastlines,
            m_p_show_scale_bar
        };
        draw_layers_window(layers_ctx, lang);

        // 2. Waypoints Window
        draw_waypoints_window(m_ctrl, m_map, lang, m_p_show_waypoints);

        // 3. Radar Sites Window
        draw_radar_sites_window(m_ctrl, m_map, lang, m_p_show_sites, m_picking_site_coords_hash);

        // 4. Data Sources (ASTERIX) Window
        draw_data_sources_window(m_ctrl, lang, m_p_show_data_sources);

        // 5. Cache Stats Window
        draw_cache_stats_window(m_map, lang, m_p_show_cache_stats);

        // 6. Jump to Coordinates Window
        draw_jump_coords_window(m_map, lang, m_p_show_jump_coords, m_jump_lat, m_jump_lon);

        // 7. World Map Window
        draw_map_window(lang);

        // 8. About Dialog
        if (m_show_about)
        {
            draw_about_dialog(m_ctrl, lang, m_show_about);
        }

        ImGui::End();
        
        // 9. Status Bar
        draw_status_bar(lang);
        
        // 10. Performance Overlay HUD
        if (m_p_show_performance && m_p_show_performance->get_value())
        {
            perf_overlay_params perf_ctx = {
                m_p_show_performance,
                m_p_perf_ovly_location,
                m_p_perf_ovly_x,
                m_p_perf_ovly_y,
                m_p_perf_ovly_content,
                m_p_gui_mode
            };
            draw_performance_overlay(perf_ctx, lang);
        }
    }

    void main_window::draw_menu_bar(adam::language lang)
    {
        if (!ImGui::BeginMenuBar())
        {
            return;
        }

        if (ImGui::BeginMenu(get_cop_string(menu_view, lang)))
        {
            const auto& show_txt = std::string(get_cop_string(lbl_show, lang));

            std::string layers_str = show_txt + std::string(get_cop_string(lbl_layers_panel, lang));
            if (m_p_show_control_panel)
            {
                ImGui::MenuItem(layers_str.c_str(), nullptr, &m_p_show_control_panel->value());
            }

            std::string asterix_str = show_txt + std::string(get_cop_string(wnd_data_sources, lang));
            if (m_p_show_data_sources)
            {
                ImGui::MenuItem(asterix_str.c_str(), nullptr, &m_p_show_data_sources->value());
            }
            
            std::string sites_str = show_txt + std::string(get_cop_string(wnd_sites, lang));
            if (m_p_show_sites)
            {
                ImGui::MenuItem(sites_str.c_str(), nullptr, &m_p_show_sites->value());
            }
            
            std::string waypoints_str = show_txt + std::string(get_cop_string(wnd_waypoints, lang));
            if (m_p_show_waypoints)
            {
                ImGui::MenuItem(waypoints_str.c_str(), nullptr, &m_p_show_waypoints->value());
            }

            std::string cache_str = show_txt + std::string(get_cop_string(lbl_cache_stats, lang));
            if (m_p_show_cache_stats)
            {
                ImGui::MenuItem(cache_str.c_str(), nullptr, &m_p_show_cache_stats->value());
            }

            std::string jump_str = show_txt + std::string(get_cop_string(lbl_jump_to_coordinates, lang));
            if (m_p_show_jump_coords)
            {
                ImGui::MenuItem(jump_str.c_str(), nullptr, &m_p_show_jump_coords->value());
            }

            if (m_p_show_performance)
            {
                bool show_perf = m_p_show_performance->get_value();
                if (ImGui::MenuItem(get_cop_string(menu_show_performance, lang), nullptr, &show_perf))
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
                    if (m_p_gui_mode)
                    {
                        m_p_gui_mode->set_value(0);
                    }
                    SDL_GL_SetSwapInterval(1);
                }
                if (ImGui::Selectable(get_cop_string(gui_mode_immediate, lang), m_p_gui_mode && m_p_gui_mode->get_value() == 1))
                {
                    if (m_p_gui_mode)
                    {
                        m_p_gui_mode->set_value(1);
                    }
                    SDL_GL_SetSwapInterval((m_p_fps_limit && m_p_fps_limit->get_value() == 4) ? 1 : 0);
                    m_ctrl.request_redraw();
                }
                ImGui::EndCombo();
            }

            if (m_p_gui_mode && m_p_gui_mode->get_value() == 1 && m_p_fps_limit)
            {
                int current_fps_limit = static_cast<int>(m_p_fps_limit->get_value());
                const char* preview_fps = "";
                switch (current_fps_limit)
                {
                    case 0: preview_fps = get_cop_string(fps_10, lang); break;
                    case 1: preview_fps = get_cop_string(fps_30, lang); break;
                    case 2: preview_fps = get_cop_string(fps_60, lang); break;
                    case 3: preview_fps = get_cop_string(fps_120, lang); break;
                    case 4: preview_fps = get_cop_string(fps_vsync, lang); break;
                    case 5: preview_fps = get_cop_string(fps_unlimited, lang); break;
                }

                if (ImGui::BeginCombo(get_cop_string(menu_fps_limit, lang), preview_fps))
                {
                    auto do_selectable = [&](int val, cop_string_id id)
                    {
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

            if (m_p_language)
            {
                const char* current_lang_str = adam::language_strings::language_name(lang, lang).data();
                if (ImGui::BeginCombo(get_cop_string(combo_language, lang), current_lang_str))
                {
                    uint64_t available_langs = ((1ULL << static_cast<int>(adam::language_english)) | (1ULL << static_cast<int>(adam::language_german)));
                    
                    for (int i = 0; i < static_cast<int>(adam::languages_count); ++i)
                    {
                        if (!(available_langs & (1ULL << i)))
                        {
                            continue;
                        }
                            
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
                    ImGui::GetIO().FontGlobalScale = static_cast<float>(m_p_font_scale->get_value()) * adam::lib::imgui::get_current_dpi_scale();
                }
            }

            ImGui::Separator();
            if (m_p_theme)
            {
                adam::string_hashed current_theme_str = m_p_theme->get_value();
                adam::lib::imgui::gui_theme current_theme = adam::lib::imgui::parse_theme(current_theme_str.get_hash());
                
                auto get_theme_str_id = [](adam::lib::imgui::gui_theme t)
                {
                    switch (t)
                    {
                        case adam::lib::imgui::gui_theme::light: return theme_light;
                        case adam::lib::imgui::gui_theme::dark: return theme_dark;
                        case adam::lib::imgui::gui_theme::dark_navy: default: return theme_dark_navy;
                    }
                };
                
                const char* preview_value = get_cop_string(get_theme_str_id(current_theme), lang);
                
                if (ImGui::BeginCombo(get_cop_string(combo_theme, lang), preview_value))
                {
                    for (std::size_t i = 0; i < adam::lib::imgui::c_themes_count; ++i)
                    {
                        auto theme_val = static_cast<adam::lib::imgui::gui_theme>(i);
                        bool is_selected = (current_theme == theme_val);
                        
                        if (ImGui::Selectable(get_cop_string(get_theme_str_id(theme_val), lang), is_selected))
                        {
                            m_p_theme->set_value(adam::lib::imgui::theme_to_string(theme_val));
                            adam::lib::imgui::apply_theme(theme_val);
                        }
                        if (is_selected)
                        {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    
                    ImGui::EndCombo();
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
            {
                ImGui::TextColored(get_cop_color(cop_color_id::commander_connected), "%s", get_cop_string(lbl_status_online, lang));
            }
            else
            {
                ImGui::TextColored(get_cop_color(cop_color_id::commander_disconnected), "%s", get_cop_string(lbl_status_offline, lang));
            }

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
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            }
            if (ImGui::IsItemClicked())
            {
                adam::language new_lang = (lang == adam::language_english) ? adam::language_german : adam::language_english;
                if (m_ctrl.is_commander_active())
                {
                    m_ctrl.commander().request_language_change(new_lang);
                }
                m_p_language->set_value(static_cast<int64_t>(new_lang));
            }
        }
        ImGui::End();
        ImGui::PopStyleVar(3);
    }

    void main_window::draw_map_window(adam::language lang)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        std::string map_title = std::string(get_cop_string(wnd_world_map, lang)) + "###MapWindow";
        ImGui::Begin(map_title.c_str(), nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        ImVec2 avail = ImGui::GetContentRegionAvail();

        m_map_options.projection = (m_p_map_projection && m_p_map_projection->get_value() == 1)
                                   ? projection_type::mercator
                                   : projection_type::equirectangular;
        m_map_options.show_grid = m_p_show_grid ? m_p_show_grid->get_value() : true;
        m_map_options.show_coastlines = m_p_show_coastlines ? m_p_show_coastlines->get_value() : true;
        m_map_options.show_scale_bar = m_p_show_scale_bar ? m_p_show_scale_bar->get_value() : true;
        m_map_options.show_markers = true;

        m_map.draw(avail, m_map_options, m_ctrl.get_waypoints(), m_ctrl.get_sites(), get_cop_string(menu_add_waypoint_here, lang));

        float add_lat, add_lon;
        if (m_map.consume_context_add_waypoint_request(add_lat, add_lon))
        {
            static uint32_t auto_wp_id = 1;
            char tag[32];
            snprintf(tag, sizeof(tag), "WP #%u", auto_wp_id++);
            auto wp = std::make_unique<waypoint>(adam::string_hashed(std::string(tag)));
            wp->set_lat(add_lat);
            wp->set_lon(add_lon);
            wp->set_color(0x00D9FF);
            m_ctrl.add_waypoint(std::move(wp));
            if (m_p_show_waypoints)
            {
                m_p_show_waypoints->set_value(true);
            }
        }

        if (m_picking_site_coords_hash != 0)
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            ImGui::SetTooltip("%s", get_cop_string(lbl_picking_map_pos, lang));

            if (ImGui::IsKeyPressed(ImGuiKey_Escape) || (m_map.is_hovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)))
            {
                m_picking_site_coords_hash = 0;
            }
            else if (m_map.is_hovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                const auto& sites = m_ctrl.get_sites();
                for (const auto& s : sites)
                {
                    if (s && s->get_name().get_hash() == m_picking_site_coords_hash)
                    {
                        s->set_lat(m_map.get_hover_lat());
                        s->set_lon(m_map.get_hover_lon());
                        s->set_auto_retrieve_coords(false);
                        m_ctrl.save_config();
                        break;
                    }
                }
                m_picking_site_coords_hash = 0;
            }
        }

        ImGui::End();
        ImGui::PopStyleVar();
    }
}
