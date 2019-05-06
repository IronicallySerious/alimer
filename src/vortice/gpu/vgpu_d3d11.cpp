//
// Copyright (c) 2017-2019 Amer Koleci and contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#if defined(VGPU_D3D11)
#include "vgpu.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <d3d11.h>
#include <dxgi.h>
#include <assert.h>
#include <stdio.h>

#define SAFE_RELEASE(obj) if (obj) { obj->Release(); obj=nullptr; }

#if defined(_DEBUG)

#if !defined(_XBOX_ONE) || !defined(_TITLE)
#   pragma comment(lib,"dxguid.lib")
#endif

// Check for SDK Layer support.
inline bool _vgpuD3D11SdkLayersAvailable()
{
    HRESULT hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_NULL,       // There is no need to create a real hardware device.
        0,
        D3D11_CREATE_DEVICE_DEBUG,  // Check for the SDK layers.
        nullptr,                    // Any feature level will do.
        0,
        D3D11_SDK_VERSION,
        nullptr,                    // No need to keep the D3D device reference.
        nullptr,                    // No need to know the feature level.
        nullptr                     // No need to keep the D3D device context reference.
    );

    return SUCCEEDED(hr);
}
#endif

/* Handle declaration */
typedef struct VGpuTexture_T {
    VGpuTextureType         type;
    VGpuPixelFormat         format;
    uint32_t                width;
    uint32_t                height;
    uint32_t                depthOrArraySize;
    uint32_t                mipLevels;
    VgpuSampleCount         sampleCount;
    VGpuTextureUsageFlags   usage;
    bool                    externalHandle;
    DXGI_FORMAT             dxgi_format;
    ID3D11Resource*         d3d11_resource;
} VGpuTexture_T;

typedef struct VGpuFramebuffer_T {
    uint32_t                numColorAttachments;
    ID3D11RenderTargetView* d3d11_rtvs[VGPU_MAX_COLOR_ATTACHMENTS];
    ID3D11DepthStencilView* d3d11_dsv;
} VGpuFramebuffer_T;

typedef struct VGpuCommandBuffer_T {
    ID3D11DeviceContext*    context;
    bool                    recording;
} VGpuCommandBuffer_T;

struct {
    bool                    initialized = false;
    ID3D11Device*           device = nullptr;
    D3D_FEATURE_LEVEL       feature_level = D3D_FEATURE_LEVEL_9_1;
    ID3D11DeviceContext*    context = nullptr;
    IDXGISwapChain*         swapChain = nullptr;
    VGpuTexture             renderTarget = nullptr;
    VGpuTexture             depthStencilTexture = nullptr;
    VGpuFramebuffer         framebuffer = nullptr;
} _d3d11;

inline void _vgpuD3D11ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        // Set a breakpoint on this line to catch DirectX API errors
        //throw std::exception();
    }
}

