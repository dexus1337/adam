#pragma once

/**
 * @file    main-window.hpp
 * @author  dexus1337
 * @brief   Operator interface main window declaration for adam-cop
 * @version 1.0
 * @date    05.08.2026
 */

#include <adam-core.hpp>
#include "../cop-controller.hpp"
#include "../map/world-map.hpp"
#include "../cop-strings.hpp"
#include "window-about.hpp"
#include "window-layers.hpp"
#include "window-waypoints.hpp"
#include "window-radar-sites.hpp"
#include "window-data-sources.hpp"
#include "window-cache-stats.hpp"
#include "window-jump-coords.hpp"
#include "window-performance.hpp"

struct SDL_Window;

namespace adam::cop
{
    enum class cop_color_id : size_t
    {
        commander_connected = 0,
        commander_disconnected,
        node_connection_line,
        node_connection_line_light,
        node_pin_active,
        count
    };

    const ImVec4& get_cop_color(cop_color_id id);

    class main_window
    {
    public:
        main_window(cop_controller& ctrl, SDL_Window* window);
        ~main_window() = default;

        void save_window_state();
        void draw();

    private:
        void draw_menu_bar(adam::language lang);
        void draw_status_bar(adam::language lang);
        void draw_map_window(adam::language lang);

        cop_controller& m_ctrl;
        SDL_Window* m_window;

        world_map m_map;
        map_render_options m_map_options;

        std::array<map_layer_config_params, 4> m_p_map_layer_params;
        adam::configuration_parameter_integer* m_p_map_projection     = nullptr;
        adam::configuration_parameter_boolean* m_p_show_grid          = nullptr;
        adam::configuration_parameter_boolean* m_p_show_coastlines    = nullptr;
        adam::configuration_parameter_boolean* m_p_show_land_fill     = nullptr;
        adam::configuration_parameter_boolean* m_p_show_scale_bar     = nullptr;
        adam::configuration_parameter_boolean* m_p_show_performance   = nullptr;
        adam::configuration_parameter_integer* m_p_perf_ovly_location = nullptr;
        adam::configuration_parameter_double*  m_p_perf_ovly_x        = nullptr;
        adam::configuration_parameter_double*  m_p_perf_ovly_y        = nullptr;
        adam::configuration_parameter_integer* m_p_perf_ovly_content  = nullptr;
        adam::configuration_parameter_integer* m_p_fps_limit          = nullptr;
        adam::configuration_parameter_integer* m_p_language           = nullptr;
        adam::configuration_parameter_string*  m_p_theme              = nullptr;
        adam::configuration_parameter_integer* m_p_gui_mode           = nullptr;
        adam::configuration_parameter_double*  m_p_font_scale         = nullptr;
        adam::configuration_parameter_string*  m_p_docking_layout     = nullptr;
        adam::configuration_parameter_double*  m_p_map_lat            = nullptr;
        adam::configuration_parameter_double*  m_p_map_lon            = nullptr;
        adam::configuration_parameter_double*  m_p_map_zoom           = nullptr;

        adam::configuration_parameter_boolean* m_p_show_control_panel = nullptr;
        adam::configuration_parameter_boolean* m_p_show_waypoints     = nullptr;
        adam::configuration_parameter_boolean* m_p_show_sites         = nullptr;
        adam::configuration_parameter_boolean* m_p_show_cache_stats   = nullptr;
        adam::configuration_parameter_boolean* m_p_show_jump_coords   = nullptr;
        adam::configuration_parameter_boolean* m_p_show_data_sources  = nullptr;

        adam::configuration_parameter_integer* m_p_window_x           = nullptr;
        adam::configuration_parameter_integer* m_p_window_y           = nullptr;
        adam::configuration_parameter_integer* m_p_window_w           = nullptr;
        adam::configuration_parameter_integer* m_p_window_h           = nullptr;
        adam::configuration_parameter_boolean* m_p_window_maximized   = nullptr;

        float m_jump_lat = 0.0f;
        float m_jump_lon = 0.0f;
        bool  m_show_about = false;
        adam::string_hash m_picking_site_coords_hash = 0;

        adam::language m_last_lang;
    };
}
