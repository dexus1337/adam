/**
 * @file    window-cache-stats.cpp
 * @author  dexus1337
 * @brief   Implementation of the tile cache statistics window for adam-cop.
 * @version 1.0
 * @date    20.08.2026
 */

#include "window-cache-stats.hpp"
#include "../cop-strings.hpp"
#include "../map/world-map.hpp"
#include <imgui.h>
#include <string>

namespace adam::cop
{
    void draw_cache_stats_window(world_map& map, adam::language lang, adam::configuration_parameter_boolean* p_show_cache_stats)
    {
        if (!p_show_cache_stats || !p_show_cache_stats->get_value())
        {
            return;
        }

        std::string title_cache = std::string(get_cop_string(lbl_cache_stats, lang)) + "###CacheStats";
        if (!ImGui::Begin(title_cache.c_str(), &p_show_cache_stats->value()))
        {
            ImGui::End();
            return;
        }

        auto& engine = map.get_tile_engine();
        ImGui::Text("GPU Loaded Textures: %zu", engine.get_loaded_texture_count());
        ImGui::Text("Pending HTTP Requests: %zu", engine.get_pending_request_count());

        if (ImGui::Button(get_cop_string(btn_clear_tile_cache, lang), ImVec2(-1.0f, 0.0f)))
        {
            engine.clear_cache();
        }

        ImGui::End();
    }
}
