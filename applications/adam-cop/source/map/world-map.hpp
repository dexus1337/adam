#pragma once

/**
 * @file    world-map.hpp
 * @author  dexus1337
 * @brief   Scalable interactive C2 world map rendering engine header with tile layer and marker support
 * @version 1.0
 * @date    05.08.2026
 */

#include <imgui.h>
#include <lib-radar.hpp>

#include <vector>
#include <array>
#include <string>
#include <cstdint>

#include "coastlines.hpp"
#include "tile-engine.hpp"
#include "tile-provider.hpp"
#include "../data/waypoint.hpp"
#include "../data/drawable-site.hpp"

namespace adam::cop
{
    using namespace adam::lib::radar;

    enum class projection_type
    {
        equirectangular = 0,
        mercator
    };


    struct map_layer
    {
        tile_provider_type provider = tile_provider_type::cartodb_dark;
        float opacity = 1.0f;
        bool visible = true;
    };

    struct map_render_options
    {
        projection_type projection = projection_type::mercator;
        std::array<map_layer, 4> tile_layers;
        bool show_grid = true;
        bool show_coastlines = true;
        bool show_scale_bar = true;
        bool show_compass = true;
        bool show_markers = true;
        ImVec4 coastline_color = ImVec4(0.00f, 0.75f, 0.95f, 0.90f);
        ImVec4 grid_color = ImVec4(0.20f, 0.30f, 0.40f, 0.45f);
        ImVec4 ocean_bg_color = ImVec4(0.06f, 0.08f, 0.12f, 1.00f);
    };

    class world_map
    {
    public:
        world_map();
        ~world_map() = default;
        /**
         * @brief Renders the map and handles all internal interactions.
         * 
         * @param size              The size of the map area.
         * @param options           Configuration struct defining visualization details.
         * @param waypoints         The list of tactical waypoints to render.
         * @param add_waypoint_text Localized text for the context menu.
         */
        void draw(const ImVec2& size, const map_render_options& options, const std::vector<std::unique_ptr<waypoint>>& waypoints, const std::vector<std::unique_ptr<drawable_site>>& sites, const char* add_waypoint_text = "Add Waypoint Here");

        /** @brief Resets map view to default origin and zoom level */
        void reset_view();

        /** @brief Centers map camera on given latitude and longitude */
        void set_center(float lat, float lon);

        /** @brief Adjusts zoom level */
        void set_zoom(float zoom);

        float get_center_lat() const { return m_center_lat; }
        float get_center_lon() const { return m_center_lon; }
        float get_zoom()       const { return m_zoom; }

        float get_hover_lat()  const { return m_hover_lat; }
        float get_hover_lon()  const { return m_hover_lon; }
        bool  is_hovered()     const { return m_is_hovered; }

        tile_engine& get_tile_engine() { return m_tile_engine; }

        /** @brief Converts latitude and longitude to screen space pixel coordinates */
        ImVec2 geo_to_screen(float lat, float lon, const ImVec2& canvas_pos, const ImVec2& canvas_size, projection_type proj) const;

        /** @brief Converts screen pixel coordinates to latitude and longitude */
        geo_point screen_to_geo(const ImVec2& screen_pos, const ImVec2& canvas_pos, const ImVec2& canvas_size, projection_type proj) const;

        bool consume_context_add_waypoint_request(float& out_lat, float& out_lon);

        void clear_measurements() { m_measurement_points.clear(); }

    private:
        void render_ocean(ImDrawList* draw_list, const ImVec2& pos, const ImVec2& size, const map_render_options& options);
        void render_tiles(ImDrawList* draw_list, const ImVec2& pos, const ImVec2& size, const map_render_options& options);
        void render_grid(ImDrawList* draw_list, const ImVec2& pos, const ImVec2& size, const map_render_options& options);
        void render_vector_land(ImDrawList* draw_list, const ImVec2& pos, const ImVec2& size, const map_render_options& options);
        void render_waypoints(ImDrawList* draw_list, const ImVec2& pos, const ImVec2& size, const map_render_options& options, const std::vector<std::unique_ptr<waypoint>>& waypoints);
        void render_sites(ImDrawList* draw_list, const ImVec2& pos, const ImVec2& size, const map_render_options& options, const std::vector<std::unique_ptr<drawable_site>>& sites);
        void render_scale_bar(ImDrawList* draw_list, const ImVec2& pos, const ImVec2& size, const map_render_options& options);
        void render_compass(ImDrawList* draw_list, const ImVec2& pos, const ImVec2& size, const map_render_options& options);
        void render_measurements(ImDrawList* draw_list, const ImVec2& pos, const ImVec2& size, const map_render_options& options);

        tile_engine m_tile_engine;

        bool  m_context_add_waypoint_requested = false;
        float m_context_waypoint_lat = 0.0f;
        float m_context_waypoint_lon = 0.0f;
        
        float m_center_lat = 20.0f;
        float m_center_lon = 10.0f;
        float m_zoom = 1.0f;

        float m_hover_lat = 0.0f;
        float m_hover_lon = 0.0f;
        bool  m_is_hovered = false;
        
        std::vector<geo_point> m_measurement_points;

    };
}
