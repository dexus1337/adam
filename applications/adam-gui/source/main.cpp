#include "setup.hpp"
#include "main-window/main-window.hpp"
#include "gui-controller.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#include <thread>
#include <chrono>

using namespace adam::string_hashed_ct_literals;

static ADAM_CONSTEXPR int event_redraw_count            = 3;
static ADAM_CONSTEXPR int perf_overlay_redraw_time      = 2000;

std::unique_ptr<adam::gui::gui_controller> g_gui_ctrl;
    
int main(int, char**) 
{
    g_gui_ctrl = std::make_unique<adam::gui::gui_controller>();
    auto& gui_ctrl = *g_gui_ctrl;

    SDL_Window* window = nullptr;
    SDL_GLContext gl_context = nullptr;
    const char* glsl_version = nullptr;

    if (!adam::gui::initialize(window, gl_context, glsl_version))
        return -1;

    gui_ctrl.set_redraw_callback([]() 
    {
        SDL_Event event;
        SDL_zerop(&event);
        event.type = SDL_EVENT_USER;
        SDL_PushEvent(&event);
    });

    gui_ctrl.start();
    adam::gui::main_window ui_window(gui_ctrl, window);

    bool done = false;
    int frames_to_render = event_redraw_count;

    auto* p_immediate = dynamic_cast<adam::configuration_parameter_integer*>(gui_ctrl.get_parameters().get("gui_mode"_ct));
    auto* p_show_perf = dynamic_cast<adam::configuration_parameter_boolean*>(gui_ctrl.get_parameters().get("show_performance"_ct));
    auto* p_fps_limit = dynamic_cast<adam::configuration_parameter_integer*>(gui_ctrl.get_parameters().get("fps_limit"_ct));

    auto last_frame_time = std::chrono::steady_clock::now();

    while (!done)
    {
        SDL_Event event;
        bool needs_redraw = p_immediate->get_value() || (frames_to_render > 0);

        auto process_events = [&]()
        { 
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
                done = true;
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
                done = true;
            if (event.type == SDL_EVENT_WINDOW_MOVED || event.type == SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED)
            {
                adam::gui::update_dpi_scale(window);
            }
            if (event.type == SDL_EVENT_WINDOW_MOVED || event.type == SDL_EVENT_WINDOW_RESIZED || event.type == SDL_EVENT_WINDOW_MAXIMIZED || event.type == SDL_EVENT_WINDOW_RESTORED)
            {
                ui_window.save_window_state();
            }
        };

        if (needs_redraw)
        {
            while (SDL_PollEvent(&event))
            {
                process_events();

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
                    process_events();
                } 
                while (SDL_PollEvent(&event));
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
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        ui_window.draw();

        // Swap Buffers
        ImGui::Render();
        ImGuiIO& io = ImGui::GetIO();
        glViewport(0, 0, (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x), (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y));
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);

        if (p_immediate->get_value() == 1)
        {
            int limit_setting = p_fps_limit ? static_cast<int>(p_fps_limit->get_value()) : 4;
            double target_fps = 0.0;
            switch(limit_setting)
            {
                case 0: target_fps = 10.0; break;
                case 1: target_fps = 30.0; break;
                case 2: target_fps = 60.0; break;
                case 3: target_fps = 120.0; break;
            }

            if (target_fps > 0.0)
            {
                auto target_frame_duration = std::chrono::duration<double>(1.0 / target_fps);
                auto next_frame_time = last_frame_time + std::chrono::duration_cast<std::chrono::steady_clock::duration>(target_frame_duration);
                std::this_thread::sleep_until(next_frame_time);

                auto now = std::chrono::steady_clock::now();
                if (now - last_frame_time > target_frame_duration * 2.0)
                {
                    last_frame_time = now;
                }
                else
                {
                    last_frame_time = next_frame_time;
                }
            }
            else
            {
                last_frame_time = std::chrono::steady_clock::now();
            }
        }
        else
        {
            last_frame_time = std::chrono::steady_clock::now();
        }
    }

    ui_window.save_window_state();
    gui_ctrl.stop();
    adam::gui::shutdown(window, gl_context);

    return 0;
}
