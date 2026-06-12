#pragma once

#include <adam-sdk.hpp>
#include "../gui-controller.hpp"
#include "gui-strings.hpp"

struct SDL_Window;
struct ImVec4;
struct ImFont;

#include <vector>
#include <map>
#include <set>
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
        node_connection_line_invalid,
        node_connection_line_invalid_light,
        node_pin_active,
        count
    };

    const ImVec4& get_gui_color(gui_color_id id);


    struct inspected_buffer
    {
        uint64_t timestamp;
        uint32_t offset;
        uint32_t size;
    };

    struct inspection_port_data
    {
        std::vector<inspected_buffer> buffers;
        std::vector<uint8_t> data_pool;
        std::set<size_t> expanded_nodes;
    };

    struct inspection_data
    {
        std::mutex mtx;
        std::map<adam::string_hash, inspection_port_data> ports;
        std::map<adam::string_hash, inspection_port_data> connections_input;
        std::map<adam::string_hash, inspection_port_data> connections_output;
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

        void draw();

    private:
        void draw_menu_bar(adam::language lang);
        void draw_log_window
        (
            adam::language lang,
            float& log_height_val,
            float max_height,
            float status_bar_height
        );
        void draw_performance_overlay(adam::language lang);

        gui_controller& m_ctrl;
        SDL_Window* m_window;

        adam::configuration_parameter_boolean* m_p_show_log;
        adam::configuration_parameter_boolean* m_p_show_performance;
        adam::configuration_parameter_integer* m_p_gui_mode;
        adam::configuration_parameter_integer* m_p_fps_limit;
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