/**
 * @file    world-map.cpp
 * @author  dexus1337
 * @brief   Scalable interactive C2 world map rendering engine with marker placement support
 * @version 1.0
 * @date    05.08.2026
 */

#include "world-map.hpp"

#include <cmath>
#include <algorithm>
#include <cstdio>
#include <string>

namespace adam::cop
{
    static constexpr float M_PI_F = 3.14159265358979323846f;

    static float degrees_to_radians(float deg)
    {
        return deg * (M_PI_F / 180.0f);
    }

    static float radians_to_degrees(float rad)
    {
        return rad * (180.0f / M_PI_F);
    }

    static float mercator_lat_to_y(float lat)
    {
        float lat_clamped = std::clamp(lat, -85.0f, 85.0f);
        float rad = degrees_to_radians(lat_clamped);
        return std::log(std::tan(M_PI_F / 4.0f + rad / 2.0f));
    }

    static float mercator_y_to_lat(float y)
    {
        return radians_to_degrees(2.0f * std::atan(std::exp(y)) - M_PI_F / 2.0f);
    }

    world_map::world_map()
    {
        reset_view();
    }

    void world_map::reset_view()
    {
        m_center_lat = 20.0f;
        m_center_lon = 10.0f;
        m_zoom = 1.0f;
    }

    void world_map::set_center(float lat, float lon)
    {
        m_center_lat = std::clamp(lat, -85.0f, 85.0f);
        m_center_lon = std::clamp(lon, -180.0f, 180.0f);
    }

    void world_map::set_zoom(float zoom)
    {
        m_zoom = std::clamp(zoom, 0.5f, 500000.0f);
    }



    ImVec2 world_map::geo_to_screen(float lat, float lon, const ImVec2& canvas_pos, const ImVec2& canvas_size, projection_type proj) const
    {
        float map_base_scale = canvas_size.x / 360.0f;
        float scaled_w = canvas_size.x * m_zoom;

        float rel_lon = lon - m_center_lon;
        while (rel_lon > 180.0f)
        {
            rel_lon -= 360.0f;
        }
        while (rel_lon < -180.0f)
        {
            rel_lon += 360.0f;
        }

        float screen_x = canvas_pos.x + canvas_size.x * 0.5f + (rel_lon / 360.0f) * scaled_w;
        float screen_y = 0.0f;

        if (proj == projection_type::equirectangular)
        {
            float rel_lat = lat - m_center_lat;
            screen_y = canvas_pos.y + canvas_size.y * 0.5f - (rel_lat / 180.0f) * (scaled_w * 0.5f);
        }
        else
        {
            float center_y_merc = mercator_lat_to_y(m_center_lat);
            float lat_y_merc = mercator_lat_to_y(lat);
            float rel_y_merc = lat_y_merc - center_y_merc;
            screen_y = canvas_pos.y + canvas_size.y * 0.5f - rel_y_merc * (map_base_scale * m_zoom * (180.0f / M_PI_F));
        }

        return ImVec2(screen_x, screen_y);
    }

    geo_point world_map::screen_to_geo(const ImVec2& screen_pos, const ImVec2& canvas_pos, const ImVec2& canvas_size, projection_type proj) const
    {
        float map_base_scale = canvas_size.x / 360.0f;
        float scaled_w = canvas_size.x * m_zoom;

        float rel_x = screen_pos.x - (canvas_pos.x + canvas_size.x * 0.5f);
        float lon = m_center_lon + (rel_x / scaled_w) * 360.0f;
        while (lon > 180.0f)
        {
            lon -= 360.0f;
        }
        while (lon < -180.0f)
        {
            lon += 360.0f;
        }

        float rel_y = (canvas_pos.y + canvas_size.y * 0.5f) - screen_pos.y;
        float lat = 0.0f;

        if (proj == projection_type::equirectangular)
        {
            lat = m_center_lat + (rel_y / (scaled_w * 0.5f)) * 180.0f;
        }
        else
        {
            float center_y_merc = mercator_lat_to_y(m_center_lat);
            float rel_y_merc = rel_y / (map_base_scale * m_zoom * (180.0f / M_PI_F));
            float target_y_merc = center_y_merc + rel_y_merc;
            lat = mercator_y_to_lat(target_y_merc);
        }

        lat = std::clamp(lat, -85.0f, 85.0f);
        return { lat, lon };
    }

