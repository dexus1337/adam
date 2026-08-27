#pragma once

/**
 * @file    cat034-structs.hpp
 * @author  dexus1337
 * @brief   Defines structs for ASTERIX CAT034
 * @version 1.0
 * @date    15.06.2026
 */

#include "api/api-asterix.hpp"
#include <types/byteswap.hpp>
#include <cstdint>

namespace adam::modules::asterix::cat034
{
    enum message_type : uint8_t
    {
        message_type_undefined          = 0,
        message_type_north_marker       = 1,
        message_type_sector_crossing    = 2,
        message_type_geo_filter         = 3,
        message_type_strobe             = 4,
        message_type_solar_storm        = 5,
        message_type_ssr_strobe         = 6,
        message_type_modes_strobe       = 7,
    };

    #pragma pack(push, 1)

    /**
     * @struct raw_antenna_rotation_period
     * @brief  Represents I034/041 Antenna Rotation Period (16-bit big-endian, LSB = 1/128 s).
     */
    struct raw_antenna_rotation_period
    {
        uint16_t period_raw;

        inline double get_period_s() const { return static_cast<double>(adam::swap_2(reinterpret_cast<const uint8_t*>(&period_raw))) / 128.0; }
    };

    /**
     * @struct raw_position_3d
     * @brief  Represents I034/120 3D-Position of Data Source (Height 2 bytes, Lat 3 bytes, Lon 3 bytes).
     */
    struct raw_position_3d
    {
        uint16_t height_raw;
        uint8_t  lat_bytes[3];
        uint8_t  lon_bytes[3];

        inline double get_height_m()  const { return static_cast<double>(static_cast<int16_t>(adam::swap_2(reinterpret_cast<const uint8_t*>(&height_raw)))) * 6.25; }
        inline double get_latitude()  const
        {
            int32_t raw_lat = (static_cast<int32_t>(lat_bytes[0]) << 24) |
                              (static_cast<int32_t>(lat_bytes[1]) << 16) |
                              (static_cast<int32_t>(lat_bytes[2]) << 8);
            raw_lat >>= 8;
            return static_cast<double>(raw_lat) * (180.0 / 8388608.0);
        }
        inline double get_longitude() const
        {
            int32_t raw_lon = (static_cast<int32_t>(lon_bytes[0]) << 24) |
                              (static_cast<int32_t>(lon_bytes[1]) << 16) |
                              (static_cast<int32_t>(lon_bytes[2]) << 8);
            raw_lon >>= 8;
            return static_cast<double>(raw_lon) * (180.0 / 8388608.0);
        }
    };

    #pragma pack(pop)
}
