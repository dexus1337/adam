#pragma once

#include <adam-sdk.hpp>
#include "../gui-controller.hpp"

struct SDL_Window;
struct ImVec4;
struct ImFont;

#include <vector>
#include <map>
#include <mutex>

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
        node_input,
        node_processor,
        node_output,
        node_connection_line,
        node_connection_line_light,
        node_pin_active,
        count
    };

    const ImVec4& get_gui_color(gui_color_id id);

    enum gui_string_id : uint32_t
    {
        main_ui,
        menu_view,
        menu_show_log,
        menu_show_performance,
        menu_settings,
        menu_gui_mode,
        gui_mode_default,
        gui_mode_immediate,
        combo_language,
        slider_font_scale,
        combo_theme,
        theme_default_dark,
        theme_default_light,
        btn_clear_log,
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
        lbl_port_type,
        btn_start,
        btn_stop,
        btn_add_input,
        btn_add_output,
        btn_add_processor,
        lbl_connection,
        btn_delete,
        dlg_delete_connection,
        msg_delete_connection_confirm,
        btn_ok,
        lbl_no_description,
        lbl_available_ports,
        lbl_available_data_formats,
        lbl_available_processors,
        lbl_port_direction,
        lbl_badge_input,
        lbl_badge_output,
        lbl_badge_in_short,
        lbl_badge_out_short,
        lbl_processor_type,
        lbl_filter,
        lbl_converter,
        lbl_existing_ports,
        lbl_new_ports,
        tt_module_missing,
        lbl_sort_by,
        sort_name_asc,
        sort_name_desc,
        sort_date_asc,
        sort_date_desc,
        sort_edited_asc,
        sort_edited_desc,
        sort_custom,
        lbl_move_connection,
        dlg_add_input_port_to,
        dlg_add_output_port_to,
        dlg_add_port,
        lbl_inject_data,
        btn_inject,
        lbl_stat_handled,
        lbl_stat_discarded,
        lbl_stat_unavailable,
        btn_remove_port,
        lbl_data_inspector,
        lbl_active_inspectors,
        col_inspect,
        col_port_name,
        col_messages,
        col_size,
        col_last_received,
        lbl_no_data,
        btn_clear_data,
        btn_clear_all_data,
        col_index,
        col_timestamp,
        col_preview_hex,
        col_preview_ascii,
        col_type,
        lbl_data_format,
        lbl_input_port,
        lbl_output_port,
        lbl_inout_port,
        lbl_data_format_transparent_none,
        lbl_handled,
        lbl_discarded,
        lbl_statistics
    };

    const char* get_gui_string(gui_string_id id, adam::language lang);

    struct inspected_buffer
    {
        uint64_t timestamp;
        std::vector<uint8_t> data;
    };

    struct inspection_data
    {
        std::mutex mtx;
        std::map<adam::string_hash, std::vector<inspected_buffer>> buffers;
        adam::string_hash selected_port = 0;
    };

    extern inspection_data g_inspection_data;
    extern ImFont* g_mono_font;

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
        adam::configuration_parameter_integer* m_p_gui_mode;
        adam::configuration_parameter_string*  m_p_theme;
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