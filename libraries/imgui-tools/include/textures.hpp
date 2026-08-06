#pragma once

#include <cstdint>
#include <imgui.h>

namespace adam::imgui_tools
{
    /** @brief Initializes the texture manager with the active graphics backend */
    void init_textures(bool is_directx11, void* d3d_device = nullptr);

    /** @brief Creates a backend-agnostic ImGui texture (D3D11 ShaderResourceView or OpenGL Texture) from RGBA pixel buffer */
    ImTextureID create_texture_rgba(int width, int height, const uint8_t* pixels);

    /** @brief Destroys a texture created with create_texture_rgba */
    void destroy_texture(ImTextureID texture_id);
}
