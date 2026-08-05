#pragma once

/**
 * @file    tile-engine.hpp
 * @author  dexus1337
 * @brief   Asynchronous raster tile loading, disk caching, and texture manager
 * @version 1.0
 * @date    05.08.2026
 */

#include "tile-provider.hpp"
#include <imgui.h>

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <queue>
#include <atomic>
#include <memory>

namespace adam::cop
{
    struct tile_texture_entry
    {
        ImTextureID texture_id = (ImTextureID)0;
        int width = 0;
        int height = 0;
        bool is_loading = false;
        bool failed = false;
        uint64_t last_accessed = 0;
    };

    struct pending_gpu_upload
    {
        std::string key;
        int width = 0;
        int height = 0;
        std::vector<uint8_t> pixels;
    };

    class tile_engine
    {
    public:
        tile_engine();
        ~tile_engine();

        /** @brief Shuts down background threads and releases GPU tile textures */
        void shutdown();

        /** @brief Process main-thread pending GPU texture uploads */
        void update();

        /** @brief Retrieves loaded GPU texture ID for a given tile. Returns 0 if loading. */
        ImTextureID get_tile_texture(tile_provider_type provider, int z, int x, int y);

        /** @brief Clears all cached disk and memory textures for specified or all providers */
        void clear_cache();

        /** @brief Retrieves active texture cache statistics */
        size_t get_loaded_texture_count() const;
        size_t get_pending_request_count() const;

    private:
        void worker_loop();
        bool fetch_and_decode_tile(tile_provider_type provider, int z, int x, int y, std::vector<uint8_t>& out_pixels, int& out_w, int& out_h);

        mutable std::mutex m_mutex;
        std::unordered_map<std::string, tile_texture_entry> m_cache;

        std::queue<std::string> m_fetch_queue;
        std::unordered_map<std::string, bool> m_in_fetch_queue;

        std::vector<pending_gpu_upload> m_pending_uploads;

        std::atomic<bool> m_running;
        std::thread m_worker_thread;
        uint64_t m_access_counter = 0;
    };
}
