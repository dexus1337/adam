#include "main-window.hpp"


#include <imgui.h>
#include <SDL.h>
#include <unordered_map>
#include <array>
#include <map>

namespace adam::gui 
{
    enum class gui_color_id : size_t
    {
        commander_connected = 0,
        commander_disconnected,
        log_trace,
        log_info,
        log_warning,
        log_error,
        count
    };

    const ImVec4& get_gui_color(gui_color_id id)
    {
        static const std::array<ImVec4, static_cast<size_t>(gui_color_id::count)> colors =
        {
            ImColor(0x26, 0xA6, 0x26), // #26A626 commander_connected
            ImColor(0xD9, 0x33, 0x33), // #D93333 commander_disconnected
            ImColor(0x33, 0x99, 0xD9), // #3399D9 log_trace
            ImColor(0x26, 0xA6, 0x26), // #26A626 log_info
            ImColor(0xD9, 0x99, 0x26), // #D99926 log_warning
            ImColor(0xD9, 0x33, 0x33)  // #D93333 log_error
        };
        return colors[static_cast<size_t>(id)];
    }

    const char* get_gui_string(gui_string_id id, adam::language lang)
    {
        static const std::unordered_map<int, std::array<const char*, adam::languages_count>> translations =
        {
            { static_cast<int>(gui_string_id::main_ui),                     { "Main UI###MainUI", "Hauptbenutzeroberfläche###MainUI" } },
            { static_cast<int>(gui_string_id::menu_view),                   { "View", "Ansicht" } },
            { static_cast<int>(gui_string_id::menu_show_log),               { "Show Log", "Protokoll anzeigen" } },
            { static_cast<int>(gui_string_id::menu_show_performance),       { "Show Performance", "Leistung anzeigen" } },
            { static_cast<int>(gui_string_id::menu_settings),               { "Settings", "Einstellungen" } },
            { static_cast<int>(gui_string_id::combo_language),              { "Language###Lang", "Sprache###Lang" } },
            { static_cast<int>(gui_string_id::slider_font_scale),           { "Font Scale###FontScale", "Schriftskalierung###FontScale" } },
            { static_cast<int>(gui_string_id::btn_reset_default),           { "Reset to Default###Reset", "Auf Standard zurücksetzen###Reset" } },
            { static_cast<int>(gui_string_id::checkbox_dark_theme),         { "Dark Theme###DarkTheme", "Dunkles Design###DarkTheme" } },
            { static_cast<int>(gui_string_id::btn_clear_log),               { "Clear Log###ClearLog", "Protokoll leeren###ClearLog" } },
            { static_cast<int>(gui_string_id::lbl_control_panel),           { "ADAM Control Panel", "ADAM-Bedienfeld" } },
            { static_cast<int>(gui_string_id::lbl_commander_connected),     { "Commander connected.", "Commander verbunden." } },
            { static_cast<int>(gui_string_id::lbl_commander_disconnected),  { "Commander disconnected.", "Commander getrennt." } },
            { static_cast<int>(gui_string_id::lbl_log_console),             { "Log Console", "Protokollkonsole" } },
            { static_cast<int>(gui_string_id::combo_log_level_options),     { "Trace\0Info\0Warning\0Error\0\0", "Trace\0Info\0Warnung\0Fehler\0\0" } },
            { static_cast<int>(gui_string_id::tbl_time),                    { "Time", "Zeit" } },
            { static_cast<int>(gui_string_id::tbl_level),                   { "Level", "Ebene" } },
            { static_cast<int>(gui_string_id::tbl_message),                 { "Message", "Nachricht" } },
            { static_cast<int>(gui_string_id::lbl_performance_overlay),     { "Performance Overlay###PerfOverlay", "Leistungs-Overlay###PerfOverlay" } },
            { static_cast<int>(gui_string_id::lbl_fps),                     { "FPS: %.1f (%.3f ms/frame)", "FPS: %.1f (%.3f ms/Frame)" } },
            { static_cast<int>(gui_string_id::lbl_cpu),                     { "CPU: %.1f%%", "CPU: %.1f%%" } },
            { static_cast<int>(gui_string_id::lbl_ram),                     { "RAM: %.1f/%.1f GB (%.0f%%)", "RAM: %.1f/%.1f GB (%.0f%%)" } },
            { static_cast<int>(gui_string_id::menu_overlay_custom),         { "Custom", "Benutzerdefiniert" } },
            { static_cast<int>(gui_string_id::menu_overlay_top_left),       { "Top-left", "Oben links" } },
            { static_cast<int>(gui_string_id::menu_overlay_top_right),      { "Top-right", "Oben rechts" } },
            { static_cast<int>(gui_string_id::menu_overlay_bottom_left),    { "Bottom-left", "Unten links" } },
            { static_cast<int>(gui_string_id::menu_overlay_bottom_right),   { "Bottom-right", "Unten rechts" } },
            { static_cast<int>(gui_string_id::menu_overlay_position),       { "Position", "Position" } },
            { static_cast<int>(gui_string_id::menu_overlay_content),        { "Content", "Inhalt" } },
            { static_cast<int>(gui_string_id::menu_overlay_show_fps),       { "Frames per Second (FPS)", "Bilder pro Sekunde (FPS)" } },
            { static_cast<int>(gui_string_id::menu_overlay_show_cpu),       { "Processor (CPU) Usage", "Prozessor (CPU) Auslastung" } },
            { static_cast<int>(gui_string_id::menu_overlay_show_ram),       { "Memory (RAM) Usage", "Arbeitsspeicher (RAM) Auslastung" } },
            { static_cast<int>(gui_string_id::tab_management),              { "Management###TabManagement", "Verwaltung###TabManagement" } },
            { static_cast<int>(gui_string_id::tab_modules),                 { "Modules###TabModules", "Module###TabModules" } },
            { static_cast<int>(gui_string_id::tab_information),             { "Information###TabInformation", "Informationen###TabInformation" } },
            { static_cast<int>(gui_string_id::tbl_load),                    { "Load", "Laden" } },
            { static_cast<int>(gui_string_id::tbl_name),                    { "Name", "Name" } },
            { static_cast<int>(gui_string_id::tbl_status),                  { "Status", "Status" } },
            { static_cast<int>(gui_string_id::tbl_version),                 { "Version", "Version" } },
            { static_cast<int>(gui_string_id::tbl_path),                    { "Path", "Pfad" } },
            { static_cast<int>(gui_string_id::stat_available),              { "Available", "Verfügbar" } },
            { static_cast<int>(gui_string_id::stat_loaded),                 { "Loaded", "Geladen" } },
            { static_cast<int>(gui_string_id::stat_unavailable),            { "Unavailable", "Nicht verfügbar" } }
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

        m_p_show_log            = static_cast<adam::configuration_parameter_boolean*>(params.get("show_log"_ct));
        m_p_show_performance    = static_cast<adam::configuration_parameter_boolean*>(params.get("show_performance"_ct));
        m_p_perf_ovly_location  = static_cast<adam::configuration_parameter_integer*>(params.get("perf_ovly_location"_ct));
        m_p_perf_ovly_x         = static_cast<adam::configuration_parameter_double*>(params.get("perf_ovly_x"_ct));
        m_p_perf_ovly_y         = static_cast<adam::configuration_parameter_double*>(params.get("perf_ovly_y"_ct));
        m_p_perf_ovly_content   = static_cast<adam::configuration_parameter_integer*>(params.get("perf_ovly_content"_ct));
        m_p_dark_theme          = static_cast<adam::configuration_parameter_boolean*>(params.get("dark_theme"_ct));
        m_p_font_scale          = static_cast<adam::configuration_parameter_double*>(params.get("font_scale"_ct));
        m_p_log_height          = static_cast<adam::configuration_parameter_double*>(params.get("log_height"_ct));
        m_p_log_level           = static_cast<adam::configuration_parameter_integer*>(params.get("log_level"_ct));

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

        ImGui::GetIO().FontGlobalScale = static_cast<float>(m_p_font_scale->get_value());
        
        bool dark_theme_val = m_p_dark_theme->get_value();
        if (dark_theme_val) 
            ImGui::StyleColorsDark();
        else 
            ImGui::StyleColorsLight();
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

    void main_window::render()
    {
        adam::language lang = m_ctrl.get_commander().get_language();

        //ImGui::ShowDemoWindow();

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        
        ImGui::SetNextWindowPos(viewport->WorkPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(viewport->WorkSize, ImGuiCond_Always);
        
        ImGui::Begin(get_gui_string(gui_string_id::main_ui, lang), nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_MenuBar);

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu(get_gui_string(gui_string_id::menu_view, lang)))
            {
                ImGui::MenuItem(get_gui_string(gui_string_id::menu_show_log, lang), nullptr, &m_p_show_log->value());
                ImGui::MenuItem(get_gui_string(gui_string_id::menu_show_performance, lang), nullptr, &m_p_show_performance->value());
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu(get_gui_string(gui_string_id::menu_settings, lang)))
            {
                if (ImGui::BeginCombo(get_gui_string(gui_string_id::combo_language, lang), adam::language_strings::language_name(lang, lang).data()))
                {
                    uint64_t available_langs = m_ctrl.get_commander().is_active() ? m_ctrl.get_commander().get_available_languages() : ((1ULL << static_cast<int>(adam::language_english)) | (1ULL << static_cast<int>(adam::language_german)));
                    
                    for (int i = 0; i < static_cast<int>(adam::languages_count); ++i)
                    {
                        if (!(available_langs & (1ULL << i)))
                            continue;
                            
                        adam::language avail_lang = static_cast<adam::language>(i);
                        bool is_selected = (lang == avail_lang);
                        if (ImGui::Selectable(adam::language_strings::language_name(avail_lang, lang).data(), is_selected))
                        {
                            if (m_ctrl.get_commander().is_active())
                                m_ctrl.get_commander().request_language_change(avail_lang);
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
                    ImGui::GetIO().FontGlobalScale = static_cast<float>(m_p_font_scale->get_value());
                }
                if (ImGui::Button(get_gui_string(gui_string_id::btn_reset_default, lang)))
                {
                    m_p_font_scale->set_value(1.0);
                    ImGui::GetIO().FontGlobalScale = 1.0f;
                }

                ImGui::Separator();
                
                if (ImGui::Checkbox(get_gui_string(gui_string_id::checkbox_dark_theme, lang), &m_p_dark_theme->value()))
                {
                    if (m_p_dark_theme->get_value()) ImGui::StyleColorsDark();
                    else ImGui::StyleColorsLight();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_control_panel, lang));
        ImGui::Separator();
        
        float status_bar_height = ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y * 2.0f;
        float content_avail_y = ImGui::GetWindowHeight() - ImGui::GetCursorPosY() - status_bar_height;
        float log_height_val = static_cast<float>(m_p_log_height->get_value());
        float max_height = content_avail_y - 100.0f;
        if (max_height < 100.0f) max_height = 100.0f;

        if (m_p_show_log->get_value())
        {
            if (log_height_val > max_height) log_height_val = max_height;
            if (log_height_val < 100.0f) log_height_val = 100.0f;

            content_avail_y -= log_height_val + ImGui::GetStyle().ItemSpacing.y * 2.0f + 4.0f;
        }

        if (ImGui::BeginChild("MainContent", ImVec2(0, content_avail_y), false))
        {
            if (ImGui::BeginTabBar("MainTabs"))
            {
                if (ImGui::BeginTabItem(get_gui_string(gui_string_id::tab_management, lang)))
                {
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem(get_gui_string(gui_string_id::tab_modules, lang)))
                {
                    if (ImGui::BeginTable("ModulesTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY))
                    {
                        ImGui::TableSetupScrollFreeze(0, 1);
                        ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_status, lang), ImGuiTableColumnFlags_WidthFixed);
                        ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_name, lang), ImGuiTableColumnFlags_WidthFixed);
                        ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_path, lang), ImGuiTableColumnFlags_WidthStretch);
                        ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_version, lang), ImGuiTableColumnFlags_WidthFixed);
                        ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_load, lang), ImGuiTableColumnFlags_WidthFixed);
                        ImGui::TableHeadersRow();

                        auto draw_module_row = [&](const std::string& name, int status, const std::string& path, uint32_t version) {
                            ImGui::TableNextRow();

                            ImGui::TableSetColumnIndex(0);
                            ImGui::AlignTextToFramePadding();
                            if (status == 0) {
                                ImGui::TextColored(get_gui_color(gui_color_id::log_trace), "%s", get_gui_string(gui_string_id::stat_available, lang));
                            } else if (status == 1) {
                                ImGui::TextColored(get_gui_color(gui_color_id::log_info), "%s", get_gui_string(gui_string_id::stat_loaded, lang));
                            } else if (status == 2) {
                                ImGui::TextColored(get_gui_color(gui_color_id::log_warning), "%s", get_gui_string(gui_string_id::stat_unavailable, lang));
                            }

                            ImGui::TableSetColumnIndex(1);
                            ImGui::AlignTextToFramePadding();
                            ImGui::TextUnformatted(name.c_str());

                            ImGui::TableSetColumnIndex(2);
                            ImGui::AlignTextToFramePadding();
                            ImGui::TextUnformatted(path.empty() ? "N/A" : path.c_str());

                            ImGui::TableSetColumnIndex(3);
                            ImGui::AlignTextToFramePadding();
                            if (version != 0) {
                                ImGui::Text("%d.%d.%d", adam::get_major(version), adam::get_minor(version), adam::get_patch(version));
                            } else {
                                ImGui::TextUnformatted("N/A");
                            }

                            ImGui::TableSetColumnIndex(4);
                            bool is_loaded = (status == 1);
                            bool checkbox_val = is_loaded;
                            ImGui::PushID(name.c_str());
                            if (status == 2) ImGui::BeginDisabled();
                            if (ImGui::Checkbox("##load", &checkbox_val)) {
                                // Module load/unload over IPC not yet supported
                            }
                            if (status == 2) ImGui::EndDisabled();
                            ImGui::PopID();
                        };

                        if (m_ctrl.is_commander_active())
                        {
                            struct module_gui_info {
                                int status; // 0=Avail, 1=Loaded, 2=Unavail
                                uint32_t version;
                                std::string path;
                            };
                            std::map<std::string, module_gui_info> merged;

                            for (const auto& [name_hash, data] : m_ctrl.get_commander().get_available_modules())
                                merged[std::string(name_hash.c_str())] = { 0, data.first, std::string(data.second.c_str()) };
                                
                            for (const auto& [name_hash, ptr] : m_ctrl.get_commander().get_loaded_modules()) {
                                std::string name_str(name_hash.c_str());
                                if (merged.find(name_str) != merged.end())
                                    merged[name_str].status = 1;
                                else
                                    merged[name_str] = { 1, 0, "" }; // dynamically loaded modules miss version/path currently
                            }
                                
                            for (const auto& [name_hash, data] : m_ctrl.get_commander().get_unavailable_modules())
                                merged[std::string(name_hash.c_str())] = { 2, data.first, std::string(data.second.c_str()) };
                                
                            for (const auto& [name, info] : merged)
                                draw_module_row(name, info.status, info.path, info.version);
                        }

                        ImGui::EndTable();
                    }
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem(get_gui_string(gui_string_id::tab_information, lang)))
                {
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
        }
        ImGui::EndChild();

        if (m_p_show_log->get_value())
        {
            
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

                m_p_log_height->set_value(static_cast<double>(log_height_val));
            }
            if (ImGui::IsItemHovered())
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
                
            ImGui::Spacing();
            ImGui::TextUnformatted(get_gui_string(gui_string_id::lbl_log_console, lang));
            
            float combo_width = 120.0f * ImGui::GetIO().FontGlobalScale;
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
                if (m_ctrl.get_log_sink().is_active() && m_ctrl.get_log_sink().queue().metadata())
                {
                    m_ctrl.get_log_sink().queue().metadata()->store(static_cast<adam::log::level>(current_log_level + 1), std::memory_order_relaxed);
                }
            }
            
            ImGui::SameLine();
            
            if (ImGui::Button(clear_log_text))
            {
                m_ctrl.clear_log_history();
            }
            
            if (ImGui::BeginTable("LogTable", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0.0f, -status_bar_height)))
            {
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_time, lang), ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn(get_gui_string(gui_string_id::tbl_level, lang), ImGuiTableColumnFlags_WidthFixed);
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
                    
                    ImVec4 color;
                    switch (log_line.level)
                    {
                        case adam::log::level::trace:   color = get_gui_color(gui_color_id::log_trace); break;
                        case adam::log::level::info:    color = get_gui_color(gui_color_id::log_info); break;
                        case adam::log::level::warning: color = get_gui_color(gui_color_id::log_warning); break;
                        case adam::log::level::error:   color = get_gui_color(gui_color_id::log_error); break;
                        default:                        color = ImVec4(r, g, b, 1.0f); break; // Fallback to SDK defaults
                    }

                    ImGui::TextColored(color, "%s", level_text);

                    ImGui::TableSetColumnIndex(2);
                    ImGui::TextUnformatted(log_line.text.c_str());
                }

                if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                    ImGui::SetScrollHereY(1.0f);
                    
                ImGui::EndTable();
            }
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
        if (ImGui::IsItemClicked() && m_ctrl.get_commander().is_active())
        {
            adam::language new_lang = (lang == adam::language_english) ? adam::language_german : adam::language_english;
            m_ctrl.get_commander().request_language_change(new_lang);
        }
        
        ImGui::End();

        if (m_p_show_performance->get_value())
        {
            int location = static_cast<int>(m_p_perf_ovly_location->get_value());
            static bool custom_pos_initialized = false;
            
            ImGuiWindowFlags overlay_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;
            
            if (location >= 0)
            {
                const float PAD = 10.0f;
                ImVec2 work_pos = viewport->WorkPos;
                ImVec2 work_size = viewport->WorkSize;
                ImVec2 window_pos, window_pos_pivot;
                
                window_pos.x = (location & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
                window_pos.y = (location & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + ImGui::GetFrameHeight() + PAD);
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
                if (content & 1) ImGui::Text(get_gui_string(gui_string_id::lbl_fps, lang), ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
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
}