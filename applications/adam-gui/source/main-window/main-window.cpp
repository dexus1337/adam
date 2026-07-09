#include "main-window.hpp"
#include "../setup.hpp"


#include <imgui.h>
#include <SDL.h>
#include <unordered_map>
#include <array>
#include <algorithm>
#include "tabs/tab-management.hpp"
#include "tabs/tab-modules.hpp"
#include "tabs/tab-information.hpp"
#include "tabs/tab-configuration.hpp"

namespace adam::gui 
{
    inspection_data g_inspection_data;
    ImFont* g_mono_font = nullptr;

    namespace
    {
        void apply_theme(bool dark)
        {
            ImGuiStyle& style = ImGui::GetStyle();
            ImVec4* colors = style.Colors;

            if (dark)
            {
                ImGui::StyleColorsDark();
                colors[ImGuiCol_Text]                   = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
                colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
                colors[ImGuiCol_WindowBg]               = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
                colors[ImGuiCol_ChildBg]                = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
                colors[ImGuiCol_PopupBg]                = ImVec4(0.12f, 0.12f, 0.12f, 0.96f);
                colors[ImGuiCol_Border]                 = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
                colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
                colors[ImGuiCol_FrameBg]                = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
                colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
                colors[ImGuiCol_FrameBgActive]          = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
                colors[ImGuiCol_TitleBg]                = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
                colors[ImGuiCol_TitleBgActive]          = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
                colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
                colors[ImGuiCol_MenuBarBg]              = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
                colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
                colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
                colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
                colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
                colors[ImGuiCol_CheckMark]              = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
                colors[ImGuiCol_SliderGrab]             = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
                colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.46f, 0.79f, 0.98f, 1.00f);
                colors[ImGuiCol_Button]                 = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
                colors[ImGuiCol_ButtonHovered]          = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
                colors[ImGuiCol_ButtonActive]           = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
                colors[ImGuiCol_Header]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.50f);
                colors[ImGuiCol_HeaderHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
                colors[ImGuiCol_HeaderActive]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
                colors[ImGuiCol_Separator]              = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
                colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
                colors[ImGuiCol_SeparatorActive]        = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
                colors[ImGuiCol_ResizeGrip]             = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
                colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
                colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
                colors[ImGuiCol_Tab]                    = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
                colors[ImGuiCol_TabHovered]             = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
                colors[ImGuiCol_TabActive]              = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
                colors[ImGuiCol_TabUnfocused]           = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
                colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
                colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
                colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
                colors[ImGuiCol_TableBorderLight]       = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
                colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
                colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.03f);
            }
            else
            {
                ImGui::StyleColorsLight();
                colors[ImGuiCol_Text]                   = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
                colors[ImGuiCol_TextDisabled]           = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
                colors[ImGuiCol_WindowBg]               = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
                colors[ImGuiCol_ChildBg]                = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
                colors[ImGuiCol_PopupBg]                = ImVec4(1.00f, 1.00f, 1.00f, 0.98f);
                colors[ImGuiCol_Border]                 = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
                colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
                colors[ImGuiCol_FrameBg]                = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
                colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
                colors[ImGuiCol_FrameBgActive]          = ImVec4(0.92f, 0.92f, 0.92f, 1.00f);
                colors[ImGuiCol_TitleBg]                = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
                colors[ImGuiCol_TitleBgActive]          = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
                colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
                colors[ImGuiCol_MenuBarBg]              = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
                colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
                colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
                colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.65f, 0.65f, 0.65f, 1.00f);
                colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);
                colors[ImGuiCol_CheckMark]              = ImVec4(0.00f, 0.48f, 1.00f, 1.00f);
                colors[ImGuiCol_SliderGrab]             = ImVec4(0.00f, 0.48f, 1.00f, 1.00f);
                colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.00f, 0.35f, 0.85f, 1.00f);
                colors[ImGuiCol_Button]                 = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
                colors[ImGuiCol_ButtonHovered]          = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
                colors[ImGuiCol_ButtonActive]           = ImVec4(0.92f, 0.92f, 0.92f, 1.00f);
                colors[ImGuiCol_Header]                 = ImVec4(0.00f, 0.48f, 1.00f, 0.20f);
                colors[ImGuiCol_HeaderHovered]          = ImVec4(0.00f, 0.48f, 1.00f, 0.40f);
                colors[ImGuiCol_HeaderActive]           = ImVec4(0.00f, 0.48f, 1.00f, 0.60f);
                colors[ImGuiCol_Separator]              = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
                colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
                colors[ImGuiCol_SeparatorActive]        = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
                colors[ImGuiCol_ResizeGrip]             = ImVec4(0.00f, 0.48f, 1.00f, 0.25f);
                colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.00f, 0.48f, 1.00f, 0.50f);
                colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.00f, 0.48f, 1.00f, 0.75f);
                colors[ImGuiCol_Tab]                    = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
                colors[ImGuiCol_TabHovered]             = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
                colors[ImGuiCol_TabActive]              = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
                colors[ImGuiCol_TabUnfocused]           = ImVec4(0.92f, 0.92f, 0.92f, 1.00f);
                colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
                colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
                colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
                colors[ImGuiCol_TableBorderLight]       = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
                colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
                colors[ImGuiCol_TableRowBgAlt]          = ImVec4(0.00f, 0.00f, 0.00f, 0.02f);
            }
        }
    }

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
            ImColor(0xC0, 0xC0, 0xC0, 0xFF), // #C0C0C0 node_connection_line_light
            ImColor(0x46, 0x46, 0x46, 0x50), // #464646 node_connection_line_invalid
            ImColor(0xE0, 0xE0, 0xE0, 0x64), // #E0E0E0 node_connection_line_invalid_light
            ImColor(0x28, 0xBC, 0x28, 0xFF)  // #28bc28 node_pin_active (Darker green, 100% Alpha)
        };
        return colors[static_cast<size_t>(id)];
    }

    
    main_window::main_window(gui_controller& ctrl, SDL_Window* window) 
        : m_ctrl(ctrl),
          m_window(window)
    {
        auto& params = m_ctrl.get_parameters();

        m_p_show_log            = static_cast<adam::configuration_parameter_boolean*>(params.get("show_log"_ct));
        m_p_show_performance    = static_cast<adam::configuration_parameter_boolean*>(params.get("show_performance"_ct));
        m_p_gui_mode            = static_cast<adam::configuration_parameter_integer*>(params.get("gui_mode"_ct));
        m_p_fps_limit           = static_cast<adam::configuration_parameter_integer*>(params.get("fps_limit"_ct));
        m_p_perf_ovly_location  = static_cast<adam::configuration_parameter_integer*>(params.get("perf_ovly_location"_ct));
        m_p_perf_ovly_x         = static_cast<adam::configuration_parameter_double*>(params.get("perf_ovly_x"_ct));
        m_p_perf_ovly_y         = static_cast<adam::configuration_parameter_double*>(params.get("perf_ovly_y"_ct));
        m_p_perf_ovly_content   = static_cast<adam::configuration_parameter_integer*>(params.get("perf_ovly_content"_ct));
        m_p_theme               = static_cast<adam::configuration_parameter_string*>(params.get("theme"_ct));
        m_p_font_scale          = static_cast<adam::configuration_parameter_double*>(params.get("font_scale"_ct));
        m_p_log_height          = static_cast<adam::configuration_parameter_double*>(params.get("log_height"_ct));
        m_p_log_level           = static_cast<adam::configuration_parameter_integer*>(params.get("log_level"_ct));
        m_p_language            = static_cast<adam::configuration_parameter_integer*>(params.get("language"_ct));
        
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
            SDL_SetWindowPosition(m_window, x, y);
            
        if (w > 0 && h > 0)
            SDL_SetWindowSize(m_window, w, h);
            
        if (maximized)
            SDL_MaximizeWindow(m_window);

        ImGui::GetIO().FontGlobalScale = static_cast<float>(m_p_font_scale->get_value()) * adam::gui::get_current_dpi_scale();
        
        bool is_dark = m_p_theme->get_value() == "default-dark"_ct;
        apply_theme(is_dark);

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
            Uint32 flags = SDL_GetWindowFlags(m_window);

            auto& params = m_ctrl.get_parameters();
            static_cast<adam::configuration_parameter_integer*>(params.get("window_x"_ct))->set_value(static_cast<int64_t>(x));
            static_cast<adam::configuration_parameter_integer*>(params.get("window_y"_ct))->set_value(static_cast<int64_t>(y));
            static_cast<adam::configuration_parameter_integer*>(params.get("window_w"_ct))->set_value(static_cast<int64_t>(w));
            static_cast<adam::configuration_parameter_integer*>(params.get("window_h"_ct))->set_value(static_cast<int64_t>(h));
            static_cast<adam::configuration_parameter_boolean*>(params.get("window_maximized"_ct))->set_value((flags & SDL_WINDOW_MAXIMIZED) != 0);
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

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        
        ImGui::SetNextWindowPos(viewport->WorkPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(viewport->WorkSize, ImGuiCond_Always);
        
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::Begin(get_gui_string(gui_string_id::main_ui, lang), nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_MenuBar);
        ImGui::PopStyleVar();

        if (ImGui::BeginMenuBar())
        {
            draw_menu_bar(lang);
            ImGui::EndMenuBar();
        }
        
        float status_bar_height = ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y * 2.0f;
        float content_avail_y = ImGui::GetWindowHeight() - ImGui::GetCursorPosY() - status_bar_height;
        float log_height_val = static_cast<float>(m_p_log_height->get_value());
        float default_space = content_avail_y * 0.2f;
        float max_height = content_avail_y - default_space;
        if (max_height < default_space) max_height = default_space;

        if (m_p_show_log->get_value())
        {
            if (log_height_val > max_height) log_height_val = max_height;
            if (log_height_val < default_space) log_height_val = default_space;

            content_avail_y -= log_height_val + ImGui::GetStyle().ItemSpacing.y * 2.0f + 4.0f;
        }

        if (ImGui::BeginChild("MainContent", ImVec2(0.0f, content_avail_y), false))
        {
            if (ImGui::BeginTabBar("MainTabs"))
            {
                if (ImGui::BeginTabItem(get_gui_string(gui_string_id::tab_management, lang)))
                {
                    ImVec2 pad = ImGui::GetStyle().WindowPadding;
                    ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPos().x + pad.x, ImGui::GetCursorPos().y + pad.y));
                    if (ImGui::BeginChild("##management_content", ImVec2(ImGui::GetContentRegionAvail().x - pad.x, ImGui::GetContentRegionAvail().y - pad.y), false))
                    {
                        draw_tab_management(m_ctrl, lang);
                    }
                    ImGui::EndChild();
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem(get_gui_string(gui_string_id::tab_configuration, lang)))
                {
                    ImVec2 pad = ImGui::GetStyle().WindowPadding;
                    ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPos().x + pad.x, ImGui::GetCursorPos().y + pad.y));
                    if (ImGui::BeginChild("##configuration_content", ImVec2(ImGui::GetContentRegionAvail().x - pad.x, ImGui::GetContentRegionAvail().y - pad.y), false))
                    {
                        draw_tab_configuration(m_ctrl, lang);
                    }
                    ImGui::EndChild();
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem(get_gui_string(gui_string_id::tab_modules, lang)))
                {
                    ImVec2 pad = ImGui::GetStyle().WindowPadding;
                    ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPos().x + pad.x, ImGui::GetCursorPos().y + pad.y));
                    if (ImGui::BeginChild("##modules_content", ImVec2(ImGui::GetContentRegionAvail().x - pad.x, ImGui::GetContentRegionAvail().y - pad.y), false))
                    {
                        draw_tab_modules(m_ctrl, lang, m_module_paths_table_id, m_modules_table_id);
                    }
                    ImGui::EndChild();
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem(get_gui_string(gui_string_id::tab_information, lang)))
                {
                    ImVec2 pad = ImGui::GetStyle().WindowPadding;
                    ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPos().x + pad.x, ImGui::GetCursorPos().y + pad.y));
                    if (ImGui::BeginChild("##information_content", ImVec2(ImGui::GetContentRegionAvail().x - pad.x, ImGui::GetContentRegionAvail().y - pad.y), false))
                    {
                        draw_tab_information(m_ctrl, lang);
                    }
                    ImGui::EndChild();
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
        }
        ImGui::EndChild();

        if (m_p_show_log->get_value())
        {
            draw_log_window(lang, log_height_val, max_height, status_bar_height);
        }

        // Status bar
        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - status_bar_height);
        ImGui::Separator();
        
        if (m_ctrl.is_commander_active())
            ImGui::TextColored(get_gui_color(gui_color_id::commander_connected), "%s", get_gui_string(gui_string_id::lbl_commander_connected, lang));
        else
            ImGui::TextColored(get_gui_color(gui_color_id::commander_disconnected), "%s", get_gui_string(gui_string_id::lbl_commander_disconnected, lang));
            
        const char* lang_short = (lang == adam::language_german) ? "DE" : "EN";
        float lang_width = ImGui::CalcTextSize(lang_short).x;
        ImGui::SameLine(ImGui::GetWindowWidth() - lang_width - ImGui::GetStyle().WindowPadding.x);
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
        
        ImGui::End();

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
                ImGui::GetIO().FontGlobalScale = static_cast<float>(m_p_font_scale->get_value()) * adam::gui::get_current_dpi_scale();
            }

            ImGui::Separator();
            
            adam::string_hashed current_theme = m_p_theme->get_value();
            const char* preview_value = (current_theme == "default-dark"_ct) ? get_gui_string(gui_string_id::theme_default_dark, lang) : get_gui_string(gui_string_id::theme_default_light, lang);
            
            if (ImGui::BeginCombo(get_gui_string(gui_string_id::combo_theme, lang), preview_value))
            {
                bool is_dark = (current_theme == "default-dark"_ct);
                if (ImGui::Selectable(get_gui_string(gui_string_id::theme_default_dark, lang), is_dark))
                {
                    m_p_theme->set_value("default-dark"_ct);
                    apply_theme(true);
                }
                if (is_dark) ImGui::SetItemDefaultFocus();
                
                bool is_light = (current_theme == "default-light"_ct);
                if (ImGui::Selectable(get_gui_string(gui_string_id::theme_default_light, lang), is_light))
                {
                    m_p_theme->set_value("default-light"_ct);
                    apply_theme(false);
                }
                if (is_light) ImGui::SetItemDefaultFocus();
                
                ImGui::EndCombo();
            }
            ImGui::EndMenu();
        }
    }

    void main_window::draw_log_window
    (
        adam::language lang,
        float& log_height_val,
        float max_height,
        float status_bar_height
    )
    {
        float dpi_scale = ImGui::GetStyle()._MainScale;
        
        // Visible splitter for vertical resizing
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_Separator));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_SeparatorHovered));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_SeparatorActive));
        ImGui::Button("##vsplitter", ImVec2(-1, 4.0f));
        ImGui::PopStyleColor(3);

        if (ImGui::IsItemActive())
        {
            log_height_val -= ImGui::GetIO().MouseDelta.y;

            // Re-apply clamping during drag to prevent getting stuck
            if (log_height_val > max_height) log_height_val = max_height;
            if (log_height_val < 100.0f * dpi_scale) log_height_val = 100.0f * dpi_scale;

            m_p_log_height->set_value(static_cast<double>(log_height_val));
        }
        if (ImGui::IsItemHovered())
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
            
        ImGui::Spacing();
        ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_log_console, lang));
        
        float max_combo_text_width = std::max(ImGui::CalcTextSize("Warning").x, ImGui::CalcTextSize("Warnung").x);
        float combo_width = max_combo_text_width + ImGui::GetFrameHeight() + ImGui::GetStyle().FramePadding.x * 2.0f;
        const char* clear_log_text = get_gui_string(gui_string_id::btn_clear_log, lang);
        float btn_width = ImGui::CalcTextSize(clear_log_text, NULL, true).x + ImGui::GetStyle().FramePadding.x * 2.0f;
        float right_align_offset = combo_width + btn_width + ImGui::GetStyle().ItemSpacing.x;
        
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - right_align_offset);

        int current_log_level = static_cast<int>(m_p_log_level->get_value());
        ImGui::SetNextItemWidth(combo_width);
        if (ImGui::Combo("##LogLevel", &current_log_level, get_gui_string(gui_string_id::combo_log_level_options, lang)))
        {
            m_p_log_level->set_value(static_cast<int64_t>(current_log_level));
            if (m_ctrl.get_log_sink().is_active() && m_ctrl.log_sink().queue().metadata())
            {
                m_ctrl.log_sink().queue().metadata()->store(static_cast<adam::log::level>(current_log_level + 1), std::memory_order_relaxed);
            }
        }
        
        ImGui::SameLine();
        
        if (ImGui::Button(clear_log_text))
        {
            m_ctrl.clear_log_history();
        }
        
        ImGui::PushID(m_log_table_id);
        if (ImGui::BeginTable("LogTable", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0.0f, -status_bar_height)))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_time, lang), ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_level, lang), ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_message, lang), ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();

            auto log_history = m_ctrl.get_log_history();
            int i = 0;
            for (const auto& log_line : log_history)
            {
                ImGui::TableNextRow();
                
                const char* level_text = "";
                float r, g, b;
                adam::get_log_appearance(log_line.level, level_text, r, g, b);
                
                ImVec4 color;
                switch (log_line.level)
                {
                    case adam::log::level::trace:   color = get_gui_color(gui_color_id::log_trace); break;
                    case adam::log::level::info:    color = get_gui_color(gui_color_id::log_info); break;
                    case adam::log::level::warning: color = get_gui_color(gui_color_id::log_warning); break;
                    case adam::log::level::error:   color = get_gui_color(gui_color_id::log_error); break;
                    default:                        color = ImVec4(r, g, b, 1.0f); break; // Fallback to SDK defaults
                }

                ImGui::TableSetColumnIndex(0);
                ImGui::PushID(i);
                std::string time_str = adam::get_log_time_string(log_line.timestamp);
                ImGui::Selectable(time_str.c_str(), false, ImGuiSelectableFlags_SpanAllColumns);
                
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    ImGui::SetClipboardText(log_line.text.c_str());
                }

                if (ImGui::BeginPopupContextItem())
                {
                    if (ImGui::MenuItem(get_gui_string(gui_string_id::btn_copy_message, lang)))
                    {
                        ImGui::SetClipboardText(log_line.text.c_str());
                    }
                    if (ImGui::MenuItem(get_gui_string(gui_string_id::btn_copy_row, lang)))
                    {
                        char row_str[1024];
                        snprintf(row_str, sizeof(row_str), "[%s] [%s] %s", 
                                 time_str.c_str(),
                                 level_text,
                                 log_line.text.c_str());
                        ImGui::SetClipboardText(row_str);
                    }
                    ImGui::EndPopup();
                }
                ImGui::PopID();

                ImGui::TableSetColumnIndex(1);
                ImGui::TextColored(color, "%s", level_text);

                ImGui::TableSetColumnIndex(2);
                ImGui::TextUnformatted(log_line.text.c_str());

                i++;
            }

            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f);
                
            ImGui::EndTable();
        }
        ImGui::PopID();
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

}