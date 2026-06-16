#pragma once

/**
 * @file    asterix-parser.hpp
 * @author  dexus1337
 * @brief   Defines the parser for the asterix data format.
 * @version 1.0
 * @date    15.06.2026
 */

#include "api/api-asterix.hpp"
#include <adam-sdk.hpp>

namespace adam::modules::asterix
{
    /**
     * @class asterix_parser
     * @brief Parses raw asterix data and constructs an internal zero-copy tree.
     */
    class ADAM_ASTERIX_API asterix_parser : public adam::parser
    {
    public:
        asterix_parser() = default;
        ~asterix_parser() override = default;

        /**
         * @brief Parses the provided buffer containing raw asterix data.
         * @param buf The buffer containing raw asterix data.
         * @param internal_data The buffer allocated by the parser to hold the parsed tree.
         * @return True if parsing is successful, false otherwise.
         */
        bool parse(class adam::buffer* buf, class adam::buffer*& internal_data) override;
    };
}
