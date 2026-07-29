#pragma once

/**
 * @file    cat021-uap-ref.hpp
 * @author  dexus1337
 * @brief   Defines the UAP for ASTERIX CAT021 Reserved Expansion Field (REF)
 * @version 1.0
 * @date    29.07.2026
 */

#include "data/asterix-uap.hpp"
#include "api/api-asterix.hpp"

/** @brief ASTERIX CAT021 REF specification edition (Appendix A, December 2021). */
#define CAT021_REF_VERSION "1.5"

namespace adam::modules::asterix::cat021::ref
{
    /**
     * @brief Retrieves the standard UAP definition for Category 021 Reserved Expansion Field (REF).
     * 
     * Implements EUROCONTROL Specification for Surveillance Data Exchange ASTERIX Part 12:
     * - Appendix A (REF): Edition 1.5 (December 2021)
     * 
     * @return Reference to the CAT021 UAP structure.
     */
    ADAM_ASTERIX_API uap& get_uap();
}
