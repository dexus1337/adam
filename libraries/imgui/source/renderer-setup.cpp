#include "renderer-setup.hpp"

#if defined(ADAM_PLATFORM_ANDROID) || defined(__ANDROID__)
#include <GLES3/gl3.h>
#include <SDL3/SDL_opengles2.h>
#else
#include <SDL3/SDL_opengl.h>
#endif
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

#if defined(ADAM_PLATFORM_WINDOWS)
#include <d3d11.h>
#include <dxgi.h>
#include <imgui_impl_dx11.h>
#endif

#include <filesystem>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <string>

#include "textures.hpp"

namespace adam::lib::imgui
{
    static void (*g_old_Platform_SetWindowTitle)(ImGuiViewport* vp, const char* str) = nullptr;

    static void setup_viewport_hooks()
    {
        ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
        g_old_Platform_SetWindowTitle = platform_io.Platform_SetWindowTitle;
        if (g_old_Platform_SetWindowTitle)
        {
            platform_io.Platform_SetWindowTitle = [](ImGuiViewport* vp, const char* str)
            {
                std::string new_title = "ADAM GUI - ";
                new_title += str;
                g_old_Platform_SetWindowTitle(vp, new_title.c_str());
            };
        }
    }

    static float g_current_dpi_scale = 0.0f;
    static renderer_context* g_active_renderer_ctx = nullptr;

    float get_current_dpi_scale()
    {
        return g_current_dpi_scale;
    }

    void update_dpi_scale(SDL_Window* window, std::function<void()> style_cb)
    {
        if (!window) return;
        
        float new_dpi_scale = SDL_GetWindowDisplayScale(window);
        if (new_dpi_scale <= 0.0f) new_dpi_scale = 1.0f;

        if (new_dpi_scale == g_current_dpi_scale) 
            return; // No DPI change

        ImGuiStyle default_style;
        ImGuiStyle& style = ImGui::GetStyle();
        
        for (int i = 0; i < ImGuiCol_COUNT; i++)
            default_style.Colors[i] = style.Colors[i];
            
        style = default_style;

        if (style_cb)
        {
            style_cb(); // Allow applications to inject custom styles
        }
        else
        {
            // Default Modern Dark Theme Styling
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

            style.WindowMinSize = ImVec2(32.0f, 32.0f);

            style.WindowBorderSize = 1.0f;
            style.FrameBorderSize = 1.0f;
            style.PopupBorderSize = 1.0f;
        }

        style.ScaleAllSizes(new_dpi_scale);
        g_current_dpi_scale = new_dpi_scale;
    }

    static void setup_fonts(SDL_Window* window, std::function<void()> style_cb)
    {
        ImGuiIO& io = ImGui::GetIO();
        update_dpi_scale(window, style_cb);

        io.Fonts->Clear();

        ImFont* default_font = nullptr;

        #if defined(ADAM_PLATFORM_WINDOWS)
        if (std::filesystem::exists("C:\\Windows\\Fonts\\tahoma.ttf"))
            default_font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\tahoma.ttf", 16.0f);
        else if (std::filesystem::exists("C:\\Windows\\Fonts\\segoeui.ttf"))
            default_font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 16.0f);
        #elif defined(ADAM_PLATFORM_ANDROID)
        if (std::filesystem::exists("/system/fonts/Roboto-Regular.ttf"))
            default_font = io.Fonts->AddFontFromFileTTF("/system/fonts/Roboto-Regular.ttf", 16.0f);
        else if (std::filesystem::exists("/system/fonts/DroidSans.ttf"))
            default_font = io.Fonts->AddFontFromFileTTF("/system/fonts/DroidSans.ttf", 16.0f);
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
        mono_config.GlyphOffset = ImVec2(0.0f, 1.5f);

        ImFont* mono_font = nullptr;

