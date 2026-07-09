#include "setup.hpp"

#include <SDL3/SDL_opengl.h>
#include "main-window/main-window.hpp"
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#include <filesystem>
#include <cstdlib>
#include <cstdio>
#include <cstring>

namespace adam::gui 
{
    static float g_current_dpi_scale = 0.0f;

    float get_current_dpi_scale()
    {
        return g_current_dpi_scale;
    }

    void update_dpi_scale(SDL_Window* window)
    {
        if (!window) return;
        
        float new_dpi_scale = SDL_GetWindowDisplayScale(window);
        if (new_dpi_scale <= 0.0f) new_dpi_scale = 1.0f;

        if (new_dpi_scale == g_current_dpi_scale) 
            return; // No DPI change across monitors, skip rebuilding

        ImGuiStyle default_style;
        ImGuiStyle& style = ImGui::GetStyle();
        
        // Preserve colors from the current style
        for (int i = 0; i < ImGuiCol_COUNT; i++)
            default_style.Colors[i] = style.Colors[i];
            
        style = default_style;

        // Re-apply our custom style settings
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

        style.ScaleAllSizes(new_dpi_scale);

        g_current_dpi_scale = new_dpi_scale;
    }

    bool initialize(SDL_Window*& window, SDL_GLContext& gl_context, const char*& glsl_version)
    {
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
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
        
        SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
        window = SDL_CreateWindow("ADAM GUI", static_cast<int>(window_min_size[0]), static_cast<int>(window_min_size[1]), window_flags);
        if (!window) return false;

        gl_context = SDL_GL_CreateContext(window);
        if (!gl_context) return false;

        SDL_GL_MakeCurrent(window, gl_context);
        SDL_GL_SetSwapInterval(1); // Enable vsync

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        // Disable the creation of imgui.ini, as we handle configuration saving ourselves
        io.IniFilename = NULL;

        // Execute the first DPI evaluation and Font Atlas building
        update_dpi_scale(window);

        io.Fonts->Clear();

        ImFont* default_font = nullptr;

        // Load nicer system fonts instead of the default pixel font
        #if defined(ADAM_PLATFORM_WINDOWS)
        if (std::filesystem::exists("C:\\Windows\\Fonts\\tahoma.ttf"))
            default_font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\tahoma.ttf", 16.0f);
        else if (std::filesystem::exists("C:\\Windows\\Fonts\\segoeui.ttf"))
            default_font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 16.0f);
        #elif defined(ADAM_PLATFORM_LINUX)
        if (std::filesystem::exists("/usr/share/fonts/dejavu/DejaVuSans.ttf"))
            default_font = io.Fonts->AddFontFromFileTTF("/usr/share/fonts/dejavu/DejaVuSans.ttf", 16.0f);
        else if (std::filesystem::exists("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"))
            default_font = io.Fonts->AddFontFromFileTTF("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 16.0f);
        else if (std::filesystem::exists("/usr/share/fonts/liberation-sans/LiberationSans-Regular.ttf"))
            default_font = io.Fonts->AddFontFromFileTTF("/usr/share/fonts/liberation-sans/LiberationSans-Regular.ttf", 16.0f);
        else if (std::filesystem::exists("/usr/share/fonts/liberation/LiberationSans-Regular.ttf"))
            default_font = io.Fonts->AddFontFromFileTTF("/usr/share/fonts/liberation/LiberationSans-Regular.ttf", 16.0f);
        #elif defined(__APPLE__)
        if (std::filesystem::exists("/System/Library/Fonts/Helvetica.ttc"))
            default_font = io.Fonts->AddFontFromFileTTF("/System/Library/Fonts/Helvetica.ttc", 16.0f);
        #endif

        if (!default_font)
            default_font = io.Fonts->AddFontDefault();

        ImFontConfig mono_config;
        std::strncpy(mono_config.Name, "monospace", sizeof(mono_config.Name));
        mono_config.Name[sizeof(mono_config.Name) - 1] = '\0';

        // Load monospace fonts for hex viewer
        #if defined(ADAM_PLATFORM_WINDOWS)
        if (std::filesystem::exists("C:\\Windows\\Fonts\\consola.ttf")) 
            g_mono_font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", 16.0f, &mono_config);
        else if (std::filesystem::exists("C:\\Windows\\Fonts\\cour.ttf")) 
            g_mono_font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\cour.ttf", 16.0f, &mono_config);
        #elif defined(ADAM_PLATFORM_LINUX)
        if (std::filesystem::exists("/usr/share/fonts/dejavu/DejaVuSansMono.ttf"))
            g_mono_font = io.Fonts->AddFontFromFileTTF("/usr/share/fonts/dejavu/DejaVuSansMono.ttf", 16.0f, &mono_config);
        else if (std::filesystem::exists("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf"))
            g_mono_font = io.Fonts->AddFontFromFileTTF("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf", 16.0f, &mono_config);
        else if (std::filesystem::exists("/usr/share/fonts/liberation-mono/LiberationMono-Regular.ttf"))
            g_mono_font = io.Fonts->AddFontFromFileTTF("/usr/share/fonts/liberation-mono/LiberationMono-Regular.ttf", 16.0f, &mono_config);
        else if (std::filesystem::exists("/usr/share/fonts/liberation/LiberationMono-Regular.ttf"))
            g_mono_font = io.Fonts->AddFontFromFileTTF("/usr/share/fonts/liberation/LiberationMono-Regular.ttf", 16.0f, &mono_config);
        #elif defined(__APPLE__)
        if (std::filesystem::exists("/System/Library/Fonts/Menlo.ttc"))
            g_mono_font = io.Fonts->AddFontFromFileTTF("/System/Library/Fonts/Menlo.ttc", 16.0f, &mono_config);
        else if (std::filesystem::exists("/System/Library/Fonts/Monaco.ttf"))
            g_mono_font = io.Fonts->AddFontFromFileTTF("/System/Library/Fonts/Monaco.ttf", 16.0f, &mono_config);
        #endif

        io.FontDefault = default_font;

        ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
        ImGui_ImplOpenGL3_Init(glsl_version);

        return true;
    }

    void shutdown(SDL_Window* window, SDL_GLContext gl_context)
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();

        SDL_GL_DestroyContext(gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
}
