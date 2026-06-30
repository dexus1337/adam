#pragma once

/**
 * @file    cat001-uap.hpp
 * @author  dexus1337
 * @brief   Defines the UAP for ASTERIX CAT048
 * @version 1.0
 * @date    15.06.2026
 */

#include "data/asterix-uap.hpp"
#include "api/api-asterix.hpp"

/** @brief ASTERIX CAT048 specification edition (Part 4, July 2024). */
#define CAT048_VERSION "1.32"

namespace adam::modules::asterix::cat048
{
    /**
     * @brief Retrieves the standard UAP definition for Category 048.
     * 
     * Implements EUROCONTROL Specification for Surveillance Data Exchange ASTERIX Part 4:
     * - Category 048: Edition 1.32 (July 2024)
     * 
     * @return Reference to the CAT048 UAP structure.
     */
    ADAM_ASTERIX_API uap& get_uap();
}
