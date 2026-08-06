#include "main-window.hpp"
#include <renderer-setup.hpp>


#include <imgui.h>
#include <imgui_internal.h>
#include <SDL3/SDL.h>
#include <unordered_map>
#include <array>
#include <algorithm>
#include "window-management.hpp"
#include "window-modules.hpp"
#include "window-about.hpp"
#include "window-configuration.hpp"
#include "window-analysis.hpp"
#include "window-log.hpp"
#include "management/inspector.hpp"
#include "main-window.hpp"
#include "../gui-strings.hpp"
#include <imgui-tools.hpp>

namespace adam::gui 
{
    inspection_data g_inspection_data;
    ImFont* g_mono_font = nullptr;


    const ImVec4& get_gui_color(gui_color_id id)
    {
        static const std::array<ImVec4, static_cast<size_t>(gui_color_id::count)> colors =
        {
            ImColor(0x26, 0xA6, 0x26), // #26A626 commander_connected
            ImColor(0xD9, 0x33, 0x33), // #D93333 commander_disconnected
            ImColor(0x33, 0x99, 0xD9), // #3399D9 log_trace
            ImColor(0x26, 0xA6, 0x26), // #26A626 log_info
            ImColor(0xD9, 0x99, 0x26), // #D99926 log_warning
            ImColor(0xD9, 0x33, 0x33),  // #D93333 log_error
            ImColor(0x26, 0x76, 0xA6, 0xDC), // #2676A6 node_input
            ImColor(0xA6, 0x76, 0x26, 0xDC), // #A67626 node_processor
            ImColor(0xA6, 0x26, 0x26, 0xDC), // #A62626 node_output
            ImColor(0xC8, 0xC8, 0xC8, 0xFF), // #C8C8C8 node_connection_line
            ImColor(0xFF, 0xFF, 0xFF, 0xFF), // #FFFFFF node_connection_line_light (bright white)
            ImColor(0x46, 0x46, 0x46, 0x50), // #464646 node_connection_line_invalid
            ImColor(0xFF, 0xFF, 0xFF, 0x50), // #FFFFFF node_connection_line_invalid_light
            ImColor(0x33, 0x33, 0x33, 0xFF), // #333333 node_connection_card_bg
            ImColor(0xAD, 0xAD, 0xAD, 0xFF), // #adadadff node_connection_card_bg_light
            ImColor(0x1F, 0x2A, 0x3A, 0xFF), // #1F2A3A node_connection_card_bg_dark_navy
            ImColor(0x33, 0x33, 0x33, 0xE6), // #333333 node_connection_card_bg_drag_preview
            ImColor(0x28, 0xBC, 0x28, 0xFF)  // #28bc28 node_pin_active (Darker green, 100% Alpha)
        };
        return colors[static_cast<size_t>(id)];
    }

