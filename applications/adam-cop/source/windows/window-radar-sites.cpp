/**
 * @file    window-radar-sites.cpp
 * @author  dexus1337
 * @brief   Implementation of the radar sites management window for adam-cop.
 * @version 1.0
 * @date    20.08.2026
 */

#include "window-radar-sites.hpp"
#include "../cop-controller.hpp"
#include "../cop-strings.hpp"
#include "../map/world-map.hpp"
#include <imgui.h>
#include <string>
#include <vector>
#include <algorithm>

namespace adam::cop
{
    /**
     * @brief Renders the top toolbar for auto-detection and adding new radar sites.
     */
    static void draw_add_site_toolbar(cop_controller& ctrl, adam::language lang)
    {
        bool auto_detect = ctrl.is_auto_detect_sites();
        if (ImGui::Checkbox(get_cop_string(lbl_auto_detect_sources, lang), &auto_detect))
        {
            ctrl.set_auto_detect_sites(auto_detect);
        }

        ImGui::Spacing();

        static int    new_site_sac   = 0;
        static int    new_site_sic   = 0;
        static char   new_site_label[64] = "";
        static ImVec4 new_site_color = ImVec4(0.0f, 0.85f, 1.0f, 1.0f);

        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("SAC:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(45.0f);
        if (ImGui::InputInt("##NewSAC", &new_site_sac, 0, 0))
        {
            if (new_site_sac < 0)
            {
                new_site_sac = 0;
            }
            if (new_site_sac > 255)
            {
                new_site_sac = 255;
            }
        }

        ImGui::SameLine();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("SIC:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(45.0f);
        if (ImGui::InputInt("##NewSIC", &new_site_sic, 0, 0))
        {
            if (new_site_sic < 0)
            {
                new_site_sic = 0;
            }
            if (new_site_sic > 255)
            {
                new_site_sic = 255;
            }
        }

        float add_btn_width = ImGui::CalcTextSize(get_cop_string(btn_add_site, lang)).x + ImGui::GetStyle().FramePadding.x * 2.0f;
        float color_edit_width = ImGui::GetFrameHeight();
        float right_space = add_btn_width + color_edit_width + ImGui::GetStyle().ItemSpacing.x * 2.0f;

        ImGui::SameLine();
        ImGui::SetNextItemWidth(-right_space);
        ImGui::InputTextWithHint("##NewSiteLabel", "Site Name", new_site_label, sizeof(new_site_label));

        ImGui::SameLine();
        ImGui::ColorEdit4("##NewSiteColor", (float*)&new_site_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

        ImGui::SameLine();
        if (ImGui::Button(get_cop_string(btn_add_site, lang)))
        {
            uint32_t rgb = (static_cast<uint32_t>(new_site_color.x * 255.0f) << 16) |
                           (static_cast<uint32_t>(new_site_color.y * 255.0f) << 8) |
                           (static_cast<uint32_t>(new_site_color.z * 255.0f));

            std::string label = new_site_label;
            if (label.empty())
            {
                char tag[64];
                snprintf(tag, sizeof(tag), "Site [%d/%d]", new_site_sac, new_site_sic);
                label = tag;
            }

            char id_tag[64];
            snprintf(id_tag, sizeof(id_tag), "site_%d_%d", new_site_sac, new_site_sic);

            auto s = std::make_unique<site>(adam::string_hashed(std::string(id_tag)));
            s->set_label(adam::string_hashed(label));
            s->set_sac(static_cast<uint8_t>(new_site_sac));
            s->set_sic(static_cast<uint8_t>(new_site_sic));
            s->set_color(rgb);
            ctrl.add_site(std::move(s));

            new_site_label[0] = '\0';
        }
    }

    /**
     * @brief Renders the expanded 2-column parameter form for a radar site.
     */
    static void draw_site_expanded_params(cop_controller& ctrl, const std::unique_ptr<site>& s, adam::language lang,
                                          adam::string_hash& picking_site_coords_hash)
    {
        if (!s)
        {
            return;
        }

        if (!ImGui::BeginTable("SiteParamsTable", 2, ImGuiTableFlags_None))
        {
            return;
        }

        ImGui::TableSetupColumn("LabelCol", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        ImGui::TableSetupColumn("ValueCol", ImGuiTableColumnFlags_WidthStretch);

        // Row 1: Site Name
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        ImGui::TextDisabled("Name");
        ImGui::TableSetColumnIndex(1);
        char label_buf[64];
        snprintf(label_buf, sizeof(label_buf), "%s", s->get_label().c_str());
        ImGui::SetNextItemWidth(-1.0f);
        if (ImGui::InputText("##EditName", label_buf, sizeof(label_buf)))
        {
            s->set_label(adam::string_hashed(&label_buf[0]));
            ctrl.save_config();
        }

        // Layout metrics for consistent vertical alignment across rows
        float input_w = 72.0f;
        float lat_text_w = ImGui::CalcTextSize("Lat").x + ImGui::GetStyle().ItemSpacing.x * 2.0f;
        float lon_start_offset = input_w + lat_text_w;
        float lon_text_w = ImGui::CalcTextSize("Lon").x + ImGui::GetStyle().ItemSpacing.x * 2.0f;
        float min_offset_for_lon = lon_start_offset + input_w + lon_text_w;

        float opt1_w = ImGui::GetFrameHeight() + ImGui::CalcTextSize(get_cop_string(lbl_show_range, lang)).x + ImGui::GetStyle().ItemInnerSpacing.x;
        float opt2_w = ImGui::GetFrameHeight() + ImGui::CalcTextSize(get_cop_string(lbl_show_sector_crossings, lang)).x + ImGui::GetStyle().ItemInnerSpacing.x;
        float max_opt_w = std::max(opt1_w, opt2_w) + ImGui::GetStyle().ItemSpacing.x * 2.0f;
        float align_start_offset = std::max(max_opt_w, min_offset_for_lon);

        // Row 2: SAC/SIC
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        ImGui::TextDisabled("SAC/SIC");
        ImGui::TableSetColumnIndex(1);

        float sac_start_x = ImGui::GetCursorPosX();
        int sac_val = static_cast<int>(s->get_sac());
        int sic_val = static_cast<int>(s->get_sic());
        ImGui::SetNextItemWidth(input_w);
        if (ImGui::InputInt("##EditSAC", &sac_val, 0, 0))
        {
            if (sac_val < 0)
            {
                sac_val = 0;
            }
            if (sac_val > 255)
            {
                sac_val = 255;
            }
            s->set_sac(sac_val);
            ctrl.save_config();
        }
        ImGui::SameLine();
        ImGui::TextUnformatted("/");
        ImGui::SameLine();
        ImGui::SetCursorPosX(sac_start_x + lon_start_offset);
        ImGui::SetNextItemWidth(input_w);
        if (ImGui::InputInt("##EditSIC", &sic_val, 0, 0))
        {
            if (sic_val < 0)
            {
                sic_val = 0;
            }
            if (sic_val > 255)
            {
                sic_val = 255;
            }
            s->set_sic(sic_val);
            ctrl.save_config();
        }

        // Row 3: Position (Lat / Lon) & Set from Map & Auto
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        ImGui::TextDisabled("Position");
        ImGui::TableSetColumnIndex(1);

        float pos_start_x = ImGui::GetCursorPosX();
        float lat_val = s->get_lat();
        float lon_val = s->get_lon();
        ImGui::SetNextItemWidth(input_w);
        if (ImGui::InputFloat("##EditLat", &lat_val, 0.0f, 0.0f, "%.4f"))
        {
            s->set_lat(lat_val);
            ctrl.save_config();
        }
        ImGui::SameLine();
        ImGui::TextUnformatted("Lat");
        ImGui::SameLine();
        ImGui::SetCursorPosX(pos_start_x + lon_start_offset);
        ImGui::SetNextItemWidth(input_w);
        if (ImGui::InputFloat("##EditLon", &lon_val, 0.0f, 0.0f, "%.4f"))
        {
            s->set_lon(lon_val);
            ctrl.save_config();
        }
        ImGui::SameLine();
        ImGui::TextUnformatted("Lon");

        // Map Picking button aligned with the opacity sliders
        ImGui::SameLine();
        ImGui::SetCursorPosX(pos_start_x + align_start_offset);
        bool is_picking = (picking_site_coords_hash == s->get_name().get_hash());
        if (is_picking)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.65f, 0.95f, 1.0f));
        }
        if (ImGui::Button(is_picking ? "Picking...##MapPick" : get_cop_string(btn_set_from_map, lang)))
        {
            picking_site_coords_hash = is_picking ? 0 : s->get_name().get_hash();
        }
        if (is_picking)
        {
            ImGui::PopStyleColor();
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("%s", get_cop_string(lbl_picking_map_pos, lang));
        }

        // Auto Coordinates Checkbox dynamically aligned to the far right
        float auto_w = ImGui::GetFrameHeight() + ImGui::CalcTextSize(get_cop_string(lbl_auto, lang)).x + ImGui::GetStyle().ItemInnerSpacing.x + 4.0f;
        float right_pos_x = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - auto_w;
        ImGui::SameLine();
        if (right_pos_x > ImGui::GetCursorPosX())
        {
            ImGui::SetCursorPosX(right_pos_x);
        }

        bool auto_coords = s->get_auto_retrieve_coords();
        char auto_coords_label[32];
        snprintf(auto_coords_label, sizeof(auto_coords_label), "%s##Coords", get_cop_string(lbl_auto, lang));
        if (ImGui::Checkbox(auto_coords_label, &auto_coords))
        {
            s->set_auto_retrieve_coords(auto_coords);
            ctrl.save_config();
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("%s", get_cop_string(tooltip_cat034_north_marker, lang));
        }

        // Row 4: Range (NM) & Auto
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        ImGui::TextDisabled("Range");
        ImGui::TableSetColumnIndex(1);

        float range_val = static_cast<float>(s->get_range_nm());
        ImGui::SetNextItemWidth(input_w);
        if (ImGui::InputFloat("##EditRange", &range_val, 0.0f, 0.0f, "%.1f"))
        {
            if (range_val < 0.0f)
            {
                range_val = 0.0f;
            }
            s->set_range_nm(static_cast<double>(range_val));
            ctrl.save_config();
        }
        ImGui::SameLine();
        ImGui::TextUnformatted("NM");

        float right_range_x = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - auto_w;
        ImGui::SameLine();
        if (right_range_x > ImGui::GetCursorPosX())
        {
            ImGui::SetCursorPosX(right_range_x);
        }

        bool auto_calc_r = s->get_auto_calc_range();
        char auto_range_label[32];
        snprintf(auto_range_label, sizeof(auto_range_label), "%s##Range", get_cop_string(lbl_auto, lang));
        if (ImGui::Checkbox(auto_range_label, &auto_calc_r))
        {
            s->set_auto_calc_range(auto_calc_r);
            ctrl.save_config();
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("%s", get_cop_string(tooltip_auto_calc_range, lang));
        }

        // Row 5: Options (Show Range & Show Sector Crossings with Sliders)
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        ImGui::TextDisabled("%s", get_cop_string(lbl_options, lang));
        ImGui::TableSetColumnIndex(1);

        float opt_start_x1 = ImGui::GetCursorPosX();
        bool show_range = s->get_show_range();
        if (ImGui::Checkbox(get_cop_string(lbl_show_range, lang), &show_range))
        {
            s->set_show_range(show_range);
            ctrl.save_config();
        }
        ImGui::SameLine();
        ImGui::SetCursorPosX(opt_start_x1 + align_start_offset);
        float range_alpha = static_cast<float>(s->get_range_alpha());
        ImGui::SetNextItemWidth(-1.0f);
        if (ImGui::SliderFloat("##RangeAlpha", &range_alpha, 0.0f, 1.0f, "%.2f"))
        {
            s->set_range_alpha(static_cast<double>(range_alpha));
            ctrl.save_config();
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Range Opacity");
        }

        float opt_start_x2 = ImGui::GetCursorPosX();
        bool show_crossings = s->get_show_sector_crossings();
        if (ImGui::Checkbox(get_cop_string(lbl_show_sector_crossings, lang), &show_crossings))
        {
            s->set_show_sector_crossings(show_crossings);
            ctrl.save_config();
        }
        ImGui::SameLine();
        ImGui::SetCursorPosX(opt_start_x2 + align_start_offset);
        float sector_alpha = static_cast<float>(s->get_sector_crossings_alpha());
        ImGui::SetNextItemWidth(-1.0f);
        if (ImGui::SliderFloat("##SectorAlpha", &sector_alpha, 0.0f, 1.0f, "%.2f"))
        {
            s->set_sector_crossings_alpha(static_cast<double>(sector_alpha));
            ctrl.save_config();
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Sector Crossings Opacity");
        }

        ImGui::EndTable();
    }

    /**
     * @brief Renders a radar site item row with header and expandable body.
     */
    static bool draw_site_item(cop_controller& ctrl, world_map& map, const std::unique_ptr<site>& s, adam::language lang,
                               std::vector<adam::string_hash>& expanded_sites, adam::string_hash& picking_site_coords_hash)
    {
        if (!s)
        {
            return false;
        }

        adam::string_hash s_hash = s->get_name().get_hash();
        auto it = std::find(expanded_sites.begin(), expanded_sites.end(), s_hash);
        bool is_open = (it != expanded_sites.end());

        ImGui::PushID(s.get());

        // 1. Left Expand Button
        if (ImGui::ArrowButton("##Exp", is_open ? ImGuiDir_Down : ImGuiDir_Right))
        {
            if (is_open)
            {
                expanded_sites.erase(it);
            }
            else
            {
                expanded_sites.push_back(s_hash);
            }
            is_open = !is_open;
        }

        // 2. Active Toggle
        ImGui::SameLine();
        bool active = s->is_enabled();
        if (ImGui::Checkbox("##Active", &active))
        {
            s->set_enabled(active);
            ctrl.save_config();
        }

        // 3. Color Swatch
        ImGui::SameLine();
        uint32_t c = s->get_color();
        ImVec4 col((float)((c >> 16) & 0xFF) / 255.0f, (float)((c >> 8) & 0xFF) / 255.0f, (float)(c & 0xFF) / 255.0f, 1.0f);
        if (ImGui::ColorEdit4("##Color", (float*)&col, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel))
        {
            uint32_t rgb = (static_cast<uint32_t>(col.x * 255.0f) << 16) |
                           (static_cast<uint32_t>(col.y * 255.0f) << 8) |
                           (static_cast<uint32_t>(col.z * 255.0f));
            s->set_color(rgb);
            ctrl.save_config();
        }

        // 4. Site Label + SAC/SIC Info
        ImGui::SameLine();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("%s", s->get_label().c_str());
        ImGui::SameLine();
        ImGui::TextDisabled("[SAC: %d, SIC: %d]", static_cast<int>(s->get_sac()), static_cast<int>(s->get_sic()));

        // 5. Actions: "Go to" and Delete "X"
        float btn_goto_width = ImGui::CalcTextSize("Go to").x + ImGui::GetStyle().FramePadding.x * 2.0f;
        float btn_del_width = ImGui::CalcTextSize("X").x + ImGui::GetStyle().FramePadding.x * 2.0f;

        ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - btn_goto_width - btn_del_width - ImGui::GetStyle().ItemSpacing.x);
        if (ImGui::Button("Go to"))
        {
            map.set_center(s->get_lat(), s->get_lon());
        }

        ImGui::SameLine();
        bool delete_requested = false;
        if (ImGui::Button("X"))
        {
            delete_requested = true;
        }

        // 6. Expanded Parameters Form
        if (is_open)
        {
            ImGui::Spacing();
            ImGui::Indent(24.0f);
            draw_site_expanded_params(ctrl, s, lang, picking_site_coords_hash);
            ImGui::Unindent(24.0f);
            ImGui::Spacing();
        }

        ImGui::Separator();
        ImGui::PopID();
        return delete_requested;
    }

