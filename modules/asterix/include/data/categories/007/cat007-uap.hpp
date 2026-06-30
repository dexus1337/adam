#pragma once

/**
 * @file    cat007-uap.hpp
 * @author  dexus1337
 * @brief   Defines the UAP for ASTERIX CAT007
 * @version 1.0
 * @date    30.06.2026
 */

#include "data/asterix-uap.hpp"
#include "api/api-asterix.hpp"

/** @brief ASTERIX CAT007 specification edition (Part 21, 01/07/2024). */
#define CAT007_VERSION "1.12"

namespace adam::modules::asterix::cat007
{
    /**
     * @brief Retrieves the standard UAP definition for Category 007.
     * 
     * Implements EUROCONTROL Specification for Surveillance Data Exchange ASTERIX Part 21:
     * - Category 007: Edition 1.12 (01/07/2024)
     * 
     * CAT007 uses two sub-UAPs selected by the Directed Interrogation Message Type (I007/410):
     * - "Downlink" UAP: applied to message types 0–4 (Acknowledge, Reject, Finished, Completed, Target Report)
     * - "Uplink"   UAP: applied to message types 5–8 (Interrogation Requests and BDS Register Request)
     * 
     * @return Reference to the CAT007 base UAP structure.
     */
    ADAM_ASTERIX_API uap& get_uap();
}