static DXGI_FORMAT _vgpuD3D11GetFormat(VGpuPixelFormat format)
{
    static DXGI_FORMAT formats[VGPU_PIXEL_FORMAT_COUNT] = {
        DXGI_FORMAT_UNKNOWN,
        // 8-bit pixel formats
        DXGI_FORMAT_A8_UNORM,           // VGPU_PIXEL_FORMAT_A8_UNORM
        DXGI_FORMAT_R8_UNORM,           // VGPU_PIXEL_FORMAT_R8_UNORM
        DXGI_FORMAT_R8_SNORM,           // VGPU_PIXEL_FORMAT_R8_SNORM
        DXGI_FORMAT_R8_UINT,            // VGPU_PIXEL_FORMAT_R8_UINT
        DXGI_FORMAT_R8_SINT,            // VGPU_PIXEL_FORMAT_R8_SINT

        // 16-bit pixel formats
        DXGI_FORMAT_R16_UNORM,          // VGPU_PIXEL_FORMAT_R16_UNORM
        DXGI_FORMAT_R16_SNORM,          // VGPU_PIXEL_FORMAT_R16_SNORM
        DXGI_FORMAT_R16_UINT,           // VGPU_PIXEL_FORMAT_R16_UINT
        DXGI_FORMAT_R16_SINT,           // VGPU_PIXEL_FORMAT_R16_SINT
        DXGI_FORMAT_R16_FLOAT,          // VGPU_PIXEL_FORMAT_R16_FLOAT
        DXGI_FORMAT_R8G8_UNORM,         // VGPU_PIXEL_FORMAT_RG8_UNORM
        DXGI_FORMAT_R8G8_SNORM,         // VGPU_PIXEL_FORMAT_RG8_SNORM
        DXGI_FORMAT_R8G8_UINT,          // VGPU_PIXEL_FORMAT_RG8_UINT
        DXGI_FORMAT_R8G8_SINT,          // VGPU_PIXEL_FORMAT_RG8_SINT

        // Packed 16-bit pixel formats
        DXGI_FORMAT_B5G6R5_UNORM,       // VGPU_PIXEL_FORMAT_R5G6B5_UNORM
        DXGI_FORMAT_B4G4R4A4_UNORM,     // VGPU_PIXEL_FORMAT_RGBA4_UNORM

        // 32-bit pixel formats
        DXGI_FORMAT_R32_UINT,               // VGPU_PIXEL_FORMAT_R32_UINT,
        DXGI_FORMAT_R32_SINT,               // VGPU_PIXEL_FORMAT_R32_SINT,
        DXGI_FORMAT_R32_FLOAT,              // VGPU_PIXEL_FORMAT_R32_FLOAT
        DXGI_FORMAT_R16G16_UNORM,           // VGPU_PIXEL_FORMAT_RG16_UNORM
        DXGI_FORMAT_R16G16_SNORM,           // VGPU_PIXEL_FORMAT_RG16_SNORM
        DXGI_FORMAT_R16G16_UINT,            // VGPU_PIXEL_FORMAT_RG16_UINT
        DXGI_FORMAT_R16G16_SINT,            // VGPU_PIXEL_FORMAT_RG16_SINT
        DXGI_FORMAT_R16G16_FLOAT,           // VGPU_PIXEL_FORMAT_RG16_FLOAT
        DXGI_FORMAT_R8G8B8A8_UNORM,         // VGPU_PIXEL_FORMAT_RGBA8_UNORM
        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,    // VGPU_PIXEL_FORMAT_RGBA8_UNORM_SRGB
        DXGI_FORMAT_R8G8B8A8_SNORM,         // VGPU_PIXEL_FORMAT_RGBA8_SNORM
        DXGI_FORMAT_R8G8B8A8_UINT,          // VGPU_PIXEL_FORMAT_RGBA8_UINT
        DXGI_FORMAT_R8G8B8A8_SINT,          // VGPU_PIXEL_FORMAT_RGBA8_SINT
        DXGI_FORMAT_B8G8R8A8_UNORM,         // VGPU_PIXEL_FORMAT_BGRA8_UNORM
        DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,    // VGPU_PIXEL_FORMAT_BGRA8_UNORM_SRGB

        // Packed 32-Bit Pixel formats
        DXGI_FORMAT_R10G10B10A2_UNORM,      // VGPU_PIXEL_FORMAT_RGB10A2_UNORM
        DXGI_FORMAT_R10G10B10A2_UINT,       // VGPU_PIXEL_FORMAT_RGB10A2_UINT
        DXGI_FORMAT_R11G11B10_FLOAT,        // VGPU_PIXEL_FORMAT_RG11B10_FLOAT
        DXGI_FORMAT_R9G9B9E5_SHAREDEXP,     // VGPU_PIXEL_FORMAT_RGB9E5_FLOAT

        // 64-Bit Pixel Formats
        DXGI_FORMAT_R32G32_UINT,            // VGPU_PIXEL_FORMAT_RG32_UINT
        DXGI_FORMAT_R32G32_SINT,            // VGPU_PIXEL_FORMAT_RG32_SINT
        DXGI_FORMAT_R32G32_FLOAT,           // VGPU_PIXEL_FORMAT_RG32_FLOAT
        DXGI_FORMAT_R16G16B16A16_UNORM,     // VGPU_PIXEL_FORMAT_RGBA16_UNORM
        DXGI_FORMAT_R16G16B16A16_SNORM,     // VGPU_PIXEL_FORMAT_RGBA16_SNORM
        DXGI_FORMAT_R16G16B16A16_UINT,      // VGPU_PIXEL_FORMAT_RGBA16_UINT
        DXGI_FORMAT_R16G16B16A16_SINT,      // VGPU_PIXEL_FORMAT_RGBA16_SINT
        DXGI_FORMAT_R16G16B16A16_FLOAT,     // VGPU_PIXEL_FORMAT_RGBA16_FLOAT

        // 128-Bit Pixel Formats
        DXGI_FORMAT_R32G32B32A32_UINT,      // VGPU_PIXEL_FORMAT_RGBA32_UINT
        DXGI_FORMAT_R32G32B32A32_SINT,      // VGPU_PIXEL_FORMAT_RGBA32_SINT
        DXGI_FORMAT_R32G32B32A32_FLOAT,     // VGPU_PIXEL_FORMAT_RGBA32_FLOAT

        // Depth-stencil
        DXGI_FORMAT_D16_UNORM,              // VGPU_PIXEL_FORMAT_D16_UNORM
        DXGI_FORMAT_D32_FLOAT,              // VGPU_PIXEL_FORMAT_D32_FLOAT
        DXGI_FORMAT_D24_UNORM_S8_UINT,      // VGPU_PIXEL_FORMAT_D24_UNORM_S8_UINT
        DXGI_FORMAT_D32_FLOAT_S8X24_UINT,   // VGPU_PIXEL_FORMAT_D32_FLOAT_S8_UINT
        DXGI_FORMAT_D24_UNORM_S8_UINT,      // VGPU_PIXEL_FORMAT_S8

        // Compressed formats
        DXGI_FORMAT_BC1_UNORM,          // VGPU_PIXEL_FORMAT_BC1_UNORM,        
        DXGI_FORMAT_BC1_UNORM_SRGB,     // VGPU_PIXEL_FORMAT_BC1_UNORM_SRGB
        DXGI_FORMAT_BC2_UNORM,          // VGPU_PIXEL_FORMAT_BC2_UNORM
        DXGI_FORMAT_BC2_UNORM_SRGB,     // VGPU_PIXEL_FORMAT_BC2_UNORM_SRGB
        DXGI_FORMAT_BC3_UNORM,          // VGPU_PIXEL_FORMAT_BC3_UNORM
        DXGI_FORMAT_BC3_UNORM_SRGB,     // VGPU_PIXEL_FORMAT_BC3_UNORM_SRGB
        DXGI_FORMAT_BC4_UNORM,          // VGPU_PIXEL_FORMAT_BC4_UNORM     
        DXGI_FORMAT_BC4_SNORM,          // VGPU_PIXEL_FORMAT_BC4_SNORM    
        DXGI_FORMAT_BC5_UNORM,          // VGPU_PIXEL_FORMAT_BC5_UNORM    
        DXGI_FORMAT_BC5_SNORM,          // VGPU_PIXEL_FORMAT_BC5_SNORM    
        DXGI_FORMAT_BC6H_SF16,          // VGPU_PIXEL_FORMAT_BC6HS16
        DXGI_FORMAT_BC6H_UF16,          // VGPU_PIXEL_FORMAT_BC6HU16
        DXGI_FORMAT_BC7_UNORM,          // VGPU_PIXEL_FORMAT_BC7_UNORM
        DXGI_FORMAT_BC7_UNORM_SRGB,     // VGPU_PIXEL_FORMAT_BC7_UNORM_SRGB

        // Compressed PVRTC Pixel Formats
        DXGI_FORMAT_UNKNOWN,    // VGPU_PIXEL_FORMAT_PVRTC_RGB2
        DXGI_FORMAT_UNKNOWN,    // VGPU_PIXEL_FORMAT_PVRTC_RGBA2
        DXGI_FORMAT_UNKNOWN,    // VGPU_PIXEL_FORMAT_PVRTC_RGB4
        DXGI_FORMAT_UNKNOWN,    // VGPU_PIXEL_FORMAT_PVRTC_RGBA4

        // Compressed ETC Pixel Formats
        DXGI_FORMAT_UNKNOWN,    // VGPU_PIXEL_FORMAT_ETC2_RGB8
        DXGI_FORMAT_UNKNOWN,    // VGPU_PIXEL_FORMAT_ETC2_RGB8A1

        // Compressed ASTC Pixel Formats
        DXGI_FORMAT_UNKNOWN,    // VGPU_PIXEL_FORMAT_ASTC4x4,
        DXGI_FORMAT_UNKNOWN,    // VGPU_PIXEL_FORMAT_ASTC5x5
        DXGI_FORMAT_UNKNOWN,    // VGPU_PIXEL_FORMAT_ASTC6x6
        DXGI_FORMAT_UNKNOWN,    // VGPU_PIXEL_FORMAT_ASTC8x5
        DXGI_FORMAT_UNKNOWN,    // VGPU_PIXEL_FORMAT_ASTC8x6
        DXGI_FORMAT_UNKNOWN,    // VGPU_PIXEL_FORMAT_ASTC8x8
        DXGI_FORMAT_UNKNOWN,    // VGPU_PIXEL_FORMAT_ASTC10x10
        DXGI_FORMAT_UNKNOWN,    // VGPU_PIXEL_FORMAT_ASTC12x12
    };

    return formats[format];
}

