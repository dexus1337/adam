#pragma once

/**
 * @file    cat048-structs.hpp
 * @author  dexus1337
 * @brief   Defines structs for ASTERIX CAT048
 * @version 1.0
 * @date    13.08.2026
 */

#include "api/api-asterix.hpp"

namespace adam::modules::asterix::cat048
{
    enum message_type : uint8_t
    {
        message_type_no_detection               = 0,
        message_type_single_psr                 = 1,
        message_type_single_ssr                 = 2,
        message_type_ssr_and_psr                = 3,
        message_type_single_mode_s_all_call     = 4,
        message_type_single_mode_s_roll_call    = 5,
        message_type_mode_s_all_call            = 6,
        message_type_mode_s_roll_call           = 7,
    };

    enum rab_type : uint8_t
    {
        rab_type_aircraft                       = 0,
        rab_type_field_monitor                  = 1
    };

    enum rdp_type : uint8_t
    {
        rdp_type_chain1                         = 0,
        rdp_type_chain2                         = 1
    };

    enum sim_type : uint8_t
    {
        sim_type_actual                         = 0,
        sim_type_simulated                      = 1
    };

    enum foe_type : uint8_t
    {
        foe_type_no_mode4_int                   = 0,
        foe_type_friendly                       = 1,
        foe_type_unknown                        = 2,
        foe_type_no_reply                       = 3
    };

    enum tst_type : uint8_t
    {
        tst_type_real                           = 0,
        tst_type_test                           = 1
    };

    struct target_report_descriptor
    {
        /* Byte 0 */
        bool                fx_0                    : 1; 
        rab_type            rab                     : 1;
        bool                has_spi                 : 1;
        rdp_type            rdp                     : 1;
        sim_type            sim                     : 1;
        message_type        msg_type                : 3;

        /* Byte 1 */
        bool                fx_1                    : 1; 
        foe_type            foe                     : 2;
        bool                is_military_emergency   : 1;
        bool                xpulse                  : 1;
        bool                extended_range          : 1;
        tst_type            tst                     : 1;
    };
}
