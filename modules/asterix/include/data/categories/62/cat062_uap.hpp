#pragma once

#include "data/asterix-uap.hpp"
#include "api/api-asterix.hpp"

namespace adam::modules::asterix
{
    /**
     * @brief Retrieves the standard UAP definition for Category 062.
     * 
     * Implements EUROCONTROL Specification for Surveillance Data Exchange ASTERIX Part 9:
     * - Category 062: Edition 1.21 (May 2025)
     * - Appendix A (REF): Edition 1.4 (June 2025)
     * 
     * @return Reference to the CAT062 UAP structure.
     */
    ADAM_ASTERIX_API uap& get_cat062_uap();
}
