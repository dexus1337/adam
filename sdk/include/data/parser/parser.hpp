#pragma once

/**
 * @file    parser.hpp
 * @author  dexus1337
 * @brief   Defines a base class for data format parsers, providing a common interface for parsing data in different formats used in the ADAM system.
 * @version 1.0
 * @date    25.04.2026
 */

 
#include "api/sdk-api.hpp"


namespace adam 
{
    /**
     * @class parser
     * @brief A base class for data format parsers, providing a common interface for parsing data in different formats used in the ADAM system.
     */
    class ADAM_SDK_API parser 
    {
    public:

        /** @brief Constructs a new parsers object. */
        parser();

        /** @brief Destroys the parser object and cleans up resources. */
        ~parser();

    };
}