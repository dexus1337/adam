#pragma once

#include <adam-sdk.hpp>
#include "gui-controller.hpp"

struct SDL_Window;

namespace adam::gui 
{
    enum class gui_string_id
    {
        main_ui,
        menu_view,
        menu_show_log,
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
        tbl_message
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
        gui_controller& m_ctrl;
        SDL_Window* m_window;
    };
}