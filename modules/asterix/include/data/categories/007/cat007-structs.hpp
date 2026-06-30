#pragma once

/**
 * @file    cat007-structs.hpp
 * @author  dexus1337
 * @brief   Defines structs for ASTERIX CAT007
 * @version 1.0
 * @date    30.06.2026
 */

#include "data/asterix-uap.hpp"
#include "api/api-asterix.hpp"

namespace adam::modules::asterix::cat007
{
    /**
     * @brief CAT007 Directed Interrogation Message Types as encoded in I007/410.
     * 
     * Message types 0–4 use the "Downlink" UAP.
     * Message types 5–8 use the "Uplink" UAP.
     */
    enum message_type : uint8_t
    {
        message_type_acknowledge        = 0,    ///< DI Acknowledge (Downlink)
        message_type_reject             = 1,    ///< DI Reject (Downlink)
        message_type_finished           = 2,    ///< DI Finished (Downlink)
        message_type_completed          = 3,    ///< DI Completed (Downlink)
        message_type_target_report      = 4,    ///< Target Report (Downlink)
        message_type_request_a          = 5,    ///< DI Request Type A - Position (Uplink)
        message_type_request_b          = 6,    ///< DI Request Type B - Window (Uplink)
        message_type_request_c          = 7,    ///< DI Request Type C - Track-Number (Uplink)
        message_type_bds_request        = 8,    ///< Selective BDS-Register Request (Uplink)
    };

    /**
     * @brief Layout of I007/410 - Directed Interrogation Message Type.
     * One-octet fixed length item; bit 8..1 = message type.
     */
    struct directed_interrogation_msg_type
    {
        message_type msg_type : 8;
    };
}
