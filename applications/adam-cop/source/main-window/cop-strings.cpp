/**
 * @file    cop-strings.cpp
 * @author  dexus1337
 * @brief   Localized string lookup tables implementation for adam-cop
 * @version 1.0
 * @date    05.08.2026
 */

#include "cop-strings.hpp"

namespace adam::cop
{
    const char* get_cop_string(cop_string_id id, adam::language lang)
    {
        switch (id)
        {
            case cop_main_title:
                return (lang == adam::language_german) ? "ADAM - COP" : "ADAM - COP";
            case menu_file:
                return (lang == adam::language_german) ? "Datei" : "File";
            case menu_view:
                return (lang == adam::language_german) ? "Ansicht" : "View";
            case menu_map:
                return (lang == adam::language_german) ? "Karte" : "Map";
            case menu_tools:
                return (lang == adam::language_german) ? "Werkzeuge" : "Tools";
            case menu_help:
                return (lang == adam::language_german) ? "Hilfe" : "Help";
            case menu_exit:
                return (lang == adam::language_german) ? "Beenden" : "Exit";
            case menu_reset_view:
                return (lang == adam::language_german) ? "Ansicht zurücksetzen" : "Reset View";
            case menu_center_origin:
                return (lang == adam::language_german) ? "Zentriere Ursprung (0°N, 0°E)" : "Center Origin (0°N, 0°E)";
            case lbl_layers_panel:
                return (lang == adam::language_german) ? "Ebenen & Steuerung" : "Layers & Controls";
            case lbl_base_map_layer:
                return (lang == adam::language_german) ? "Basiskarte (Raster-Kacheln)" : "Base Map Layer";
            case provider_cartodb:
                return (lang == adam::language_german) ? "CartoDB Dark Matter (Taktisch Dark)" : "CartoDB Dark Matter (Tactical Dark)";
            case provider_osm:
                return (lang == adam::language_german) ? "OpenStreetMap Standard" : "OpenStreetMap Standard";
            case provider_esri:
                return (lang == adam::language_german) ? "Esri Luftbild (Satellit)" : "Esri World Imagery (Satellite)";
            case provider_opentopo:
                return (lang == adam::language_german) ? "OpenTopoMap (Topographisch)" : "OpenTopoMap (Topographic)";
            case provider_vector:
                return (lang == adam::language_german) ? "Nur Vektorgitter (Keine Kacheln)" : "Vector Only (No Tiles)";
            case lbl_map_opacity:
                return (lang == adam::language_german) ? "Kachel-Deckkraft:" : "Tile Opacity:";
            case lbl_projection:
                return (lang == adam::language_german) ? "Kartenprojektion" : "Map Projection";
            case proj_equirectangular:
                return (lang == adam::language_german) ? "Plattkarte (Equirectangular)" : "Plate Carrée (Equirectangular)";
            case proj_mercator:
                return (lang == adam::language_german) ? "Mercator" : "Mercator";
            case lbl_grid_toggle:
                return (lang == adam::language_german) ? "Taktisches Koordinatengitter" : "Tactical Coordinate Grid";
            case lbl_coastlines_toggle:
                return (lang == adam::language_german) ? "Küstenlinien anzeigen" : "Show Coastlines";
            case lbl_land_fill_toggle:
                return (lang == adam::language_german) ? "Landmassen einfärben" : "Fill Landmass Polygons";
            case lbl_scale_bar_toggle:
                return (lang == adam::language_german) ? "Maßstabsleiste (km / NM)" : "Distance Scale Bar (km / NM)";
            case lbl_compass_toggle:
                return (lang == adam::language_german) ? "Kompassrose anzeigen" : "Show Compass Rose";
            case lbl_cache_stats:
                return (lang == adam::language_german) ? "Kachel-Cache Status" : "Tile Cache Status";
            case btn_clear_tile_cache:
                return (lang == adam::language_german) ? "Cache leeren" : "Clear Tile Cache";
            case lbl_placed_markers:
                return (lang == adam::language_german) ? "Platzierte Wegpunkte / Marker" : "Placed Waypoints & Markers";
            case btn_clear_markers:
                return (lang == adam::language_german) ? "Alle Marker löschen" : "Clear All Markers";
            case lbl_no_markers:
                return (lang == adam::language_german) ? "Klicken Sie auf die Karte, um einen Marker zu platzieren." : "Click on the map to place a tactical marker.";
            case lbl_jump_to_coordinates:
                return (lang == adam::language_german) ? "Gehe zu Koordinate" : "Jump to Coordinates";
            case lbl_lat:
                return (lang == adam::language_german) ? "Breite (Lat °):" : "Latitude (Lat °):";
            case lbl_lon:
                return (lang == adam::language_german) ? "Länge (Lon °):" : "Longitude (Lon °):";
            case btn_jump:
                return (lang == adam::language_german) ? "Zentrieren" : "Center Camera";
            case btn_reset_camera:
                return (lang == adam::language_german) ? "Kamera zurücksetzen" : "Reset Camera";
            case lbl_cursor_coords:
                return (lang == adam::language_german) ? "Zeiger:" : "Cursor:";
            case lbl_map_center:
                return (lang == adam::language_german) ? "Zentrum:" : "Center:";
            case lbl_zoom_level:
                return (lang == adam::language_german) ? "Zoom:" : "Zoom:";
            case lbl_fps:
                return (lang == adam::language_german) ? "FPS:" : "FPS:";
            case lbl_status_online:
                return (lang == adam::language_german) ? "SYSTEM BEREIT" : "SYSTEM READY";
            case lbl_status_offline:
                return (lang == adam::language_german) ? "GETRENNT" : "DISCONNECTED";
            case wnd_log_console:
                return (lang == adam::language_german) ? "Systemprotokoll" : "System Log Console";
            case wnd_asterix_connections:
                return (lang == adam::language_german) ? "ASTERIX Quellen & Multi-Sensor Tracking" : "ASTERIX Sources & Multi-Sensor Tracking";
            case wnd_about:
                return (lang == adam::language_german) ? "Über adam-cop" : "About adam-cop";
            case msg_cop_description:
                return (lang == adam::language_german) ? "ADAM Common Operational Picture (COP) für ASTERIX Multi-Sensor Tracking und Lagebilddarstellung." : "ADAM Common Operational Picture (COP) for ASTERIX multi-sensor tracking and map overview.";
            case msg_cop_copyright:
                return "Copyright (c) 2026 dexus1337. All rights reserved.";
            default:
                return "";
        }
    }
}
