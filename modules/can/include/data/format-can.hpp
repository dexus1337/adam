#pragma once

/**
 * @file    format-can.hpp
 * @author  dexus1337
 * @brief   Defines the can data format with its parsers and deserializers
 * @version 1.0
 * @date    25.04.2026
 */

 
#include "api/api-can.hpp"

#include <adam-sdk.hpp>

#include "module/module-can.hpp"


namespace adam::modules::can
{
    static const data_format data_format_can = data_format("can", nullptr, nullptr, get_adam_module());
}