#pragma once

/**
 * @file    cat001_uap.hpp
 * @author  dexus1337
 * @brief   Defines structs for ASTERIX CAT001
 * @version 1.0
 * @date    15.06.2026
 */

#include "data/asterix-uap.hpp"
#include "api/api-asterix.hpp"

namespace adam::modules::asterix::cat001
{
    enum message_type : uint8_t
    {
        message_type_plot   = 0,
        message_type_track  = 1
    };

    enum report_type : uint8_t
    {
        report_type_aircraft    = 0,
        report_type_field       = 1
    };

    enum antenna_number : uint8_t
    {
        antenna_number_1        = 0,
        antenna_number_2        = 1
    };

    enum detection_type : uint8_t
    {
        detection_type_none     = 0,
        detection_type_psr      = 1,
        detection_type_ssr      = 2,
        detection_type_combined = (detection_type_psr | detection_type_ssr)
    };

    enum origin : uint8_t
    {
        origin_real             = 0,
        origin_sim              = 1
    };

    struct target_report_descriptor
    {
        bool            extended    : 1;
        report_type     rep_type    : 1;
        bool            special_pos : 1;
        antenna_number  antenna_num : 1;
        detection_type  detect_type : 2;
        origin          orig        : 1;
        message_type    msg_type    : 1;
    };
}
