#include "setup.hpp"
#include "main-window/main-window.hpp"
#include "gui-controller.hpp"

#include <SDL.h>
#include <SDL_opengl.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>
#include <thread>
#include <chrono>

using namespace adam::string_hashed_ct_literals;

static ADAM_CONSTEXPR int event_redraw_count            = 3;
static ADAM_CONSTEXPR int perf_overlay_redraw_time      = 2000;

adam::gui::gui_controller gui_ctrl;
    
int main(int, char**) 
{
    SDL_Window* window = nullptr;
    SDL_GLContext gl_context = nullptr;
    const char* glsl_version = nullptr;

    if (!adam::gui::initialize(window, gl_context, glsl_version))
        return -1;

    gui_ctrl.set_redraw_callback([]() 
    {
        SDL_Event event;
        SDL_zero(event);
        event.type = SDL_USEREVENT;
        SDL_PushEvent(&event);
    });

    gui_ctrl.start();
    adam::gui::main_window ui_window(gui_ctrl, window);

    bool done = false;
    int frames_to_render = event_redraw_count;

    auto* p_immediate = dynamic_cast<adam::configuration_parameter_integer*>(gui_ctrl.get_parameters().get("gui_mode"_ct));
    auto* p_show_perf = dynamic_cast<adam::configuration_parameter_boolean*>(gui_ctrl.get_parameters().get("show_performance"_ct));

    while (!done)
    {
        SDL_Event event;
        bool needs_redraw = p_immediate->get_value() || (frames_to_render > 0);

        if (needs_redraw)
        {
            while (SDL_PollEvent(&event))
            {
                ImGui_ImplSDL2_ProcessEvent(&event);
                if (event.type == SDL_QUIT)
                    done = true;
                if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                    done = true;
                if (event.type == SDL_WINDOWEVENT && (event.window.event == SDL_WINDOWEVENT_DISPLAY_CHANGED || event.window.event == SDL_WINDOWEVENT_MOVED))
                {
                    adam::gui::update_dpi_scale(window);
                }

                frames_to_render = event_redraw_count;
            }
        }
        else
        {
            bool has_event = false;
            if (p_show_perf->get_value())
                has_event = SDL_WaitEventTimeout(&event, perf_overlay_redraw_time);
            else
                has_event = SDL_WaitEvent(&event);

            if (has_event)
            {
                frames_to_render = event_redraw_count;
                needs_redraw = true;
                do
                {
                    ImGui_ImplSDL2_ProcessEvent(&event);
                    if (event.type == SDL_QUIT)
                        done = true;
                    if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                        done = true;
                    if (event.type == SDL_WINDOWEVENT && (event.window.event == SDL_WINDOWEVENT_DISPLAY_CHANGED || event.window.event == SDL_WINDOWEVENT_MOVED))
                    {
                        adam::gui::update_dpi_scale(window);
                    }
                } while (SDL_PollEvent(&event));
            }
            else if (p_show_perf->get_value())
            {
                frames_to_render = 1;
                needs_redraw = true;
            }
        }

        if (!needs_redraw && !done)
            continue;

        if (frames_to_render > 0)
            frames_to_render--;

        // Render ImGui Frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ui_window.render();

        // Swap Buffers
        ImGui::Render();
        ImGuiIO& io = ImGui::GetIO();
        glViewport(0, 0, (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x), (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y));
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    ui_window.save_window_state();
    gui_ctrl.stop();
    adam::gui::shutdown(window, gl_context);

    return 0;
}
