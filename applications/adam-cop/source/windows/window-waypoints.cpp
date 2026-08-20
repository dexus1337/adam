/**
 * @file    window-waypoints.cpp
 * @author  dexus1337
 * @brief   Implementation of the waypoints management window for adam-cop.
 * @version 1.0
 * @date    20.08.2026
 */

#include "window-waypoints.hpp"
#include "../cop-controller.hpp"
#include "../cop-strings.hpp"
#include "../map/world-map.hpp"
#include <imgui.h>
#include <string>

namespace adam::cop
{
    /**
     * @brief Renders the top toolbar for quickly adding a new waypoint.
     */
    static void draw_add_waypoint_toolbar(cop_controller& ctrl, adam::language lang)
    {
        (void)lang;
        static float  new_wp_lat   = 50.0f;
        static float  new_wp_lon   = 10.0f;
        static char   new_wp_label[64] = "";
        static ImVec4 new_wp_color = ImVec4(0.0f, 0.85f, 1.0f, 1.0f);

        float input_w = 72.0f;
        ImGui::SetNextItemWidth(input_w);
        ImGui::InputFloat("##NewWpLat", &new_wp_lat, 0.0f, 0.0f, "%.4f");
        ImGui::SameLine();
        ImGui::TextUnformatted("Lat");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(input_w);
        ImGui::InputFloat("##NewWpLon", &new_wp_lon, 0.0f, 0.0f, "%.4f");
        ImGui::SameLine();
        ImGui::TextUnformatted("Lon");
        ImGui::SameLine();
        
        float add_btn_width = ImGui::CalcTextSize("Add").x + ImGui::GetStyle().FramePadding.x * 2.0f;
        float color_edit_width = ImGui::GetFrameHeight();
        float right_space = add_btn_width + color_edit_width + ImGui::GetStyle().ItemSpacing.x * 2.0f;
        
        ImGui::SetNextItemWidth(-right_space);
        ImGui::InputTextWithHint("##WpLabel", "Waypoint Name", new_wp_label, sizeof(new_wp_label));
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Waypoint Name");
        }
        
        ImGui::SameLine();
        ImGui::ColorEdit4("##WpColor", (float*)&new_wp_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
        
        ImGui::SameLine();
        if (ImGui::Button("Add"))
        {
            uint32_t rgb = (static_cast<uint32_t>(new_wp_color.x * 255.0f) << 16) |
                           (static_cast<uint32_t>(new_wp_color.y * 255.0f) << 8) |
                           (static_cast<uint32_t>(new_wp_color.z * 255.0f));

            std::string label = new_wp_label;
            if (label.empty())
            {
                static uint32_t auto_wp_id = 1;
                char tag[32];
                snprintf(tag, sizeof(tag), "WP #%u", auto_wp_id++);
                label = tag;
            }

            auto wp = std::make_unique<waypoint>(adam::string_hashed(label));
            wp->set_lat(new_wp_lat);
            wp->set_lon(new_wp_lon);
            wp->set_color(rgb);
            ctrl.add_waypoint(std::move(wp));

            new_wp_label[0] = '\0';
        }
    }

