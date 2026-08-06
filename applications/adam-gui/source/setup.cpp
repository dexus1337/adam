#include "setup.hpp"

#include <SDL3/SDL_opengl.h>
#include "main-window/main-window.hpp"
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

#include <imgui-tools.hpp>

#if defined(ADAM_PLATFORM_WINDOWS)
#include <d3d11.h>
#include <dxgi.h>
#include <imgui_impl_dx11.h>
#endif

#include <filesystem>
#include <cstdlib>
#include <cstdio>
#include <cstring>

namespace adam::gui 
{
    static float g_current_dpi_scale = 0.0f;
    static renderer_context* g_active_renderer_ctx = nullptr;

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

        style.WindowMinSize = ImVec2(32.0f, 32.0f);

        style.WindowBorderSize = 1.0f;
        style.FrameBorderSize = 1.0f;
        style.PopupBorderSize = 1.0f;

        style.ScaleAllSizes(new_dpi_scale);

        g_current_dpi_scale = new_dpi_scale;
    }

    static void setup_fonts(SDL_Window* window)
    {
        ImGuiIO& io = ImGui::GetIO();
        update_dpi_scale(window);

        io.Fonts->Clear();

        ImFont* default_font = nullptr;

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

        if (std::filesystem::exists("font.ttf"))
            g_mono_font = io.Fonts->AddFontFromFileTTF("font.ttf", 16.0f);
        #if defined(ADAM_PLATFORM_WINDOWS)
        else if (std::filesystem::exists("C:\\Windows\\Fonts\\consola.ttf")) 
            g_mono_font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", 16.0f, &mono_config);
        else if (std::filesystem::exists("C:\\Windows\\Fonts\\cour.ttf")) 
            g_mono_font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\cour.ttf", 16.0f, &mono_config);
        #elif defined(ADAM_PLATFORM_LINUX)
        else if (std::filesystem::exists("/usr/share/fonts/dejavu/DejaVuSansMono.ttf"))
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
    }

#if defined(ADAM_PLATFORM_WINDOWS)
    static bool create_render_target_d3d11(renderer_context& ctx)
    {
        if (!ctx.d3d_device || !ctx.swap_chain) return false;

        ID3D11Device* pDevice = static_cast<ID3D11Device*>(ctx.d3d_device);
        IDXGISwapChain* pSwapChain = static_cast<IDXGISwapChain*>(ctx.swap_chain);

        ID3D11Texture2D* pBackBuffer = nullptr;
        HRESULT hr = pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        if (FAILED(hr) || !pBackBuffer) return false;

        ID3D11RenderTargetView* pRTV = nullptr;
        hr = pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pRTV);
        pBackBuffer->Release();

        if (FAILED(hr)) return false;

        ctx.main_render_target_view = pRTV;
        return true;
    }

    static void cleanup_render_target_d3d11(renderer_context& ctx)
    {
        if (ctx.main_render_target_view)
        {
            ID3D11RenderTargetView* pRTV = static_cast<ID3D11RenderTargetView*>(ctx.main_render_target_view);
            pRTV->Release();
            ctx.main_render_target_view = nullptr;
        }
    }

    static bool create_device_d3d11(renderer_context& ctx)
    {
        if (!ctx.window) return false;

        HWND hwnd = reinterpret_cast<HWND>(SDL_GetPointerProperty(SDL_GetWindowProperties(ctx.window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));
        if (!hwnd) return false;

        DXGI_SWAP_CHAIN_DESC sd;
        std::memset(&sd, 0, sizeof(sd));
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

        ID3D11Device* pd3dDevice = nullptr;
        ID3D11DeviceContext* pd3dDeviceContext = nullptr;
        IDXGISwapChain* pSwapChain = nullptr;

        HRESULT res = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            createDeviceFlags,
            featureLevelArray,
            2,
            D3D11_SDK_VERSION,
            &sd,
            &pSwapChain,
            &pd3dDevice,
            &featureLevel,
            &pd3dDeviceContext
        );

        if (FAILED(res)) return false;

        ctx.d3d_device = pd3dDevice;
        ctx.d3d_device_context = pd3dDeviceContext;
        ctx.swap_chain = pSwapChain;

        return create_render_target_d3d11(ctx);
    }

    static void cleanup_device_d3d11(renderer_context& ctx)
    {
        cleanup_render_target_d3d11(ctx);

        if (ctx.swap_chain)
        {
            static_cast<IDXGISwapChain*>(ctx.swap_chain)->Release();
            ctx.swap_chain = nullptr;
        }
        if (ctx.d3d_device_context)
        {
            static_cast<ID3D11DeviceContext*>(ctx.d3d_device_context)->Release();
            ctx.d3d_device_context = nullptr;
        }
        if (ctx.d3d_device)
        {
            static_cast<ID3D11Device*>(ctx.d3d_device)->Release();
            ctx.d3d_device = nullptr;
        }
    }
