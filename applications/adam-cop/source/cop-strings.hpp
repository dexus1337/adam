#pragma once

/**
 * @file    cop-strings.hpp
 * @author  dexus1337
 * @brief   String ID definitions and localization lookup for adam-cop C2 application
 * @version 1.0
 * @date    05.08.2026
 */

#include <adam-sdk.hpp>
#include <cstdint>

namespace adam::cop
{
    enum cop_string_id : uint32_t
    {
        cop_main_title = 0,
        menu_file,
        menu_view,
        menu_map,
        menu_tools,
        menu_help,
        menu_exit,
        menu_reset_view,
        menu_center_origin,
        lbl_layers_panel,
        lbl_base_map_layer,
        provider_cartodb,
        provider_osm,
        provider_esri,
        provider_opentopo,
        provider_vector,
        lbl_map_opacity,
        lbl_projection,
        proj_equirectangular,
        proj_mercator,
        lbl_grid_toggle,
        lbl_coastlines_toggle,
        lbl_land_fill_toggle,
        lbl_scale_bar_toggle,
        lbl_compass_toggle,
        lbl_show,
        lbl_cache_stats,
        btn_clear_tile_cache,
        lbl_placed_markers,
        btn_clear_markers,
        lbl_no_markers,
        lbl_jump_to_coordinates,
        lbl_lat,
        lbl_lon,
        btn_jump,
        btn_reset_camera,
        lbl_cursor_coords,
        lbl_map_center,
        lbl_zoom_level,
        lbl_fps,
        lbl_status_online,
        lbl_status_offline,
        msg_cop_description,
        msg_cop_copyright,
        menu_settings,
        menu_gui_mode,
        gui_mode_default,
        gui_mode_immediate,
        menu_fps_limit,
        fps_10,
        fps_30,
        fps_60,
        fps_120,
        fps_vsync,
        fps_unlimited,
        combo_language,
        slider_font_scale,
        menu_show_performance,
        combo_theme,
        theme_dark,
        theme_light,
        theme_dark_navy,
        lbl_performance_overlay,
        lbl_cpu,
        lbl_ram,
        menu_overlay_position,
        menu_overlay_custom,
        menu_overlay_top_left,
        menu_overlay_top_right,
        menu_overlay_bottom_left,
        menu_overlay_bottom_right,
        menu_overlay_content,
        menu_overlay_show_fps,
        menu_overlay_show_cpu,
        menu_overlay_show_ram,
        wnd_waypoints,
        wnd_world_map,
        wnd_settings,
        wnd_log_console,
        wnd_data_sources,
        wnd_sites,
        wnd_about,
        menu_add_waypoint_here
    };

    const char* get_cop_string(cop_string_id id, adam::language lang);
}
