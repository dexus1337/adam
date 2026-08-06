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
            { lbl_base_map_layer, { "Base Map Layer", "Basiskarte (Raster-Kacheln)" } },
            { provider_cartodb, { "CartoDB Dark Matter (Tactical Dark)", "CartoDB Dark Matter (Taktisch Dark)" } },
            { provider_osm, { "OpenStreetMap Standard", "OpenStreetMap Standard" } },
            { provider_esri, { "Esri World Imagery (Satellite)", "Esri Luftbild (Satellit)" } },
            { provider_opentopo, { "OpenTopoMap (Topographic)", "OpenTopoMap (Topographisch)" } },
            { provider_vector, { "Vector Only (No Tiles)", "Nur Vektorgitter (Keine Kacheln)" } },
            { lbl_map_opacity, { "Tile Opacity:", "Kachel-Deckkraft:" } },
            { lbl_projection, { "Map Projection", "Kartenprojektion" } },
            { proj_equirectangular, { "Plate Carrée (Equirectangular)", "Plattkarte (Equirectangular)" } },
            { proj_mercator, { "Mercator", "Mercator" } },
            { lbl_grid_toggle, { "Tactical Coordinate Grid", "Taktisches Koordinatengitter" } },
            { lbl_coastlines_toggle, { "Show Coastlines", "Küstenlinien anzeigen" } },
            { lbl_land_fill_toggle, { "Fill Landmass Polygons", "Landmassen einfärben" } },
            { lbl_scale_bar_toggle, { "Distance Scale Bar (km / NM)", "Maßstabsleiste (km / NM)" } },
            { lbl_compass_toggle, { "Show Compass Rose", "Kompassrose anzeigen" } },
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
            { wnd_log_console, { "System Log Console", "Systemprotokoll" } },
            { wnd_asterix_connections, { "ASTERIX Sources & Multi-Sensor Tracking", "ASTERIX Quellen & Multi-Sensor Tracking" } },
            { wnd_about, { "About adam-cop", "Über adam-cop" } },
            { msg_cop_description, { "ADAM Common Operational Picture (COP) for ASTERIX multi-sensor tracking and map overview.", "ADAM Common Operational Picture (COP) für ASTERIX Multi-Sensor Tracking und Lagebilddarstellung." } },
            { combo_theme, { "Theme", "Design" } },
            { theme_dark, { "Dark", "Dunkel" } },
            { theme_light, { "Light", "Hell" } },
            { theme_dark_navy, { "Dark Navy", "Dunkelblau" } }
        };

        auto it = translations.find(id);
        if (it != translations.end())
        {
            return it->second[static_cast<size_t>(lang)];
        }
        
        return "UNKNOWN_STRING_ID";
    }
}