static VGpuPixelFormat _vgpuD3D11GetPixelFormat(DXGI_FORMAT format)
{
    switch (format)
    {
    case DXGI_FORMAT_UNKNOWN:
        return VGPU_PIXEL_FORMAT_UNDEFINED;

        // 8-bit pixel formats
    case DXGI_FORMAT_A8_UNORM:
        return VGPU_PIXEL_FORMAT_A8_UNORM;

    case DXGI_FORMAT_R8_UNORM:
        return VGPU_PIXEL_FORMAT_R8_UNORM;

    case DXGI_FORMAT_R8_SNORM:
        return VGPU_PIXEL_FORMAT_R8_SNORM;

    case DXGI_FORMAT_R8_UINT:
        return VGPU_PIXEL_FORMAT_R8_UINT;

    case DXGI_FORMAT_R8_SINT:
        return VGPU_PIXEL_FORMAT_R8_SINT;

        // 16-bit pixel formats
    case DXGI_FORMAT_R16_UNORM:
        return VGPU_PIXEL_FORMAT_R16_UNORM;

    case DXGI_FORMAT_R16_SNORM:
        return VGPU_PIXEL_FORMAT_R16_SNORM;

    case DXGI_FORMAT_R16_UINT:
        return VGPU_PIXEL_FORMAT_R16_UINT;

    case DXGI_FORMAT_R16_SINT:
        return VGPU_PIXEL_FORMAT_R16_SINT;

    case DXGI_FORMAT_R16_FLOAT:
        return VGPU_PIXEL_FORMAT_R16_FLOAT;

    case DXGI_FORMAT_R8G8_UNORM:
        return VGPU_PIXEL_FORMAT_RG8_UNORM;

    case DXGI_FORMAT_R8G8_SNORM:
        return VGPU_PIXEL_FORMAT_RG8_SNORM;

    case DXGI_FORMAT_R8G8_UINT:
        return VGPU_PIXEL_FORMAT_RG8_UINT;

    case DXGI_FORMAT_R8G8_SINT:
        return VGPU_PIXEL_FORMAT_RG8_SINT;

        // Packed 16-bit pixel formats
    case DXGI_FORMAT_B5G6R5_UNORM:
        return VGPU_PIXEL_FORMAT_R5G6B5_UNORM;

    case DXGI_FORMAT_B4G4R4A4_UNORM:
        return VGPU_PIXEL_FORMAT_RGBA4_UNORM;

        // 32-bit pixel formats
    case DXGI_FORMAT_R32_UINT:
        return VGPU_PIXEL_FORMAT_R32_UINT;

    case DXGI_FORMAT_R32_SINT:
        return VGPU_PIXEL_FORMAT_R32_SINT;

    case DXGI_FORMAT_R32_FLOAT:
        return VGPU_PIXEL_FORMAT_R32_FLOAT;

    case DXGI_FORMAT_R16G16_UNORM:
        return VGPU_PIXEL_FORMAT_RG16_UNORM;

    case DXGI_FORMAT_R16G16_SNORM:
        return VGPU_PIXEL_FORMAT_RG16_SNORM;

    case DXGI_FORMAT_R16G16_UINT:
        return VGPU_PIXEL_FORMAT_RG16_UINT;

    case DXGI_FORMAT_R16G16_SINT:
        return VGPU_PIXEL_FORMAT_RG16_SINT;

    case DXGI_FORMAT_R16G16_FLOAT:
        return VGPU_PIXEL_FORMAT_RG16_FLOAT;

    case DXGI_FORMAT_R8G8B8A8_UNORM:
        return VGPU_PIXEL_FORMAT_RGBA8_UNORM;

    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        return VGPU_PIXEL_FORMAT_RGBA8_UNORM_SRGB;

    case DXGI_FORMAT_R8G8B8A8_SNORM:
        return VGPU_PIXEL_FORMAT_RGBA8_SNORM;

    case DXGI_FORMAT_R8G8B8A8_UINT:
        return VGPU_PIXEL_FORMAT_RGBA8_UINT;

    case DXGI_FORMAT_R8G8B8A8_SINT:
        return VGPU_PIXEL_FORMAT_RGBA8_SINT;

    case DXGI_FORMAT_B8G8R8A8_UNORM:
        return VGPU_PIXEL_FORMAT_BGRA8_UNORM;

    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        return VGPU_PIXEL_FORMAT_BGRA8_UNORM_SRGB;

        // Packed 32-Bit Pixel formats
    case DXGI_FORMAT_R10G10B10A2_UNORM:
        return VGPU_PIXEL_FORMAT_RGB10A2_UNORM;

    case DXGI_FORMAT_R10G10B10A2_UINT:
        return VGPU_PIXEL_FORMAT_RGB10A2_UINT;

    case DXGI_FORMAT_R11G11B10_FLOAT:
        return VGPU_PIXEL_FORMAT_RG11B10_FLOAT;

    case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
        return VGPU_PIXEL_FORMAT_RGB9E5_FLOAT;

        // 64-Bit Pixel Formats
    case DXGI_FORMAT_R32G32_UINT:
        return VGPU_PIXEL_FORMAT_RG32_UINT;

    case DXGI_FORMAT_R32G32_SINT:
        return VGPU_PIXEL_FORMAT_RG32_SINT;

    case DXGI_FORMAT_R32G32_FLOAT:
        return VGPU_PIXEL_FORMAT_RG32_FLOAT;

    case DXGI_FORMAT_R16G16B16A16_UNORM:
        return VGPU_PIXEL_FORMAT_RGBA16_UNORM;

    case DXGI_FORMAT_R16G16B16A16_SNORM:
        return VGPU_PIXEL_FORMAT_RGBA16_SNORM;

    case DXGI_FORMAT_R16G16B16A16_UINT:
        return VGPU_PIXEL_FORMAT_RGBA16_UINT;

    case DXGI_FORMAT_R16G16B16A16_SINT:
        return VGPU_PIXEL_FORMAT_RGBA16_SINT;

    case DXGI_FORMAT_R16G16B16A16_FLOAT:
        return VGPU_PIXEL_FORMAT_RGBA16_FLOAT;

        // 128-Bit Pixel Formats
    case DXGI_FORMAT_R32G32B32A32_UINT:
        return VGPU_PIXEL_FORMAT_RGBA32_UINT;

    case DXGI_FORMAT_R32G32B32A32_SINT:
        return VGPU_PIXEL_FORMAT_RGBA32_SINT;

    case DXGI_FORMAT_R32G32B32A32_FLOAT:
        return VGPU_PIXEL_FORMAT_RGBA32_FLOAT;

        // 128-Bit Pixel Formats
    case DXGI_FORMAT_D16_UNORM:
        return VGPU_PIXEL_FORMAT_D16_UNORM;

    case DXGI_FORMAT_D32_FLOAT:
        return VGPU_PIXEL_FORMAT_D32_FLOAT;

    case DXGI_FORMAT_D24_UNORM_S8_UINT:
        return VGPU_PIXEL_FORMAT_D24_UNORM_S8_UINT;

    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
        return VGPU_PIXEL_FORMAT_D32_FLOAT_S8_UINT;

        // We map VGPU_PIXEL_FORMAT_S8 to  DXGI_FORMAT_D24_UNORM_S8_UINT

        // Compressed formats
    case DXGI_FORMAT_BC1_UNORM:
        return VGPU_PIXEL_FORMAT_BC1_UNORM;

    case DXGI_FORMAT_BC1_UNORM_SRGB:
        return VGPU_PIXEL_FORMAT_BC1_UNORM_SRGB;

    case DXGI_FORMAT_BC2_UNORM:
        return VGPU_PIXEL_FORMAT_BC2_UNORM;

    case DXGI_FORMAT_BC2_UNORM_SRGB:
        return VGPU_PIXEL_FORMAT_BC2_UNORM_SRGB;

    case DXGI_FORMAT_BC3_UNORM:
        return VGPU_PIXEL_FORMAT_BC3_UNORM;

    case DXGI_FORMAT_BC3_UNORM_SRGB:
        return VGPU_PIXEL_FORMAT_BC3_UNORM_SRGB;

    case DXGI_FORMAT_BC4_UNORM:
        return VGPU_PIXEL_FORMAT_BC4_UNORM;

    case DXGI_FORMAT_BC4_SNORM:
        return VGPU_PIXEL_FORMAT_BC4_SNORM;

    case DXGI_FORMAT_BC5_UNORM:
        return VGPU_PIXEL_FORMAT_BC5_UNORM;

    case DXGI_FORMAT_BC5_SNORM:
        return VGPU_PIXEL_FORMAT_BC5_SNORM;

    case DXGI_FORMAT_BC6H_SF16:
        return VGPU_PIXEL_FORMAT_BC6HS16;

    case DXGI_FORMAT_BC6H_UF16:
        return VGPU_PIXEL_FORMAT_BC6HU16;

    case DXGI_FORMAT_BC7_UNORM:
        return VGPU_PIXEL_FORMAT_BC7_UNORM;

    case DXGI_FORMAT_BC7_UNORM_SRGB:
        return VGPU_PIXEL_FORMAT_BC7_UNORM_SRGB;

    default:
        return VGPU_PIXEL_FORMAT_UNDEFINED;
    }
}

