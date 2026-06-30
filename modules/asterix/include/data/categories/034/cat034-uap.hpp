#pragma once

/**
 * @file    cat034-uap.hpp
 * @author  dexus1337
 * @brief   Defines the UAP for ASTERIX CAT034
 * @version 1.0
 * @date    30.06.2026
 */

#include "data/asterix-uap.hpp"
#include "api/api-asterix.hpp"

/** @brief ASTERIX CAT034 specification edition (Part 2b, 01/12/2015). */
#define CAT034_VERSION "1.29"

namespace adam::modules::asterix::cat034
{
    /**
     * @brief Retrieves the standard UAP definition for Category 034.
     *
     * Implements EUROCONTROL Specification for Surveillance Data Exchange ASTERIX Part 2b:
     * - Category 034: Edition 1.29 (01/12/2015)
     *
     * CAT034 carries Monoradar Service Messages (north markers, sector crossings,
     * geographical filters, jamming strobes, solar storms). A single UAP is defined
     * for all message types — I034/000 (Message Type) at FRN 2 identifies the
     * specific message variant.
     *
     * @return Reference to the CAT034 UAP structure.
     */
    ADAM_ASTERIX_API uap& get_uap();
}