    void get_search_bar_layout(adam::language lang, float window_avail_x, float& out_pos_x, float& out_width)
    {
        float pad_x = ImGui::GetStyle().WindowPadding.x;
        float spacing_x = ImGui::GetStyle().ItemSpacing.x;
        float border = ImGui::GetStyle().WindowBorderSize;

        float card_avail_x = window_avail_x - pad_x * 2.0f - border * 2.0f;
        float char_width = ImGui::CalcTextSize("x").x;
        float desired_node_w = char_width * 40.0f + ImGui::GetStyle().FramePadding.x * 4.0f;
        
        float port_w = std::min(desired_node_w, card_avail_x * 0.25f);
        if (port_w < char_width * 15.0f) port_w = char_width * 15.0f; 
        
        float color_w = ImGui::GetFrameHeight();
        float btn_start_w = ImGui::CalcTextSize(get_gui_string(gui_string_id::btn_start, lang)).x + ImGui::GetStyle().FramePadding.x * 2.0f;
        float btn_stop_w = ImGui::CalcTextSize(get_gui_string(gui_string_id::btn_stop, lang)).x + ImGui::GetStyle().FramePadding.x * 2.0f;
        float btn_delete_w = ImGui::CalcTextSize(get_gui_string(gui_string_id::btn_delete, lang)).x + ImGui::GetStyle().FramePadding.x * 2.0f;
        float btn_add_port_w = ImGui::GetFrameHeight();

        float total_controls_w = color_w + spacing_x + port_w;
        total_controls_w += spacing_x + btn_start_w + spacing_x + btn_stop_w;
        total_controls_w += spacing_x + btn_delete_w;
        total_controls_w += spacing_x + btn_add_port_w;

        float start_mid_x = pad_x + (card_avail_x - total_controls_w) * 0.5f;
        float min_start_x = pad_x + port_w + spacing_x;
        if (start_mid_x < min_start_x) start_mid_x = min_start_x;

        out_pos_x = ImGui::GetCursorPosX() + start_mid_x;
        out_width = total_controls_w;
    }

    
    main_window::main_window(gui_controller& ctrl, SDL_Window* window) 
        : m_ctrl(ctrl),
          m_window(window)
    {
        auto& params = m_ctrl.get_parameters();

        m_p_show_log            = static_cast<adam::configuration_parameter_boolean*>(params.get("show_log"_ct));
        m_p_show_inspector      = static_cast<adam::configuration_parameter_boolean*>(params.get("show_inspector"_ct));
        m_p_show_performance    = static_cast<adam::configuration_parameter_boolean*>(params.get("show_performance"_ct));
        m_p_gui_mode            = static_cast<adam::configuration_parameter_integer*>(params.get("gui_mode"_ct));
        m_p_fps_limit           = static_cast<adam::configuration_parameter_integer*>(params.get("fps_limit"_ct));
        m_p_perf_ovly_location  = static_cast<adam::configuration_parameter_integer*>(params.get("perf_ovly_location"_ct));
        m_p_perf_ovly_x         = static_cast<adam::configuration_parameter_double*>(params.get("perf_ovly_x"_ct));
        m_p_perf_ovly_y         = static_cast<adam::configuration_parameter_double*>(params.get("perf_ovly_y"_ct));
        m_p_perf_ovly_content   = static_cast<adam::configuration_parameter_integer*>(params.get("perf_ovly_content"_ct));
        m_p_theme               = static_cast<adam::configuration_parameter_string*>(params.get("theme"_ct));
        m_p_docking_layout      = static_cast<adam::configuration_parameter_string*>(params.get("docking_layout"_ct));
        m_p_font_scale          = static_cast<adam::configuration_parameter_double*>(params.get("font_scale"_ct));
        m_p_log_height          = static_cast<adam::configuration_parameter_double*>(params.get("log_height"_ct));
        m_p_log_level           = static_cast<adam::configuration_parameter_integer*>(params.get("log_level"_ct));
        m_p_language            = static_cast<adam::configuration_parameter_integer*>(params.get("language"_ct));

        if (m_p_docking_layout && !m_p_docking_layout->get_value().empty())
        {
            std::string layout_data = m_p_docking_layout->get_value().data();
            ImGui::LoadIniSettingsFromMemory(layout_data.c_str(), layout_data.size());
        }
        
        m_last_lang         = static_cast<adam::language>(255);
        m_modules_was_empty = true;
        m_log_was_empty     = true;
        m_module_paths_was_empty = true;
        m_modules_table_id  = 0;
        m_log_table_id      = 0;
        m_module_paths_table_id = 0;

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
            SDL_DisplayID display_id = SDL_GetDisplayForWindow(m_window);
            if (display_id == 0) display_id = SDL_GetPrimaryDisplay();
            
            if (display_id != 0)
            {
                SDL_Rect bounds;
                if (SDL_GetDisplayUsableBounds(display_id, &bounds) == 0)
                {
                    if (w > bounds.w) w = bounds.w;
                    if (h > bounds.h) h = bounds.h;
                }
            }
            SDL_SetWindowSize(m_window, w, h);
        }
            
        if (maximized)
            SDL_MaximizeWindow(m_window);

        ImGui::GetIO().FontGlobalScale = static_cast<float>(m_p_font_scale->get_value()) * adam::imgui_tools::get_current_dpi_scale();
                adam::imgui_tools::gui_theme active_theme = adam::imgui_tools::parse_theme(m_p_theme->get_value().get_hash());
        adam::imgui_tools::apply_theme(active_theme);