static inline VGpuTextureUsageFlags _vgpuD3D11GetTextureUsage(UINT bindFlags)
{
    VGpuTextureUsageFlags usage = VGPU_TEXTURE_USAGE_NONE;
    if (bindFlags & D3D11_BIND_SHADER_RESOURCE)
    {
        usage |= VGPU_TEXTURE_USAGE_SHADER_READ;
    }

    if (bindFlags & D3D11_BIND_UNORDERED_ACCESS)
    {
        usage |= VGPU_TEXTURE_USAGE_SHADER_WRITE;
    }

    if (bindFlags & D3D11_BIND_RENDER_TARGET
        || bindFlags & D3D11_BIND_DEPTH_STENCIL)
    {
        usage |= VGPU_TEXTURE_USAGE_RENDER_TARGET;
    }

    return usage;
}

static VGpuTextureDescriptor _vgpuD3D11GetTextureDescriptor(const D3D11_TEXTURE2D_DESC& desc)
{
    return VGpuTextureDescriptor{
        VGPU_TEXTURE_TYPE_2D,
        _vgpuD3D11GetPixelFormat(desc.Format),
        desc.Width,
        desc.Height,
        desc.ArraySize,
        desc.MipLevels,
        static_cast<VgpuSampleCount>(desc.SampleDesc.Count),
        _vgpuD3D11GetTextureUsage(desc.BindFlags)
    };
}

