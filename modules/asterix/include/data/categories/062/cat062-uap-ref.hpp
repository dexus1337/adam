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

/** @brief ASTERIX CAT062 REF specification edition (Appendix A, June 2025). */
#define CAT062_REF_VERSION "1.4"

namespace adam::modules::asterix::cat062::ref
{
    /**
     * @brief Retrieves the standard UAP definition for Category 062 Reserved Expansion Field (REF).
     * 
     * Implements EUROCONTROL Specification for Surveillance Data Exchange ASTERIX Part 9:
     * - Appendix A (REF): Edition 1.4 (June 2025)
     * 
     * @return Reference to the CAT062 UAP structure.
     */
    ADAM_ASTERIX_API uap& get_uap();
}
