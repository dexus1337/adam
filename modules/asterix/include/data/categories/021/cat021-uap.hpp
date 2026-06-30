#pragma once

/**
 * @file    cat021-uap.hpp
 * @author  dexus1337
 * @brief   Defines the UAP for ASTERIX CAT021
 * @version 1.0
 * @date    30.06.2026
 */

#include "data/asterix-uap.hpp"
#include "api/api-asterix.hpp"

/** @brief ASTERIX CAT021 specification edition (Part 12, Edition 2.7). */
#define CAT021_VERSION "2.7"

namespace adam::modules::asterix::cat021
{
    /**
     * @brief Retrieves the standard UAP definition for Category 021.
     *
     * Implements EUROCONTROL Specification for Surveillance Data Exchange ASTERIX Part 12:
     * - Category 021: Edition 2.7 (ADS-B Target Reports)
     *
     * @return Reference to the CAT021 UAP structure.
     */
    ADAM_ASTERIX_API uap& get_uap();
}
