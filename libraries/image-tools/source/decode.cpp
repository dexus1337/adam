#include "decode.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#include "stb_image.h"

namespace adam::image_tools
{
    bool decode_image(const uint8_t* encoded_data, size_t encoded_size, std::vector<uint8_t>& out_pixels, int& out_w, int& out_h)
    {
        if (!encoded_data || encoded_size == 0) return false;

        int channels = 0;
        uint8_t* decoded = stbi_load_from_memory(encoded_data, static_cast<int>(encoded_size), &out_w, &out_h, &channels, 4);
        
        if (!decoded)
        {
            return false;
        }

        size_t total_bytes = out_w * out_h * 4;
        out_pixels.assign(decoded, decoded + total_bytes);
        
        stbi_image_free(decoded);
        return true;
    }
}
