/**
 * @file    setup.cpp
 * @author  dexus1337
 * @brief   Renderer context and ImGui setup implementation for adam-cop
 * @version 1.0
 * @date    05.08.2026
 */

#include "setup.hpp"

#include <SDL3/SDL_opengl.h>
#include <imgui.h>
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

namespace adam::cop
{
    static float g_current_dpi_scale = 0.0f;
    static renderer_context* g_active_renderer_ctx = nullptr;

    float get_current_dpi_scale()
    {
        return g_current_dpi_scale;
    }

    void update_dpi_scale(SDL_Window* window)
    {
        if (!window)
        {
            return;
        }

        float new_dpi_scale = SDL_GetWindowDisplayScale(window);
        if (new_dpi_scale <= 0.0f)
        {
            new_dpi_scale = 1.0f;
        }

        if (new_dpi_scale == g_current_dpi_scale)
        {
            return;
        }

        ImGuiStyle default_style;
        ImGuiStyle& style = ImGui::GetStyle();

        for (int i = 0; i < ImGuiCol_COUNT; i++)
        {
            default_style.Colors[i] = style.Colors[i];
        }

        style = default_style;

        // Tactical C2 Operator Theme Styling
        style.WindowRounding = 4.0f;
        style.FrameRounding = 3.0f;
        style.PopupRounding = 4.0f;
        style.ScrollbarRounding = 4.0f;
        style.GrabRounding = 3.0f;
        style.TabRounding = 4.0f;
        style.ChildRounding = 4.0f;

        style.WindowPadding = ImVec2(8.0f, 8.0f);
        style.FramePadding = ImVec2(6.0f, 4.0f);
        style.ItemSpacing = ImVec2(6.0f, 4.0f);
        style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);

        style.WindowMinSize = ImVec2(32.0f, 32.0f);

        style.WindowBorderSize = 1.0f;
        style.FrameBorderSize = 1.0f;
        style.PopupBorderSize = 1.0f;

        style.ScaleAllSizes(new_dpi_scale);

