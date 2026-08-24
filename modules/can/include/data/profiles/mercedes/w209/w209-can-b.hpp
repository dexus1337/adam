#pragma once

/**
 * @file    w209-can-b.hpp
 * @author  dexus1337
 * @brief   Defines the Mercedes-Benz W209 Interior CAN-B profile.
 * @version 1.0
 * @date    24.08.2026
 */

#include "data/can-profile.hpp"

namespace adam::modules::can::profiles::mercedes::w209
{
    /**
     * @brief Gets the singleton instance of the Mercedes W209 CAN-B profile.
     * @return Reference to the CAN profile.
     */
    ADAM_CAN_API can_profile& get_profile();
}
