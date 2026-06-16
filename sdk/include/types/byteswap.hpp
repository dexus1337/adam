#pragma once

/**
 * @file    byteswap.hpp
 * @author  dexus1337
 * @brief   Architecture-optimized byte swap utilities for unaligned memory loads (2 to 8 octets).
 * @version 1.0
 * @date    16.06.2026
 */

#include <cstdint>
#include <cstring>
#include <cstdlib>

namespace adam
{
    // Architecture-optimized big-endian byte swap utilities for unaligned memory loads (2 to 8 octets)
    inline uint16_t swap_2(const uint8_t* ptr)
    {
        #if defined(_MSC_VER)
        return _byteswap_ushort(*reinterpret_cast<const uint16_t*>(ptr));
        #elif defined(__GNUC__) || defined(__clang__)
        return __builtin_bswap16(*reinterpret_cast<const uint16_t*>(ptr));
        #else
        uint16_t val = *reinterpret_cast<const uint16_t*>(ptr);
        return (val >> 8) | (val << 8);
        #endif
    }

    inline uint32_t swap_3(const uint8_t* ptr)
    {
        return (static_cast<uint32_t>(ptr[0]) << 16) |
               (static_cast<uint32_t>(ptr[1]) << 8)  |
               (static_cast<uint32_t>(ptr[2]));
    }

    inline uint32_t swap_4(const uint8_t* ptr)
    {
        #if defined(_MSC_VER)
        return _byteswap_ulong(*reinterpret_cast<const uint32_t*>(ptr));
        #elif defined(__GNUC__) || defined(__clang__)
        return __builtin_bswap32(*reinterpret_cast<const uint32_t*>(ptr));
        #else
        uint32_t val = *reinterpret_cast<const uint32_t*>(ptr);
        return ((val >> 24) & 0x000000FF) |
               ((val >> 8)  & 0x0000FF00) |
               ((val << 8)  & 0x00FF0000) |
               ((val << 24) & 0xFF000000);
        #endif
    }

    inline uint64_t swap_5(const uint8_t* ptr)
    {
        return (static_cast<uint64_t>(ptr[0]) << 32) |
               (static_cast<uint64_t>(ptr[1]) << 24) |
               (static_cast<uint64_t>(ptr[2]) << 16) |
               (static_cast<uint64_t>(ptr[3]) << 8)  |
               (static_cast<uint64_t>(ptr[4]));
    }

    inline uint64_t swap_6(const uint8_t* ptr)
    {
        return (static_cast<uint64_t>(ptr[0]) << 40) |
               (static_cast<uint64_t>(ptr[1]) << 32) |
               (static_cast<uint64_t>(ptr[2]) << 24) |
               (static_cast<uint64_t>(ptr[3]) << 16) |
               (static_cast<uint64_t>(ptr[4]) << 8)  |
               (static_cast<uint64_t>(ptr[5]));
    }

    inline uint64_t swap_7(const uint8_t* ptr)
    {
        return (static_cast<uint64_t>(ptr[0]) << 48) |
               (static_cast<uint64_t>(ptr[1]) << 40) |
               (static_cast<uint64_t>(ptr[2]) << 32) |
               (static_cast<uint64_t>(ptr[3]) << 24) |
               (static_cast<uint64_t>(ptr[4]) << 16) |
               (static_cast<uint64_t>(ptr[5]) << 8)  |
               (static_cast<uint64_t>(ptr[6]));
    }

    inline uint64_t swap_8(const uint8_t* ptr)
    {
        #if defined(_MSC_VER)
        return _byteswap_uint64(*reinterpret_cast<const uint64_t*>(ptr));
        #elif defined(__GNUC__) || defined(__clang__)
        return __builtin_bswap64(*reinterpret_cast<const uint64_t*>(ptr));
        #else
        uint64_t val = *reinterpret_cast<const uint64_t*>(ptr);
        return ((val >> 56) & 0x00000000000000FFull) |
               ((val >> 40) & 0x000000000000FF00ull) |
               ((val >> 24) & 0x0000000000FF0000ull) |
               ((val >> 8)  & 0x00000000FF000000ull) |
               ((val << 8)  & 0x000000FF00000000ull) |
               ((val << 24) & 0x0000FF0000000000ull) |
               ((val << 40) & 0x00FF000000000000ull) |
               ((val << 56) & 0xFF00000000000000ull);
        #endif
    }
}