static void _vgpuD3D11SetupTexture(VGpuTexture texture, const VGpuTextureDescriptor* descriptor)
{
    texture->type = descriptor->type;
    texture->format = descriptor->format;
    texture->width = descriptor->width;
    texture->height = descriptor->height;
    texture->depthOrArraySize = descriptor->depthOrArraySize;
    texture->mipLevels = descriptor->mipLevels;
    texture->sampleCount = descriptor->sampleCount;
    texture->usage = descriptor->sampleCount;
    texture->dxgi_format = _vgpuD3D11GetFormat(descriptor->format);
}

VGpuResult _vgpuD3D11Resize(VGpuPlatformHandle handle, uint32_t width, uint32_t height, const VGpuSwapchainDescriptor* descriptor);

VGpuBackend vgpuGetBackend()
{
    return VGPU_BACKEND_D3D11;
}

VGpuResult vgpuInitialize(const char* app_name, const VGpuRendererSettings * settings)
{
    if (_d3d11.initialized) {
        return VGPU_ALREADY_INITIALIZED;
    }

    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
    if (_vgpuD3D11SdkLayersAvailable())
    {
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
    }
#endif

    static const D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };

    HRESULT hr = D3D11CreateDevice(
        nullptr,                            // specify nullptr to use the default adapter
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        creationFlags,
        featureLevels,
        _countof(featureLevels),
        D3D11_SDK_VERSION,
        &_d3d11.device,    // returns the Direct3D device created
        &_d3d11.feature_level,                    // returns feature level of device created
        &_d3d11.context   // returns the device immediate context
    );

#ifndef NDEBUG
    ID3D11Debug* d3dDebug;
    if (SUCCEEDED(_d3d11.device->QueryInterface(&d3dDebug)))
    {
        ID3D11InfoQueue* d3dInfoQueue;
        if (SUCCEEDED(d3dDebug->QueryInterface(&d3dInfoQueue)))
        {
#ifdef _DEBUG
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif
            D3D11_MESSAGE_ID hide[] =
            {
                D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
                // TODO: Add more message IDs here as needed.
            };
            D3D11_INFO_QUEUE_FILTER filter = {};
            filter.DenyList.NumIDs = _countof(hide);
            filter.DenyList.pIDList = hide;
            d3dInfoQueue->AddStorageFilterEntries(&filter);
            d3dInfoQueue->Release();
        }

        d3dDebug->Release();
    }
#endif

    _vgpuD3D11Resize(settings->handle, settings->width, settings->height, &settings->swapchain);

    _d3d11.initialized = true;
    return VGPU_SUCCESS;
}

void vgpuShutdown()
{
    if (!_d3d11.initialized) {
        return;
    }

    SAFE_RELEASE(_d3d11.context);
    SAFE_RELEASE(_d3d11.device);

    _d3d11.initialized = false;
}

