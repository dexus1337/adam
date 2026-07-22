#pragma once

/**
 * @file    cat034-uap.hpp
 * @author  dexus1337
 * @brief   Defines structs for ASTERIX CAT034
 * @version 1.0
 * @date    15.06.2026
 */

#include "data/asterix-uap.hpp"
#include "api/api-asterix.hpp"

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
}
