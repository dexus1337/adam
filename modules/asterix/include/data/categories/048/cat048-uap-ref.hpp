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

/** @brief ASTERIX CAT048 REF specification edition (Appendix A, December 2024). */
#define CAT048_REF_VERSION "1.13"

namespace adam::modules::asterix::cat048::ref
{
    /**
     * @brief Retrieves the standard UAP definition for Category 048 Reserved Expansion Field (REF).
     * 
     * Implements EUROCONTROL Specification for Surveillance Data Exchange ASTERIX Part 4:
     * - Appendix A (REF): Edition 1.13 (December 2024)
     * 
     * @return Reference to the CAT048 UAP structure.
     */
    ADAM_ASTERIX_API uap& get_uap();
}