VGpuResult _vgpuD3D11Resize(VGpuPlatformHandle handle, uint32_t width, uint32_t height, const VGpuSwapchainDescriptor* descriptor)
{
    // Clear the previous window size specific context.
    ID3D11RenderTargetView* nullViews[] = { nullptr };
    _d3d11.context->OMSetRenderTargets(_countof(nullViews), nullViews, nullptr);
    vgpuDestroyFramebuffer(_d3d11.framebuffer);
    vgpuDestroyTexture(_d3d11.renderTarget);
    vgpuDestroyTexture(_d3d11.depthStencilTexture);
    _d3d11.context->Flush();

    // Determine the render target size in pixels.
    const DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
    const uint32_t sample_count = 1;
    const UINT BackBufferCount = 2u;

    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    if (_d3d11.swapChain)
    {
        // If the swap chain already exists, resize it.
        _d3d11.swapChain->GetDesc(&swapChainDesc);

        HRESULT hr = _d3d11.swapChain->ResizeBuffers(
            swapChainDesc.BufferCount,
            width,
            height,
            swapChainDesc.BufferDesc.Format,
            swapChainDesc.Flags
        );

        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
        {
#ifdef _DEBUG
            char buff[64] = {};
            sprintf_s(buff, "Device Lost on ResizeBuffers: Reason code 0x%08X\n", (hr == DXGI_ERROR_DEVICE_REMOVED) ? _d3d11.device->GetDeviceRemovedReason() : hr);
            OutputDebugStringA(buff);
#endif
            // If the device was removed for any reason, a new device and swap chain will need to be created.
            //HandleDeviceLost();

            // Everything is set up now. Do not continue execution of this method. HandleDeviceLost will reenter this method 
            // and correctly set up the new device.
            return VGPU_ERROR_SWAPCHAIN_CREATION_FAILED;
        }
        else
        {
            if (FAILED(hr))
            {
                return VGPU_ERROR_SWAPCHAIN_CREATION_FAILED;
            }
        }
    }
    else
    {


        // First, retrieve the underlying DXGI Device from the D3D Device.
        IDXGIDevice1* dxgiDevice;
        _vgpuD3D11ThrowIfFailed(_d3d11.device->QueryInterface(&dxgiDevice));

        // Identify the physical adapter (GPU or card) this device is running on.
        IDXGIAdapter* dxgiAdapter;
        _vgpuD3D11ThrowIfFailed(dxgiDevice->GetAdapter(&dxgiAdapter));

        // And obtain the factory object that created it.
        IDXGIFactory1* dxgiFactory;
        _vgpuD3D11ThrowIfFailed(dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory)));

        // Create a descriptor for the swap chain.

        swapChainDesc.BufferDesc.Width = width;
        swapChainDesc.BufferDesc.Height = height;
        swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
        swapChainDesc.BufferDesc.Format = backBufferFormat;
        swapChainDesc.SampleDesc.Count = sample_count;
        swapChainDesc.SampleDesc.Quality = sample_count > 1 ? D3D11_STANDARD_MULTISAMPLE_PATTERN : 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = BackBufferCount;
        swapChainDesc.OutputWindow = handle.hwnd;
        swapChainDesc.Windowed = TRUE;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        // Create a SwapChain from a Win32 window.
        _vgpuD3D11ThrowIfFailed(dxgiFactory->CreateSwapChain(
            _d3d11.device,
            &swapChainDesc,
            &_d3d11.swapChain
        ));

        // This template does not support exclusive fullscreen mode and prevents DXGI from responding to the ALT+ENTER shortcut.
        _vgpuD3D11ThrowIfFailed(dxgiFactory->MakeWindowAssociation(handle.hwnd, DXGI_MWA_NO_ALT_ENTER));

        SAFE_RELEASE(dxgiFactory);
        SAFE_RELEASE(dxgiAdapter);
        SAFE_RELEASE(dxgiDevice);
    }

    // Obtain the backbuffer for this window which will be the final 3D rendertarget.
    ID3D11Texture2D* backBuffer;
    _vgpuD3D11ThrowIfFailed(_d3d11.swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)));
    D3D11_TEXTURE2D_DESC backBufferD3D11TextureDesc;
    backBuffer->GetDesc(&backBufferD3D11TextureDesc);
    VGpuTextureDescriptor backBufferTextureDescriptor = _vgpuD3D11GetTextureDescriptor(backBufferD3D11TextureDesc);
    _d3d11.renderTarget = vgpuCreateExternalTexture(&backBufferTextureDescriptor, backBuffer);
    if (descriptor->depthStencilFormat != VGPU_PIXEL_FORMAT_UNDEFINED)
    {
        VGpuPixelFormat depthStencilFormat = VGPU_PIXEL_FORMAT_UNDEFINED;

        D3D11_FEATURE_DATA_FORMAT_SUPPORT featureDataFormatSupport = {};
        featureDataFormatSupport.InFormat = _vgpuD3D11GetFormat(descriptor->depthStencilFormat);
        if (SUCCEEDED(_d3d11.device->CheckFeatureSupport(D3D11_FEATURE_FORMAT_SUPPORT, &featureDataFormatSupport, sizeof(featureDataFormatSupport)))
            && featureDataFormatSupport.OutFormatSupport & D3D11_FORMAT_SUPPORT_RENDER_TARGET | D3D11_FORMAT_SUPPORT_DEPTH_STENCIL)
        {
            depthStencilFormat = descriptor->depthStencilFormat;
        }
        else
        {
            // Fall back to something that's guaranteed to work
            depthStencilFormat = VGPU_PIXEL_FORMAT_D16_UNORM;
        }

        VGpuTextureDescriptor depthStencilTextureDescriptor = {};
        depthStencilTextureDescriptor.type = VGPU_TEXTURE_TYPE_2D;
        depthStencilTextureDescriptor.format = depthStencilFormat;
        depthStencilTextureDescriptor.width = width;
        depthStencilTextureDescriptor.height = height;
        depthStencilTextureDescriptor.depthOrArraySize = 1;
        depthStencilTextureDescriptor.mipLevels = 1;
        depthStencilTextureDescriptor.sampleCount = descriptor->sampleCount;
        depthStencilTextureDescriptor.usage = VGPU_TEXTURE_USAGE_RENDER_TARGET;
        depthStencilTextureDescriptor.label = "BackbufferDepthStencilTexture";
        _d3d11.depthStencilTexture = vgpuCreateTexture(&depthStencilTextureDescriptor);
        //handle->depthStencilClearValue.depth = descriptor->depthStencilClearValue.depth;
        //handle->depthStencilClearValue.stencil = descriptor->depthStencilClearValue.stencil;
    }

    return VGPU_SUCCESS;
}

