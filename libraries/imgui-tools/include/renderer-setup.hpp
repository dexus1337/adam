#pragma once

#include <SDL3/SDL.h>
#include <imgui.h>
#include <functional>

namespace adam::imgui_tools 
{
    enum class gfx_backend
    {
        directx11,
        opengl3
    };

    struct renderer_config
    {
        const char* window_title = "ADAM Application";
        ImVec2 window_min_size = ImVec2(1280, 720);
        bool enable_viewports = false;
        
        // Custom styling callback to override default ImGui Style
        std::function<void()> on_style_setup = nullptr;
    };

    struct renderer_context
    {
        gfx_backend backend = gfx_backend::opengl3;
        SDL_Window* window = nullptr;
        SDL_GLContext gl_context = nullptr;
        const char* glsl_version = nullptr;
        renderer_config config;

#if defined(ADAM_PLATFORM_WINDOWS)
        void* d3d_device = nullptr;                  // ID3D11Device*
        void* d3d_device_context = nullptr;          // ID3D11DeviceContext*
        void* swap_chain = nullptr;                  // IDXGISwapChain*
        void* main_render_target_view = nullptr;     // ID3D11RenderTargetView*
#endif
    };

    /** @brief Initializes SDL3, graphics context (DX11 or OpenGL3 fallback), and Dear ImGui */
    bool initialize(renderer_context& ctx, const renderer_config& config);

    /** @brief Cleans up and destroys contexts */
    void shutdown(renderer_context& ctx);

    /** @brief Prepares a new frame for ImGui and the active graphics backend */
    void new_frame(renderer_context& ctx);

    /** @brief Renders the current frame and handles buffer swap / presentation */
    void render_frame(renderer_context& ctx, bool vsync_enabled);

    /** @brief Resizes render target buffers when the window size changes */
    void handle_resize(renderer_context& ctx);

    /** @brief Dynamically updates the DPI scale and rebuilding fonts */
    void update_dpi_scale(SDL_Window* window, std::function<void()> style_cb = nullptr);

    /** @brief Retrieves the currently evaluated OS DPI scale */
    float get_current_dpi_scale();
}
