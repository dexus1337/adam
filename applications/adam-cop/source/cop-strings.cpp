/**
 * @file    cop-strings.cpp
 * @author  dexus1337
 * @brief   Localized string lookup tables implementation for adam-cop
 * @version 1.0
 * @date    05.08.2026
 */

#include "cop-strings.hpp"

#include <unordered_map>
#include <array>

namespace adam::cop
{
    const char* get_cop_string(cop_string_id id, adam::language lang)
    {
        static const std::unordered_map<cop_string_id, std::array<const char*, adam::languages_count>> translations =
        {

            { cop_main_title, { "ADAM - COP", "ADAM - COP" } },
            { menu_file, { "File", "Datei" } },
            { menu_view, { "View", "Ansicht" } },
            { menu_map, { "Map", "Karte" } },
            { menu_tools, { "Tools", "Werkzeuge" } },
            { menu_help, { "Help", "Hilfe" } },
            { menu_exit, { "Exit", "Beenden" } },
            { menu_reset_view, { "Reset View", "Ansicht zurücksetzen" } },
            { menu_center_origin, { "Center Origin (0°N, 0°E)", "Zentriere Ursprung (0°N, 0°E)" } },
            { lbl_layers_panel, { "Layers & Controls", "Ebenen & Steuerung" } },
            { provider_cartodb, { "Tactical", "Taktisch" } },
            { provider_osm, { "City", "Stadtkarte" } },
            { provider_esri, { "Satellite", "Satellit" } },
            { provider_opentopo, { "Topographic", "Topographisch" } },
            { lbl_map_opacity, { "Tile Opacity:", "Kachel-Deckkraft:" } },
            { lbl_projection, { "Map Projection", "Kartenprojektion" } },
            { proj_equirectangular, { "Plate Carrée (Equirectangular)", "Plattkarte (Equirectangular)" } },
            { proj_mercator, { "Mercator", "Mercator" } },
            { lbl_grid_toggle, { "Tactical Coordinate Grid", "Taktisches Koordinatengitter" } },
            { lbl_coastlines_toggle, { "Show Coastlines", "Küstenlinien anzeigen" } },
            { lbl_land_fill_toggle, { "Fill Landmass Polygons", "Landmassen einfärben" } },
            { lbl_scale_bar_toggle, { "Distance Scale Bar (km / NM)", "Maßstabsleiste (km / NM)" } },
            { lbl_compass_toggle, { "Show Compass Rose", "Kompassrose anzeigen" } },
            { lbl_show, { "Show", "Zeige" } },
            { lbl_cache_stats, { "Tile Cache Status", "Kachel-Cache Status" } },
            { btn_clear_tile_cache, { "Clear Tile Cache", "Cache leeren" } },
            { lbl_placed_markers, { "Placed Waypoints & Markers", "Platzierte Wegpunkte / Marker" } },
            { btn_clear_markers, { "Clear All Markers", "Alle Marker löschen" } },
            { lbl_no_markers, { "Click on the map to place a tactical marker.", "Klicken Sie auf die Karte, um einen Marker zu platzieren." } },
            { lbl_jump_to_coordinates, { "Jump to Coordinates", "Gehe zu Koordinate" } },
            { lbl_lat, { "Latitude (Lat °):", "Breite (Lat °):" } },
            { lbl_lon, { "Longitude (Lon °):", "Länge (Lon °):" } },
            { btn_jump, { "Center Camera", "Zentrieren" } },
            { btn_reset_camera, { "Reset Camera", "Kamera zurücksetzen" } },
            { lbl_cursor_coords, { "Cursor:", "Zeiger:" } },
            { lbl_map_center, { "Center:", "Zentrum:" } },
            { lbl_zoom_level, { "Zoom:", "Zoom:" } },
            { lbl_fps, { "FPS:", "FPS:" } },
            { lbl_status_online, { "SYSTEM READY", "SYSTEM BEREIT" } },
            { lbl_status_offline, { "DISCONNECTED", "GETRENNT" } },
            { msg_cop_description, { "ADAM Common Operational Picture (COP) for ASTERIX multi-sensor tracking and map overview.", "ADAM Common Operational Picture (COP) für ASTERIX Multi-Sensor Tracking und Lagebilddarstellung." } },
            { msg_cop_copyright, { "© 2026 dexus1337. All rights reserved.", "© 2026 dexus1337. Alle Rechte vorbehalten." } },
            { menu_settings, { "Settings", "Einstellungen" } },
            { menu_gui_mode, { "GUI Mode", "GUI-Modus" } },
            { gui_mode_default, { "Default", "Standard" } },
            { gui_mode_immediate, { "Immediate", "Unmittelbar" } },
            { menu_fps_limit, { "FPS Limit", "FPS-Limit" } },
            { fps_10, { "10 FPS", "10 FPS" } },
            { fps_30, { "30 FPS", "30 FPS" } },
            { fps_60, { "60 FPS", "60 FPS" } },
            { fps_120, { "120 FPS", "120 FPS" } },
            { fps_vsync, { "VSync", "VSync" } },
            { fps_unlimited, { "Unlimited", "Unbegrenzt" } },
            { combo_language, { "Language", "Sprache" } },
            { slider_font_scale, { "Font Scale", "Schriftskalierung" } },
            { menu_show_performance, { "Show Performance Overlay", "Leistungsübersicht anzeigen" } },
            { combo_theme, { "Theme", "Design" } },
            { theme_dark, { "Dark", "Dunkel" } },
            { theme_light, { "Light", "Hell" } },
            { theme_dark_navy, { "Dark Navy", "Dunkelblau" } },
            { lbl_performance_overlay, { "Performance Overlay", "Leistungsübersicht" } },
            { lbl_cpu, { "CPU: %.1f%%", "CPU: %.1f%%" } },
            { lbl_ram, { "RAM: %.1f GB / %.1f GB (%.1f%%)", "RAM: %.1f GB / %.1f GB (%.1f%%)" } },
            { menu_overlay_position, { "Position", "Position" } },
            { menu_overlay_custom, { "Custom", "Benutzerdefiniert" } },
            { menu_overlay_top_left, { "Top-Left", "Oben links" } },
            { menu_overlay_top_right, { "Top-Right", "Oben rechts" } },
            { menu_overlay_bottom_left, { "Bottom-Left", "Unten links" } },
            { menu_overlay_bottom_right, { "Bottom-Right", "Unten rechts" } },
            { menu_overlay_content, { "Content", "Inhalt" } },
            { menu_overlay_show_fps, { "Show FPS", "FPS anzeigen" } },
            { menu_overlay_show_cpu, { "Show CPU Usage", "CPU-Auslastung anzeigen" } },
            { menu_overlay_show_ram, { "Show RAM Usage", "RAM-Auslastung anzeigen" } },
            { wnd_waypoints, { "Waypoints", "Wegpunkte" } },
            { wnd_world_map, { "World Map Overview", "Weltkarte Übersicht" } },
            { wnd_settings, { "Settings", "Einstellungen" } },
            { wnd_log_console, { "System Log Console", "Systemprotokoll" } },
            { wnd_data_sources, { "Data Sources", "Datenquellen" } },
            { wnd_about, { "About###WndAbout", "Über###WndAbout" } },
            { wnd_sites, { "Radar Sites", "Radar-Stationen" } },
            { menu_add_waypoint_here, { "Add Waypoint Here", "Wegpunkt hier hinzufügen" } }
        };

        auto it = translations.find(id);
        if (it != translations.end())
        {
            return it->second[static_cast<size_t>(lang)];
        }
        
        return "UNKNOWN_STRING_ID";
    }
}
