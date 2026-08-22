#pragma once

/**
 * @file    window-data-sources.hpp
 * @author  dexus1337
 * @brief   Header for the data sources / ASTERIX streams window in adam-cop.
 * @version 1.0
 * @date    20.08.2026
 */

#include <adam-core.hpp>

namespace adam::cop
{
    class cop_controller;

    /**
     * @brief Renders the Data Sources / ASTERIX streams window.
     * @param ctrl Reference to the COP controller.
     * @param lang Current active user language.
     * @param p_show_data_sources Configuration parameter controlling window visibility.
     */
    void draw_data_sources_window(cop_controller& ctrl, adam::language lang, adam::configuration_parameter_boolean* p_show_data_sources);
}
