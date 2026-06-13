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

#include "module/module-asterix.hpp"


namespace adam::modules::asterix
{
    static const data_format data_format_asterix = data_format("asterix", nullptr, nullptr, get_adam_module());
}