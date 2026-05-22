#pragma once

#include <SDL.h>
#include <imgui.h>

namespace adam::gui 
{
    static ImVec2 window_min_size = ImVec2(1280, 720);

    /** @brief Initializes SDL2, OpenGL context, and Dear ImGui */
    bool initialize(SDL_Window*& window, SDL_GLContext& gl_context, const char*& glsl_version);

    /** @brief Cleans up and destroys contexts */
    void shutdown(SDL_Window* window, SDL_GLContext gl_context);

    /** @brief Dynamically updates the DPI scale and rebuilding fonts */
    void update_dpi_scale(SDL_Window* window, bool is_init = false);

    /** @brief Retrieves the currently evaluated OS DPI scale */
    float get_current_dpi_scale();
}