    void world_map::draw(const ImVec2& size, const map_render_options& options, const std::vector<std::unique_ptr<waypoint>>& waypoints, const char* add_waypoint_text)
    {
        m_tile_engine.update();

        ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
        ImVec2 canvas_size = size;

        if (canvas_size.x <= 0.0f || canvas_size.y <= 0.0f)
        {
            return;
        }
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        // Canvas Input Handling
        ImGui::InvisibleButton("##map_canvas", canvas_size, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
        m_is_hovered = ImGui::IsItemHovered();

        ImGuiIO& io = ImGui::GetIO();
        if (m_is_hovered)
        {
            geo_point hover_pt = screen_to_geo(io.MousePos, canvas_pos, canvas_size, options.projection);
            m_hover_lat = hover_pt.lat;
            m_hover_lon = hover_pt.lon;

            // Pan control via right mouse drag (only if NOT holding Shift)
            if (ImGui::IsMouseDragging(ImGuiMouseButton_Right) && !io.KeyShift)
            {
                ImVec2 delta = io.MouseDelta;
                float scaled_w = canvas_size.x * m_zoom;

                float delta_lon = -(delta.x / scaled_w) * 360.0f;
                float delta_lat = (delta.y / (scaled_w * 0.5f)) * 180.0f;

                m_center_lon += delta_lon;
                m_center_lat += delta_lat;

                m_center_lat = std::clamp(m_center_lat, -85.0f, 85.0f);
                while (m_center_lon > 180.0f)
                {
                    m_center_lon -= 360.0f;
                }
                while (m_center_lon < -180.0f)
                {
                    m_center_lon += 360.0f;
                }
            }

            // Scroll zoom control
            if (io.MouseWheel != 0.0f)
            {
                float zoom_mult = (io.MouseWheel > 0.0f) ? 1.20f : 0.8333f;
                float old_zoom = m_zoom;
                float new_zoom = std::clamp(m_zoom * zoom_mult, 0.5f, 500000.0f);

                if (new_zoom != old_zoom)
                {
                    geo_point pivot = screen_to_geo(io.MousePos, canvas_pos, canvas_size, options.projection);
                    m_zoom = new_zoom;
                    ImVec2 new_pivot_screen = geo_to_screen(pivot.lat, pivot.lon, canvas_pos, canvas_size, options.projection);

                    float dx = new_pivot_screen.x - io.MousePos.x;
                    float dy = new_pivot_screen.y - io.MousePos.y;

                    float scaled_w = canvas_size.x * m_zoom;
                    m_center_lon += (dx / scaled_w) * 360.0f;
                    m_center_lat -= (dy / (scaled_w * 0.5f)) * 180.0f;

                    m_center_lat = std::clamp(m_center_lat, -85.0f, 85.0f);
                }
            }
            
            // Context menu logic
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && io.KeyShift)
            {
                ImGui::OpenPopup("MapContextMenu");
            }
        }

        if (ImGui::BeginPopup("MapContextMenu"))
        {
            static geo_point click_geo = {0.0f, 0.0f};
            if (ImGui::IsWindowAppearing())
            {
                click_geo = screen_to_geo(io.MousePos, canvas_pos, canvas_size, options.projection);
            }
            if (ImGui::MenuItem(add_waypoint_text))
            {
                m_context_add_waypoint_requested = true;
                m_context_waypoint_lat = click_geo.lat;
                m_context_waypoint_lon = click_geo.lon;
            }
            ImGui::EndPopup();
        }

        // Render Base Ocean Background
        render_ocean(draw_list, canvas_pos, canvas_size, options);

        // Render Raster Map Tiles
        if (options.base_provider != tile_provider_type::vector_only)
        {
            render_tiles(draw_list, canvas_pos, canvas_size, options);
        }

        // Render Tactical Grid
        if (options.show_grid)
        {
            render_grid(draw_list, canvas_pos, canvas_size, options);
        }

        // Render Coastline Vector Overlay
        if (options.show_coastlines)
        {
            render_vector_land(draw_list, canvas_pos, canvas_size, options);
        }

        // Render Tactical Waypoints
        if (options.show_markers)
        {
            render_waypoints(draw_list, canvas_pos, canvas_size, options, waypoints);
        }

        // Render Scale Bar
        if (options.show_scale_bar)
        {
            render_scale_bar(draw_list, canvas_pos, canvas_size, options);
        }

        // Render Compass Rose
        if (options.show_compass)
        {
            render_compass(draw_list, canvas_pos, canvas_size, options);
        }
    }

