#pragma once

/**
 * @file    data-format-asterix.hpp
 * @author  dexus1337
 * @brief   Defines the asterix data format with its parsers and deserializers
 * @version 1.0
 * @date    10.06.2026
 */

 
#include "api/api-asterix.hpp"

#include <adam-sdk.hpp>


namespace adam::modules::asterix
{
    static const data_format data_format_asterix = data_format( "asterix" );
}