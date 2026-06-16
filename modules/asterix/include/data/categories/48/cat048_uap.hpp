#pragma once

#include "data/asterix-uap.hpp"
#include "api/api-asterix.hpp"

namespace adam::modules::asterix
{
    /**
     * @brief Retrieves the standard UAP definition for Category 048.
     * 
     * Implements EUROCONTROL Specification for Surveillance Data Exchange ASTERIX Part 4:
     * - Category 048: Edition 1.32 (July 2024)
     * - Appendix A (REF): Edition 1.13 (December 2024)
     * 
     * @return Reference to the CAT048 UAP structure.
     */
    ADAM_ASTERIX_API uap& get_cat048_uap();
}