        g_current_dpi_scale = new_dpi_scale;
    }

    static void setup_c2_theme()
    {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        // Dark Tactical Operator Palette (Navy / Charcoal / Cyan accents)
        colors[ImGuiCol_Text]                  = ImVec4(0.90f, 0.94f, 0.98f, 1.00f);
        colors[ImGuiCol_TextDisabled]          = ImVec4(0.40f, 0.48f, 0.55f, 1.00f);
        colors[ImGuiCol_WindowBg]              = ImVec4(0.07f, 0.09f, 0.12f, 0.98f);
        colors[ImGuiCol_ChildBg]               = ImVec4(0.09f, 0.11f, 0.15f, 0.95f);
        colors[ImGuiCol_PopupBg]               = ImVec4(0.08f, 0.10f, 0.14f, 0.98f);
        colors[ImGuiCol_Border]                = ImVec4(0.18f, 0.25f, 0.32f, 0.70f);
        colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg]               = ImVec4(0.12f, 0.16f, 0.22f, 1.00f);
        colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.18f, 0.24f, 0.32f, 1.00f);
        colors[ImGuiCol_FrameBgActive]         = ImVec4(0.22f, 0.30f, 0.40f, 1.00f);
        colors[ImGuiCol_TitleBg]               = ImVec4(0.06f, 0.08f, 0.11f, 1.00f);
        colors[ImGuiCol_TitleBgActive]         = ImVec4(0.10f, 0.14f, 0.20f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.05f, 0.07f, 0.09f, 1.00f);
        colors[ImGuiCol_MenuBarBg]             = ImVec4(0.08f, 0.10f, 0.14f, 1.00f);
        colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.06f, 0.08f, 0.10f, 0.80f);
        colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.18f, 0.25f, 0.32f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.25f, 0.35f, 0.45f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.00f, 0.60f, 0.80f, 1.00f);
        colors[ImGuiCol_CheckMark]             = ImVec4(0.00f, 0.75f, 0.95f, 1.00f);
        colors[ImGuiCol_SliderGrab]            = ImVec4(0.00f, 0.65f, 0.85f, 1.00f);
        colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.00f, 0.85f, 1.00f, 1.00f);
        colors[ImGuiCol_Button]                = ImVec4(0.14f, 0.20f, 0.28f, 1.00f);
        colors[ImGuiCol_ButtonHovered]         = ImVec4(0.20f, 0.30f, 0.42f, 1.00f);
        colors[ImGuiCol_ButtonActive]          = ImVec4(0.00f, 0.55f, 0.75f, 1.00f);
        colors[ImGuiCol_Header]                = ImVec4(0.14f, 0.20f, 0.28f, 1.00f);
        colors[ImGuiCol_HeaderHovered]         = ImVec4(0.20f, 0.30f, 0.42f, 1.00f);
        colors[ImGuiCol_HeaderActive]          = ImVec4(0.00f, 0.55f, 0.75f, 1.00f);
        colors[ImGuiCol_Separator]             = ImVec4(0.18f, 0.25f, 0.32f, 0.70f);
        colors[ImGuiCol_SeparatorHovered]      = ImVec4(0.00f, 0.60f, 0.80f, 1.00f);
        colors[ImGuiCol_SeparatorActive]       = ImVec4(0.00f, 0.80f, 1.00f, 1.00f);
        colors[ImGuiCol_ResizeGrip]            = ImVec4(0.14f, 0.20f, 0.28f, 0.50f);
        colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.00f, 0.60f, 0.80f, 0.78f);
        colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.00f, 0.80f, 1.00f, 1.00f);
        colors[ImGuiCol_Tab]                   = ImVec4(0.10f, 0.14f, 0.20f, 1.00f);
        colors[ImGuiCol_TabHovered]            = ImVec4(0.20f, 0.30f, 0.42f, 1.00f);
        colors[ImGuiCol_TabActive]             = ImVec4(0.16f, 0.24f, 0.34f, 1.00f);
        colors[ImGuiCol_TabUnfocused]          = ImVec4(0.08f, 0.10f, 0.14f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive]    = ImVec4(0.12f, 0.16f, 0.22f, 1.00f);
        colors[ImGuiCol_DockingPreview]        = ImVec4(0.00f, 0.60f, 0.80f, 0.70f);
        colors[ImGuiCol_DockingEmptyBg]        = ImVec4(0.05f, 0.06f, 0.08f, 1.00f);
        colors[ImGuiCol_PlotLines]             = ImVec4(0.00f, 0.75f, 0.95f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered]      = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        colors[ImGuiCol_PlotHistogram]         = ImVec4(0.00f, 0.60f, 0.80f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(0.00f, 0.80f, 1.00f, 1.00f);
        colors[ImGuiCol_TableHeaderBg]         = ImVec4(0.10f, 0.14f, 0.20f, 1.00f);
        colors[ImGuiCol_TableBorderStrong]     = ImVec4(0.18f, 0.25f, 0.32f, 1.00f);
        colors[ImGuiCol_TableBorderLight]      = ImVec4(0.14f, 0.18f, 0.24f, 1.00f);
        colors[ImGuiCol_TableRowBg]            = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_TableRowBgAlt]         = ImVec4(1.00f, 1.00f, 1.00f, 0.02f);
        colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.00f, 0.50f, 0.75f, 0.35f);
        colors[ImGuiCol_DragDropTarget]        = ImVec4(1.00f, 0.80f, 0.00f, 0.90f);
        colors[ImGuiCol_NavHighlight]          = ImVec4(0.00f, 0.60f, 0.80f, 1.00f);
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.00f, 0.00f, 0.00f, 0.60f);
    }

    static void setup_fonts(SDL_Window* window)
    {
        ImGuiIO& io = ImGui::GetIO();
        update_dpi_scale(window);

        io.Fonts->Clear();

        ImFont* default_font = nullptr;

#if defined(ADAM_PLATFORM_WINDOWS)
        if (std::filesystem::exists("C:\\Windows\\Fonts\\tahoma.ttf"))
        {
            default_font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\tahoma.ttf", 16.0f);
        }
        else if (std::filesystem::exists("C:\\Windows\\Fonts\\segoeui.ttf"))
        {
            default_font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 16.0f);
        }
#elif defined(ADAM_PLATFORM_LINUX)
        if (std::filesystem::exists("/usr/share/fonts/dejavu/DejaVuSans.ttf"))
        {
            default_font = io.Fonts->AddFontFromFileTTF("/usr/share/fonts/dejavu/DejaVuSans.ttf", 16.0f);
        }
        else if (std::filesystem::exists("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"))
        {
            default_font = io.Fonts->AddFontFromFileTTF("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 16.0f);
        }
#endif

        if (!default_font)
        {
            io.Fonts->AddFontDefault();
        }
    }

