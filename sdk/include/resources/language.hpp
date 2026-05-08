#pragma once

/**
 * @file    language.hpp
 * @author  dexus1337
 * @brief   Defines enums and structs related to language
 * @version 1.0
 * @date    08.05.2026
 */


#include <string>

#include "controller/controller.hpp"
#include "controller/response.hpp"


namespace adam
{
    enum language : uint8_t
    {
        language_english,
        language_german,

        languages_count
    };
}