VGpuResult vgpuBeginFrame()
{
    return VGPU_SUCCESS;
}

VGpuResult vgpuEndFrame()
{
    HRESULT hr = _d3d11.swapChain->Present(1, 0);

    // If the device was reset we must completely reinitialize the renderer.
    if (hr == DXGI_ERROR_DEVICE_REMOVED
        || hr == DXGI_ERROR_DEVICE_RESET)
    {
        //OnDeviceLost();
    }
    else
    {
        if (FAILED(hr))
        {
            return VGPU_ERROR_END_FRAME_FAILED;
        }
    }

    return VGPU_SUCCESS;
}

VgpuBool32 vgpuQueryFeature(VGpuFeature feature)
{
    assert(_d3d11.initialized);
    return false;
}

void vgpuQueryLimits(VGpuLimits* pLimits)
{
    assert(_d3d11.initialized);
}

VGpuTexture vgpuCreateTexture(const VGpuTextureDescriptor* descriptor)
{
    VGpuTexture texture = new VGpuTexture_T();
    _vgpuD3D11SetupTexture(texture, descriptor);

    UINT d3d11_bind_flags = 0;
    UINT d3d11_misc_flags = 0;

    if (descriptor->usage & VGPU_TEXTURE_USAGE_SHADER_READ)
    {
        d3d11_bind_flags |= D3D11_BIND_SHADER_RESOURCE;
    }

    if (descriptor->usage & VGPU_TEXTURE_USAGE_SHADER_WRITE)
    {
        d3d11_bind_flags |= D3D11_BIND_UNORDERED_ACCESS;
    }

    if (descriptor->usage & VGPU_TEXTURE_USAGE_RENDER_TARGET)
    {
        if (!vgpuIsDepthStencilFormat(descriptor->format))
        {
            d3d11_bind_flags |= D3D11_BIND_RENDER_TARGET;
            if (descriptor->mipLevels == 0 || descriptor->mipLevels > 1)
            {
                d3d11_misc_flags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
            }
        }
        else
        {
            d3d11_bind_flags |= D3D11_BIND_DEPTH_STENCIL;
        }
    }

    D3D11_SUBRESOURCE_DATA* subResourceData = nullptr;
    D3D11_TEXTURE1D_DESC d3d11_tex1d_desc;
    D3D11_TEXTURE2D_DESC d3d11_tex2d_desc;
    D3D11_TEXTURE3D_DESC d3d11_tex3d_desc;
    HRESULT hr = S_OK;
    switch (descriptor->type)
    {
    case VGPU_TEXTURE_TYPE_1D:
        memset(&d3d11_tex1d_desc, 0, sizeof(d3d11_tex1d_desc));
        d3d11_tex1d_desc.Width = descriptor->width;
        d3d11_tex1d_desc.MipLevels = descriptor->mipLevels;
        d3d11_tex1d_desc.ArraySize = descriptor->depthOrArraySize;
        d3d11_tex1d_desc.Format = texture->dxgi_format;
        d3d11_tex1d_desc.Usage = D3D11_USAGE_DEFAULT;
        d3d11_tex1d_desc.BindFlags = d3d11_bind_flags;
        d3d11_tex1d_desc.CPUAccessFlags = 0;
        d3d11_tex1d_desc.MiscFlags = d3d11_misc_flags;

        ID3D11Texture1D* d3d11_text1d;
        hr = _d3d11.device->CreateTexture1D(&d3d11_tex1d_desc, subResourceData, &d3d11_text1d);
        if (FAILED(hr))
        {
            //ALIMER_LOGCRITICAL("[D3D11]: Failed to create 1D texture.");
        }

        texture->d3d11_resource = d3d11_text1d;
        break;

    case VGPU_TEXTURE_TYPE_2D:
    case VGPU_TEXTURE_TYPE_CUBE:
        memset(&d3d11_tex2d_desc, 0, sizeof(d3d11_tex2d_desc));
        d3d11_tex2d_desc.Width = descriptor->width;
        d3d11_tex2d_desc.Height = descriptor->height;
        d3d11_tex2d_desc.MipLevels = descriptor->mipLevels;
        d3d11_tex2d_desc.ArraySize = descriptor->depthOrArraySize;
        d3d11_tex2d_desc.Format = texture->dxgi_format;
        d3d11_tex2d_desc.SampleDesc.Count = descriptor->sampleCount;
        d3d11_tex2d_desc.SampleDesc.Quality = descriptor->sampleCount > 1 ? D3D11_STANDARD_MULTISAMPLE_PATTERN : 0;
        d3d11_tex2d_desc.Usage = D3D11_USAGE_DEFAULT;
        d3d11_tex2d_desc.BindFlags = d3d11_bind_flags;
        d3d11_tex2d_desc.CPUAccessFlags = 0;
        d3d11_tex2d_desc.MiscFlags = d3d11_misc_flags;

        if (descriptor->type == VGPU_TEXTURE_TYPE_CUBE)
        {
            d3d11_tex2d_desc.MiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
        }

        ID3D11Texture2D* d3d11_text2d;
        hr = _d3d11.device->CreateTexture2D(&d3d11_tex2d_desc, subResourceData, &d3d11_text2d);
        if (FAILED(hr))
        {
            //ALIMER_LOGCRITICAL("[D3D11]: Failed to create 1D texture.");
        }

        texture->d3d11_resource = d3d11_text2d;
        break;
    }

#if defined(_DEBUG)
    const UINT labelLength = descriptor->label ? (UINT)strlen(descriptor->label) : 0;
    if (labelLength > 0)
    {
        texture->d3d11_resource->SetPrivateData(WKPDID_D3DDebugObjectName, labelLength, descriptor->label);
    }
#endif

    return texture;
}

