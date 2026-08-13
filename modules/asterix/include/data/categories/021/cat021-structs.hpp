#pragma once

/**
 * @file    cat021-structs.hpp
 * @author  dexus1337
 * @brief   Defines structs for ASTERIX CAT021
 * @version 1.0
 * @date    13.08.2026
 */

#include "api/api-asterix.hpp"

namespace adam::modules::asterix::cat021
{
    enum address_type : uint8_t
    {
        address_type_type_icao       = 0,
        address_type_type_duplicate  = 1,
        address_type_type_surface    = 2,
        address_type_type_anonymous  = 3
    };
    
    enum arc_type : uint8_t
    {
        arc_type_25ft       = 0,
        arc_type_100ft      = 1,
        arc_type_unknown    = 2,
        arc_type_invalid    = 3
    };
    
    enum rab_type : uint8_t
    {
        rab_type_target_transponder = 0,
        rab_type_field_monitor      = 1,
    };
    
    struct target_report_descriptor
    {
        /* Byte 0 */    
        bool                fx_0    : 1;
        rab_type            rab     : 1;
        bool                rc      : 1;
        arc_type            arc     : 3;
        address_type        atp     : 3;
    };
}
