#pragma once

#include <adam-sdk.hpp>
#include "management/shared-state.hpp"
#include "management/modals.hpp"
#include "management/node.hpp"
#include "management/connection-card.hpp"
#include "management/inspector.hpp"

namespace adam::gui 
{
    class gui_controller;

    void render_tab_management(gui_controller& ctrl, adam::language lang);
}
