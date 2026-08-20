/**
 * @file    window-jump-coords.cpp
 * @author  dexus1337
 * @brief   Implementation of the coordinate jump helper window for adam-cop.
 * @version 1.0
 * @date    20.08.2026
 */

#include "window-jump-coords.hpp"
#include "../cop-strings.hpp"
#include "../map/world-map.hpp"
#include <imgui.h>
#include <string>

namespace adam::cop
{
    void draw_jump_coords_window(world_map& map, adam::language lang, 
                                adam::configuration_parameter_boolean* p_show_jump_coords,
                                float& jump_lat, float& jump_lon)
    {
        if (!p_show_jump_coords || !p_show_jump_coords->get_value())
        {
            return;
        }

        std::string title_jump = std::string(get_cop_string(lbl_jump_to_coordinates, lang)) + "###JumpCoords";
        if (!ImGui::Begin(title_jump.c_str(), &p_show_jump_coords->value()))
        {
            ImGui::End();
            return;
        }

        ImGui::InputFloat(get_cop_string(lbl_lat, lang), &jump_lat, 1.0f, 5.0f, "%.4f");
        ImGui::InputFloat(get_cop_string(lbl_lon, lang), &jump_lon, 1.0f, 5.0f, "%.4f");

        if (ImGui::Button(get_cop_string(btn_jump, lang), ImVec2(-1.0f, 0.0f)))
        {
            map.set_center(jump_lat, jump_lon);
        }

        if (ImGui::Button(get_cop_string(btn_reset_camera, lang), ImVec2(-1.0f, 0.0f)))
        {
            map.reset_view();
        }

        ImGui::End();
    }
}