#if defined(ADAM_PLATFORM_WINDOWS)
    static bool init_d3d11(renderer_context& ctx)
    {
        HWND hwnd = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(ctx.window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
        if (!hwnd)
        {
            return false;
        }

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

        if (FAILED(hr))
        {
            return false;
        }

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

    bool initialize(renderer_context& ctx)
    {
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
        {
            return false;
        }

        float display_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
        if (display_scale <= 0.0f)
        {
            display_scale = 1.0f;
        }

        int window_w = static_cast<int>(window_min_size.x * display_scale);
        int window_h = static_cast<int>(window_min_size.y * display_scale);

        SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY);

#if defined(ADAM_PLATFORM_WINDOWS)
        ctx.backend = gfx_backend::directx11;
        ctx.window = SDL_CreateWindow("adam-cop (Common Operational Picture)", window_w, window_h, window_flags);
        if (!ctx.window || !init_d3d11(ctx))
        {
            if (ctx.window)
            {
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
            ctx.window = SDL_CreateWindow("adam-cop (Common Operational Picture)", window_w, window_h, window_flags);
            if (!ctx.window)
            {
                return false;
            }

            ctx.gl_context = SDL_GL_CreateContext(ctx.window);
            if (!ctx.gl_context)
            {
                SDL_DestroyWindow(ctx.window);
                return false;
            }
            SDL_GL_MakeCurrent(ctx.window, ctx.gl_context);
            SDL_GL_SetSwapInterval(1);
        }

        SDL_SetWindowMinimumSize(ctx.window, static_cast<int>(window_min_size.x), static_cast<int>(window_min_size.y));
        SDL_SetWindowPosition(ctx.window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        SDL_ShowWindow(ctx.window);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        setup_c2_theme();
        setup_fonts(ctx.window);

        ImGui_ImplSDL3_InitForOther(ctx.window);

        if (ctx.backend == gfx_backend::directx11)
        {
#if defined(ADAM_PLATFORM_WINDOWS)
            ImGui_ImplDX11_Init((ID3D11Device*)ctx.d3d_device, (ID3D11DeviceContext*)ctx.d3d_device_context);
#endif
        }
        else if (ctx.backend == gfx_backend::opengl3)
        {
            ImGui_ImplSDL3_InitForOpenGL(ctx.window, ctx.gl_context);
            ImGui_ImplOpenGL3_Init(ctx.glsl_version);
        }

        g_active_renderer_ctx = &ctx;
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
        if (!ctx.window)
        {
            return;
        }

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

        if (ctx.backend == gfx_backend::directx11)
        {
#if defined(ADAM_PLATFORM_WINDOWS)
            const float clear_color[4] = { 0.07f, 0.09f, 0.12f, 1.00f };
            auto* d3d_context = (ID3D11DeviceContext*)ctx.d3d_device_context;
            auto* main_rtv = (ID3D11RenderTargetView*)ctx.main_render_target_view;
            auto* swap_chain = (IDXGISwapChain*)ctx.swap_chain;

            if (d3d_context && main_rtv && swap_chain)
            {
                d3d_context->OMSetRenderTargets(1, &main_rtv, NULL);
                d3d_context->ClearRenderTargetView(main_rtv, clear_color);
                ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
                swap_chain->Present(vsync_enabled ? 1 : 0, 0);
            }
#endif
        }
        else if (ctx.backend == gfx_backend::opengl3)
        {
            ImGuiIO& io = ImGui::GetIO();
            glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
            glClearColor(0.07f, 0.09f, 0.12f, 1.00f);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            SDL_GL_SwapWindow(ctx.window);
        }
    }

    ImTextureID create_texture_rgba(int width, int height, const uint8_t* pixels)
    {
        if (width <= 0 || height <= 0 || !pixels)
        {
            return (ImTextureID)0;
        }

#if defined(ADAM_PLATFORM_WINDOWS)
        if (g_active_renderer_ctx && g_active_renderer_ctx->backend == gfx_backend::directx11)
        {
            ID3D11Device* pDevice = static_cast<ID3D11Device*>(g_active_renderer_ctx->d3d_device);
            if (!pDevice)
            {
                return (ImTextureID)0;
            }

            D3D11_TEXTURE2D_DESC desc;
            std::memset(&desc, 0, sizeof(desc));
            desc.Width = width;
            desc.Height = height;
            desc.MipLevels = 1;
            desc.ArraySize = 1;
            desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            desc.SampleDesc.Count = 1;
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

            ID3D11Texture2D* pTexture = nullptr;
            D3D11_SUBRESOURCE_DATA initData;
            std::memset(&initData, 0, sizeof(initData));
            initData.pSysMem = pixels;
            initData.SysMemPitch = width * 4;

            HRESULT hr = pDevice->CreateTexture2D(&desc, &initData, &pTexture);
            if (FAILED(hr) || !pTexture)
            {
                return (ImTextureID)0;
            }

            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
            std::memset(&srvDesc, 0, sizeof(srvDesc));
            srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;

            ID3D11ShaderResourceView* pSRV = nullptr;
            hr = pDevice->CreateShaderResourceView(pTexture, &srvDesc, &pSRV);
            pTexture->Release();

            if (FAILED(hr))
            {
                return (ImTextureID)0;
            }

            return (ImTextureID)pSRV;
        }
#endif

        GLuint tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

        return (ImTextureID)(uintptr_t)tex;
    }

    void destroy_texture(ImTextureID texture_id)
    {
        if (!texture_id)
        {
            return;
        }

#if defined(ADAM_PLATFORM_WINDOWS)
        if (g_active_renderer_ctx && g_active_renderer_ctx->backend == gfx_backend::directx11)
        {
            ID3D11ShaderResourceView* pSRV = reinterpret_cast<ID3D11ShaderResourceView*>(texture_id);
            pSRV->Release();
            return;
        }
#endif

        GLuint tex = static_cast<GLuint>((uintptr_t)texture_id);
        glDeleteTextures(1, &tex);
    }
}
