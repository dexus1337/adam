#include <renderer-setup.hpp>
#include "main-window/main-window.hpp"
#include "gui-controller.hpp"

#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <thread>
#include <chrono>

using namespace adam::string_hashed_ct_literals;

static ADAM_CONSTEXPR int event_redraw_count            = 3;
static ADAM_CONSTEXPR int perf_overlay_redraw_time      = 2000;

std::unique_ptr<adam::gui::gui_controller> g_gui_ctrl;
    
int main(int, char**) 
{
    g_gui_ctrl = std::make_unique<adam::gui::gui_controller>();

    adam::imgui_tools::renderer_context renderer_ctx;

    adam::imgui_tools::renderer_config config;
    config.window_title = "ADAM GUI";
    config.enable_viewports = true;

    if (!adam::imgui_tools::initialize(renderer_ctx, config))
        return -1;

    g_gui_ctrl->set_redraw_callback([]() 
    {
        SDL_Event event;
        SDL_zerop(&event);
        event.type = SDL_EVENT_USER;
        SDL_PushEvent(&event);
    });

    g_gui_ctrl->start();
    adam::gui::main_window ui_window(*g_gui_ctrl, renderer_ctx.window);

    bool done = false;
    int frames_to_render = event_redraw_count;

    auto* p_immediate = dynamic_cast<adam::configuration_parameter_integer*>(g_gui_ctrl->get_parameters().get("gui_mode"_ct));
    auto* p_show_perf = dynamic_cast<adam::configuration_parameter_boolean*>(g_gui_ctrl->get_parameters().get("show_performance"_ct));
    auto* p_fps_limit = dynamic_cast<adam::configuration_parameter_integer*>(g_gui_ctrl->get_parameters().get("fps_limit"_ct));

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
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(renderer_ctx.window))
                done = true;
            if (event.window.windowID == SDL_GetWindowID(renderer_ctx.window))
            {
                if (event.type == SDL_EVENT_WINDOW_MOVED || event.type == SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED)
                {
                    adam::imgui_tools::update_dpi_scale(renderer_ctx.window);
                }
                if (event.type == SDL_EVENT_WINDOW_RESIZED)
                {
                    adam::imgui_tools::handle_resize(renderer_ctx);
                }
                if (event.type == SDL_EVENT_WINDOW_MOVED || event.type == SDL_EVENT_WINDOW_RESIZED || event.type == SDL_EVENT_WINDOW_MAXIMIZED || event.type == SDL_EVENT_WINDOW_RESTORED)
                {
                    ui_window.save_window_state();
                }
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

        if (ImGui::IsAnyItemActive() || ImGui::IsAnyItemHovered() || ImGui::IsMouseDown(ImGuiMouseButton_Left) || ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {
            frames_to_render = event_redraw_count;
            needs_redraw = true;
        }

        if (!needs_redraw && !done)
            continue;

        if (frames_to_render > 0)
            frames_to_render--;

        // Render ImGui Frame
        adam::imgui_tools::new_frame(renderer_ctx);

        ui_window.draw();

        // Render & Present
        bool vsync_enabled = (p_immediate->get_value() == 1) ? (p_fps_limit && p_fps_limit->get_value() == 4) : true;
        adam::imgui_tools::render_frame(renderer_ctx, vsync_enabled);

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
    g_gui_ctrl->stop();
    adam::imgui_tools::shutdown(renderer_ctx);

    return 0;
}
