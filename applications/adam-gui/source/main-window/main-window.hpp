#pragma once

#include <adam-sdk.hpp>
#include "../gui-controller.hpp"

struct SDL_Window;
struct ImVec4;

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

    const ImVec4& get_gui_color(gui_color_id id);

    enum class gui_string_id
    {
        main_ui,
        menu_view,
        menu_show_log,
        menu_show_performance,
        menu_settings,
        combo_language,
        slider_font_scale,
        btn_reset_default,
        checkbox_dark_theme,
        btn_clear_log,
        lbl_control_panel,
        lbl_commander_connected,
        lbl_commander_disconnected,
        lbl_log_console,
        combo_log_level_options,
        tbl_time,
        tbl_level,
        tbl_message,
        lbl_performance_overlay,
        lbl_fps,
        lbl_cpu,
        lbl_ram,
        menu_overlay_custom,
        menu_overlay_top_left,
        menu_overlay_top_right,
        menu_overlay_bottom_left,
        menu_overlay_bottom_right,
        menu_overlay_position,
        menu_overlay_content,
        menu_overlay_show_fps,
        menu_overlay_show_cpu,
        menu_overlay_show_ram,
        tab_management,
        tab_modules,
        tab_information,
        tab_configuration,
        tbl_load,
        tbl_name,
        tbl_status,
        tbl_version,
        tbl_path,
        stat_available,
        stat_loaded,
        stat_unavailable,
        tt_incompat_sdk,
        tt_incompat_unknown,
        lbl_module_paths,
        btn_add_path,
        btn_remove_path,
        btn_scan_modules,
        ph_new_path,
        lbl_modules,
        tbl_index,
        btn_create_connection,
        btn_create_port,
        lbl_inputs,
        lbl_outputs,
        lbl_processors,
        dlg_create_connection,
        dlg_create_port,
        btn_cancel,
        btn_create,
        lbl_port_type
    };

    const char* get_gui_string(gui_string_id id, adam::language lang);

    class main_window 
    {
    public:
        main_window(gui_controller& ctrl, SDL_Window* window);
        ~main_window();

        void save_window_state();

        void render();

    private:
        void render_menu_bar(adam::language lang);
        void render_log_window(adam::language lang, float& log_height_val, float max_height, float status_bar_height);
        void render_performance_overlay(adam::language lang);

        gui_controller& m_ctrl;
        SDL_Window* m_window;

        adam::configuration_parameter_boolean* m_p_show_log;
        adam::configuration_parameter_boolean* m_p_show_performance;
        adam::configuration_parameter_boolean* m_p_dark_theme;
        adam::configuration_parameter_double*  m_p_font_scale;
        adam::configuration_parameter_double*  m_p_log_height;
        adam::configuration_parameter_integer* m_p_log_level;
        adam::configuration_parameter_integer* m_p_language;
        adam::configuration_parameter_integer* m_p_perf_ovly_location;
        adam::configuration_parameter_double*  m_p_perf_ovly_x;
        adam::configuration_parameter_double*  m_p_perf_ovly_y;
        adam::configuration_parameter_integer* m_p_perf_ovly_content;

        adam::language m_last_lang;
        bool           m_modules_was_empty;
        bool           m_log_was_empty;
        bool           m_module_paths_was_empty;
        int            m_modules_table_id;
        int            m_log_table_id;
        int            m_module_paths_table_id;
    };
}