        if (std::filesystem::exists("font.ttf"))
            mono_font = io.Fonts->AddFontFromFileTTF("font.ttf", 16.0f, &mono_config);
        #if defined(ADAM_PLATFORM_WINDOWS)
        else if (std::filesystem::exists("C:\\Windows\\Fonts\\consola.ttf")) 
            mono_font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", 16.0f, &mono_config);
        else if (std::filesystem::exists("C:\\Windows\\Fonts\\cour.ttf")) 
            mono_font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\cour.ttf", 16.0f, &mono_config);
        #elif defined(ADAM_PLATFORM_ANDROID)
        else if (std::filesystem::exists("/system/fonts/DroidSansMono.ttf"))
            mono_font = io.Fonts->AddFontFromFileTTF("/system/fonts/DroidSansMono.ttf", 16.0f, &mono_config);
        else if (std::filesystem::exists("/system/fonts/CutiveMono.ttf"))
            mono_font = io.Fonts->AddFontFromFileTTF("/system/fonts/CutiveMono.ttf", 16.0f, &mono_config);
        #elif defined(ADAM_PLATFORM_LINUX)
        else if (std::filesystem::exists("/usr/share/fonts/dejavu/DejaVuSansMono.ttf"))
            mono_font = io.Fonts->AddFontFromFileTTF("/usr/share/fonts/dejavu/DejaVuSansMono.ttf", 16.0f, &mono_config);
        else if (std::filesystem::exists("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf"))
            mono_font = io.Fonts->AddFontFromFileTTF("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf", 16.0f, &mono_config);
        else if (std::filesystem::exists("/usr/share/fonts/liberation-mono/LiberationMono-Regular.ttf"))
            mono_font = io.Fonts->AddFontFromFileTTF("/usr/share/fonts/liberation-mono/LiberationMono-Regular.ttf", 16.0f, &mono_config);
        else if (std::filesystem::exists("/usr/share/fonts/liberation/LiberationMono-Regular.ttf"))
            mono_font = io.Fonts->AddFontFromFileTTF("/usr/share/fonts/liberation/LiberationMono-Regular.ttf", 16.0f, &mono_config);
        #elif defined(__APPLE__)
        if (std::filesystem::exists("/System/Library/Fonts/Menlo.ttc"))
            mono_font = io.Fonts->AddFontFromFileTTF("/System/Library/Fonts/Menlo.ttc", 16.0f, &mono_config);
        else if (std::filesystem::exists("/System/Library/Fonts/Monaco.ttf"))
            mono_font = io.Fonts->AddFontFromFileTTF("/System/Library/Fonts/Monaco.ttf", 16.0f, &mono_config);
        #endif

        if (!mono_font)
            mono_font = io.Fonts->AddFontDefault(&mono_config);

        io.FontDefault = default_font;

