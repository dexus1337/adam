#pragma once

/**
 * @file    window-about.hpp
 * @author  dexus1337
 * @brief   Header for the about dialog in adam-cop.
 * @version 1.0
 * @date    20.08.2026
 */

#include <adam-core.hpp>

namespace adam::cop
{
    class cop_controller;

    /**
     * @brief Renders the About modal / dialog window for adam-cop.
     * @param ctrl Reference to the COP controller.
     * @param lang Current active user language.
     * @param show_about Reference to the boolean flag controlling dialog visibility.
     */
    void draw_about_dialog(cop_controller& ctrl, adam::language lang, bool& show_about);
}
