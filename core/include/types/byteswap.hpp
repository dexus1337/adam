#pragma once

/**
 * @file    byteswap.hpp
 * @author  dexus1337
 * @brief   Architecture-optimized byte swap utilities for values and unaligned memory loads (2 to 8 octets).
 * @version 1.0
 * @date    16.06.2026
 */

#include <cstdint>
#include <cstring>
#include <cstdlib>

namespace adam
{
    // ============================================================================================================== //
    // 2-byte (16-bit) byte swap
    // ============================================================================================================== //

    inline uint16_t swap_2(uint16_t val)
    {
        #if defined(_MSC_VER)
        return _byteswap_ushort(val);
        #elif defined(__GNUC__) || defined(__clang__)
        return __builtin_bswap16(val);
        #else
        return static_cast<uint16_t>((val >> 8) | (val << 8));
        #endif
    }

    inline uint16_t swap_2(const uint8_t* ptr)
    {
        uint16_t val = 0;
        std::memcpy(&val, ptr, sizeof(val));
        return swap_2(val);
    }

    // ============================================================================================================== //
    // 3-byte (24-bit) byte swap
    // ============================================================================================================== //

    inline uint32_t swap_3(uint32_t val)
    {
        return ((val & 0x000000FF) << 16) |
               (val & 0x0000FF00)         |
               ((val & 0x00FF0000) >> 16);
    }

    inline uint32_t swap_3(const uint8_t* ptr)
    {
        return (static_cast<uint32_t>(ptr[0]) << 16) |
               (static_cast<uint32_t>(ptr[1]) << 8)  |
               (static_cast<uint32_t>(ptr[2]));
    }

    // ============================================================================================================== //
    // 4-byte (32-bit) byte swap
    // ============================================================================================================== //

    inline uint32_t swap_4(uint32_t val)
    {
        #if defined(_MSC_VER)
        return _byteswap_ulong(val);
        #elif defined(__GNUC__) || defined(__clang__)
        return __builtin_bswap32(val);
        #else
        return ((val >> 24) & 0x000000FF) |
               ((val >> 8)  & 0x0000FF00) |
               ((val << 8)  & 0x00FF0000) |
               ((val << 24) & 0xFF000000);
        #endif
    }

    inline uint32_t swap_4(const uint8_t* ptr)
    {
        uint32_t val = 0;
        std::memcpy(&val, ptr, sizeof(val));
        return swap_4(val);
    }

    // ============================================================================================================== //
    // 5-byte (40-bit) byte swap
    // ============================================================================================================== //

    inline uint64_t swap_5(uint64_t val)
    {
        return ((val & 0x00000000000000FFULL) << 32) |
               ((val & 0x000000000000FF00ULL) << 16) |
               (val & 0x0000000000FF0000ULL)         |
               ((val & 0x00000000FF000000ULL) >> 16) |
               ((val & 0x000000FF00000000ULL) >> 32);
    }

    inline uint64_t swap_5(const uint8_t* ptr)
    {
        return (static_cast<uint64_t>(ptr[0]) << 32) |
               (static_cast<uint64_t>(ptr[1]) << 24) |
               (static_cast<uint64_t>(ptr[2]) << 16) |
               (static_cast<uint64_t>(ptr[3]) << 8)  |
               (static_cast<uint64_t>(ptr[4]));
    }

    // ============================================================================================================== //
    // 6-byte (48-bit) byte swap
    // ============================================================================================================== //

    inline uint64_t swap_6(uint64_t val)
    {
        return ((val & 0x00000000000000FFULL) << 40) |
               ((val & 0x000000000000FF00ULL) << 24) |
               ((val & 0x0000000000FF0000ULL) << 8)  |
               ((val & 0x00000000FF000000ULL) >> 8)  |
               ((val & 0x000000FF00000000ULL) >> 24) |
               ((val & 0x0000FF0000000000ULL) >> 40);
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

    // ============================================================================================================== //
    // 7-byte (56-bit) byte swap
    // ============================================================================================================== //

    inline uint64_t swap_7(uint64_t val)
    {
        return ((val & 0x00000000000000FFULL) << 48) |
               ((val & 0x000000000000FF00ULL) << 32) |
               ((val & 0x0000000000FF0000ULL) << 16) |
               (val & 0x00000000FF000000ULL)         |
               ((val & 0x000000FF00000000ULL) >> 16) |
               ((val & 0x0000FF0000000000ULL) >> 32) |
               ((val & 0x00FF000000000000ULL) >> 48);
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

    // ============================================================================================================== //
    // 8-byte (64-bit) byte swap
    // ============================================================================================================== //

    inline uint64_t swap_8(uint64_t val)
    {
        #if defined(_MSC_VER)
        return _byteswap_uint64(val);
        #elif defined(__GNUC__) || defined(__clang__)
        return __builtin_bswap64(val);
        #else
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

    inline uint64_t swap_8(const uint8_t* ptr)
    {
        uint64_t val = 0;
        std::memcpy(&val, ptr, sizeof(val));
        return swap_8(val);
    }
}
