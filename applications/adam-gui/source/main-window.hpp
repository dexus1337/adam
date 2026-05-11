#pragma once

#include <adam-sdk.hpp>
#include "gui-controller.hpp"

struct SDL_Window;

namespace adam::gui 
{
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