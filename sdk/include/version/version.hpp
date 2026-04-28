#pragma once

/**
 * @file    version.hpp
 * @author  dexus1337
 * @brief   Defines the sdk version and functions to decode/encode versions
 * @version 1.0
 * @date    25.04.2026
 */


#include <cstdint>


namespace adam 
{
    /**
     * @brief Version components: Major (8 bits), Minor (8 bits), Patch (8 bits)
     * Format: 0xMMmmPP
     */
    struct version_info 
    {
        uint8_t major;
        uint8_t minor;
        uint8_t patch;
    };

    // Extractors using 8-bit shifts
    static constexpr uint8_t get_major(uint32_t ver) { return static_cast<uint8_t>((ver >> 16) & 0xFF); }
    static constexpr uint8_t get_minor(uint32_t ver) { return static_cast<uint8_t>((ver >> 8) & 0xFF); }
    static constexpr uint8_t get_patch(uint32_t ver) { return static_cast<uint8_t>(ver & 0xFF); }

    // Helper to decode into a struct
    static constexpr version_info decode_version(uint32_t ver) 
    {
        return { get_major(ver), get_minor(ver), get_patch(ver) };
    }

    // Helper to encode (useful for internal logic or comparison)
    static constexpr uint32_t make_version(uint8_t maj, uint8_t min, uint8_t pat) 
    {
        return (static_cast<uint32_t>(maj) << 16) | 
               (static_cast<uint32_t>(min) << 8)  | 
                static_cast<uint32_t>(pat);
    }

    static constexpr uint32_t sdk_version = make_version(1, 0, 0);
}