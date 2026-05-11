#pragma once

#include <adam-sdk.hpp>
#include "gui-controller.hpp"

namespace adam::gui 
{
    class main_window 
    {
    public:
        main_window(gui_controller& ctrl);
        ~main_window();

        void render();

    private:
        gui_controller& m_ctrl;
    };
}