    /**
     * @brief Renders an individual waypoint item row.
     */
    static bool draw_waypoint_item(cop_controller& ctrl, world_map& map, const std::unique_ptr<waypoint>& wp, adam::language lang)
    {
        (void)lang;
        if (!wp)
        {
            return false;
        }

        ImGui::PushID(wp.get());

        bool active = wp->is_enabled();
        if (ImGui::Checkbox("##Active", &active))
        {
            wp->set_enabled(active);
            ctrl.save_config();
        }

        ImGui::SameLine();
        uint32_t c = wp->get_color();
        ImVec4 col((float)((c >> 16) & 0xFF) / 255.0f, (float)((c >> 8) & 0xFF) / 255.0f, (float)(c & 0xFF) / 255.0f, 1.0f);
        if (ImGui::ColorEdit4("##Color", (float*)&col, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel))
        {
            uint32_t rgb = (static_cast<uint32_t>(col.x * 255.0f) << 16) |
                           (static_cast<uint32_t>(col.y * 255.0f) << 8) |
                           (static_cast<uint32_t>(col.z * 255.0f));
            wp->set_color(rgb);
            ctrl.save_config();
        }

        ImGui::SameLine();
        char lat_lon_text[64];
        snprintf(lat_lon_text, sizeof(lat_lon_text), "(%.4f, %.4f)", wp->get_lat(), wp->get_lon());
        
        // Fixed space reserved for the longest possible coordinate string (-180.0000, -180.0000)
        float max_coord_text_width = ImGui::CalcTextSize("(-180.0000, -180.0000)").x;
        float btn_jump_width = ImGui::CalcTextSize("Go to").x + ImGui::GetStyle().FramePadding.x * 2.0f;
        float btn_x_width = ImGui::CalcTextSize("X").x + ImGui::GetStyle().FramePadding.x * 2.0f;
        float total_right_width = max_coord_text_width + btn_jump_width + btn_x_width + ImGui::GetStyle().ItemSpacing.x * 3.0f;

        ImGui::SetNextItemWidth(-total_right_width);
        char label_buf[64];
        snprintf(label_buf, sizeof(label_buf), "%s", wp->get_label().c_str());
        if (ImGui::InputText("##Label", label_buf, sizeof(label_buf)))
        {
            wp->set_label(adam::string_hashed(&label_buf[0]));
            ctrl.save_config();
        }
        
        ImGui::SameLine();
        float coord_start_x = ImGui::GetWindowContentRegionMax().x - total_right_width + ImGui::GetStyle().ItemSpacing.x;
        ImGui::SetCursorPosX(coord_start_x);
        ImGui::TextUnformatted(lat_lon_text);

        ImGui::SameLine();
        float target_x = ImGui::GetWindowContentRegionMax().x - btn_jump_width - btn_x_width - ImGui::GetStyle().ItemSpacing.x;
        ImGui::SetCursorPosX(target_x);
        if (ImGui::Button("Go to"))
        {
            map.set_center(wp->get_lat(), wp->get_lon());
        }

        ImGui::SameLine();
        bool delete_requested = false;
        if (ImGui::Button("X"))
        {
            delete_requested = true;
        }

        ImGui::PopID();
        return delete_requested;
    }

    void draw_waypoints_window(cop_controller& ctrl, world_map& map, adam::language lang, adam::configuration_parameter_boolean* p_show_waypoints)
    {
        if (!p_show_waypoints || !p_show_waypoints->get_value())
        {
            return;
        }

        std::string title = std::string(get_cop_string(wnd_waypoints, lang)) + "###Waypoints";
        if (!ImGui::Begin(title.c_str(), &p_show_waypoints->value()))
        {
            ImGui::End();
            return;
        }

        draw_add_waypoint_toolbar(ctrl, lang);

        ImGui::Separator();

        const auto& waypoints = ctrl.get_waypoints();
        if (waypoints.empty())
        {
            ImGui::TextDisabled("%s", get_cop_string(lbl_no_markers, lang));
        }
        else
        {
            for (const auto& wp : waypoints)
            {
                if (draw_waypoint_item(ctrl, map, wp, lang))
                {
                    ctrl.remove_waypoint(wp->get_name().get_hash());
                    break;
                }
            }

            ImGui::Spacing();
            std::string modal_title_wp = std::string(get_cop_string(lbl_confirm_delete, lang)) + "###ClearAllWaypointsModal";
            if (ImGui::Button(get_cop_string(btn_clear_markers, lang), ImVec2(-1.0f, 0.0f)))
            {
                ImGui::OpenPopup(modal_title_wp.c_str());
            }

            if (ImGui::BeginPopupModal(modal_title_wp.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("%s", get_cop_string(msg_confirm_clear_waypoints, lang));
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                float btn_width = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
                if (ImGui::Button(get_cop_string(btn_delete_all, lang), ImVec2(btn_width, 0.0f)))
                {
                    ctrl.clear_waypoints();
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SetItemDefaultFocus();
                ImGui::SameLine();
                if (ImGui::Button(get_cop_string(btn_cancel, lang), ImVec2(btn_width, 0.0f)))
                {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }

        ImGui::End();
    }
}
