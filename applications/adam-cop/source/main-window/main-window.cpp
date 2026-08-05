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
#include <cstdio>

using namespace adam::string_hashed_ct_literals;

namespace adam::cop
{
    main_window::main_window(cop_controller& ctrl, SDL_Window* window)
        : m_ctrl(ctrl)
        , m_window(window)
        , m_last_lang(adam::language_english)
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
        m_p_fps_limit      = dynamic_cast<adam::configuration_parameter_integer*>(params.get("fps_limit"_ct));
        m_p_language       = dynamic_cast<adam::configuration_parameter_integer*>(params.get("language"_ct));
    }

    void main_window::save_window_state()
    {
        // Saved automatically via ImGui ini / parameter backend
    }

    void main_window::draw()
    {
        adam::language lang = adam::language_english;
        if (m_p_language)
        {
            lang = static_cast<adam::language>(m_p_language->get_value());
        }
        m_last_lang = lang;

        // Setup Main Dockspace Viewport
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
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
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

        draw_menu_bar(lang);
        draw_status_bar(lang);

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
        float height = 24.0f;
        ImVec2 status_pos = ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + viewport->WorkSize.y - height);
        ImVec2 status_size = ImVec2(viewport->WorkSize.x, height);

        ImGui::SetNextWindowPos(status_pos);
        ImGui::SetNextWindowSize(status_size);
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 2.0f));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.06f, 0.08f, 0.11f, 1.00f));

        if (ImGui::Begin("COP_StatusBar", nullptr, flags))
        {
            ImVec4 status_col = m_ctrl.is_commander_active() ? ImVec4(0.00f, 0.85f, 0.40f, 1.00f) : ImVec4(0.00f, 0.75f, 0.95f, 1.00f);
            ImGui::TextColored(status_col, "[%s]", get_cop_string(lbl_status_online, lang));

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

            ImGui::SameLine(ImGui::GetWindowWidth() - 100.0f);
            ImGui::Text("%s %.1f", get_cop_string(lbl_fps, lang), ImGui::GetIO().Framerate);

            ImGui::End();
        }

        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
    }

    void main_window::draw_control_panel(adam::language lang)
    {
        ImGui::SetNextWindowSize(ImVec2(340.0f, 550.0f), ImGuiCond_FirstUseEver);

        if (!ImGui::Begin(get_cop_string(lbl_layers_panel, lang), &m_show_control_panel))
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

        ImGui::Begin("World Map Overview", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

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

    void main_window::draw_about_dialog(adam::language lang)
    {
        ImGui::OpenPopup(get_cop_string(wnd_about, lang));

        if (ImGui::BeginPopupModal(get_cop_string(wnd_about, lang), &m_show_about, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::TextUnformatted("ADAM - COP");
            ImGui::Separator();
            ImGui::TextWrapped("%s", get_cop_string(msg_cop_description, lang));
            ImGui::Spacing();
            ImGui::TextDisabled("%s", get_cop_string(msg_cop_copyright, lang));
            ImGui::Spacing();

            if (ImGui::Button("OK", ImVec2(120.0f, 0.0f)))
            {
                m_show_about = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
}
