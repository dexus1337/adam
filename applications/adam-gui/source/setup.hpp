#pragma once

#include <SDL.h>

namespace adam::gui 
{
    /** @brief Initializes SDL2, OpenGL context, and Dear ImGui */
    bool initialize(SDL_Window*& window, SDL_GLContext& gl_context, const char*& glsl_version);

    /** @brief Cleans up and destroys contexts */
    void shutdown(SDL_Window* window, SDL_GLContext gl_context);
}