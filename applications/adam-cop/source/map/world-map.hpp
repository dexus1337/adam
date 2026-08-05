#pragma once

/**
 * @file    world-map.hpp
 * @author  dexus1337
 * @brief   Scalable interactive C2 world map rendering engine header with tile layer and marker support
 * @version 1.0
 * @date    05.08.2026
 */

#include "world-map-data.hpp"
#include "tile-engine.hpp"
#include "tile-provider.hpp"
#include <imgui.h>

#include <vector>
#include <string>
#include <cstdint>

namespace adam::cop
{
    enum class projection_type
    {
        equirectangular = 0,
        mercator
    };

    struct map_marker
    {
        uint32_t id = 0;
        float lat = 0.0f;
        float lon = 0.0f;
        std::string label;
        ImVec4 color = ImVec4(1.00f, 0.35f, 0.10f, 1.00f);
    };

    struct map_render_options
    {
        projection_type projection = projection_type::mercator;
        tile_provider_type base_provider = tile_provider_type::cartodb_dark;
        bool show_grid = true;
        bool show_coastlines = true;
        bool show_scale_bar = true;
        bool show_compass = true;
        bool show_markers = true;
        float map_opacity = 1.0f;
        ImVec4 coastline_color = ImVec4(0.00f, 0.75f, 0.95f, 0.90f);
        ImVec4 grid_color = ImVec4(0.20f, 0.30f, 0.40f, 0.45f);
        ImVec4 ocean_bg_color = ImVec4(0.06f, 0.08f, 0.12f, 1.00f);
    };

    class world_map
    {
    public:
        world_map();
        ~world_map() = default;

        /** @brief Renders the interactive map within the current ImGui window region */
        void draw(const ImVec2& size, const map_render_options& options);

        /** @brief Resets map view to default origin and zoom level */
        void reset_view();

        /** @brief Centers map camera on given latitude and longitude */
        void set_center(float lat, float lon);

        /** @brief Adjusts zoom level */
        void set_zoom(float zoom);

        /** @brief Adds a new tactical marker at lat/lon */
        void add_marker(float lat, float lon, const std::string& label = "");

        /** @brief Removes all tactical markers */
        void clear_markers();

        /** @brief Removes a marker by ID */
        void remove_marker(uint32_t id);

        float get_center_lat() const { return m_center_lat; }
        float get_center_lon() const { return m_center_lon; }
        float get_zoom()       const { return m_zoom; }

        float get_hover_lat()  const { return m_hover_lat; }
        float get_hover_lon()  const { return m_hover_lon; }
        bool  is_hovered()     const { return m_is_hovered; }

        const std::vector<map_marker>& get_markers() const { return m_markers; }
        tile_engine& get_tile_engine() { return m_tile_engine; }

        /** @brief Converts latitude and longitude to screen space pixel coordinates */
        ImVec2 geo_to_screen(float lat, float lon, const ImVec2& canvas_pos, const ImVec2& canvas_size, projection_type proj) const;

        /** @brief Converts screen pixel coordinates to latitude and longitude */
        geo_point screen_to_geo(const ImVec2& screen_pos, const ImVec2& canvas_pos, const ImVec2& canvas_size, projection_type proj) const;

    private:
        void render_ocean(ImDrawList* draw_list, const ImVec2& pos, const ImVec2& size, const map_render_options& options);
        void render_tiles(ImDrawList* draw_list, const ImVec2& pos, const ImVec2& size, const map_render_options& options);
        void render_grid(ImDrawList* draw_list, const ImVec2& pos, const ImVec2& size, const map_render_options& options);
        void render_vector_land(ImDrawList* draw_list, const ImVec2& pos, const ImVec2& size, const map_render_options& options);
        void render_markers(ImDrawList* draw_list, const ImVec2& pos, const ImVec2& size, const map_render_options& options);
        void render_scale_bar(ImDrawList* draw_list, const ImVec2& pos, const ImVec2& size, const map_render_options& options);
        void render_compass(ImDrawList* draw_list, const ImVec2& pos, const ImVec2& size, const map_render_options& options);

        tile_engine m_tile_engine;

        float m_center_lat = 20.0f;
        float m_center_lon = 10.0f;
        float m_zoom = 1.0f;

        float m_hover_lat = 0.0f;
        float m_hover_lon = 0.0f;
        bool  m_is_hovered = false;

        std::vector<map_marker> m_markers;
        uint32_t m_next_marker_id = 1;
    };
}
