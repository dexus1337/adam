#pragma once

/**
 * @file    radar-stream.hpp
 * @author  dexus1337
 * @brief   Radar data stream model and telemetry structures for adam-cop
 * @version 1.0
 * @date    20.08.2026
 */

#include <adam-sdk.hpp>
#include <cstdint>
#include <string>
#include <functional>

namespace adam::cop
{
    struct radar_stream_stats
    {
        uint64_t msg_count = 0;
        uint64_t total_bytes = 0;
        uint64_t last_timestamp = 0;
        char last_preview_hex[64] = "";
    };

    struct radar_stream_key
    {
        adam::string_hash conn_hash;
        bool is_input;

        bool operator==(const radar_stream_key& other) const
        {
            return conn_hash == other.conn_hash && is_input == other.is_input;
        }
    };

    struct radar_stream_key_hash
    {
        size_t operator()(const radar_stream_key& k) const
        {
            return static_cast<size_t>(k.conn_hash ^ (k.is_input ? 0x9e3779b97f4a7c15ULL : 0x517cc1b727220a95ULL));
        }
    };

    inline void format_stream_bytes_to_buf(uint64_t bytes, char* out_buf, size_t buf_size)
    {
        if (!out_buf || buf_size == 0)
        {
            return;
        }

        if (bytes < 1024ULL)
        {
            snprintf(out_buf, buf_size, "%llu B", static_cast<unsigned long long>(bytes));
            return;
        }

        if (bytes < 1024ULL * 1024ULL)
        {
            snprintf(out_buf, buf_size, "%.1f KB", static_cast<double>(bytes) / 1024.0);
            return;
        }

        if (bytes < 1024ULL * 1024ULL * 1024ULL)
        {
            snprintf(out_buf, buf_size, "%.2f MB", static_cast<double>(bytes) / (1024.0 * 1024.0));
            return;
        }

        snprintf(out_buf, buf_size, "%.2f GB", static_cast<double>(bytes) / (1024.0 * 1024.0 * 1024.0));
    }

    inline void format_stream_preview_hex(const uint8_t* data, size_t size, char* out_buf, size_t buf_size)
    {
        if (!out_buf || buf_size == 0)
        {
            return;
        }

        if (!data || size == 0)
        {
            out_buf[0] = '\0';
            return;
        }

        size_t preview_len = size < 8 ? size : 8;
        size_t written = 0;

        for (size_t i = 0; i < preview_len; ++i)
        {
            if (written + 3 >= buf_size)
            {
                break;
            }

            int n = snprintf(out_buf + written, buf_size - written, "%02X ", data[i]);
            if (n < 0)
            {
                break;
            }
            written += static_cast<size_t>(n);
        }

        if (size > preview_len && written + 4 < buf_size)
        {
            snprintf(out_buf + written, buf_size - written, "...");
        }
    }
}
