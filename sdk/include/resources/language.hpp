#pragma once

/**
 * @file    language.hpp
 * @author  dexus1337
 * @brief   Defines enums and structs related to language
 * @version 1.0
 * @date    08.05.2026
 */


#include <string>


namespace adam
{
    enum language : uint8_t
    {
        language_english    = 0,
        language_german     = 1,

        languages_count
    };

    struct language_info
    {
        language lang                = language_english;                                    /**< The current language of the controller. */
        uint32_t supported_languages = (1 << language_english) | (1 << language_german);    /**< Bitfield of supported languages, indexed by the language enum value. */
    };

}