    void draw_radar_sites_window(cop_controller& ctrl, world_map& map, adam::language lang, 
                                 adam::configuration_parameter_boolean* p_show_sites,
                                 adam::string_hash& picking_site_coords_hash)
    {
        if (!p_show_sites || !p_show_sites->get_value())
        {
            return;
        }

        static std::vector<adam::string_hash> s_expanded_sites;

        std::string title = std::string(get_cop_string(wnd_sites, lang)) + "###RadarSites";
        if (!ImGui::Begin(title.c_str(), &p_show_sites->value()))
        {
            ImGui::End();
            return;
        }

        draw_add_site_toolbar(ctrl, lang);

        ImGui::Separator();

        const auto& sites = ctrl.get_sites();
        if (sites.empty())
        {
            ImGui::TextDisabled("%s", get_cop_string(lbl_no_sites, lang));
        }
        else
        {
            for (const auto& s : sites)
            {
                if (draw_site_item(ctrl, map, s, lang, s_expanded_sites, picking_site_coords_hash))
                {
                    ctrl.remove_site(s->get_name().get_hash());
                    break;
                }
            }

            ImGui::Spacing();
            std::string modal_title_sites = std::string(get_cop_string(lbl_confirm_delete, lang)) + "###ClearAllSitesModal";
            if (ImGui::Button(get_cop_string(btn_clear_sites, lang), ImVec2(-1.0f, 0.0f)))
            {
                ImGui::OpenPopup(modal_title_sites.c_str());
            }

            if (ImGui::BeginPopupModal(modal_title_sites.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("%s", get_cop_string(msg_confirm_clear_sites, lang));
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                float btn_width = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
                if (ImGui::Button(get_cop_string(btn_delete_all, lang), ImVec2(btn_width, 0.0f)))
                {
                    ctrl.clear_sites();
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
