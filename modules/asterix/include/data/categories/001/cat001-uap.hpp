#pragma once

/**
 * @file    cat001-uap.hpp
 * @author  dexus1337
 * @brief   Defines the UAP for ASTERIX CAT001
 * @version 1.0
 * @date    15.06.2026
 */

#include "data/asterix-uap.hpp"
#include "api/api-asterix.hpp"

namespace adam::modules::asterix::cat001
{
    /**
     * @brief Retrieves the standard UAP definition for Category 001.
     * 
     * Implements EUROCONTROL Specification for Surveillance Data Exchange ASTERIX Part 2a:
     * - Category 001: Edition 1.4 (August 2022)
     * 
     * @return Reference to the CAT001 UAP structure.
     */
    ADAM_ASTERIX_API uap& get_uap();
}
