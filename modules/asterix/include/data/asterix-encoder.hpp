#pragma once

/**
 * @file    asterix-encoder.hpp
 * @author  dexus1337
 * @brief   Defines the encoder for the asterix data format.
 * @version 1.0
 * @date    19.06.2026
 */

#include "api/api-asterix.hpp"
#include <adam-sdk.hpp>

namespace adam::modules::asterix
{
    /**
     * @class asterix_encoder
     * @brief Encodes parsed internal data tree back to raw big-endian asterix format.
     */
    class ADAM_ASTERIX_API asterix_encoder : public adam::encoder
    {
    public:
        asterix_encoder() = default;
        ~asterix_encoder() override = default;

        /**
         * @brief Encodes the parsed internal tree back into a raw asterix buffer.
         * @param buf The output buffer allocated by the encoder or referenced from internal_data.
         * @param internal_data The buffer containing the parsed tree.
         * @return True if encoding is successful, false otherwise.
         */
        bool encode(class adam::buffer*& buf, class adam::buffer* internal_data) override;
    };
}