    void world_map::render_ocean(ImDrawList* draw_list, const ImVec2& pos, const ImVec2& size, const map_render_options& options)
    {
        draw_list->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), ImGui::ColorConvertFloat4ToU32(options.ocean_bg_color));
    }

    void world_map::render_tiles(ImDrawList* draw_list, const ImVec2& pos, const ImVec2& size, const map_render_options& options)
    {
        tile_provider_info info = get_tile_provider_info(options.base_provider);
        if (info.max_zoom <= 0)
        {
            return;
        }

        float canvas_ratio = std::max(size.x, 256.0f) / 256.0f;
        int z = static_cast<int>(std::round(std::log2(m_zoom * canvas_ratio)));
        z = std::clamp(z, 0, info.max_zoom);

        int num_tiles = 1 << z;
        float scaled_w = size.x * m_zoom;
        float tile_px_w = scaled_w / static_cast<float>(num_tiles);

        float cam_tx = (m_center_lon + 180.0f) / 360.0f * static_cast<float>(num_tiles);

        float lat_rad = degrees_to_radians(std::clamp(m_center_lat, -85.0f, 85.0f));
        float cam_ty = (1.0f - std::log(std::tan(lat_rad) + 1.0f / std::cos(lat_rad)) / M_PI_F) / 2.0f * static_cast<float>(num_tiles);

        float half_tiles_x = (size.x * 0.5f) / tile_px_w + 1.5f;
        float half_tiles_y = (size.y * 0.5f) / tile_px_w + 1.5f;

        int start_x = static_cast<int>(std::floor(cam_tx - half_tiles_x));
        int end_x   = static_cast<int>(std::ceil(cam_tx + half_tiles_x));

        int start_y = static_cast<int>(std::floor(cam_ty - half_tiles_y));
        int end_y   = static_cast<int>(std::ceil(cam_ty + half_tiles_y));

        start_y = std::clamp(start_y, 0, num_tiles - 1);
        end_y   = std::clamp(end_y, 0, num_tiles - 1);

        auto tile_y_to_lat_f = [num_tiles](float ty) -> float
        {
            float n = M_PI_F - 2.0f * M_PI_F * ty / static_cast<float>(num_tiles);
            return radians_to_degrees(std::atan(0.5f * (std::exp(n) - std::exp(-n))));
        };

        ImU32 tile_tint = ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, std::clamp(options.map_opacity, 0.1f, 1.0f)));

        for (int ty = start_y; ty <= end_y; ty++)
        {
            float p_tl_y = 0.0f;
            float p_br_y = 0.0f;

            if (options.projection == projection_type::mercator)
            {
                p_tl_y = pos.y + size.y * 0.5f + (static_cast<float>(ty) - cam_ty) * tile_px_w;
                p_br_y = p_tl_y + tile_px_w;
            }
            else
            {
                float lat_top = tile_y_to_lat_f(static_cast<float>(ty));
                float lat_bot = tile_y_to_lat_f(static_cast<float>(ty + 1));

                p_tl_y = pos.y + size.y * 0.5f - ((lat_top - m_center_lat) / 180.0f) * (scaled_w * 0.5f);
                p_br_y = pos.y + size.y * 0.5f - ((lat_bot - m_center_lat) / 180.0f) * (scaled_w * 0.5f);
            }

            for (int tx = start_x; tx <= end_x; tx++)
            {
                float p_tl_x = pos.x + size.x * 0.5f + (static_cast<float>(tx) - cam_tx) * tile_px_w;
                float p_br_x = p_tl_x + tile_px_w;

                if (p_br_x < pos.x || p_tl_x > pos.x + size.x || p_br_y < pos.y || p_tl_y > pos.y + size.y)
                {
                    continue;
                }

                int wrapped_tx = ((tx % num_tiles) + num_tiles) % num_tiles;
                float dist_to_center = std::abs(static_cast<float>(tx) - cam_tx) + std::abs(static_cast<float>(ty) - cam_ty);

                ImTextureID tex_id = m_tile_engine.get_tile_texture(options.base_provider, z, wrapped_tx, ty, dist_to_center);

                if (tex_id)
                {
                    draw_list->AddImage(tex_id, ImVec2(p_tl_x, p_tl_y), ImVec2(p_br_x, p_br_y), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), tile_tint);
                    continue;
                }

                // Recursive parent tile fallback search down to level 0
                for (int pz = z - 1; pz >= 0; pz--)
                {
                    int level_diff = z - pz;
                    int factor = 1 << level_diff;
                    int p_tx = wrapped_tx / factor;
                    int p_ty = ty / factor;

                    float p_cam_tx = cam_tx / static_cast<float>(factor);
                    float p_cam_ty = cam_ty / static_cast<float>(factor);
                    float p_dist = std::abs(static_cast<float>(p_tx) - p_cam_tx) + std::abs(static_cast<float>(p_ty) - p_cam_ty);

                    ImTextureID p_tex = m_tile_engine.get_tile_texture(options.base_provider, pz, p_tx, p_ty, p_dist);
                    if (p_tex)
                    {
                        float f = static_cast<float>(factor);
                        float u0 = static_cast<float>(wrapped_tx % factor) / f;
                        float u1 = u0 + (1.0f / f);
                        float v0 = static_cast<float>(ty % factor) / f;
                        float v1 = v0 + (1.0f / f);

                        draw_list->AddImage(p_tex, ImVec2(p_tl_x, p_tl_y), ImVec2(p_br_x, p_br_y), ImVec2(u0, v0), ImVec2(u1, v1), tile_tint);
                        break;
                    }
                }
            }
        }
    }

    void world_map::render_grid(ImDrawList* draw_list, const ImVec2& pos, const ImVec2& size, const map_render_options& options)
    {
        ImU32 col_grid = ImGui::ColorConvertFloat4ToU32(options.grid_color);
        ImU32 col_text = ImGui::ColorConvertFloat4ToU32(ImVec4(0.60f, 0.70f, 0.80f, 0.75f));

        float grid_step = 30.0f;
        const char* fmt = "%.1f°%c";

        if (m_zoom >= 100000.0f)
        {
            grid_step = 0.0005f;
            fmt = "%.4f°%c";
        }
        else if (m_zoom >= 20000.0f)
        {
            grid_step = 0.002f;
            fmt = "%.4f°%c";
        }
        else if (m_zoom >= 5000.0f)
        {
            grid_step = 0.01f;
            fmt = "%.3f°%c";
        }
        else if (m_zoom >= 1000.0f)
        {
            grid_step = 0.05f;
            fmt = "%.3f°%c";
        }
        else if (m_zoom >= 300.0f)
        {
            grid_step = 0.1f;
            fmt = "%.2f°%c";
        }
        else if (m_zoom >= 100.0f)
        {
            grid_step = 0.5f;
            fmt = "%.2f°%c";
        }
        else if (m_zoom >= 30.0f)
        {
            grid_step = 1.0f;
            fmt = "%.1f°%c";
        }
        else if (m_zoom >= 10.0f)
        {
            grid_step = 5.0f;
            fmt = "%.1f°%c";
        }
        else if (m_zoom >= 3.0f)
        {
            grid_step = 15.0f;
            fmt = "%.1f°%c";
        }

        // Longitude grid lines
        float start_lon = std::floor((m_center_lon - 180.0f / m_zoom) / grid_step) * grid_step;
        float end_lon = std::ceil((m_center_lon + 180.0f / m_zoom) / grid_step) * grid_step;

        start_lon = std::clamp(start_lon, -360.0f, 360.0f);
        end_lon = std::clamp(end_lon, -360.0f, 360.0f);

        for (float lon = start_lon; lon <= end_lon; lon += grid_step)
        {
            ImVec2 p1 = geo_to_screen(80.0f, lon, pos, size, options.projection);

            if (p1.x >= pos.x && p1.x <= pos.x + size.x)
            {
                draw_list->AddLine(ImVec2(p1.x, pos.y), ImVec2(p1.x, pos.y + size.y), col_grid, 1.0f);

                char buf[32];
                float norm_lon = std::fmod(lon + 180.0f, 360.0f);
                if (norm_lon < 0)
                {
                    norm_lon += 360.0f;
                }
                norm_lon -= 180.0f;
                std::snprintf(buf, sizeof(buf), fmt, std::abs(norm_lon), norm_lon >= 0 ? 'E' : 'W');
                draw_list->AddText(ImVec2(p1.x + 4.0f, pos.y + 6.0f), col_text, buf);
            }
        }

        // Latitude grid lines
        float start_lat = std::floor((m_center_lat - 90.0f / m_zoom) / grid_step) * grid_step;
        float end_lat = std::ceil((m_center_lat + 90.0f / m_zoom) / grid_step) * grid_step;

        start_lat = std::clamp(start_lat, -80.0f, 80.0f);
        end_lat = std::clamp(end_lat, -80.0f, 80.0f);

        for (float lat = start_lat; lat <= end_lat; lat += grid_step)
        {
            ImVec2 p1 = geo_to_screen(lat, -180.0f, pos, size, options.projection);

            if (p1.y >= pos.y && p1.y <= pos.y + size.y)
            {
                draw_list->AddLine(ImVec2(pos.x, p1.y), ImVec2(pos.x + size.x, p1.y), col_grid, 1.0f);

                char buf[32];
                std::snprintf(buf, sizeof(buf), fmt, std::abs(lat), lat >= 0 ? 'N' : 'S');
                draw_list->AddText(ImVec2(pos.x + 6.0f, p1.y - 14.0f), col_text, buf);
            }
        }
    }

    void world_map::render_vector_land(ImDrawList* draw_list, const ImVec2& pos, const ImVec2& size, const map_render_options& options)
    {
        const auto& polylines = get_coastlines_vector_data();
        ImU32 col_coast = ImGui::ColorConvertFloat4ToU32(options.coastline_color);

        float map_base_scale = size.x / 360.0f;
        float scaled_w = size.x * m_zoom;

        std::vector<ImVec2> screen_pts;

        for (const auto& poly : polylines)
        {
            if (poly.points.size() < 2)
            {
                continue;
            }

            screen_pts.clear();
            screen_pts.reserve(poly.points.size());

            bool broke_at_antimeridian = false;

            float curr_rel_lon = poly.points[0].lon - m_center_lon;
            while (curr_rel_lon > 180.0f)
            {
                curr_rel_lon -= 360.0f;
            }
            while (curr_rel_lon < -180.0f)
            {
                curr_rel_lon += 360.0f;
            }

            for (size_t i = 0; i < poly.points.size(); i++)
            {
                const auto& pt = poly.points[i];
                if (i > 0)
                {
                    float dlon = pt.lon - poly.points[i - 1].lon;

                    // Detect antimeridian crossing (raw delta > 90° means the polygon wraps around ±180°)
                    if (std::abs(dlon) > 90.0f)
                    {
                        // Flush accumulated segment
                        if (screen_pts.size() >= 2)
                        {
                            draw_list->AddPolyline(screen_pts.data(), static_cast<int>(screen_pts.size()), col_coast, 0, 1.5f);
                        }
                        screen_pts.clear();
                        broke_at_antimeridian = true;
                    }

                    while (dlon > 180.0f)
                    {
                        dlon -= 360.0f;
                    }
                    while (dlon < -180.0f)
                    {
                        dlon += 360.0f;
                    }
                    curr_rel_lon += dlon;
                }

                float screen_x = pos.x + size.x * 0.5f + (curr_rel_lon / 360.0f) * scaled_w;
                float screen_y = 0.0f;

                if (options.projection == projection_type::equirectangular)
                {
                    float rel_lat = pt.lat - m_center_lat;
                    screen_y = pos.y + size.y * 0.5f - (rel_lat / 180.0f) * (scaled_w * 0.5f);
                }
                else
                {
                    float center_y_merc = mercator_lat_to_y(m_center_lat);
                    float lat_y_merc = mercator_lat_to_y(pt.lat);
                    float rel_y_merc = lat_y_merc - center_y_merc;
                    screen_y = pos.y + size.y * 0.5f - rel_y_merc * (map_base_scale * m_zoom * (180.0f / M_PI_F));
                }

                screen_pts.push_back(ImVec2(screen_x, screen_y));
            }

            if (screen_pts.size() >= 2)
            {
                ImDrawFlags flags = (poly.is_closed && !broke_at_antimeridian) ? ImDrawFlags_Closed : 0;
                draw_list->AddPolyline(screen_pts.data(), static_cast<int>(screen_pts.size()), col_coast, flags, 1.5f);
            }
        }
    }

    void world_map::render_waypoints(ImDrawList* draw_list, const ImVec2& pos, const ImVec2& size, const map_render_options& options, const std::vector<std::unique_ptr<waypoint>>& waypoints)
    {
        for (const auto& wp : waypoints)
        {
            if (!wp->is_enabled()) continue;

            ImVec2 screen_pt = geo_to_screen(wp->get_lat(), wp->get_lon(), pos, size, options.projection);

            // Frustum culling check
            if (screen_pt.x < pos.x - 150.0f || screen_pt.x > pos.x + size.x + 150.0f ||
                screen_pt.y < pos.y - 150.0f || screen_pt.y > pos.y + size.y + 150.0f)
            {
                continue;
            }

            uint32_t c = wp->get_color();
            ImU32 col_primary = IM_COL32((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF, 255);
            ImU32 col_bg = ImGui::ColorConvertFloat4ToU32(ImVec4(0.06f, 0.08f, 0.12f, 0.90f));
            ImU32 col_text = ImGui::ColorConvertFloat4ToU32(ImVec4(0.95f, 0.98f, 1.00f, 1.00f));
            ImU32 col_subtext = ImGui::ColorConvertFloat4ToU32(ImVec4(0.00f, 0.85f, 1.00f, 1.00f));

            // Reticle Target Icon
            float r = 12.0f;
            draw_list->AddCircleFilled(screen_pt, 4.0f, col_primary);
            draw_list->AddCircle(screen_pt, r, col_primary, 0, 1.8f);

            // Crosshair lines
            draw_list->AddLine(ImVec2(screen_pt.x - r - 6.0f, screen_pt.y), ImVec2(screen_pt.x - 4.0f, screen_pt.y), col_primary, 1.8f);
            draw_list->AddLine(ImVec2(screen_pt.x + 4.0f, screen_pt.y), ImVec2(screen_pt.x + r + 6.0f, screen_pt.y), col_primary, 1.8f);
            draw_list->AddLine(ImVec2(screen_pt.x, screen_pt.y - r - 6.0f), ImVec2(screen_pt.x, screen_pt.y - 4.0f), col_primary, 1.8f);
            draw_list->AddLine(ImVec2(screen_pt.x, screen_pt.y + 4.0f), ImVec2(screen_pt.x, screen_pt.y + r + 6.0f), col_primary, 1.8f);

            // Callout Box
            char lat_str[32];
            char lon_str[32];
            std::snprintf(lat_str, sizeof(lat_str), "Lat: %.4f°%c", std::abs(wp->get_lat()), wp->get_lat() >= 0 ? 'N' : 'S');
            std::snprintf(lon_str, sizeof(lon_str), "Lon: %.4f°%c", std::abs(wp->get_lon()), wp->get_lon() >= 0 ? 'E' : 'W');

            ImVec2 tag_pos = ImVec2(screen_pt.x + r + 8.0f, screen_pt.y - 22.0f);
            float box_w = 145.0f;
            float box_h = 44.0f;

            draw_list->AddRectFilled(tag_pos, ImVec2(tag_pos.x + box_w, tag_pos.y + box_h), col_bg, 4.0f);
            draw_list->AddRect(tag_pos, ImVec2(tag_pos.x + box_w, tag_pos.y + box_h), col_primary, 4.0f, 0, 1.2f);

            draw_list->AddText(ImVec2(tag_pos.x + 6.0f, tag_pos.y + 3.0f), col_text, wp->get_label().c_str());
            draw_list->AddText(ImVec2(tag_pos.x + 6.0f, tag_pos.y + 16.0f), col_subtext, lat_str);
            draw_list->AddText(ImVec2(tag_pos.x + 6.0f, tag_pos.y + 28.0f), col_subtext, lon_str);
        }
    }

    void world_map::render_scale_bar(ImDrawList* draw_list, const ImVec2& pos, const ImVec2& size, const map_render_options& options)
    {
        ImVec2 bar_pos = ImVec2(pos.x + 20.0f, pos.y + size.y - 40.0f);
        float bar_width_px = 120.0f;

        geo_point g1 = screen_to_geo(bar_pos, pos, size, options.projection);
        geo_point g2 = screen_to_geo(ImVec2(bar_pos.x + bar_width_px, bar_pos.y), pos, size, options.projection);

        // Haversine distance in KM
        float dlat = degrees_to_radians(g2.lat - g1.lat);
        float dlon = degrees_to_radians(g2.lon - g1.lon);
        float a = std::sin(dlat / 2.0f) * std::sin(dlat / 2.0f) +
                  std::cos(degrees_to_radians(g1.lat)) * std::cos(degrees_to_radians(g2.lat)) *
                  std::sin(dlon / 2.0f) * std::sin(dlon / 2.0f);
        float c = 2.0f * std::atan2(std::sqrt(a), std::sqrt(1.0f - a));
        float dist_km = 6371.0f * c;
        float dist_nm = dist_km * 0.539957f;

        ImU32 col_bg = ImGui::ColorConvertFloat4ToU32(ImVec4(0.06f, 0.08f, 0.12f, 0.85f));
        ImU32 col_border = ImGui::ColorConvertFloat4ToU32(ImVec4(0.00f, 0.60f, 0.80f, 0.80f));
        ImU32 col_text = ImGui::ColorConvertFloat4ToU32(ImVec4(0.90f, 0.94f, 0.98f, 1.00f));

        draw_list->AddRectFilled(ImVec2(bar_pos.x - 8.0f, bar_pos.y - 20.0f), ImVec2(bar_pos.x + bar_width_px + 8.0f, bar_pos.y + 15.0f), col_bg, 4.0f);
        draw_list->AddRect(ImVec2(bar_pos.x - 8.0f, bar_pos.y - 20.0f), ImVec2(bar_pos.x + bar_width_px + 8.0f, bar_pos.y + 15.0f), col_border, 4.0f);

        draw_list->AddLine(ImVec2(bar_pos.x, bar_pos.y), ImVec2(bar_pos.x + bar_width_px, bar_pos.y), col_text, 2.0f);
        draw_list->AddLine(ImVec2(bar_pos.x, bar_pos.y - 4.0f), ImVec2(bar_pos.x, bar_pos.y + 4.0f), col_text, 2.0f);
        draw_list->AddLine(ImVec2(bar_pos.x + bar_width_px, bar_pos.y - 4.0f), ImVec2(bar_pos.x + bar_width_px, bar_pos.y + 4.0f), col_text, 2.0f);

        char label[64];
        if (dist_km >= 10.0f)
        {
            std::snprintf(label, sizeof(label), "%.0f km / %.0f NM", dist_km, dist_nm);
        }
        else
        {
            std::snprintf(label, sizeof(label), "%.1f km / %.1f NM", dist_km, dist_nm);
        }

        draw_list->AddText(ImVec2(bar_pos.x, bar_pos.y - 18.0f), col_text, label);
    }

    void world_map::render_compass(ImDrawList* draw_list, const ImVec2& pos, const ImVec2& size, const map_render_options& options)
    {
        (void)options;

        ImVec2 center = ImVec2(pos.x + size.x - 45.0f, pos.y + 45.0f);
        float radius = 22.0f;

        ImU32 col_bg = ImGui::ColorConvertFloat4ToU32(ImVec4(0.06f, 0.08f, 0.12f, 0.85f));
        ImU32 col_border = ImGui::ColorConvertFloat4ToU32(ImVec4(0.00f, 0.60f, 0.80f, 0.80f));
        ImU32 col_north = ImGui::ColorConvertFloat4ToU32(ImVec4(1.00f, 0.30f, 0.25f, 1.00f));
        ImU32 col_south = ImGui::ColorConvertFloat4ToU32(ImVec4(0.80f, 0.85f, 0.90f, 0.80f));

        draw_list->AddCircleFilled(center, radius, col_bg);
        draw_list->AddCircle(center, radius, col_border, 0, 1.5f);

        // North triangle
        ImVec2 p_n1 = ImVec2(center.x, center.y - radius + 4.0f);
        ImVec2 p_n2 = ImVec2(center.x - 5.0f, center.y);
        ImVec2 p_n3 = ImVec2(center.x + 5.0f, center.y);
        draw_list->AddTriangleFilled(p_n1, p_n2, p_n3, col_north);

        // South triangle
        ImVec2 p_s1 = ImVec2(center.x, center.y + radius - 4.0f);
        draw_list->AddTriangleFilled(p_s1, p_n2, p_n3, col_south);

        draw_list->AddText(ImVec2(center.x - 4.0f, center.y - radius - 14.0f), col_north, "N");
    }

    bool world_map::consume_context_add_waypoint_request(float& out_lat, float& out_lon)
    {
        if (m_context_add_waypoint_requested)
        {
            out_lat = m_context_waypoint_lat;
            out_lon = m_context_waypoint_lon;
            m_context_add_waypoint_requested = false;
            return true;
        }
        return false;
    }
}
