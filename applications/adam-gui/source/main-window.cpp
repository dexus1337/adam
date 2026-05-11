#include "main-window.hpp"


#include <imgui.h>
#include <SDL.h>


namespace adam::gui 
{
    main_window::main_window(gui_controller& ctrl, SDL_Window* window) 
        : m_ctrl(ctrl),
          m_window(window)
    {
        auto& params = m_ctrl.get_parameters();

        int x = static_cast<adam::configuration_parameter_integer*>(params.get("window_x"))->get_value();
        int y = static_cast<adam::configuration_parameter_integer*>(params.get("window_y"))->get_value();
        int w = static_cast<adam::configuration_parameter_integer*>(params.get("window_w"))->get_value();
        int h = static_cast<adam::configuration_parameter_integer*>(params.get("window_h"))->get_value();
        bool maximized = static_cast<adam::configuration_parameter_boolean*>(params.get("window_maximized"))->get_value();
        
        if (x != -1 && y != -1)
            SDL_SetWindowPosition(m_window, x, y);
            
        if (w > 0 && h > 0)
            SDL_SetWindowSize(m_window, w, h);
            
        if (maximized)
            SDL_MaximizeWindow(m_window);

        ImGui::GetIO().FontGlobalScale = static_cast<float>(static_cast<adam::configuration_parameter_double*>(params.get("font_scale"))->get_value());
        
        bool dark_theme_val = static_cast<adam::configuration_parameter_boolean*>(params.get("dark_theme"))->get_value();
        if (dark_theme_val) ImGui::StyleColorsDark();
        else ImGui::StyleColorsLight();
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
            static_cast<adam::configuration_parameter_integer*>(params.get("window_x"))->set_value(x);
            static_cast<adam::configuration_parameter_integer*>(params.get("window_y"))->set_value(y);
            static_cast<adam::configuration_parameter_integer*>(params.get("window_w"))->set_value(w);
            static_cast<adam::configuration_parameter_integer*>(params.get("window_h"))->set_value(h);
            static_cast<adam::configuration_parameter_boolean*>(params.get("window_maximized"))->set_value((flags & SDL_WINDOW_MAXIMIZED) != 0);
        }
    }

    void main_window::render()
    {
        auto& params = m_ctrl.get_parameters();
        auto* p_show_log = static_cast<adam::configuration_parameter_boolean*>(params.get("show_log"));
        auto* p_dark_theme = static_cast<adam::configuration_parameter_boolean*>(params.get("dark_theme"));
        auto* p_font_scale = static_cast<adam::configuration_parameter_double*>(params.get("font_scale"));
        auto* p_log_height = static_cast<adam::configuration_parameter_double*>(params.get("log_height"));

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        
        ImGui::SetNextWindowPos(viewport->WorkPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(viewport->WorkSize, ImGuiCond_Always);
        
        ImGui::Begin("Main UI", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_MenuBar);

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("View"))
            {
                ImGui::MenuItem("Show Log", nullptr, &p_show_log->value());
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Settings"))
            {
                float font_scale_val = static_cast<float>(p_font_scale->get_value());
                if (ImGui::SliderFloat("Font Scale", &font_scale_val, 0.5f, 3.0f, "%.2f"))
                {
                    p_font_scale->set_value(static_cast<double>(font_scale_val));
                    ImGui::GetIO().FontGlobalScale = font_scale_val;
                }
                if (ImGui::Button("Reset to Default"))
                {
                    p_font_scale->set_value(1.0);
                    ImGui::GetIO().FontGlobalScale = 1.0f;
                }

                ImGui::Separator();
                
                if (ImGui::Checkbox("Dark Theme", &p_dark_theme->value()))
                {
                    if (p_dark_theme->get_value()) ImGui::StyleColorsDark();
                    else ImGui::StyleColorsLight();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        ImGui::Text("ADAM Control Panel");
        ImGui::Separator();
        
        if (m_ctrl.is_commander_active())
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Commander connected.");
        else
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Commander disconnected.");
            
        if (p_show_log->get_value())
        {
            float log_height_val = static_cast<float>(p_log_height->get_value());
            float max_height = ImGui::GetWindowHeight() - 100.0f;
            if (max_height < 100.0f) max_height = 100.0f;
            if (log_height_val > max_height) log_height_val = max_height;
            if (log_height_val < 100.0f) log_height_val = 100.0f;

            ImGui::SetCursorPosY(ImGui::GetWindowHeight() - log_height_val - ImGui::GetStyle().WindowPadding.y);
            
            // Invisible splitter for vertical resizing
            ImGui::InvisibleButton("##vsplitter", ImVec2(-1, 4.0f));
            if (ImGui::IsItemActive())
            {
                log_height_val -= ImGui::GetIO().MouseDelta.y;
                p_log_height->set_value(static_cast<double>(log_height_val));
            }
            if (ImGui::IsItemHovered())
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
                
            ImGui::Separator();
            ImGui::Text("Log");
            
            ImGui::BeginChild("LogRegion", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_HorizontalScrollbar);
            auto log_history = m_ctrl.get_log_history();
            for (const auto& log_line : log_history)
            {
                ImGui::TextUnformatted(adam::get_log_time_string(log_line.timestamp).c_str());
                ImGui::SameLine();

                const char* level_text = "";
                float r, g, b;
                adam::get_log_appearance(log_line.level, level_text, r, g, b);
                
                ImGui::TextColored(ImVec4(r, g, b, 1.0f), "[%s]", level_text);
                ImGui::SameLine();
                ImGui::TextUnformatted(log_line.text.c_str());
            }
            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f);
                
            ImGui::EndChild();
        }
        
        ImGui::End();
    }
}