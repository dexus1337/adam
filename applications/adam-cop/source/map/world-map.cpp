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

    static float calculate_haversine_distance_km(float lat1, float lon1, float lat2, float lon2)
    {
        float dlat = degrees_to_radians(lat2 - lat1);
        float dlon = degrees_to_radians(lon2 - lon1);
        float a = std::sin(dlat / 2.0f) * std::sin(dlat / 2.0f) +
                  std::cos(degrees_to_radians(lat1)) * std::cos(degrees_to_radians(lat2)) *
                  std::sin(dlon / 2.0f) * std::sin(dlon / 2.0f);
        float c = 2.0f * std::atan2(std::sqrt(a), std::sqrt(1.0f - a));
        return 6371.0f * c;
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

    void world_map::draw(const ImVec2& size, const map_render_options& options, const std::vector<std::unique_ptr<waypoint>>& waypoints, const std::vector<std::unique_ptr<drawable_site>>& sites, const char* add_waypoint_text)
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

            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                m_measurement_points.push_back({m_hover_lat, m_hover_lon});
            }
            if (ImGui::IsKeyPressed(ImGuiKey_Escape))
            {
                clear_measurements();
            }

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

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));
        if (ImGui::BeginPopup("MapContextMenu"))
        {
            static geo_point click_geo = {0.0f, 0.0f};
            if (ImGui::IsWindowAppearing())
            {
                click_geo = screen_to_geo(io.MousePos, canvas_pos, canvas_size, options.projection);
            }
            if (ImGui::MenuItem(add_waypoint_text, "Shift+RClick"))
            {
                m_context_add_waypoint_requested = true;
                m_context_waypoint_lat = click_geo.lat;
                m_context_waypoint_lon = click_geo.lon;
            }
            if (ImGui::MenuItem("Clear Measurements", "Esc", false, !m_measurement_points.empty()))
            {
                clear_measurements();
            }
            ImGui::EndPopup();
        }
        ImGui::PopStyleVar();

        // Push Clip Rect
        draw_list->PushClipRect(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), true);

        // Render Base Ocean Background
        render_ocean(draw_list, canvas_pos, canvas_size, options);

        // Render Raster Map Tiles
        render_tiles(draw_list, canvas_pos, canvas_size, options);

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

        // Render Tactical Waypoints & Radar Sites
        if (options.show_markers)
        {
            render_sites(draw_list, canvas_pos, canvas_size, options, sites);
            render_waypoints(draw_list, canvas_pos, canvas_size, options, waypoints);
        }

        render_measurements(draw_list, canvas_pos, canvas_size, options);

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
        if (options.tile_layers.empty())
        {
            return;
        }

        float canvas_ratio = std::max(size.x, 256.0f) / 256.0f;
        float un_clamped_z_float = std::log2(m_zoom * canvas_ratio);
        float scaled_w_base = size.x * m_zoom;

        float lat_rad = degrees_to_radians(std::clamp(m_center_lat, -85.0f, 85.0f));
        float cam_ty_base = (1.0f - std::log(std::tan(lat_rad) + 1.0f / std::cos(lat_rad)) / M_PI_F) / 2.0f;
        float cam_tx_base = (m_center_lon + 180.0f) / 360.0f;

        auto tile_y_to_lat_f = [](float ty, int num_tiles) -> float
        {
            float n = M_PI_F - 2.0f * M_PI_F * ty / static_cast<float>(num_tiles);
            return radians_to_degrees(std::atan(0.5f * (std::exp(n) - std::exp(-n))));
        };

        for (const auto& layer : options.tile_layers)
        {
            if (!layer.visible)
            {
                continue;
            }

            tile_provider_info info = get_tile_provider_info(layer.provider);
            if (info.max_zoom <= 0)
            {
                continue;
            }

            int z = static_cast<int>(std::round(un_clamped_z_float));
            z = std::clamp(z, 0, info.max_zoom);

            int num_tiles = 1 << z;
            float scaled_w = scaled_w_base;
            float tile_px_w = scaled_w / static_cast<float>(num_tiles);

            float cam_tx = cam_tx_base * static_cast<float>(num_tiles);
            float cam_ty = cam_ty_base * static_cast<float>(num_tiles);

            float half_tiles_x = (size.x * 0.5f) / tile_px_w + 1.5f;
            float half_tiles_y = (size.y * 0.5f) / tile_px_w + 1.5f;

            int start_x = static_cast<int>(std::floor(cam_tx - half_tiles_x));
            int end_x   = static_cast<int>(std::ceil(cam_tx + half_tiles_x));

            int start_y = static_cast<int>(std::floor(cam_ty - half_tiles_y));
            int end_y   = static_cast<int>(std::ceil(cam_ty + half_tiles_y));

            start_y = std::clamp(start_y, 0, num_tiles - 1);
            end_y   = std::clamp(end_y, 0, num_tiles - 1);

            ImU32 tile_tint = ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, std::clamp(layer.opacity, 0.0f, 1.0f)));

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
                    float lat_top = tile_y_to_lat_f(static_cast<float>(ty), num_tiles);
                    float lat_bot = tile_y_to_lat_f(static_cast<float>(ty + 1), num_tiles);

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

                    ImTextureID tex_id = m_tile_engine.get_tile_texture(layer.provider, z, wrapped_tx, ty, dist_to_center);

                    if (tex_id)
                    {
                        draw_list->AddImage(tex_id, ImVec2(p_tl_x, p_tl_y), ImVec2(p_br_x, p_br_y), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), tile_tint);
                        continue;
                    }

                    for (int pz = z - 1; pz >= 0; pz--)
                    {
                        int level_diff = z - pz;
                        int factor = 1 << level_diff;
                        int p_tx = wrapped_tx / factor;
                        int p_ty = ty / factor;

                        float p_cam_tx = cam_tx / static_cast<float>(factor);
                        float p_cam_ty = cam_ty / static_cast<float>(factor);
                        float p_dist = std::abs(static_cast<float>(p_tx) - p_cam_tx) + std::abs(static_cast<float>(p_ty) - p_cam_ty);

                        ImTextureID p_tex = m_tile_engine.get_tile_texture(layer.provider, pz, p_tx, p_ty, p_dist);
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
            
            ImVec4 bg_color = ImGui::GetStyleColorVec4(ImGuiCol_PopupBg);
            bg_color.w = std::min(bg_color.w, 0.90f); // Make sure it's slightly transparent if the theme is opaque
            ImU32 col_bg = ImGui::ColorConvertFloat4ToU32(bg_color);
            ImU32 col_text = ImGui::GetColorU32(ImGuiCol_Text);

            // Reticle Target Icon
            float r = 12.0f;
            draw_list->AddCircleFilled(screen_pt, 4.0f, col_primary);
            draw_list->AddCircle(screen_pt, r, col_primary, 0, 1.8f);

            // Crosshair lines
            draw_list->AddLine(ImVec2(screen_pt.x - r - 6.0f, screen_pt.y), ImVec2(screen_pt.x - 4.0f, screen_pt.y), col_primary, 1.8f);
            draw_list->AddLine(ImVec2(screen_pt.x + 4.0f, screen_pt.y), ImVec2(screen_pt.x + r + 6.0f, screen_pt.y), col_primary, 1.8f);
            draw_list->AddLine(ImVec2(screen_pt.x, screen_pt.y - r - 6.0f), ImVec2(screen_pt.x, screen_pt.y - 4.0f), col_primary, 1.8f);
            draw_list->AddLine(ImVec2(screen_pt.x, screen_pt.y + 4.0f), ImVec2(screen_pt.x, screen_pt.y + r + 6.0f), col_primary, 1.8f);

            // Callout Box (Label only)
            ImVec2 label_size = ImGui::CalcTextSize(wp->get_label().c_str());
            float box_w = label_size.x + 12.0f;
            float box_h = label_size.y + 8.0f;
            
            ImVec2 tag_pos = ImVec2(screen_pt.x + r + 8.0f, screen_pt.y - box_h * 0.5f);

            draw_list->AddRectFilled(tag_pos, ImVec2(tag_pos.x + box_w, tag_pos.y + box_h), col_bg, 4.0f);
            draw_list->AddRect(tag_pos, ImVec2(tag_pos.x + box_w, tag_pos.y + box_h), col_primary, 4.0f, 0, 1.2f);

            draw_list->AddText(ImVec2(tag_pos.x + 6.0f, tag_pos.y + 4.0f), col_text, wp->get_label().c_str());
        }
    }

    void world_map::render_sites(ImDrawList* draw_list, const ImVec2& pos, const ImVec2& size, const map_render_options& options, const std::vector<std::unique_ptr<drawable_site>>& sites)
    {
        for (const auto& s : sites)
        {
            if (!s || !s->is_enabled())
            {
                continue;
            }

            ImVec2 screen_pt = geo_to_screen(s->get_lat(), s->get_lon(), pos, size, options.projection);

            uint32_t c = s->get_color();
            ImU32 col_primary = IM_COL32((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF, 255);

            // Coverage / Range Ring & Sector Crossings
            double range_nm = s->get_range_nm();
            float range_alpha = static_cast<float>(s->get_range_alpha());
            float sector_alpha = static_cast<float>(s->get_sector_crossings_alpha());

            ImU32 col_ring = IM_COL32((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF, static_cast<int>(range_alpha * 255.0f * 0.4f));
            ImU32 col_ring_outline = IM_COL32((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF, static_cast<int>(range_alpha * 255.0f));
            ImU32 col_sector_line = IM_COL32((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF, static_cast<int>(sector_alpha * 255.0f));

            if (range_nm > 0.0)
            {
                float lat_offset = static_cast<float>(range_nm / 60.0); // 1 deg lat = 60 NM
                ImVec2 range_pt = geo_to_screen(s->get_lat() + lat_offset, s->get_lon(), pos, size, options.projection);
                float radius_px = std::abs(range_pt.y - screen_pt.y);

                if (radius_px > 1.0f)
                {
                    if (s->get_show_range())
                    {
                        draw_list->AddCircleFilled(screen_pt, radius_px, col_ring, 64);
                        draw_list->AddCircle(screen_pt, radius_px, col_ring_outline, 64, 1.2f);
                    }

                    // Sector Crossings (32 radial azimuth lines by default)
                    if (s->get_show_sector_crossings())
                    {
                        constexpr int sector_count = 32;
                        constexpr float angle_step = 6.28318530718f / static_cast<float>(sector_count);
                        for (int i = 0; i < sector_count; ++i)
                        {
                            float angle = static_cast<float>(i) * angle_step - 1.57079632679f;
                            ImVec2 line_end(screen_pt.x + std::cos(angle) * radius_px, screen_pt.y + std::sin(angle) * radius_px);
                            draw_list->AddLine(screen_pt, line_end, col_sector_line, 1.0f);
                        }
                    }

                    // Display active antenna rotation sweep beam & phosphor trail
                    if (s->has_live_rotation())
                    {
                        float az_rad = static_cast<float>(s->get_current_azimuth_deg() * 0.017453292519943295) - 1.57079632679f;
                        
                        // Phosphor trail wedge (trailing 25 degrees)
                        constexpr int trail_segments = 10;
                        constexpr float trail_span_rad = 25.0f * 0.017453292519943295f;
                        float seg_step = trail_span_rad / static_cast<float>(trail_segments);

                        for (int i = 0; i < trail_segments; ++i)
                        {
                            float a0 = az_rad - static_cast<float>(i + 1) * seg_step;
                            float a1 = az_rad - static_cast<float>(i) * seg_step;
                            float trail_factor = 1.0f - (static_cast<float>(i) / static_cast<float>(trail_segments));
                            int alpha = static_cast<int>(trail_factor * trail_factor * 90.0f);
                            ImU32 col_trail = IM_COL32((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF, alpha);

                            ImVec2 p0(screen_pt.x + std::cos(a0) * radius_px, screen_pt.y + std::sin(a0) * radius_px);
                            ImVec2 p1(screen_pt.x + std::cos(a1) * radius_px, screen_pt.y + std::sin(a1) * radius_px);
                            draw_list->AddTriangleFilled(screen_pt, p0, p1, col_trail);
                        }

                        // Main antenna sweep line
                        ImVec2 beam_end(screen_pt.x + std::cos(az_rad) * radius_px, screen_pt.y + std::sin(az_rad) * radius_px);
                        draw_list->AddLine(screen_pt, beam_end, col_primary, 1.8f);
                        draw_list->AddCircleFilled(beam_end, 2.5f, col_primary);
                    }
                }
            }

            // Frustum culling check for marker and label
            if (screen_pt.x < pos.x - 150.0f || screen_pt.x > pos.x + size.x + 150.0f ||
                screen_pt.y < pos.y - 150.0f || screen_pt.y > pos.y + size.y + 150.0f)
            {
                continue;
            }

            ImVec4 bg_color = ImGui::GetStyleColorVec4(ImGuiCol_PopupBg);
            bg_color.w = std::min(bg_color.w, 0.90f);
            ImU32 col_bg = ImGui::ColorConvertFloat4ToU32(bg_color);
            ImU32 col_text = ImGui::GetColorU32(ImGuiCol_Text);

            // Radar Diamond Icon
            float r = 10.0f;
            draw_list->AddCircleFilled(screen_pt, 3.5f, col_primary);
            
            ImVec2 p_top(screen_pt.x, screen_pt.y - r);
            ImVec2 p_right(screen_pt.x + r, screen_pt.y);
            ImVec2 p_bottom(screen_pt.x, screen_pt.y + r);
            ImVec2 p_left(screen_pt.x - r, screen_pt.y);
            draw_list->AddQuad(p_top, p_right, p_bottom, p_left, col_primary, 1.8f);

            // Label Callout
            char badge[192];
            if (s->has_live_rotation())
            {
                snprintf(badge, sizeof(badge), "%s [%llu/%llu] Az:%.1f° (%.1f RPM)",
                         s->get_label().c_str(),
                         static_cast<unsigned long long>(s->get_sac()),
                         static_cast<unsigned long long>(s->get_sic()),
                         s->get_current_azimuth_deg(),
                         s->get_rotation_rpm());
            }
            else
            {
                snprintf(badge, sizeof(badge), "%s (SAC:%llu SIC:%llu)",
                         s->get_label().c_str(),
                         static_cast<unsigned long long>(s->get_sac()),
                         static_cast<unsigned long long>(s->get_sic()));
            }

            ImVec2 label_size = ImGui::CalcTextSize(badge);
            float box_w = label_size.x + 12.0f;
            float box_h = label_size.y + 8.0f;
            
            ImVec2 tag_pos = ImVec2(screen_pt.x + r + 8.0f, screen_pt.y - box_h * 0.5f);

            draw_list->AddRectFilled(tag_pos, ImVec2(tag_pos.x + box_w, tag_pos.y + box_h), col_bg, 4.0f);
            draw_list->AddRect(tag_pos, ImVec2(tag_pos.x + box_w, tag_pos.y + box_h), col_primary, 4.0f, 0, 1.2f);

            draw_list->AddText(ImVec2(tag_pos.x + 6.0f, tag_pos.y + 4.0f), col_text, badge);
        }
    }

    void world_map::render_measurements(ImDrawList* draw_list, const ImVec2& pos, const ImVec2& size, const map_render_options& options)
    {
        if (m_measurement_points.empty()) return;

        ImU32 col_line = ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 0.65f, 0.0f, 0.8f));
        ImU32 col_text = ImGui::GetColorU32(ImGuiCol_Text);
        ImU32 col_bg = ImGui::ColorConvertFloat4ToU32(ImVec4(0.06f, 0.08f, 0.12f, 0.7f));
        
        float total_dist_km = 0.0f;
        
        for (size_t i = 0; i < m_measurement_points.size(); ++i)
        {
            ImVec2 p1 = geo_to_screen(m_measurement_points[i].lat, m_measurement_points[i].lon, pos, size, options.projection);
            
            // Frustum culling check
            bool p1_visible = !(p1.x < pos.x - 50.0f || p1.x > pos.x + size.x + 50.0f || p1.y < pos.y - 50.0f || p1.y > pos.y + size.y + 50.0f);
            
            if (p1_visible)
            {
                draw_list->AddCircleFilled(p1, 5.0f, col_line);
                draw_list->AddCircle(p1, 7.0f, ImGui::GetColorU32(ImGuiCol_WindowBg), 0, 1.5f);
            }
            
            if (i > 0)
            {
                ImVec2 p0 = geo_to_screen(m_measurement_points[i - 1].lat, m_measurement_points[i - 1].lon, pos, size, options.projection);
                
                float dist_km = calculate_haversine_distance_km(m_measurement_points[i - 1].lat, m_measurement_points[i - 1].lon, m_measurement_points[i].lat, m_measurement_points[i].lon);
                total_dist_km += dist_km;
                
                bool p0_visible = !(p0.x < pos.x - 50.0f || p0.x > pos.x + size.x + 50.0f || p0.y < pos.y - 50.0f || p0.y > pos.y + size.y + 50.0f);
                if (p0_visible || p1_visible)
                {
                    draw_list->AddLine(p0, p1, col_line, 2.5f);
                    
                    char label[64];
                    if (dist_km < 1.0f) std::snprintf(label, sizeof(label), "%.0f m", dist_km * 1000.0f);
                    else std::snprintf(label, sizeof(label), "%.1f km", dist_km);
                    
                    ImVec2 mid = ImVec2((p0.x + p1.x) * 0.5f, (p0.y + p1.y) * 0.5f);
                    ImVec2 text_size = ImGui::CalcTextSize(label);
                    
                    draw_list->AddRectFilled(ImVec2(mid.x - text_size.x * 0.5f - 4.0f, mid.y - text_size.y * 0.5f - 2.0f), 
                                             ImVec2(mid.x + text_size.x * 0.5f + 4.0f, mid.y + text_size.y * 0.5f + 2.0f), col_bg, 3.0f);
                    draw_list->AddText(ImVec2(mid.x - text_size.x * 0.5f, mid.y - text_size.y * 0.5f), col_text, label);
                }
            }
        }
        
        if (m_measurement_points.size() > 1)
        {
            ImVec2 last_p = geo_to_screen(m_measurement_points.back().lat, m_measurement_points.back().lon, pos, size, options.projection);
            char label[64];
            if (total_dist_km < 1.0f) std::snprintf(label, sizeof(label), "Total: %.0f m", total_dist_km * 1000.0f);
            else std::snprintf(label, sizeof(label), "Total: %.1f km", total_dist_km);
            
            ImVec2 text_size = ImGui::CalcTextSize(label);
            draw_list->AddRectFilled(ImVec2(last_p.x + 10.0f, last_p.y - text_size.y * 0.5f - 2.0f), 
                                     ImVec2(last_p.x + 10.0f + text_size.x + 8.0f, last_p.y + text_size.y * 0.5f + 2.0f), col_bg, 3.0f);
            draw_list->AddText(ImVec2(last_p.x + 14.0f, last_p.y - text_size.y * 0.5f), col_text, label);
        }
    }

    void world_map::render_scale_bar(ImDrawList* draw_list, const ImVec2& pos, const ImVec2& size, const map_render_options& options)
    {
        ImVec2 bar_pos = ImVec2(pos.x + 30.0f, pos.y + size.y - 30.0f);
        float bar_width_px = 120.0f;

        geo_point g1 = screen_to_geo(bar_pos, pos, size, options.projection);
        geo_point g2 = screen_to_geo(ImVec2(bar_pos.x + bar_width_px, bar_pos.y), pos, size, options.projection);

        // Haversine distance in KM
        float dist_km = calculate_haversine_distance_km(g1.lat, g1.lon, g2.lat, g2.lon);
        float dist_nm = dist_km * 0.539957f;

        char label[64];
        if (dist_km < 1.0f)
        {
            float dist_m = dist_km * 1000.0f;
            std::snprintf(label, sizeof(label), "%.0f m / %.2f NM", dist_m, dist_nm);
        }
        else if (dist_km >= 10.0f)
        {
            std::snprintf(label, sizeof(label), "%.0f km / %.0f NM", dist_km, dist_nm);
        }
        else
        {
            std::snprintf(label, sizeof(label), "%.1f km / %.1f NM", dist_km, dist_nm);
        }

        ImVec2 text_size = ImGui::CalcTextSize(label);
        
        float padding_x = 12.0f;
        float padding_y = 8.0f;
        float line_margin_top = 4.0f;
        float line_tick_h = 4.0f;
        
        float bg_width = std::max(bar_width_px + padding_x * 2.0f, text_size.x + padding_x * 2.0f);
        float bg_height = text_size.y + line_margin_top + line_tick_h * 2.0f + padding_y * 2.0f;
        
        float center_x = bar_pos.x + bar_width_px * 0.5f;
        ImVec2 bg_p0 = ImVec2(center_x - bg_width * 0.5f, bar_pos.y - bg_height);
        ImVec2 bg_p1 = ImVec2(bg_p0.x + bg_width, bg_p0.y + bg_height);

        ImVec4 theme_bg = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
        theme_bg.w = 0.85f; // Keep it semi-transparent
        
        ImU32 col_shadow = IM_COL32(0, 0, 0, 160);
        ImU32 col_bg = ImGui::ColorConvertFloat4ToU32(theme_bg);
        ImU32 col_border = ImGui::GetColorU32(ImGuiCol_Border);
        ImU32 col_text = ImGui::GetColorU32(ImGuiCol_Text);
        ImU32 col_line = ImGui::GetColorU32(ImGuiCol_Text);

        // Shadow
        draw_list->AddRectFilled(ImVec2(bg_p0.x + 3.0f, bg_p0.y + 3.0f), ImVec2(bg_p1.x + 3.0f, bg_p1.y + 3.0f), col_shadow, 6.0f);
        // Background
        draw_list->AddRectFilled(bg_p0, bg_p1, col_bg, 6.0f);
        // Border
        draw_list->AddRect(bg_p0, bg_p1, col_border, 6.0f, 0, 1.5f);

        // Text
        ImVec2 text_pos = ImVec2(center_x - text_size.x * 0.5f, bg_p0.y + padding_y);
        draw_list->AddText(text_pos, col_text, label);

        // Line
        float line_y = bg_p1.y - padding_y - line_tick_h;
        ImVec2 line_start = ImVec2(bar_pos.x, line_y);
        ImVec2 line_end = ImVec2(bar_pos.x + bar_width_px, line_y);
        draw_list->AddLine(line_start, line_end, col_line, 2.0f);
        draw_list->AddLine(ImVec2(line_start.x, line_start.y - line_tick_h), ImVec2(line_start.x, line_start.y + line_tick_h), col_line, 2.0f);
        draw_list->AddLine(ImVec2(line_end.x, line_end.y - line_tick_h), ImVec2(line_end.x, line_end.y + line_tick_h), col_line, 2.0f);
        
        // Add middle tick
        draw_list->AddLine(ImVec2(center_x, line_y - line_tick_h * 0.5f), ImVec2(center_x, line_y + line_tick_h * 0.5f), col_line, 1.5f);
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
