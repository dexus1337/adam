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
#include <unordered_set>
#include <algorithm>
#include <cctype>

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

        static int    new_site_sac       = 0;
        static int    new_site_sic       = 0;
        static char   new_site_label[64] = "";
        static ImVec4 new_site_color     = []()
        {
            uint32_t c = site::generate_random_color();
            return ImVec4(((c >> 16) & 0xFF) / 255.0f, ((c >> 8) & 0xFF) / 255.0f, (c & 0xFF) / 255.0f, 1.0f);
        }();

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

        float add_btn_width    = ImGui::CalcTextSize(get_cop_string(btn_add_site, lang)).x + ImGui::GetStyle().FramePadding.x * 2.0f;
        float color_edit_width = ImGui::GetFrameHeight();
        float right_space      = add_btn_width + color_edit_width + ImGui::GetStyle().ItemSpacing.x * 2.0f;

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

            auto s = std::make_unique<drawable_site>(adam::string_hashed(std::string(id_tag)));
            s->set_label(adam::string_hashed(label));
            s->set_sac(static_cast<uint8_t>(new_site_sac));
            s->set_sic(static_cast<uint8_t>(new_site_sic));
            s->set_color(rgb);
            ctrl.add_site(std::move(s));

            new_site_label[0] = '\0';
            uint32_t next_c = site::generate_random_color();
            new_site_color = ImVec4(((next_c >> 16) & 0xFF) / 255.0f, ((next_c >> 8) & 0xFF) / 255.0f, (next_c & 0xFF) / 255.0f, 1.0f);
        }
    }

    /**
     * @brief Checks if a site matches the given search filter string.
     */
    static bool site_matches_filter(const drawable_site* s, const char* filter)
    {
        if (!s)
        {
            return false;
        }

        if (!filter || filter[0] == '\0')
        {
            return true;
        }

        // Case-insensitive substring search in site label
        const std::string& label = s->get_label().string();
        auto it = std::search(label.begin(), label.end(), filter, filter + strlen(filter),
                              [](char ch1, char ch2) { return std::tolower(ch1) == std::tolower(ch2); });
        if (it != label.end())
        {
            return true;
        }

        // Search in SAC/SIC representation
        char sac_buf[32];
        snprintf(sac_buf, sizeof(sac_buf), "%lld", static_cast<long long>(s->get_sac()));
        if (strstr(sac_buf, filter))
        {
            return true;
        }

        char sic_buf[32];
        snprintf(sic_buf, sizeof(sic_buf), "%lld", static_cast<long long>(s->get_sic()));
        if (strstr(sic_buf, filter))
        {
            return true;
        }

        char sacsic_buf[64];
        snprintf(sacsic_buf, sizeof(sacsic_buf), "%lld/%lld", static_cast<long long>(s->get_sac()), static_cast<long long>(s->get_sic()));
        if (strstr(sacsic_buf, filter))
        {
            return true;
        }

        return false;
    }

    /**
     * @brief Renders the expanded 2-column parameter form for a radar site.
     */
    static void draw_site_expanded_params(cop_controller& ctrl, const std::unique_ptr<drawable_site>& s, adam::language lang,
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
        float input_w            = 72.0f;
        float lat_text_w         = ImGui::CalcTextSize("Lat").x + ImGui::GetStyle().ItemSpacing.x * 2.0f;
        float lon_start_offset   = input_w + lat_text_w;
        float lon_text_w         = ImGui::CalcTextSize("Lon").x + ImGui::GetStyle().ItemSpacing.x * 2.0f;
        float min_offset_for_lon = lon_start_offset + input_w + lon_text_w;

        float opt1_w             = ImGui::GetFrameHeight() + ImGui::CalcTextSize(get_cop_string(lbl_show_range, lang)).x + ImGui::GetStyle().ItemInnerSpacing.x;
        float opt2_w             = ImGui::GetFrameHeight() + ImGui::CalcTextSize(get_cop_string(lbl_show_azimuth, lang)).x + ImGui::GetStyle().ItemInnerSpacing.x;
        float opt3_w             = ImGui::GetFrameHeight() + ImGui::CalcTextSize(get_cop_string(lbl_show_sectors, lang)).x + ImGui::GetStyle().ItemInnerSpacing.x;
        float max_opt_w          = std::max({ opt1_w, opt2_w, opt3_w }) + ImGui::GetStyle().ItemSpacing.x * 2.0f;
        float align_start_offset = std::max(max_opt_w, min_offset_for_lon);

        // Row 2: SAC/SIC
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        ImGui::TextDisabled("SAC/SIC");
        ImGui::TableSetColumnIndex(1);

        float sac_start_x = ImGui::GetCursorPosX();
        int sac_val       = static_cast<int>(s->get_sac());
        int sic_val       = static_cast<int>(s->get_sic());
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
        float lat_val     = s->get_lat();
        float lon_val     = s->get_lon();
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
        float auto_w      = ImGui::GetFrameHeight() + ImGui::CalcTextSize(get_cop_string(lbl_auto, lang)).x + ImGui::GetStyle().ItemInnerSpacing.x + 4.0f;
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

        // Row 5: Drawing Settings (Show Range, Azimuth, Sectors with Opacity Sliders)
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        ImGui::TextDisabled("%s", get_cop_string(lbl_options, lang));
        ImGui::TableSetColumnIndex(1);

        // Option 1: Range
        float opt_start_x1 = ImGui::GetCursorPosX();
        bool  show_range   = s->get_show_range();
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

        // Option 2: Azimuth
        float opt_start_x2 = ImGui::GetCursorPosX();
        bool  show_azimuth = s->get_show_azimuth();
        if (ImGui::Checkbox(get_cop_string(lbl_show_azimuth, lang), &show_azimuth))
        {
            s->set_show_azimuth(show_azimuth);
            ctrl.save_config();
        }
        ImGui::SameLine();
        ImGui::SetCursorPosX(opt_start_x2 + align_start_offset);
        float az_alpha = static_cast<float>(s->get_azimuth_alpha());
        ImGui::SetNextItemWidth(-1.0f);
        if (ImGui::SliderFloat("##AzimuthAlpha", &az_alpha, 0.0f, 1.0f, "%.2f"))
        {
            s->set_azimuth_alpha(static_cast<double>(az_alpha));
            ctrl.save_config();
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Azimuth Beam Opacity");
        }

        // Option 3: Sectors
        float opt_start_x3 = ImGui::GetCursorPosX();
        bool  show_sectors = s->get_show_sectors();
        if (ImGui::Checkbox(get_cop_string(lbl_show_sectors, lang), &show_sectors))
        {
            s->set_show_sectors(show_sectors);
            ctrl.save_config();
        }
        ImGui::SameLine();
        ImGui::SetCursorPosX(opt_start_x3 + align_start_offset);
        float sector_alpha = static_cast<float>(s->get_sectors_alpha());
        ImGui::SetNextItemWidth(-1.0f);
        if (ImGui::SliderFloat("##SectorAlpha", &sector_alpha, 0.0f, 1.0f, "%.2f"))
        {
            s->set_sectors_alpha(static_cast<double>(sector_alpha));
            ctrl.save_config();
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Sectors Opacity");
        }

        // Option 4, 5, 6: Name, SAC/SIC, Rotation Duration Toggles
        bool show_name     = s->get_show_name();
        bool show_sacsic   = s->get_show_sacsic();
        bool show_rot_dur  = s->get_show_rotation_duration();

        if (ImGui::Checkbox(get_cop_string(lbl_show_name, lang), &show_name))
        {
            s->set_show_name(show_name);
            ctrl.save_config();
        }
        ImGui::SameLine();
        if (ImGui::Checkbox(get_cop_string(lbl_show_sacsic, lang), &show_sacsic))
        {
            s->set_show_sacsic(show_sacsic);
            ctrl.save_config();
        }
        ImGui::SameLine();
        if (ImGui::Checkbox(get_cop_string(lbl_show_rotation_duration, lang), &show_rot_dur))
        {
            s->set_show_rotation_duration(show_rot_dur);
            ctrl.save_config();
        }

        // Row 6: Live CAT034 Antenna Rotation & Sector Crossing
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        ImGui::TextDisabled("CAT034");
        ImGui::TableSetColumnIndex(1);

        if (s->has_live_rotation())
        {
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f), "Az: %.1f°", s->get_current_azimuth_deg());
            ImGui::SameLine();
            ImGui::TextDisabled("|");
            ImGui::SameLine();
            ImGui::Text("Sec: %u/256", static_cast<uint32_t>(s->get_current_sector()));
            ImGui::SameLine();
            ImGui::TextDisabled("|");
            ImGui::SameLine();
            ImGui::Text("%.2f s", s->get_rotation_period_s());
            ImGui::SameLine();
            ImGui::TextDisabled("(%.1f RPM)", s->get_rotation_rpm());
        }
        else
        {
            ImGui::TextDisabled("Waiting for CAT034 sector crossings...");
        }

        ImGui::EndTable();
    }

    /**
     * @brief Renders the context menu for bulk actions on selected sites.
     */
    static void draw_bulk_context_menu(cop_controller& ctrl, const std::unordered_set<adam::string_hash>& selected_hashes, 
                                       adam::language lang, bool& delete_selected_requested)
    {
        if (ImGui::BeginPopup("SiteContextMenu"))
        {
            ImGui::TextDisabled("Selected: %zu site(s)", selected_hashes.size());
            ImGui::Separator();

            if (ImGui::MenuItem(get_cop_string(menu_enable_selected, lang), nullptr, false, !selected_hashes.empty()))
            {
                for (auto& s : ctrl.sites())
                {
                    if (s && selected_hashes.count(s->get_name().get_hash()))
                    {
                        s->set_enabled(true);
                    }
                }
                ctrl.save_config();
            }

            if (ImGui::MenuItem(get_cop_string(menu_disable_selected, lang), nullptr, false, !selected_hashes.empty()))
            {
                for (auto& s : ctrl.sites())
                {
                    if (s && selected_hashes.count(s->get_name().get_hash()))
                    {
                        s->set_enabled(false);
                    }
                }
                ctrl.save_config();
            }

            ImGui::Separator();

            if (ImGui::BeginMenu(get_cop_string(lbl_options, lang), !selected_hashes.empty()))
            {
                // Range
                if (ImGui::MenuItem("Enable Range", nullptr, false))
                {
                    for (auto& s : ctrl.sites())
                    {
                        if (s && selected_hashes.count(s->get_name().get_hash())) s->set_show_range(true);
                    }
                    ctrl.save_config();
                }
                if (ImGui::MenuItem("Disable Range", nullptr, false))
                {
                    for (auto& s : ctrl.sites())
                    {
                        if (s && selected_hashes.count(s->get_name().get_hash())) s->set_show_range(false);
                    }
                    ctrl.save_config();
                }
                ImGui::Separator();

                // Azimuth
                if (ImGui::MenuItem("Enable Azimuth", nullptr, false))
                {
                    for (auto& s : ctrl.sites())
                    {
                        if (s && selected_hashes.count(s->get_name().get_hash())) s->set_show_azimuth(true);
                    }
                    ctrl.save_config();
                }
                if (ImGui::MenuItem("Disable Azimuth", nullptr, false))
                {
                    for (auto& s : ctrl.sites())
                    {
                        if (s && selected_hashes.count(s->get_name().get_hash())) s->set_show_azimuth(false);
                    }
                    ctrl.save_config();
                }
                ImGui::Separator();

                // Sectors
                if (ImGui::MenuItem("Enable Sectors", nullptr, false))
                {
                    for (auto& s : ctrl.sites())
                    {
                        if (s && selected_hashes.count(s->get_name().get_hash())) s->set_show_sectors(true);
                    }
                    ctrl.save_config();
                }
                if (ImGui::MenuItem("Disable Sectors", nullptr, false))
                {
                    for (auto& s : ctrl.sites())
                    {
                        if (s && selected_hashes.count(s->get_name().get_hash())) s->set_show_sectors(false);
                    }
                    ctrl.save_config();
                }
                ImGui::Separator();

                // Name
                if (ImGui::MenuItem("Enable Name", nullptr, false))
                {
                    for (auto& s : ctrl.sites())
                    {
                        if (s && selected_hashes.count(s->get_name().get_hash())) s->set_show_name(true);
                    }
                    ctrl.save_config();
                }
                if (ImGui::MenuItem("Disable Name", nullptr, false))
                {
                    for (auto& s : ctrl.sites())
                    {
                        if (s && selected_hashes.count(s->get_name().get_hash())) s->set_show_name(false);
                    }
                    ctrl.save_config();
                }
                ImGui::Separator();

                // SAC/SIC
                if (ImGui::MenuItem("Enable SAC/SIC", nullptr, false))
                {
                    for (auto& s : ctrl.sites())
                    {
                        if (s && selected_hashes.count(s->get_name().get_hash())) s->set_show_sacsic(true);
                    }
                    ctrl.save_config();
                }
                if (ImGui::MenuItem("Disable SAC/SIC", nullptr, false))
                {
                    for (auto& s : ctrl.sites())
                    {
                        if (s && selected_hashes.count(s->get_name().get_hash())) s->set_show_sacsic(false);
                    }
                    ctrl.save_config();
                }
                ImGui::Separator();

                // Rotation Duration
                if (ImGui::MenuItem("Enable Rotation Duration", nullptr, false))
                {
                    for (auto& s : ctrl.sites())
                    {
                        if (s && selected_hashes.count(s->get_name().get_hash())) s->set_show_rotation_duration(true);
                    }
                    ctrl.save_config();
                }
                if (ImGui::MenuItem("Disable Rotation Duration", nullptr, false))
                {
                    for (auto& s : ctrl.sites())
                    {
                        if (s && selected_hashes.count(s->get_name().get_hash())) s->set_show_rotation_duration(false);
                    }
                    ctrl.save_config();
                }

                ImGui::EndMenu();
            }

            ImGui::Separator();

            if (ImGui::MenuItem(get_cop_string(menu_delete_selected, lang), nullptr, false, !selected_hashes.empty()))
            {
                delete_selected_requested = true;
            }

            ImGui::EndPopup();
        }
    }

    /**
     * @brief Renders a radar site item row with header and expandable body.
     */
    static bool draw_site_item(cop_controller& ctrl, world_map& map, const std::unique_ptr<drawable_site>& s, adam::language lang,
                               std::vector<adam::string_hash>& expanded_sites, adam::string_hash& picking_site_coords_hash,
                               std::unordered_set<adam::string_hash>& selected_sites)
    {
        if (!s)
        {
            return false;
        }

        adam::string_hash s_hash = s->get_name().get_hash();
        auto it = std::find(expanded_sites.begin(), expanded_sites.end(), s_hash);
        bool is_open = (it != expanded_sites.end());

        ImGui::PushID(s.get());

        // 0. Selection Checkbox for Bulk Actions
        bool is_selected = (selected_sites.find(s_hash) != selected_sites.end());
        if (ImGui::Checkbox("##Select", &is_selected))
        {
            if (is_selected)
            {
                selected_sites.insert(s_hash);
            }
            else
            {
                selected_sites.erase(s_hash);
            }
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Select for bulk actions");
        }

        // 1. Left Expand Button
        ImGui::SameLine();
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

        // Right-click context menu trigger on row item
        if (ImGui::IsItemClicked(ImGuiMouseButton_Right) || ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            if (selected_sites.find(s_hash) == selected_sites.end())
            {
                selected_sites.clear();
                selected_sites.insert(s_hash);
            }
            ImGui::OpenPopup("SiteContextMenu");
        }

        // 5. Actions: "Go to" and Delete "X"
        float btn_goto_width = ImGui::CalcTextSize("Go to").x + ImGui::GetStyle().FramePadding.x * 2.0f;
        float btn_del_width  = ImGui::CalcTextSize("X").x + ImGui::GetStyle().FramePadding.x * 2.0f;

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

        static std::vector<adam::string_hash>       s_expanded_sites;
        static std::unordered_set<adam::string_hash> s_selected_sites;
        static char                                 s_filter_buf[64] = "";

        std::string title = std::string(get_cop_string(wnd_sites, lang)) + "###RadarSites";
        if (!ImGui::Begin(title.c_str(), &p_show_sites->value()))
        {
            ImGui::End();
            return;
        }

        draw_add_site_toolbar(ctrl, lang);

        ImGui::Separator();

        // Search / Filter Bar & Bulk Selection Toolbar
        float clear_filter_btn_w = ImGui::CalcTextSize("X").x + ImGui::GetStyle().FramePadding.x * 2.0f;
        ImGui::SetNextItemWidth(-clear_filter_btn_w - ImGui::GetStyle().ItemSpacing.x);
        ImGui::InputTextWithHint("##SiteFilter", get_cop_string(lbl_filter_sites, lang), s_filter_buf, sizeof(s_filter_buf));
        ImGui::SameLine();
        if (ImGui::Button("X##ClearFilter"))
        {
            s_filter_buf[0] = '\0';
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Clear filter");
        }

        // Selection Action Bar
        const auto& sites = ctrl.get_sites();
        if (!sites.empty())
        {
            bool all_selected = (!sites.empty() && s_selected_sites.size() == sites.size());
            if (ImGui::Button(all_selected ? get_cop_string(menu_deselect_all, lang) : get_cop_string(menu_select_all, lang)))
            {
                if (all_selected)
                {
                    s_selected_sites.clear();
                }
                else
                {
                    s_selected_sites.clear();
                    for (const auto& s : sites)
                    {
                        if (s)
                        {
                            s_selected_sites.insert(s->get_name().get_hash());
                        }
                    }
                }
            }

            ImGui::SameLine();
            if (ImGui::Button(get_cop_string(menu_invert_selection, lang)))
            {
                std::unordered_set<adam::string_hash> inverted;
                for (const auto& s : sites)
                {
                    if (s && s_selected_sites.find(s->get_name().get_hash()) == s_selected_sites.end())
                    {
                        inverted.insert(s->get_name().get_hash());
                    }
                }
                s_selected_sites = std::move(inverted);
            }

            if (!s_selected_sites.empty())
            {
                ImGui::SameLine();
                ImGui::TextDisabled("(%zu selected)", s_selected_sites.size());

                ImGui::SameLine();
                if (ImGui::Button("Actions...##BulkActions"))
                {
                    ImGui::OpenPopup("SiteContextMenu");
                }
            }
        }

        ImGui::Separator();

        bool delete_selected_requested = false;
        draw_bulk_context_menu(ctrl, s_selected_sites, lang, delete_selected_requested);

        if (delete_selected_requested && !s_selected_sites.empty())
        {
            for (auto hash : s_selected_sites)
            {
                ctrl.remove_site(hash);
            }
            s_selected_sites.clear();
        }

        if (sites.empty())
        {
            ImGui::TextDisabled("%s", get_cop_string(lbl_no_sites, lang));
        }
        else
        {
            size_t visible_count = 0;
            for (const auto& s : sites)
            {
                if (!site_matches_filter(s.get(), s_filter_buf))
                {
                    continue;
                }

                visible_count++;
                if (draw_site_item(ctrl, map, s, lang, s_expanded_sites, picking_site_coords_hash, s_selected_sites))
                {
                    adam::string_hash h = s->get_name().get_hash();
                    s_selected_sites.erase(h);
                    ctrl.remove_site(h);
                    break;
                }
            }

            if (visible_count == 0)
            {
                ImGui::TextDisabled("No radar sites match filter '%s'.", s_filter_buf);
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
                    s_selected_sites.clear();
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

