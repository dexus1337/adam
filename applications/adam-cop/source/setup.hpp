#pragma once

/**
 * @file    setup.hpp
 * @author  dexus1337
 * @brief   Renderer and window context initialization for adam-cop
 * @version 1.0
 * @date    05.08.2026
 */

#include <SDL3/SDL.h>
#include <imgui.h>

namespace adam::cop
{
    static ImVec2 window_min_size = ImVec2(1280, 720);

    enum class gfx_backend
    {
        directx11,
        opengl3
    };

    struct renderer_context
    {
        gfx_backend backend = gfx_backend::opengl3;
        SDL_Window* window = nullptr;
        SDL_GLContext gl_context = nullptr;
        const char* glsl_version = nullptr;

#if defined(ADAM_PLATFORM_WINDOWS)
        void* d3d_device = nullptr;                  // ID3D11Device*
        void* d3d_device_context = nullptr;          // ID3D11DeviceContext*
        void* swap_chain = nullptr;                  // IDXGISwapChain*
        void* main_render_target_view = nullptr;     // ID3D11RenderTargetView*
#endif
    };

    /** @brief Initializes SDL3, graphics context, and Dear ImGui */
    bool initialize(renderer_context& ctx);

    /** @brief Cleans up and destroys renderer contexts */
    void shutdown(renderer_context& ctx);

    /** @brief Prepares a new frame for ImGui and active backend */
    void new_frame(renderer_context& ctx);

    /** @brief Renders current frame and presents buffers */
    void render_frame(renderer_context& ctx, bool vsync_enabled);

    /** @brief Handles window resize events */
    void handle_resize(renderer_context& ctx);

    /** @brief Dynamically updates DPI scaling */
    void update_dpi_scale(SDL_Window* window);

    /** @brief Retrieves active DPI scale factor */
    float get_current_dpi_scale();

    /** @brief Creates a backend-agnostic ImGui texture from RGBA pixel buffer */
    ImTextureID create_texture_rgba(int width, int height, const uint8_t* pixels);

    /** @brief Destroys a created ImGui texture */
    void destroy_texture(ImTextureID texture_id);
}
