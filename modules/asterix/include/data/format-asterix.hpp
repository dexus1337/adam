#pragma once

/**
 * @file    format-asterix.hpp
 * @author  dexus1337
 * @brief   Defines the asterix format with its parser and encoder
 * @version 1.0
 * @date    10.06.2026
 */

 
#include "api/api-asterix.hpp"

#include <adam-sdk.hpp>

namespace adam::modules::asterix
{
    extern ADAM_ASTERIX_API  data_format data_format_asterix; // The Asterix data format, used for parsing and encoding Asterix messages in the ADAM system.
}