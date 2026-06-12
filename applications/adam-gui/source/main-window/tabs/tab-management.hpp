#pragma once

/**
 * @file    tab-management.hpp
 * @author  dexus1337
 * @brief   Header for the management tab drawing entry point.
 * @version 1.0
 * @date    12.06.2026
 */

#include <adam-sdk.hpp>
#include "management/shared-state.hpp"
#include "management/modals.hpp"
#include "management/node.hpp"
#include "management/connection-card.hpp"
#include "management/inspector.hpp"

namespace adam::gui 
{
    class gui_controller;

    void draw_tab_management(gui_controller& ctrl, adam::language lang);
}
