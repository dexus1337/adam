#pragma once

/**
 * @file    cat001-uap.hpp
 * @author  dexus1337
 * @brief   Defines the UAP for ASTERIX CAT062
 * @version 1.0
 * @date    15.06.2026
 */

#include "data/asterix-uap.hpp"
#include "api/api-asterix.hpp"

/** @brief ASTERIX CAT062 specification edition (Part 9, May 2025). */
#define CAT062_VERSION "1.21"

namespace adam::modules::asterix::cat062
{
    /**
     * @brief Retrieves the standard UAP definition for Category 062.
     * 
     * Implements EUROCONTROL Specification for Surveillance Data Exchange ASTERIX Part 9:
     * - Category 062: Edition 1.21 (May 2025)
     * 
     * @return Reference to the CAT062 UAP structure.
     */
    ADAM_ASTERIX_API uap& get_uap();
}
