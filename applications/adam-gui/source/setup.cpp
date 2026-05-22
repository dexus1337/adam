#include "setup.hpp"

#include <SDL_opengl.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>
#include <filesystem>

namespace adam::gui 
{
    static float g_current_dpi_scale = 1.0f;

    float get_current_dpi_scale()
    {
        return g_current_dpi_scale;
    }

    void update_dpi_scale(SDL_Window* window, bool is_init)
    {
        // Calculate DPI scale (using standard 96 DPI as base)
        float ddpi = 96.0f, hdpi = 96.0f, vdpi = 96.0f;
        int display_index = SDL_GetWindowDisplayIndex(window);
        if (display_index >= 0)
        {
            if(SDL_GetDisplayDPI(display_index, &ddpi, &hdpi, &vdpi) != 0 )
                ddpi = 96.f;
        }
        float new_dpi_scale = ddpi / 96.0f;

        if (!is_init && new_dpi_scale == g_current_dpi_scale) 
            return; // No DPI change across monitors, skip rebuilding

        ImGuiIO& io = ImGui::GetIO();

        // 1. Scale all standard ImGui sizes relatively
        float relative_scale = new_dpi_scale / g_current_dpi_scale;
        ImGui::GetStyle().ScaleAllSizes(relative_scale);

        // 2. Scale font size dynamically without rebuilding the atlas
        if (!is_init) 
        {
            io.FontGlobalScale *= relative_scale;
        } 
        else 
        {
            io.Fonts->Clear();

            // Load nicer system fonts instead of the default pixel font
            #if defined(ADAM_PLATFORM_WINDOWS)
            if (std::filesystem::exists("C:\\Windows\\Fonts\\tahoma.ttf"))
                io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\tahoma.ttf", 16.0f * new_dpi_scale);
            else if (std::filesystem::exists("C:\\Windows\\Fonts\\segoeui.ttf"))
                io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 18.0f * new_dpi_scale);
            #elif defined(ADAM_PLATFORM_LINUX)
            if (std::filesystem::exists("/usr/share/fonts/dejavu/DejaVuSans.ttf"))
                io.Fonts->AddFontFromFileTTF("/usr/share/fonts/dejavu/DejaVuSans.ttf", 16.0f * new_dpi_scale);
            else if (std::filesystem::exists("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"))
                io.Fonts->AddFontFromFileTTF("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 16.0f * new_dpi_scale);
            else if (std::filesystem::exists("/usr/share/fonts/liberation-sans/LiberationSans-Regular.ttf"))
                io.Fonts->AddFontFromFileTTF("/usr/share/fonts/liberation-sans/LiberationSans-Regular.ttf", 16.0f * new_dpi_scale);
            else if (std::filesystem::exists("/usr/share/fonts/liberation/LiberationSans-Regular.ttf"))
                io.Fonts->AddFontFromFileTTF("/usr/share/fonts/liberation/LiberationSans-Regular.ttf", 16.0f * new_dpi_scale);
            #elif defined(__APPLE__)
            if (std::filesystem::exists("/System/Library/Fonts/Helvetica.ttc"))
                io.Fonts->AddFontFromFileTTF("/System/Library/Fonts/Helvetica.ttc", 16.0f * new_dpi_scale);
            #endif
        }

        g_current_dpi_scale = new_dpi_scale;
    }

    bool initialize(SDL_Window*& window, SDL_GLContext& gl_context, const char*& glsl_version)
    {
        // Tell Windows we handle DPI ourselves to prevent blurry upscaling
        #if defined(ADAM_PLATFORM_WINDOWS)
        SDL_SetHint(SDL_HINT_WINDOWS_DPI_AWARENESS, "permonitorv2");
        #endif

        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
            return false;

        #if defined(ADAM_PLATFORM_LINUX) || defined(ADAM_PLATFORM_WINDOWS)
        glsl_version = "#version 130";
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        #elif defined(__APPLE__)
        glsl_version = "#version 150";
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
        #else
        glsl_version = "#version 130";
        #endif

        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
        
        SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
        window = SDL_CreateWindow("ADAM GUI", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_min_size[0], window_min_size[1], window_flags);
        if (!window) return false;

        gl_context = SDL_GL_CreateContext(window);
        if (!gl_context) return false;

        SDL_GL_MakeCurrent(window, gl_context);
        //SDL_GL_SetSwapInterval(1); // Enable vsync

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        // Disable the creation of imgui.ini, as we handle configuration saving ourselves
        io.IniFilename = NULL;

        ImGuiStyle& style = ImGui::GetStyle();
        
        style.WindowRounding = 8.0f;
        style.FrameRounding = 6.0f;
        style.PopupRounding = 8.0f;
        style.ScrollbarRounding = 6.0f;
        style.GrabRounding = 6.0f;
        style.TabRounding = 6.0f;
        style.ChildRounding = 6.0f;

        style.WindowPadding = ImVec2(10.0f, 10.0f);
        style.FramePadding = ImVec2(8.0f, 4.0f);
        style.ItemSpacing = ImVec2(8.0f, 6.0f);
        style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);

        style.WindowMinSize = window_min_size;

        style.WindowBorderSize = 1.0f;
        style.FrameBorderSize = 1.0f;
        style.PopupBorderSize = 1.0f;
        
        // Execute the first DPI evaluation and Font Atlas building
        update_dpi_scale(window, true);

        ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
        ImGui_ImplOpenGL3_Init(glsl_version);

        return true;
    }

    void shutdown(SDL_Window* window, SDL_GLContext gl_context)
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();

        SDL_GL_DeleteContext(gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
}