VGpuTexture vgpuCreateExternalTexture(const VGpuTextureDescriptor* descriptor, void* handle)
{
    VGpuTexture texture = new VGpuTexture_T();
    _vgpuD3D11SetupTexture(texture, descriptor);
    texture->d3d11_resource = (ID3D11Texture2D*)handle;
    texture->externalHandle = true;
    return texture;
}

void vgpuDestroyTexture(VGpuTexture texture)
{
    if (!texture) {
        return;
    }

    if (texture->d3d11_resource) {
        texture->d3d11_resource->Release();
    }
}

VGpuFramebuffer vgpuCreateFramebuffer(const VGpuFramebufferDescriptor* descriptor)
{
    VGpuFramebuffer framebuffer = new VGpuFramebuffer_T();

    const VGpuFramebufferAttachment* attachment;
    for (uint32_t i = 0; i < VGPU_MAX_COLOR_ATTACHMENTS; i++) {
        attachment = &descriptor->colorAttachments[i];
        if (attachment->texture == nullptr) {
            continue;
        }

        const uint32_t arraySize = attachment->texture->depthOrArraySize - attachment->slice;
        const bool isTextureMs = attachment->texture->sampleCount > 1;

        D3D11_RENDER_TARGET_VIEW_DESC desc = {};
        desc.Format = attachment->texture->dxgi_format;
        switch (attachment->texture->type)
        {
        case VGPU_TEXTURE_TYPE_1D:
            if (arraySize > 1)
            {
                desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1DARRAY;
                desc.Texture1DArray.MipSlice = attachment->level;
                desc.Texture1DArray.FirstArraySlice = attachment->slice;
                desc.Texture1DArray.ArraySize = arraySize;
            }
            else
            {
                desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1D;
                desc.Texture1D.MipSlice = attachment->level;
            }
            break;
        case VGPU_TEXTURE_TYPE_2D:
            if (arraySize > 1)
            {
                if (isTextureMs)
                {
                    desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
                    desc.Texture2DMSArray.FirstArraySlice = attachment->slice;
                    desc.Texture2DMSArray.ArraySize = arraySize;
                }
                else
                {
                    desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
                    desc.Texture2DArray.MipSlice = attachment->level;
                    desc.Texture2DArray.FirstArraySlice = attachment->slice;
                    desc.Texture2DArray.ArraySize = arraySize;
                }
            }
            else
            {
                if (isTextureMs)
                {
                    desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
                }
                else
                {
                    desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
                    desc.Texture2D.MipSlice = attachment->level;
                }
            }

            break;

        case VGPU_TEXTURE_TYPE_3D:
            assert(arraySize == 1);
            desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
            desc.Texture3D.MipSlice = attachment->level;
            desc.Texture3D.FirstWSlice = attachment->slice;
            desc.Texture3D.WSize = 1;
            break;

        case VGPU_TEXTURE_TYPE_CUBE:
            desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
            desc.Texture2DArray.MipSlice = attachment->level;
            desc.Texture2DArray.FirstArraySlice = attachment->slice * 6;
            desc.Texture2DArray.ArraySize = arraySize * 6;
            break;

        default:
            desc.ViewDimension = D3D11_RTV_DIMENSION_UNKNOWN;
            //ALIMER_LOGCRITICAL("Invalid texture type");
            break;
        }

        ID3D11Resource* d3d11_resource = attachment->texture->d3d11_resource;
        HRESULT hr = _d3d11.device->CreateRenderTargetView(d3d11_resource, &desc, &framebuffer->d3d11_rtvs[framebuffer->numColorAttachments]);
        if (FAILED(hr))
        {
            //ALIMER_LOGCRITICAL("[D3D11] - CreateRenderTargetView failed");
        }


        framebuffer->numColorAttachments++;
    }
    return framebuffer;
}

void vgpuDestroyFramebuffer(VGpuFramebuffer framebuffer)
{
    if (!framebuffer) {
        return;
    }

    for (uint32_t i = 0; i < VGPU_MAX_COLOR_ATTACHMENTS; i++) {
        if (framebuffer->d3d11_rtvs[i]) {
            framebuffer->d3d11_rtvs[i]->Release();
        }
    }
    if (framebuffer->d3d11_dsv) {
        framebuffer->d3d11_dsv->Release();
    }
}

VGpuCommandBuffer vgpuGetCommandBuffer()
{
    return nullptr;
}

VGpuResult vgpuBeginCommandBuffer(VGpuCommandBuffer commandBuffer)
{
    return VGPU_SUCCESS;
}

VGpuResult vgpuEndCommandBuffer(VGpuCommandBuffer commandBuffer)
{
    return VGPU_SUCCESS;
}

void vgpuCmdEndRenderPass(VGpuCommandBuffer commandBuffer)
{

}

void vgpuCmdSetViewport(VGpuCommandBuffer commandBuffer, float x, float y, float width, float height)
{

}

void vgpuCmdSetScissor(VGpuCommandBuffer commandBuffer, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
    D3D11_RECT rect;
    rect.left = (LONG)x;
    rect.top = (LONG)y;
    rect.right = (LONG)(x + width);
    rect.bottom = (LONG)(y + height);
    commandBuffer->context->RSSetScissorRects(1, &rect);
}

void vgpuSubmitCommandBuffer(VGpuCommandBuffer commandBuffer)
{

}

#endif
