#pragma once

#include <vector>
#include <cstdint>
#include <adam-sdk.hpp>

namespace adam::lib::image
{
    /**
     * @brief Decodes an image from memory.
     * @param encoded_data Pointer to the raw image bytes (e.g. PNG/JPEG).
     * @param encoded_size Size of the image bytes in memory.
     * @param out_pixels Output buffer populated with 32-bit RGBA pixel data.
     * @param out_w Width of the decoded image.
     * @param out_h Height of the decoded image.
     * @return true if successful, false otherwise.
     */
    bool decode_image(const uint8_t* encoded_data, size_t encoded_size, std::vector<uint8_t>& out_pixels, int& out_w, int& out_h);
}