#endif

    #include <string>

    static void(*g_old_Platform_SetWindowTitle)(ImGuiViewport*, const char*) = nullptr;

    static void setup_viewport_hooks()
    {
        ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
        g_old_Platform_SetWindowTitle = platform_io.Platform_SetWindowTitle;
        if (g_old_Platform_SetWindowTitle)
        {
            platform_io.Platform_SetWindowTitle = [](ImGuiViewport* vp, const char* str)
            {
                std::string new_title = "ADAM - ";
                new_title += str;
                g_old_Platform_SetWindowTitle(vp, new_title.c_str());
            };
        }
    }

    bool initialize(renderer_context& ctx)
    {
        g_active_renderer_ctx = &ctx;

        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
            return false;

#if defined(ADAM_PLATFORM_WINDOWS)
        // Try DirectX 11 backend first on Windows
        SDL_WindowFlags win_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
        ctx.window = SDL_CreateWindow("ADAM GUI", static_cast<int>(window_min_size[0]), static_cast<int>(window_min_size[1]), win_flags);
        
        if (ctx.window && create_device_d3d11(ctx))
        {
            ctx.backend = gfx_backend::directx11;

            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
            io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
            io.ConfigViewportsNoDecoration = false;
            io.IniFilename = nullptr;

            setup_fonts(ctx.window);

            ImGui_ImplSDL3_InitForD3D(ctx.window);
            ImGui_ImplDX11_Init(static_cast<ID3D11Device*>(ctx.d3d_device), static_cast<ID3D11DeviceContext*>(ctx.d3d_device_context));

            setup_viewport_hooks();
            
            adam::imgui_tools::init_textures(true, ctx.d3d_device);
            return true;
        }

        // Cleanup temporary window if D3D11 initialization failed
        if (ctx.window)
        {
            cleanup_device_d3d11(ctx);
            SDL_DestroyWindow(ctx.window);
            ctx.window = nullptr;
        }
#endif

        // OpenGL 3 Fallback (Primary for Linux/macOS, fallback for Windows)
        #if defined(ADAM_PLATFORM_LINUX) || defined(ADAM_PLATFORM_WINDOWS)
        ctx.glsl_version = "#version 130";
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        #elif defined(__APPLE__)
        ctx.glsl_version = "#version 150";
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
        #else
        ctx.glsl_version = "#version 130";
        #endif

        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

        SDL_WindowFlags gl_window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
        ctx.window = SDL_CreateWindow("ADAM GUI", static_cast<int>(window_min_size[0]), static_cast<int>(window_min_size[1]), gl_window_flags);
        if (!ctx.window) return false;

        ctx.gl_context = SDL_GL_CreateContext(ctx.window);
        if (!ctx.gl_context) return false;

        SDL_GL_MakeCurrent(ctx.window, ctx.gl_context);
        SDL_GL_SetSwapInterval(1);

        ctx.backend = gfx_backend::opengl3;

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        io.ConfigViewportsNoDecoration = false;
        io.IniFilename = nullptr;

        setup_fonts(ctx.window);

        ImGui_ImplSDL3_InitForOpenGL(ctx.window, ctx.gl_context);
        ImGui_ImplOpenGL3_Init(ctx.glsl_version);

        setup_viewport_hooks();
        
        adam::imgui_tools::init_textures(false, nullptr);
        return true;
    }

    void new_frame(renderer_context& ctx)
    {
        if (ctx.backend == gfx_backend::directx11)
        {
#if defined(ADAM_PLATFORM_WINDOWS)
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();
#endif
        }
        else
        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();
        }
    }

    void render_frame(renderer_context& ctx, bool vsync_enabled)
    {
        ImGui::Render();
        ImGuiIO& io = ImGui::GetIO();

        if (ctx.backend == gfx_backend::directx11)
        {
#if defined(ADAM_PLATFORM_WINDOWS)
            ID3D11DeviceContext* pContext = static_cast<ID3D11DeviceContext*>(ctx.d3d_device_context);
            ID3D11RenderTargetView* pRTV = static_cast<ID3D11RenderTargetView*>(ctx.main_render_target_view);
            IDXGISwapChain* pSwapChain = static_cast<IDXGISwapChain*>(ctx.swap_chain);

            const float clear_color[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
            pContext->OMSetRenderTargets(1, &pRTV, nullptr);
            pContext->ClearRenderTargetView(pRTV, clear_color);
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
            }

            pSwapChain->Present(vsync_enabled ? 1 : 0, 0);
#endif
        }
        else
        {
            glViewport(0, 0, static_cast<int>(io.DisplaySize.x * io.DisplayFramebufferScale.x), static_cast<int>(io.DisplaySize.y * io.DisplayFramebufferScale.y));
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
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

    void handle_resize(renderer_context& ctx)
    {
        if (ctx.backend == gfx_backend::directx11)
        {
#if defined(ADAM_PLATFORM_WINDOWS)
            if (!ctx.swap_chain) return;

            cleanup_render_target_d3d11(ctx);
            static_cast<IDXGISwapChain*>(ctx.swap_chain)->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
            create_render_target_d3d11(ctx);
#endif
        }
    }

    void shutdown(renderer_context& ctx)
    {
        if (ctx.backend == gfx_backend::directx11)
        {
#if defined(ADAM_PLATFORM_WINDOWS)
            ImGui_ImplDX11_Shutdown();
            ImGui_ImplSDL3_Shutdown();
            ImGui::DestroyContext();

            cleanup_device_d3d11(ctx);
            if (ctx.window)
            {
                SDL_DestroyWindow(ctx.window);
                ctx.window = nullptr;
            }
#endif
        }
        else
        {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplSDL3_Shutdown();
            ImGui::DestroyContext();

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
        }

        g_active_renderer_ctx = nullptr;
        SDL_Quit();
    }

}