        if (io.Fonts->Fonts.Size > 1)
            io.Fonts->Fonts[1] = mono_font;
    }

    #if defined(ADAM_PLATFORM_WINDOWS)
    static bool init_d3d11(renderer_context& ctx)
    {
        HWND hwnd = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(ctx.window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
        if (!hwnd) return false;

        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = 2;
        sd.BufferDesc.Width = 0;
        sd.BufferDesc.Height = 0;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hwnd;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        UINT createDeviceFlags = 0;
        D3D_FEATURE_LEVEL featureLevel;
        const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };

        ID3D11Device* d3d_device = nullptr;
        ID3D11DeviceContext* d3d_context = nullptr;
        IDXGISwapChain* swap_chain = nullptr;

        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags,
            featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &swap_chain,
            &d3d_device, &featureLevel, &d3d_context
        );

        if (FAILED(hr)) return false;

        ctx.d3d_device = d3d_device;
        ctx.d3d_device_context = d3d_context;
        ctx.swap_chain = swap_chain;

        ID3D11Texture2D* pBackBuffer = nullptr;
        swap_chain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        if (pBackBuffer)
        {
            ID3D11RenderTargetView* main_rtv = nullptr;
            d3d_device->CreateRenderTargetView(pBackBuffer, NULL, &main_rtv);
            pBackBuffer->Release();
            ctx.main_render_target_view = main_rtv;
        }

        return true;
    }

    static void cleanup_d3d11(renderer_context& ctx)
    {
        if (ctx.main_render_target_view)
        {
            ((ID3D11RenderTargetView*)ctx.main_render_target_view)->Release();
            ctx.main_render_target_view = nullptr;
        }
        if (ctx.swap_chain)
        {
            ((IDXGISwapChain*)ctx.swap_chain)->Release();
            ctx.swap_chain = nullptr;
        }
        if (ctx.d3d_device_context)
        {
            ((ID3D11DeviceContext*)ctx.d3d_device_context)->Release();
            ctx.d3d_device_context = nullptr;
        }
        if (ctx.d3d_device)
        {
            ((ID3D11Device*)ctx.d3d_device)->Release();
            ctx.d3d_device = nullptr;
        }
    }
    #endif

    bool initialize(renderer_context& ctx, const renderer_config& config)
    {
        ctx.config = config;

        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
        {
            return false;
        }

        float display_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
        if (display_scale <= 0.0f)
        {
            display_scale = 1.0f;
        }

        int window_w = static_cast<int>(config.window_min_size.x * display_scale);
        int window_h = static_cast<int>(config.window_min_size.y * display_scale);

        SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY);

        #if defined(ADAM_PLATFORM_WINDOWS)
        ctx.backend = gfx_backend::directx11;
        ctx.window = SDL_CreateWindow(config.window_title, window_w, window_h, window_flags);
        if (!ctx.window || !init_d3d11(ctx))
        {
            if (ctx.window)
            {
                cleanup_d3d11(ctx);
                SDL_DestroyWindow(ctx.window);
                ctx.window = nullptr;
            }
            ctx.backend = gfx_backend::opengl3;
        }
        #endif

        if (ctx.backend == gfx_backend::opengl3)
        {
            #if defined(ADAM_PLATFORM_APPLE)
            ctx.glsl_version = "#version 150";
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
            #elif defined(ADAM_PLATFORM_ANDROID) || defined(__ANDROID__)
            ctx.glsl_version = "#version 300 es";
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
            #else
            ctx.glsl_version = "#version 130";
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
            #endif
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
            SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

            window_flags = (SDL_WindowFlags)(window_flags | SDL_WINDOW_OPENGL);
            ctx.window = SDL_CreateWindow(config.window_title, window_w, window_h, window_flags);
            if (!ctx.window) return false;

            ctx.gl_context = SDL_GL_CreateContext(ctx.window);
            if (!ctx.gl_context)
            {
                SDL_DestroyWindow(ctx.window);
                return false;
            }

            SDL_GL_MakeCurrent(ctx.window, ctx.gl_context);
            SDL_GL_SetSwapInterval(1);
        }

        SDL_SetWindowMinimumSize(ctx.window, static_cast<int>(config.window_min_size.x), static_cast<int>(config.window_min_size.y));
        SDL_SetWindowPosition(ctx.window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        SDL_ShowWindow(ctx.window);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        
        if (config.enable_viewports)
        {
            io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
            io.ConfigViewportsNoDecoration = false;
        }

        io.IniFilename = nullptr;

        setup_fonts(ctx.window, config.on_style_setup);

        if (ctx.backend == gfx_backend::directx11)
        {
            #if defined(ADAM_PLATFORM_WINDOWS)
            ImGui_ImplSDL3_InitForOther(ctx.window);
            ImGui_ImplDX11_Init((ID3D11Device*)ctx.d3d_device, (ID3D11DeviceContext*)ctx.d3d_device_context);
            #endif
        }
        else if (ctx.backend == gfx_backend::opengl3)
        {
            ImGui_ImplSDL3_InitForOpenGL(ctx.window, ctx.gl_context);
            ImGui_ImplOpenGL3_Init(ctx.glsl_version);
        }

        g_active_renderer_ctx = &ctx;

        if (config.enable_viewports)
        {
            setup_viewport_hooks();
        }

        #if defined(ADAM_PLATFORM_WINDOWS)
        adam::lib::imgui::init_textures(ctx.backend == gfx_backend::directx11, ctx.d3d_device);
        #else
        adam::lib::imgui::init_textures();
        #endif

        return true;
    }

    void shutdown(renderer_context& ctx)
    {
        if (ctx.backend == gfx_backend::directx11)
        {
            #if defined(ADAM_PLATFORM_WINDOWS)
            ImGui_ImplDX11_Shutdown();
            #endif
        }
        else if (ctx.backend == gfx_backend::opengl3)
        {
            ImGui_ImplOpenGL3_Shutdown();
        }

        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();

        #if defined(ADAM_PLATFORM_WINDOWS)
        if (ctx.backend == gfx_backend::directx11)
        {
            cleanup_d3d11(ctx);
        }
        #endif

        if (ctx.gl_context)
        {
            SDL_GL_DestroyContext(ctx.gl_context);
            ctx.gl_context = nullptr;
        }

        if (ctx.window)
        {
            SDL_DestroyWindow(ctx.window);
            ctx.window = nullptr;
        }

        SDL_Quit();
        g_active_renderer_ctx = nullptr;
    }

    void handle_resize(renderer_context& ctx)
    {
        if (!ctx.window) return;

        #if defined(ADAM_PLATFORM_WINDOWS)
        if (ctx.backend == gfx_backend::directx11 && ctx.swap_chain && ctx.d3d_device)
        {
            if (ctx.main_render_target_view)
            {
                ((ID3D11RenderTargetView*)ctx.main_render_target_view)->Release();
                ctx.main_render_target_view = nullptr;
            }
            ((IDXGISwapChain*)ctx.swap_chain)->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

            ID3D11Texture2D* pBackBuffer = nullptr;
            ((IDXGISwapChain*)ctx.swap_chain)->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
            if (pBackBuffer)
            {
                ID3D11RenderTargetView* main_rtv = nullptr;
                ((ID3D11Device*)ctx.d3d_device)->CreateRenderTargetView(pBackBuffer, NULL, &main_rtv);
                pBackBuffer->Release();
                ctx.main_render_target_view = main_rtv;
            }
        }
        #endif
    }

    void new_frame(renderer_context& ctx)
    {
        if (ctx.backend == gfx_backend::directx11)
        {
            #if defined(ADAM_PLATFORM_WINDOWS)
            ImGui_ImplDX11_NewFrame();
            #endif
        }
        else if (ctx.backend == gfx_backend::opengl3)
        {
            ImGui_ImplOpenGL3_NewFrame();
        }

        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
    }

    void render_frame(renderer_context& ctx, bool vsync_enabled)
    {
        ImGui::Render();
        ImGuiIO& io = ImGui::GetIO();

        if (ctx.backend == gfx_backend::directx11)
        {
            #if defined(ADAM_PLATFORM_WINDOWS)
            ImVec4 bg = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
            const float clear_color[4] = { bg.x, bg.y, bg.z, bg.w };
            auto* d3d_context = (ID3D11DeviceContext*)ctx.d3d_device_context;
            auto* main_rtv = (ID3D11RenderTargetView*)ctx.main_render_target_view;
            auto* swap_chain = (IDXGISwapChain*)ctx.swap_chain;

            if (d3d_context && main_rtv && swap_chain)
            {
                d3d_context->OMSetRenderTargets(1, &main_rtv, NULL);
                d3d_context->ClearRenderTargetView(main_rtv, clear_color);
                ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
                
                if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
                {
                    ImGui::UpdatePlatformWindows();
                    ImGui::RenderPlatformWindowsDefault();
                }

                swap_chain->Present(vsync_enabled ? 1 : 0, 0);
            }
            #else
            (void)vsync_enabled;
            #endif
        }
        else if (ctx.backend == gfx_backend::opengl3)
        {
            glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
            ImVec4 bg = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
            glClearColor(bg.x, bg.y, bg.z, bg.w);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
                SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
                SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
            }

            SDL_GL_SwapWindow(ctx.window);
        }
    }
}
