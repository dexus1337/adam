#pragma once

/**
 * @file    cat062-structs.hpp
 * @author  dexus1337
 * @brief   Defines structs for ASTERIX CAT062
 * @version 1.0
 * @date    13.08.2026
 */

#include "api/api-asterix.hpp"

namespace adam::modules::asterix::cat062
{
    /**
     * @brief This is not a real message type designed by the CAT, but rather by me for displaying basic information
     */
    enum message_type : uint8_t
    {
        message_type_multisensor_confirmed  = 0,
        message_type_monosensor_confirmed   = 1,
        message_type_mulisensor_tentative   = 2,
        message_type_monosensor_tentative   = 3,
    };

    enum mrh_type
    {
        mrh_type_barometric                 = 0,
        mrh_type_geometric                  = 1
    };

    enum altitude_source
    {
        altitude_source_none                = 0,
        altitude_source_gnss                = 1,
        altitude_source_3d_radar            = 2,
        altitude_source_triangulation       = 3,
        altitude_source_coverage            = 4,
        altitude_source_speed_lookup        = 5,
        altitude_source_default             = 6,
        altitude_source_multiateration      = 7
    };    
    
    struct track_status
    {
        /* Byte 0 */
        bool                fx_0                    : 1;
        bool                is_confirmed            : 1;
        altitude_source     alt_src                 : 3;
        mrh_type            mrh                     : 1;
        bool                spi                     : 1;
        bool                is_monosensor           : 1; 

        inline message_type get_message_type() const { return static_cast<message_type>((is_monosensor << 1) | (is_confirmed << 2)); }
    };
}
