/**
 * @file    tile-engine.cpp
 * @author  dexus1337
 * @brief   Asynchronous raster tile manager and HTTP download engine implementation
 * @version 1.0
 * @date    05.08.2026
 */

#include "tile-engine.hpp"
#include "../setup.hpp"

#include <adam-sdk.hpp>
#include <imgui.h>
#include <imgui-tools.hpp>

#if defined(ADAM_PLATFORM_WINDOWS)
#include <windows.h>
#include <wininet.h>
#include <wincodec.h>
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "windowscodecs.lib")
#endif

#include <filesystem>
#include <fstream>
#include <iostream>

namespace adam::cop
{
#if defined(ADAM_PLATFORM_WINDOWS)
    static bool decode_image_wic(const uint8_t* encoded_data, size_t encoded_size, std::vector<uint8_t>& out_pixels, int& out_w, int& out_h)
    {
        if (!encoded_data || encoded_size == 0)
        {
            return false;
        }

        IWICImagingFactory* pFactory = nullptr;
        HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFactory));
        if (FAILED(hr) || !pFactory)
        {
            return false;
        }

        IWICStream* pStream = nullptr;
        hr = pFactory->CreateStream(&pStream);
        if (FAILED(hr) || !pStream)
        {
            pFactory->Release();
            return false;
        }

        hr = pStream->InitializeFromMemory(const_cast<BYTE*>(encoded_data), static_cast<DWORD>(encoded_size));
        if (FAILED(hr))
        {
            pStream->Release();
            pFactory->Release();
            return false;
        }

        IWICBitmapDecoder* pDecoder = nullptr;
        hr = pFactory->CreateDecoderFromStream(pStream, NULL, WICDecodeMetadataCacheOnDemand, &pDecoder);
        if (FAILED(hr) || !pDecoder)
        {
            pStream->Release();
            pFactory->Release();
            return false;
        }

        IWICBitmapFrameDecode* pFrame = nullptr;
        hr = pDecoder->GetFrame(0, &pFrame);
        if (FAILED(hr) || !pFrame)
        {
            pDecoder->Release();
            pStream->Release();
            pFactory->Release();
            return false;
        }

        UINT w = 0;
        UINT h = 0;
        pFrame->GetSize(&w, &h);
        out_w = static_cast<int>(w);
        out_h = static_cast<int>(h);

        IWICFormatConverter* pConverter = nullptr;
        hr = pFactory->CreateFormatConverter(&pConverter);
        if (FAILED(hr) || !pConverter)
        {
            pFrame->Release();
            pDecoder->Release();
            pStream->Release();
            pFactory->Release();
            return false;
        }

        hr = pConverter->Initialize(pFrame, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, NULL, 0.0, WICBitmapPaletteTypeCustom);
        if (FAILED(hr))
        {
            pConverter->Release();
            pFrame->Release();
            pDecoder->Release();
            pStream->Release();
            pFactory->Release();
            return false;
        }

        out_pixels.resize(w * h * 4);
        hr = pConverter->CopyPixels(NULL, w * 4, static_cast<UINT>(out_pixels.size()), out_pixels.data());

        pConverter->Release();
        pFrame->Release();
        pDecoder->Release();
        pStream->Release();
        pFactory->Release();

        return SUCCEEDED(hr);
    }

    static bool http_download_tile(const std::string& url, const std::string& user_agent, std::vector<uint8_t>& out_bytes)
    {
        if (url.empty())
        {
            return false;
        }

        HINTERNET hInternet = InternetOpenA(user_agent.c_str(), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
        if (!hInternet)
        {
            return false;
        }

        DWORD timeout_ms = 5000;
        InternetSetOptionA(hInternet, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout_ms, sizeof(timeout_ms));
        InternetSetOptionA(hInternet, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout_ms, sizeof(timeout_ms));

        HINTERNET hUrl = InternetOpenUrlA(hInternet, url.c_str(), "Accept: image/png,image/jpeg\r\n", static_cast<DWORD>(-1L), INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);
        if (!hUrl)
        {
            InternetCloseHandle(hInternet);
            return false;
        }

        uint8_t buffer[8192];
        DWORD bytes_read = 0;
        out_bytes.clear();

        while (InternetReadFile(hUrl, buffer, sizeof(buffer), &bytes_read) && bytes_read > 0)
        {
            out_bytes.insert(out_bytes.end(), buffer, buffer + bytes_read);
        }

        InternetCloseHandle(hUrl);
        InternetCloseHandle(hInternet);

        return !out_bytes.empty();
    }
#endif

    tile_engine::tile_engine()
        : m_running(true)
    {
#if defined(ADAM_PLATFORM_WINDOWS)
        CoInitializeEx(NULL, COINIT_MULTITHREADED);
#endif
        m_worker_thread = std::thread(&tile_engine::worker_loop, this);
    }

    tile_engine::~tile_engine()
    {
        shutdown();
#if defined(ADAM_PLATFORM_WINDOWS)
        CoUninitialize();
#endif
    }

    void tile_engine::shutdown()
    {
        if (!m_running.exchange(false))
        {
            return;
        }

        if (m_worker_thread.joinable())
        {
            m_worker_thread.join();
        }

        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& pair : m_cache)
        {
            if (pair.second.texture_id)
            {
                adam::imgui_tools::destroy_texture(pair.second.texture_id);
                pair.second.texture_id = (ImTextureID)0;
            }
        }
        m_cache.clear();
    }

    void tile_engine::update()
    {
        std::vector<pending_gpu_upload> uploads;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            uploads.swap(m_pending_uploads);
        }

        for (auto& item : uploads)
        {
            ImTextureID tex_id = adam::imgui_tools::create_texture_rgba(item.width, item.height, item.pixels.data());

            std::lock_guard<std::mutex> lock(m_mutex);
            auto it = m_cache.find(item.key);
            if (it != m_cache.end())
            {
                it->second.texture_id = tex_id;
                it->second.width = item.width;
                it->second.height = item.height;
                it->second.is_loading = false;
                it->second.failed = (tex_id == (ImTextureID)0);
            }
        }
    }

    ImTextureID tile_engine::get_tile_texture(tile_provider_type provider, int z, int x, int y, float priority)
    {
        if (provider == tile_provider_type::vector_only)
        {
            return (ImTextureID)0;
        }

        char key_buf[128];
        std::snprintf(key_buf, sizeof(key_buf), "%d_%d_%d_%d", static_cast<int>(provider), z, x, y);
        std::string key = key_buf;

        std::lock_guard<std::mutex> lock(m_mutex);
        m_access_counter++;

        auto it = m_cache.find(key);
        if (it != m_cache.end())
        {
            it->second.last_accessed = m_access_counter;
            return it->second.texture_id;
        }

        // Add placeholder entry and enqueue async fetch with priority
        tile_texture_entry entry;
        entry.is_loading = true;
        entry.last_accessed = m_access_counter;
        m_cache[key] = entry;

        if (!m_in_fetch_queue[key])
        {
            m_in_fetch_queue[key] = true;
            m_fetch_requests.push_back({ key, provider, z, x, y, priority });
        }
        else
        {
            // Update priority if closer to center
            for (auto& req : m_fetch_requests)
            {
                if (req.key == key && priority < req.priority)
                {
                    req.priority = priority;
                    break;
                }
            }
        }

        return (ImTextureID)0;
    }

    void tile_engine::clear_cache()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& pair : m_cache)
        {
            if (pair.second.texture_id)
            {
                adam::imgui_tools::destroy_texture(pair.second.texture_id);
            }
        }
        m_cache.clear();
        m_in_fetch_queue.clear();
        m_fetch_requests.clear();
    }

    size_t tile_engine::get_loaded_texture_count() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        size_t count = 0;
        for (const auto& pair : m_cache)
        {
            if (pair.second.texture_id)
            {
                count++;
            }
        }
        return count;
    }

    size_t tile_engine::get_pending_request_count() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_fetch_requests.size();
    }

    void tile_engine::worker_loop()
    {
        while (m_running.load())
        {
            fetch_request req;
            bool has_work = false;

            {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (!m_fetch_requests.empty())
                {
                    // Sort by priority (descending so back element has smallest priority = closest to center)
                    std::sort(m_fetch_requests.begin(), m_fetch_requests.end(), [](const fetch_request& a, const fetch_request& b)
                    {
                        return a.priority > b.priority;
                    });

                    req = m_fetch_requests.back();
                    m_fetch_requests.pop_back();
                    has_work = true;
                }
            }

            if (!has_work)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(15));
                continue;
            }

            std::vector<uint8_t> pixels;
            int width = 0;
            int height = 0;

            bool success = fetch_and_decode_tile(req.provider, req.z, req.x, req.y, pixels, width, height);

            std::lock_guard<std::mutex> lock(m_mutex);
            m_in_fetch_queue.erase(req.key);

            if (success && !pixels.empty())
            {
                m_pending_uploads.push_back({ req.key, width, height, std::move(pixels) });
            }
            else
            {
                auto it = m_cache.find(req.key);
                if (it != m_cache.end())
                {
                    it->second.is_loading = false;
                    it->second.failed = true;
                }
            }
        }
    }

    bool tile_engine::fetch_and_decode_tile(tile_provider_type provider, int z, int x, int y, std::vector<uint8_t>& out_pixels, int& out_w, int& out_h)
    {
        std::string cache_path = build_tile_cache_path(provider, z, x, y);
        std::vector<uint8_t> file_bytes;

        // Step 1: Try reading local disk cache
        if (std::filesystem::exists(cache_path))
        {
            std::ifstream file(cache_path, std::ios::binary | std::ios::ate);
            if (file.is_open())
            {
                std::streamsize size = file.tellg();
                file.seekg(0, std::ios::beg);
                if (size > 0)
                {
                    file_bytes.resize(static_cast<size_t>(size));
                    file.read(reinterpret_cast<char*>(file_bytes.data()), size);
                }
                file.close();
            }
        }

        // Step 2: Download if missing from disk cache
        if (file_bytes.empty())
        {
            std::string url = build_tile_url(provider, z, x, y);
            tile_provider_info info = get_tile_provider_info(provider);

#if defined(ADAM_PLATFORM_WINDOWS)
            if (!http_download_tile(url, info.user_agent, file_bytes))
            {
                return false;
            }
#else
            return false;
#endif

            // Save downloaded bytes to disk cache
            if (!file_bytes.empty())
            {
                try
                {
                    std::filesystem::create_directories(std::filesystem::path(cache_path).parent_path());
                    std::ofstream out_file(cache_path, std::ios::binary);
                    if (out_file.is_open())
                    {
                        out_file.write(reinterpret_cast<const char*>(file_bytes.data()), file_bytes.size());
                        out_file.close();
                    }
                }
                catch (...)
                {
                }
            }
        }

        // Step 3: Decode image bytes to RGBA pixels
        if (file_bytes.empty())
        {
            return false;
        }

#if defined(ADAM_PLATFORM_WINDOWS)
        return decode_image_wic(file_bytes.data(), file_bytes.size(), out_pixels, out_w, out_h);
#else
        return false;
#endif
    }
}
