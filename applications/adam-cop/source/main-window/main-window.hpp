#pragma once

/**
 * @file    main-window.hpp
 * @author  dexus1337
 * @brief   Operator interface main window declaration for adam-cop
 * @version 1.0
 * @date    05.08.2026
 */

#include <adam-sdk.hpp>
#include "../cop-controller.hpp"
#include "../map/world-map.hpp"
#include "cop-strings.hpp"

struct SDL_Window;

namespace adam::cop
{
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
        void draw_control_panel(adam::language lang);
        void draw_map_window(adam::language lang);
        void draw_about_dialog(adam::language lang);

        cop_controller& m_ctrl;
        SDL_Window* m_window;

        world_map m_map;
        map_render_options m_map_options;

        adam::configuration_parameter_integer* m_p_base_provider;
        adam::configuration_parameter_double*  m_p_map_opacity;
        adam::configuration_parameter_integer* m_p_map_projection;
        adam::configuration_parameter_boolean* m_p_show_grid;
        adam::configuration_parameter_boolean* m_p_show_coastlines;
        adam::configuration_parameter_boolean* m_p_show_land_fill;
        adam::configuration_parameter_boolean* m_p_show_scale_bar;
        adam::configuration_parameter_boolean* m_p_show_performance;
        adam::configuration_parameter_integer* m_p_fps_limit;
        adam::configuration_parameter_integer* m_p_language;
        adam::configuration_parameter_string*  m_p_theme;

        float m_jump_lat = 0.0f;
        float m_jump_lon = 0.0f;
        bool  m_show_about = false;
        bool  m_show_control_panel = true;

        adam::language m_last_lang;
    };
}