        // Initialize VSync swap interval
        if (m_p_gui_mode->get_value() == 1)
        {
            SDL_GL_SetSwapInterval((m_p_fps_limit->get_value() == 4) ? 1 : 0);
        }
        else
        {
            SDL_GL_SetSwapInterval(1);
        }
    }

    main_window::~main_window()
    {
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
        adam::language lang;
        if (m_ctrl.is_commander_active())
        {
            lang = m_ctrl.get_commander().get_language();
            m_p_language->set_value(static_cast<int64_t>(lang)); // Sync local language with commander
        }
        else
        {
            lang = static_cast<adam::language>(m_p_language->get_value());
        }
        
        if (m_last_lang != lang)
        {
            m_modules_table_id++;
            m_log_table_id++;
            m_module_paths_table_id++;
            m_last_lang = lang;
        }
        
        bool modules_empty = true;
        bool paths_empty = true;
        if (m_ctrl.is_commander_active())
        {
            modules_empty = m_ctrl.get_commander().get_modules().get_available().empty() &&
                            m_ctrl.get_commander().get_modules().get_loaded().empty() &&
                            m_ctrl.get_commander().get_modules().get_unavailable().empty();
            paths_empty = m_ctrl.get_commander().get_modules().get_paths().empty();
        }
        
        if (m_modules_was_empty && !modules_empty)
        {
            m_modules_table_id++;
            m_modules_was_empty = false;
        }
        else if (!m_modules_was_empty && modules_empty)
            m_modules_was_empty = true;
            
        bool log_empty = m_ctrl.is_log_history_empty();
        if (m_log_was_empty && !log_empty)
        {
            m_log_table_id++;
            m_log_was_empty = false;
        }
        else if (!m_log_was_empty && log_empty)
            m_log_was_empty = true;
            
        if (m_module_paths_was_empty && !paths_empty)
        {
            m_module_paths_table_id++;
            m_module_paths_was_empty = false;
        }
        else if (!m_module_paths_was_empty && paths_empty)
            m_module_paths_was_empty = true;

        //ImGui::ShowDemoWindow();

        if (ImGui::BeginMainMenuBar())
        {
            draw_menu_bar(lang);
            ImGui::EndMainMenuBar();
        }

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        float bar_height = ImGui::GetFrameHeight() + ImGui::GetStyle().FramePadding.y * 2.0f;
        
        ImGuiWindowFlags dock_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, viewport->WorkSize.y - bar_height));
        ImGui::SetNextWindowViewport(viewport->ID);
        
        ImGui::PushStyleColor(ImGuiCol_TitleBg, ImGui::GetStyleColorVec4(ImGuiCol_TitleBgActive));
        ImGui::PushStyleColor(ImGuiCol_TabUnfocused, ImGui::GetStyleColorVec4(ImGuiCol_Tab));
        ImGui::PushStyleColor(ImGuiCol_TabUnfocusedActive, ImGui::GetStyleColorVec4(ImGuiCol_TabActive));
        ImGui::PushStyleColor(ImGuiCol_TabDimmed, ImGui::GetStyleColorVec4(ImGuiCol_Tab));
        ImGui::PushStyleColor(ImGuiCol_TabDimmedSelected, ImGui::GetStyleColorVec4(ImGuiCol_TabSelected));
        ImGui::PushStyleColor(ImGuiCol_TabDimmedSelectedOverline, ImGui::GetStyleColorVec4(ImGuiCol_TabSelectedOverline));

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("MainDockSpaceWindow", nullptr, dock_flags);
        ImGui::PopStyleVar(3);
        
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        
        // Build default docking layout if no node layout exists yet
        if (ImGui::DockBuilderGetNode(dockspace_id) == nullptr)
        {
            ImGui::DockBuilderRemoveNode(dockspace_id);
            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, ImVec2(viewport->WorkSize.x, viewport->WorkSize.y - bar_height));

            ImGuiID dock_main_id = dockspace_id;
            ImGuiID dock_bottom_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.25f, nullptr, &dock_main_id);

            ImGui::DockBuilderDockWindow(get_gui_string(gui_string_id::wnd_management, lang), dock_main_id);
            ImGui::DockBuilderDockWindow(get_gui_string(gui_string_id::wnd_analysis, lang), dock_main_id);
            ImGui::DockBuilderDockWindow(get_gui_string(gui_string_id::wnd_configuration, lang), dock_main_id);
            ImGui::DockBuilderDockWindow(get_gui_string(gui_string_id::wnd_modules, lang), dock_main_id);
            ImGui::DockBuilderDockWindow(get_gui_string(gui_string_id::wnd_log_console, lang), dock_bottom_id);

            ImGui::DockBuilderFinish(dockspace_id);
        }
        
        ImGui::Spacing();

        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f));
        ImGui::End();

        if (g_request_open_inspector)
        {
            m_p_show_inspector->set_value(true);
            g_request_open_inspector = false;
        }

        ImGuiWindowClass no_close_tab_class;
        no_close_tab_class.DockNodeFlagsOverrideSet = (ImGuiDockNodeFlags)(1 << 15);
        no_close_tab_class.TabItemFlagsOverrideSet = (ImGuiTabItemFlags)(1 << 20);

        bool open_management = true;
        ImGui::SetNextWindowClass(&no_close_tab_class);
        if (ImGui::Begin(get_gui_string(gui_string_id::wnd_management, lang), &open_management, ImGuiWindowFlags_NoCollapse))
        {
            draw_window_management(m_ctrl, lang);
        }
        ImGui::End();
        if (!open_management) ImGui::DockBuilderDockWindow(get_gui_string(gui_string_id::wnd_management, lang), dockspace_id);

        bool open_analysis = true;
        ImGui::SetNextWindowClass(&no_close_tab_class);
        if (ImGui::Begin(get_gui_string(gui_string_id::wnd_analysis, lang), &open_analysis, ImGuiWindowFlags_NoCollapse))
        {
            draw_window_analysis(m_ctrl, lang);
        }
        ImGui::End();
        if (!open_analysis) ImGui::DockBuilderDockWindow(get_gui_string(gui_string_id::wnd_analysis, lang), dockspace_id);

        bool open_configuration = true;
        ImGui::SetNextWindowClass(&no_close_tab_class);
        if (ImGui::Begin(get_gui_string(gui_string_id::wnd_configuration, lang), &open_configuration, ImGuiWindowFlags_NoCollapse))
        {
            draw_window_configuration(m_ctrl, lang);
        }
        ImGui::End();
        if (!open_configuration) ImGui::DockBuilderDockWindow(get_gui_string(gui_string_id::wnd_configuration, lang), dockspace_id);

        bool open_modules = true;
        ImGui::SetNextWindowClass(&no_close_tab_class);
        if (ImGui::Begin(get_gui_string(gui_string_id::wnd_modules, lang), &open_modules, ImGuiWindowFlags_NoCollapse))
        {
            draw_window_modules(m_ctrl, lang, m_module_paths_table_id, m_modules_table_id);
        }
        ImGui::End();
        if (!open_modules) ImGui::DockBuilderDockWindow(get_gui_string(gui_string_id::wnd_modules, lang), dockspace_id);

        if (m_show_about)
        {
            draw_about_dialog(m_ctrl, lang, m_show_about);
        }

        if (m_p_show_log->get_value())
        {
            draw_window_log(m_ctrl, lang, m_log_table_id);
        }

        draw_dockable_inspector_windows(m_ctrl, lang);

        ImGui::PopStyleColor(6);

        // Fixed status bar at bottom of viewport
        {
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
                    ImGui::TextColored(get_gui_color(gui_color_id::commander_connected), "%s", get_gui_string(gui_string_id::lbl_commander_connected, lang));
                else
                    ImGui::TextColored(get_gui_color(gui_color_id::commander_disconnected), "%s", get_gui_string(gui_string_id::lbl_commander_disconnected, lang));
                    
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

        if (m_p_show_performance->get_value())
        {
            draw_performance_overlay(lang);
        }
    }

    void main_window::draw_menu_bar(adam::language lang)
    {
        if (ImGui::BeginMenu(get_gui_string(gui_string_id::menu_view, lang)))
        {
            ImGui::MenuItem(get_gui_string(gui_string_id::menu_show_log, lang), nullptr, &m_p_show_log->value());
            ImGui::MenuItem(get_gui_string(gui_string_id::menu_show_performance, lang), nullptr, &m_p_show_performance->value());
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(get_gui_string(gui_string_id::menu_settings, lang)))
        {
            const char* preview_mode = (m_p_gui_mode->get_value() == 1) ? get_gui_string(gui_string_id::gui_mode_immediate, lang) : get_gui_string(gui_string_id::gui_mode_default, lang);
            if (ImGui::BeginCombo(get_gui_string(gui_string_id::menu_gui_mode, lang), preview_mode))
            {
                if (ImGui::Selectable(get_gui_string(gui_string_id::gui_mode_default, lang), m_p_gui_mode->get_value() == 0))
                {
                    m_p_gui_mode->set_value(0);
                    SDL_GL_SetSwapInterval(1);
                }
                if (ImGui::Selectable(get_gui_string(gui_string_id::gui_mode_immediate, lang), m_p_gui_mode->get_value() == 1))
                {
                    m_p_gui_mode->set_value(1);
                    SDL_GL_SetSwapInterval((m_p_fps_limit->get_value() == 4) ? 1 : 0);
                    m_ctrl.request_redraw();
                }
                ImGui::EndCombo();
            }

            if (m_p_gui_mode->get_value() == 1)
            {
                int current_fps_limit = static_cast<int>(m_p_fps_limit->get_value());
                const char* preview_fps = "";
                switch (current_fps_limit) {
                    case 0: preview_fps = get_gui_string(gui_string_id::fps_10, lang); break;
                    case 1: preview_fps = get_gui_string(gui_string_id::fps_30, lang); break;
                    case 2: preview_fps = get_gui_string(gui_string_id::fps_60, lang); break;
                    case 3: preview_fps = get_gui_string(gui_string_id::fps_120, lang); break;
                    case 4: preview_fps = get_gui_string(gui_string_id::fps_vsync, lang); break;
                    case 5: preview_fps = get_gui_string(gui_string_id::fps_unlimited, lang); break;
                }

                if (ImGui::BeginCombo(get_gui_string(gui_string_id::menu_fps_limit, lang), preview_fps))
                {
                    auto do_selectable = [&](int val, gui_string_id id) {
                        if (ImGui::Selectable(get_gui_string(id, lang), current_fps_limit == val))
                        {
                            m_p_fps_limit->set_value(val);
                            SDL_GL_SetSwapInterval((val == 4) ? 1 : 0);
                        }
                    };
                    
                    do_selectable(0, gui_string_id::fps_10);
                    do_selectable(1, gui_string_id::fps_30);
                    do_selectable(2, gui_string_id::fps_60);
                    do_selectable(3, gui_string_id::fps_120);
                    do_selectable(4, gui_string_id::fps_vsync);
                    do_selectable(5, gui_string_id::fps_unlimited);
                    
                    ImGui::EndCombo();
                }
            }
            
            ImGui::Separator();
            if (ImGui::BeginCombo(get_gui_string(gui_string_id::combo_language, lang), adam::language_strings::language_name(lang, lang).data()))
            {
                uint64_t available_langs = m_ctrl.is_commander_active() ? m_ctrl.get_commander().get_available_languages() : ((1ULL << static_cast<int>(adam::language_english)) | (1ULL << static_cast<int>(adam::language_german)));
                
                for (int i = 0; i < static_cast<int>(adam::languages_count); ++i)
                {
                    if (!(available_langs & (1ULL << i)))
                        continue;
                        
                    adam::language avail_lang = static_cast<adam::language>(i);
                    bool is_selected = (lang == avail_lang);
                    if (ImGui::Selectable(adam::language_strings::language_name(avail_lang, lang).data(), is_selected))
                    {
                        if (m_ctrl.is_commander_active())
                            m_ctrl.commander().request_language_change(avail_lang);
                            
                        m_p_language->set_value(static_cast<int64_t>(avail_lang));
                    }
                    if (is_selected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            float font_scale_val = static_cast<float>(m_p_font_scale->get_value());
            if (ImGui::SliderFloat(get_gui_string(gui_string_id::slider_font_scale, lang), &font_scale_val, 0.5f, 3.0f, "%.2f"))
            {
                m_p_font_scale->set_value(static_cast<double>(font_scale_val));
            }
            if (ImGui::IsItemDeactivatedAfterEdit())
            {
                ImGui::GetIO().FontGlobalScale = static_cast<float>(m_p_font_scale->get_value()) * adam::imgui_tools::get_current_dpi_scale();
            }

            ImGui::Separator();
            adam::string_hashed current_theme_str = m_p_theme->get_value();
            adam::imgui_tools::gui_theme current_theme = adam::imgui_tools::parse_theme(current_theme_str.get_hash());
            
            auto get_theme_str_id = [](adam::imgui_tools::gui_theme t) {
                switch (t) {
                    case adam::imgui_tools::gui_theme::light: return gui_string_id::theme_light;
                    case adam::imgui_tools::gui_theme::dark_navy: return gui_string_id::theme_dark_navy;
                    case adam::imgui_tools::gui_theme::dark: default: return gui_string_id::theme_dark;
                }
            };
            
            const char* preview_value = get_gui_string(get_theme_str_id(current_theme), lang);
            
            if (ImGui::BeginCombo(get_gui_string(gui_string_id::combo_theme, lang), preview_value))
            {
                for (std::size_t i = 0; i < adam::imgui_tools::c_themes_count; ++i)
                {
                    auto theme_val = static_cast<adam::imgui_tools::gui_theme>(i);
                    bool is_selected = (current_theme == theme_val);
                    
                    if (ImGui::Selectable(get_gui_string(get_theme_str_id(theme_val), lang), is_selected))
                    {
                        m_p_theme->set_value(adam::imgui_tools::theme_to_string(theme_val));
                        adam::imgui_tools::apply_theme(theme_val);
                    }
                    if (is_selected) ImGui::SetItemDefaultFocus();
                }
                
                ImGui::EndCombo();
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu(get_gui_string(gui_string_id::menu_help, lang)))
        {
            if (ImGui::MenuItem(get_gui_string(gui_string_id::wnd_about, lang)))
            {
                m_show_about = true;
            }
            ImGui::EndMenu();
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
        
        if (ImGui::Begin(get_gui_string(gui_string_id::lbl_performance_overlay, lang), nullptr, overlay_flags))
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
            if ((content & 1) && m_p_gui_mode->get_value() == 1) ImGui::Text(get_gui_string(gui_string_id::lbl_fps, lang), ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
            if (content & 2) ImGui::Text(get_gui_string(gui_string_id::lbl_cpu, lang), adam::os::get_cpu_usage());
            if (content & 4)
            {
                float used_mb, avail_mb;
                adam::os::get_ram_usage_mb(used_mb, avail_mb);
                float used_gb = used_mb / 1024.0f;
                float total_gb = (used_mb + avail_mb) / 1024.0f;
                float percentage = (used_mb + avail_mb > 0.0f) ? (used_mb / (used_mb + avail_mb) * 100.0f) : 0.0f;
                ImGui::Text(get_gui_string(gui_string_id::lbl_ram, lang), used_gb, total_gb, percentage);
            }

            if (ImGui::BeginPopupContextWindow("PerformanceOverlayPopup"))
            {
                if (ImGui::BeginMenu(get_gui_string(gui_string_id::menu_overlay_position, lang)))
                {
                    auto update_loc = [&](int loc) { m_p_perf_ovly_location->set_value(static_cast<int64_t>(loc)); };
                    if (ImGui::MenuItem(get_gui_string(gui_string_id::menu_overlay_custom, lang),       nullptr, location == -1)) update_loc(-1);
                    if (ImGui::MenuItem(get_gui_string(gui_string_id::menu_overlay_top_left, lang),     nullptr, location == 0)) update_loc(0);
                    if (ImGui::MenuItem(get_gui_string(gui_string_id::menu_overlay_top_right, lang),    nullptr, location == 1)) update_loc(1);
                    if (ImGui::MenuItem(get_gui_string(gui_string_id::menu_overlay_bottom_left, lang),  nullptr, location == 2)) update_loc(2);
                    if (ImGui::MenuItem(get_gui_string(gui_string_id::menu_overlay_bottom_right, lang), nullptr, location == 3)) update_loc(3);
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu(get_gui_string(gui_string_id::menu_overlay_content, lang)))
                {
                    bool show_fps = (content & 1) != 0, show_cpu = (content & 2) != 0, show_ram = (content & 4) != 0;
                    if (ImGui::MenuItem(get_gui_string(gui_string_id::menu_overlay_show_fps, lang), nullptr, &show_fps)) m_p_perf_ovly_content->set_value((content & ~1) | (show_fps ? 1 : 0));
                    if (ImGui::MenuItem(get_gui_string(gui_string_id::menu_overlay_show_cpu, lang), nullptr, &show_cpu)) m_p_perf_ovly_content->set_value((content & ~2) | (show_cpu ? 2 : 0));
                    if (ImGui::MenuItem(get_gui_string(gui_string_id::menu_overlay_show_ram, lang), nullptr, &show_ram)) m_p_perf_ovly_content->set_value((content & ~4) | (show_ram ? 4 : 0));
                    ImGui::EndMenu();
                }
                ImGui::EndPopup();
            }
        }
        ImGui::End();
    }

    int draw_modal_buttons(const std::vector<modal_button>& buttons)
    {
        int clicked_index = -1;
        if (buttons.empty()) return clicked_index;

        float available_width = ImGui::GetContentRegionAvail().x;
        float spacing = ImGui::GetStyle().ItemSpacing.x;
        float button_width = (available_width - (buttons.size() - 1) * spacing) / static_cast<float>(buttons.size());

        for (size_t i = 0; i < buttons.size(); ++i)
        {
            if (i > 0)
                ImGui::SameLine();
            
            if (buttons[i].disabled)
                ImGui::BeginDisabled();
                
            if (ImGui::Button(buttons[i].label, ImVec2(button_width, 0.0f)))
            {
                clicked_index = static_cast<int>(i);
            }
            
            if (buttons[i].disabled)
                ImGui::EndDisabled();
        }
        return clicked_index;
    }

}