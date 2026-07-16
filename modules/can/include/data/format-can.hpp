#pragma once

/**
 * @file    format-can.hpp
 * @author  dexus1337
 * @brief   Defines the can data format with its parser and encoder
 * @version 1.0
 * @date    25.04.2026
 */

 
#include "api/api-can.hpp"

#include <adam-sdk.hpp>

#include "module/module-can.hpp"


namespace adam::modules::can
{
    extern ADAM_CAN_API data_format data_format_can; // The CAN data format, used for parsing and encoding CAN messages in the ADAM system.
}