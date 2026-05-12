#include "main-window.hpp"


#include <imgui.h>
#include <SDL.h>
#include <unordered_map>
#include <array>


namespace adam::gui 
{
    const char* get_gui_string(gui_string_id id, adam::language lang)
    {
        static const std::unordered_map<int, std::array<const char*, adam::languages_count>> translations =
        {
            { static_cast<int>(gui_string_id::main_ui),                     { "Main UI##MainUI", "Hauptbenutzeroberfläche##MainUI" } },
            { static_cast<int>(gui_string_id::menu_view),                   { "View", "Ansicht" } },
            { static_cast<int>(gui_string_id::menu_show_log),               { "Show Log", "Protokoll anzeigen" } },
            { static_cast<int>(gui_string_id::menu_settings),               { "Settings", "Einstellungen" } },
            { static_cast<int>(gui_string_id::combo_language),              { "Language##Lang", "Sprache##Lang" } },
            { static_cast<int>(gui_string_id::slider_font_scale),           { "Font Scale##FontScale", "Schriftskalierung##FontScale" } },
            { static_cast<int>(gui_string_id::btn_reset_default),           { "Reset to Default##Reset", "Auf Standard zurücksetzen##Reset" } },
            { static_cast<int>(gui_string_id::checkbox_dark_theme),         { "Dark Theme##DarkTheme", "Dunkles Design##DarkTheme" } },
            { static_cast<int>(gui_string_id::lbl_control_panel),           { "ADAM Control Panel", "ADAM-Bedienfeld" } },
            { static_cast<int>(gui_string_id::lbl_commander_connected),     { "Commander connected.", "Commander verbunden." } },
            { static_cast<int>(gui_string_id::lbl_commander_disconnected),  { "Commander disconnected.", "Commander getrennt." } },
            { static_cast<int>(gui_string_id::lbl_log_console),             { "Log Console", "Protokollkonsole" } },
            { static_cast<int>(gui_string_id::combo_log_level_options),     { "Trace\0Info\0Warning\0Error\0\0", "Trace\0Info\0Warnung\0Fehler\0\0" } },
            { static_cast<int>(gui_string_id::tbl_time),                    { "Time", "Zeit" } },
            { static_cast<int>(gui_string_id::tbl_level),                   { "Level", "Ebene" } },
            { static_cast<int>(gui_string_id::tbl_message),                 { "Message", "Nachricht" } }
        };

        auto val = static_cast<int>(id);
        auto it = translations.find(val);
        if (it != translations.end())
            return it->second[static_cast<int>(lang)];
        
        return "UNKNOWN_STRING";
    }

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
        auto* p_log_level = static_cast<adam::configuration_parameter_integer*>(params.get("log_level"));

        adam::language lang = m_ctrl.get_language();

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        
        ImGui::SetNextWindowPos(viewport->WorkPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(viewport->WorkSize, ImGuiCond_Always);
        
        ImGui::Begin(get_gui_string(gui_string_id::main_ui, lang), nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_MenuBar);

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu(get_gui_string(gui_string_id::menu_view, lang)))
            {
                ImGui::MenuItem(get_gui_string(gui_string_id::menu_show_log, lang), nullptr, &p_show_log->value());
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu(get_gui_string(gui_string_id::menu_settings, lang)))
            {
                if (ImGui::BeginCombo(get_gui_string(gui_string_id::combo_language, lang), adam::language_strings::language_name(lang, lang).data()))
                {
                    for (adam::language avail_lang : m_ctrl.get_available_languages())
                    {
                        bool is_selected = (lang == avail_lang);
                        if (ImGui::Selectable(adam::language_strings::language_name(avail_lang, lang).data(), is_selected))
                        {
                            m_ctrl.set_language(avail_lang);
                        }
                        if (is_selected)
                        {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }

                float font_scale_val = static_cast<float>(p_font_scale->get_value());
                if (ImGui::SliderFloat(get_gui_string(gui_string_id::slider_font_scale, lang), &font_scale_val, 0.5f, 3.0f, "%.2f"))
                {
                    p_font_scale->set_value(static_cast<double>(font_scale_val));
                    ImGui::GetIO().FontGlobalScale = font_scale_val;
                }
                if (ImGui::Button(get_gui_string(gui_string_id::btn_reset_default, lang)))
                {
                    p_font_scale->set_value(1.0);
                    ImGui::GetIO().FontGlobalScale = 1.0f;
                }

                ImGui::Separator();
                
                if (ImGui::Checkbox(get_gui_string(gui_string_id::checkbox_dark_theme, lang), &p_dark_theme->value()))
                {
                    if (p_dark_theme->get_value()) ImGui::StyleColorsDark();
                    else ImGui::StyleColorsLight();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_control_panel, lang));
        ImGui::Separator();
        
        if (m_ctrl.is_commander_active())
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s", get_gui_string(gui_string_id::lbl_commander_connected, lang));
        else
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", get_gui_string(gui_string_id::lbl_commander_disconnected, lang));
            
        if (p_show_log->get_value())
        {
            float log_height_val = static_cast<float>(p_log_height->get_value());
            float max_height = ImGui::GetWindowHeight() - 100.0f;
            if (max_height < 100.0f) max_height = 100.0f;
            if (log_height_val > max_height) log_height_val = max_height;
            if (log_height_val < 100.0f) log_height_val = 100.0f;

            ImGui::SetCursorPosY(ImGui::GetWindowHeight() - log_height_val);
            
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
                if (log_height_val < 100.0f) log_height_val = 100.0f;

                p_log_height->set_value(static_cast<double>(log_height_val));
            }
            if (ImGui::IsItemHovered())
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
                
            ImGui::Spacing();
            ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_log_console, lang));
            
            float combo_width = 120.0f;
            ImGui::SameLine(ImGui::GetWindowWidth() - combo_width - ImGui::GetStyle().WindowPadding.x);
            
            int current_log_level = static_cast<int>(p_log_level->get_value());
            ImGui::SetNextItemWidth(combo_width);
            if (ImGui::Combo("##LogLevel", &current_log_level, get_gui_string(gui_string_id::combo_log_level_options, lang)))
            {
                p_log_level->set_value(current_log_level);
                m_ctrl.set_log_level(current_log_level);
            }
            
            ImGui::BeginChild("LogRegion", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
            
            if (ImGui::BeginTable("LogTable", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_RowBg))
            {
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_time, lang), ImGuiTableColumnFlags_WidthFixed, 150.0f);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_level, lang), ImGuiTableColumnFlags_WidthFixed, 60.0f);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_message, lang), ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();

                auto log_history = m_ctrl.get_log_history();
                for (const auto& log_line : log_history)
                {
                    ImGui::TableNextRow();
                    
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted(adam::get_log_time_string(log_line.timestamp).c_str());

                    ImGui::TableSetColumnIndex(1);
                    const char* level_text = "";
                    float r, g, b;
                    adam::get_log_appearance(log_line.level, level_text, r, g, b);
                    ImGui::TextColored(ImVec4(r, g, b, 1.0f), "%s", level_text);

                    ImGui::TableSetColumnIndex(2);
                    ImGui::TextUnformatted(log_line.text.c_str());
                }
                ImGui::EndTable();
            }

            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f);
                
            ImGui::EndChild();
        }
        
        ImGui::End();
    }
}