#include "imgui-tools.hpp"

#if defined(ADAM_PLATFORM_WINDOWS)
#include <d3d11.h>
#endif

#if defined(__APPLE__)
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

namespace adam::imgui_tools
{
    static bool g_is_directx11 = false;
    static void* g_d3d_device = nullptr;

    void init_textures(bool is_directx11, void* d3d_device)
    {
        g_is_directx11 = is_directx11;
        g_d3d_device = d3d_device;
    }

    ImTextureID create_texture_rgba(int width, int height, const uint8_t* pixels)
    {
        if (width <= 0 || height <= 0 || !pixels)
        {
            return (ImTextureID)0;
        }

#if defined(ADAM_PLATFORM_WINDOWS)
        if (g_is_directx11)
        {
            ID3D11Device* pDevice = static_cast<ID3D11Device*>(g_d3d_device);
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
        if (g_is_directx11)
        {
            ID3D11ShaderResourceView* pSRV = reinterpret_cast<ID3D11ShaderResourceView*>(texture_id);
            if (pSRV)
            {
                pSRV->Release();
            }
            return;
        }
#endif

        GLuint tex = static_cast<GLuint>((uintptr_t)texture_id);
        glDeleteTextures(1, &tex);
    }
}
