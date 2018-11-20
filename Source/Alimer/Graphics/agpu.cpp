//
// Copyright (c) 2018 Amer Koleci and contributors.
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

#include "../Base/Debug.h"

#define AGPU_IMPLEMENTATION
#include "agpu_backend.h"
#include "../Core/Log.h"
#include <vector>

#if defined(_WIN32)
#include <windows.h>

// Prefer the high-performance GPU on switchable GPU systems
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 1;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif /* defined(_WIN32) */

static AGpuRendererI* s_renderer = nullptr;

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4201)   /* nonstandard extension used: nameless struct/union */
#endif

std::vector<AgpuBackend> agpuGetSupportedBackends()
{
    static std::vector<AgpuBackend> backends;

    if (backends.empty())
    {
        backends.push_back(AGPU_BACKEND_EMPTY);

#if AGPU_D3D11
        if (agpuIsD3D11Supported())
            backends.push_back(AGPU_BACKEND_D3D11);
#endif

#if AGPU_D3D12
        if (agpuIsD3D12Supported())
            backends.push_back(AGPU_BACKEND_D3D12);
#endif
    }

    return backends;
}

AgpuBackend agpuGetDefaultPlatformBackend()
{
#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP || ALIMER_PLATFORM_XBOX_ONE
    if (agpuIsBackendSupported(AGPU_BACKEND_D3D12))
    {
        return AGPU_BACKEND_D3D12;
    }

    return AGPU_BACKEND_D3D11;
#elif ALIMER_PLATFORM_LINUX || ALIMER_PLATFORM_ANDROID
    return AGPU_BACKEND_OPENGL;
#elif ALIMER_PLATFORM_MACOS || ALIMER_PLATFORM_IOS || ALIMER_PLATFORM_TVOS
    return AGPU_BACKEND_METAL;
#else
    return AGPU_BACKEND_OPENGL;
#endif
}

AgpuBool32 agpuIsBackendSupported(AgpuBackend backend)
{
    if (backend == AGPU_BACKEND_DEFAULT)
    {
        backend = agpuGetDefaultPlatformBackend();
    }

    switch (backend)
    {
    case AGPU_BACKEND_EMPTY:
        return AGPU_TRUE;
    case AGPU_BACKEND_VULKAN:
#if AGPU_VULKAN
        return AGPU_TRUE; // VulkanGraphicsDevice::IsSupported();
#else
        return AGPU_FALSE;
#endif
    case AGPU_BACKEND_D3D11:
#if AGPU_D3D11
        return agpuIsD3D11Supported();
#else
        return AGPU_FALSE;
#endif

    case AGPU_BACKEND_D3D12:
#if AGPU_D3D12
        return agpuIsD3D12Supported();
#else
        return AGPU_FALSE;
#endif

    case AGPU_BACKEND_OPENGL:
#if AGPU_OPENGL
        return AGPU_TRUE;
#else
        return AGPU_FALSE;
#endif
    default:
        return AGPU_FALSE;
    }
}

uint32_t agpuGetAvailableBackendsCount()
{
    return (uint32_t)agpuGetSupportedBackends().size();
}

AgpuBackend agpuGetAvailableBackend(uint32_t index)
{
    const std::vector<AgpuBackend> backends = agpuGetSupportedBackends();
    ALIMER_ASSERT(index < backends.size());
    return backends[index];
}

AgpuResult agpuInitialize(const AgpuDescriptor* descriptor)
{
    if (s_renderer != nullptr)
        return AGPU_ALREADY_INITIALIZED;

    AgpuBackend backend = descriptor->preferredBackend;
    if (backend == AGPU_BACKEND_DEFAULT)
    {
        backend = agpuGetDefaultPlatformBackend();
    }

    AgpuResult result = AGPU_ERROR;
    AGpuRendererI* renderer = nullptr;
    switch (backend)
    {
    case AGPU_BACKEND_EMPTY:
        break;
    case AGPU_BACKEND_VULKAN:
        break;
    case AGPU_BACKEND_D3D11:
#if AGPU_D3D11
        result = agpuCreateD3D11Backend(descriptor, &renderer);
#else
        ALIMER_LOGERROR("D3D11 backend is not supported");
        result = AGPU_ERROR;
#endif
        break;
    case AGPU_BACKEND_D3D12:
#if AGPU_D3D12
        result = agpuCreateD3D12Backend(descriptor, &renderer);
#else
        ALIMER_LOGERROR("D3D12 backend is not supported");
        result = AGPU_ERROR;
#endif
        break;
    case AGPU_BACKEND_METAL:
        break;
    case AGPU_BACKEND_OPENGL:
        break;
    default:
        break;
    }

    if (result == AGPU_OK)
    {
        s_renderer = renderer;
        return AGPU_OK;
    }

    return result;
}

void agpuShutdown()
{
    if (s_renderer != nullptr)
    {
        s_renderer->Shutdown();
        delete s_renderer;
        s_renderer = nullptr;
    }
}

uint64_t agpuFrame()
{
    return s_renderer->Frame();
}

AgpuBuffer agpuCreateBuffer(const AgpuBufferDescriptor* descriptor)
{
    return s_renderer->CreateBuffer(descriptor, NULL);
}

AgpuBuffer agpuCreateExternalBuffer(const AgpuBufferDescriptor* descriptor, void* handle)
{
    return s_renderer->CreateBuffer(descriptor, handle);
}

void agpuDestroyBuffer(AgpuBuffer buffer)
{
    s_renderer->DestroyBuffer(buffer);
}

AgpuTexture agpuCreateTexture(const AgpuTextureDescriptor* descriptor)
{
    return s_renderer->CreateTexture(descriptor, NULL);
}

AgpuTexture agpuCreateExternalTexture(const AgpuTextureDescriptor* descriptor, void* handle)
{
    return s_renderer->CreateTexture(descriptor, handle);
}

void agpuDestroyTexture(AgpuTexture texture)
{
    s_renderer->DestroyTexture(texture);
}

AgpuFramebuffer agpuCreateFramebuffer(const AgpuFramebufferDescriptor* descriptor)
{
    return s_renderer->CreateFramebuffer(descriptor);
}

void agpuDestroyFramebuffer(AgpuFramebuffer framebuffer)
{
    s_renderer->DestroyFramebuffer(framebuffer);
}

AgpuShader agpuCreateShader(const AgpuShaderDescriptor* descriptor)
{
    return s_renderer->CreateShader(descriptor);
}

void agpuDestroyShader(AgpuShader shader)
{
    s_renderer->DestroyShader(shader);
}

AgpuPipeline agpuCreateRenderPipeline(const AgpuRenderPipelineDescriptor* descriptor)
{
    ALIMER_ASSERT_MSG(descriptor, "Invalid render pipeline descriptor.");
    ALIMER_ASSERT_MSG(descriptor->vertex, "Invalid vertex shader.");
    ALIMER_ASSERT_MSG(descriptor->fragment, "Invalid fragment shader.");
    return s_renderer->CreateRenderPipeline(descriptor);
}

AgpuPipeline agpuCreateComputePipeline(const AgpuComputePipelineDescriptor* descriptor)
{
    ALIMER_ASSERT_MSG(descriptor, "Invalid compute pipeline descriptor.");
    ALIMER_ASSERT_MSG(descriptor->shader, "Invalid shader");

    return s_renderer->CreateComputePipeline(descriptor);
}

void agpuDestroyPipeline(AgpuPipeline pipeline)
{
    s_renderer->DestroyPipeline(pipeline);
}

typedef enum AgpuPixelFormatType
{
    /// Unknown format Type
    AGPU_PIXEL_FORMAT_TYPE_UNKNOWN      = 0,
    /// _FLOATing-point formats
    AGPU_PIXEL_FORMAT_TYPE_FLOAT        = 1,
    /// Unsigned normalized formats
    AGPU_PIXEL_FORMAT_TYPE_UNORM        = 2,
    /// Unsigned normalized SRGB formats
    AGPU_PIXEL_FORMAT_TYPE_UNORM_SRGB   = 3,
    /// Signed normalized formats
    AGPU_PIXEL_FORMAT_TYPE_SNORM        = 4,
    /// Unsigned integer formats
    AGPU_PIXEL_FORMAT_TYPE_UINT         = 5,
    /// Signed integer formats
    AGPU_PIXEL_FORMAT_TYPE_SINT         = 6
} AgpuPixelFormatType;

struct AgpuPixelFormatDesc
{
    AgpuPixelFormat format;
    const char* name;
    uint32_t bytesPerBlock;
    uint32_t channelCount;
    AgpuPixelFormatType Type;
    struct
    {
        bool isDepth;
        bool isStencil;
        bool isCompressed;
    };
    struct
    {
        uint32_t width;
        uint32_t height;
    } compressionRatio;
};

const AgpuPixelFormatDesc FormatDesc[] =
{
    // Format                               Name,               BytesPerBlock, ChannelCount,  Type  {Depth,  Stencil, Compressed},      {CompressionRatio.Width, CompressionRatio.Height}
    { AGPU_PIXEL_FORMAT_UNKNOWN,            "Unknown",          0,  0,  AGPU_PIXEL_FORMAT_TYPE_UNKNOWN,    { false, false, false},            {1, 1}},
    { AGPU_PIXEL_FORMAT_R8_UNORM,           "R8_UNORM",         1,  1,  AGPU_PIXEL_FORMAT_TYPE_UNORM,      { false, false, false},            {1, 1}},
    { AGPU_PIXEL_FORMAT_R8_SNORM,           "R8_SNORM",         1,  1,  AGPU_PIXEL_FORMAT_TYPE_SNORM,      { false, false, false},            {1, 1}},
    { AGPU_PIXEL_FORMAT_R16_UNORM,          "R16_UNORM",        2,  1,  AGPU_PIXEL_FORMAT_TYPE_UNORM,      { false, false, false},            {1, 1}},
    { AGPU_PIXEL_FORMAT_R16_SNORM,          "R16_SNORM",        2,  1,  AGPU_PIXEL_FORMAT_TYPE_SNORM,      { false, false, false},            {1, 1}},
    { AGPU_PIXEL_FORMAT_RG8_UNORM,          "RG8_UNORM",        2,  2,  AGPU_PIXEL_FORMAT_TYPE_UNORM,      { false, false, false},            {1, 1}},
    { AGPU_PIXEL_FORMAT_RG8_SNORM,          "RG8_SNORM",        2,  2,  AGPU_PIXEL_FORMAT_TYPE_SNORM,      { false, false, false,},            {1, 1}},
    { AGPU_PIXEL_FORMAT_RG16_UNORM,         "RG16_UNORM",       4,  2,  AGPU_PIXEL_FORMAT_TYPE_UNORM,      { false, false, false},            {1, 1}},
    { AGPU_PIXEL_FORMAT_RG16_SNORM,         "RG16_SNORM",       4,  2,  AGPU_PIXEL_FORMAT_TYPE_SNORM,      { false, false, false},            {1, 1}},
    { AGPU_PIXEL_FORMAT_RGB16_UNORM,        "RGB16_UNORM",      6,  3,  AGPU_PIXEL_FORMAT_TYPE_UNORM,      { false, false, false},            {1, 1}},
    { AGPU_PIXEL_FORMAT_RGB16_SNORM,        "RGB16_SNORM",      6,  3,  AGPU_PIXEL_FORMAT_TYPE_SNORM,      { false, false, false},            {1, 1}},

    { AGPU_PIXEL_FORMAT_RGBA8_UNORM,        "RGBA8_UNORM",      4,  4,  AGPU_PIXEL_FORMAT_TYPE_UNORM,      { false, false, false},            {1, 1}},
    { AGPU_PIXEL_FORMAT_RGBA8_UNORM_SRGB,   "RGBA8_UNORMSrgb",  4,  4,  AGPU_PIXEL_FORMAT_TYPE_UNORM_SRGB,  { false, false, false},            {1, 1}},
    { AGPU_PIXEL_FORMAT_RGBA8_SNORM,        "RGBA8_SNORM",      4,  4,  AGPU_PIXEL_FORMAT_TYPE_SNORM,      { false, false, false},            {1, 1}},

    { AGPU_PIXEL_FORMAT_BGRA8_UNORM,        "BGRA8_UNORM",      4,  4,  AGPU_PIXEL_FORMAT_TYPE_UNORM,      { false,  false, false},           {1, 1}},
    { AGPU_PIXEL_FORMAT_BGRA8_UNORM_SRGB,   "BGRA8_UNORMSrgb",  4,  4,  AGPU_PIXEL_FORMAT_TYPE_UNORM_SRGB,  { false,  false, false},           {1, 1}},

    { AGPU_PIXEL_FORMAT_D32_FLOAT,          "D32_FLOAT",        4,  1,  AGPU_PIXEL_FORMAT_TYPE_FLOAT,      { true,   false, false},           {1, 1}},
    { AGPU_PIXEL_FORMAT_D16_UNORM,          "D16_UNORM",        2,  1,  AGPU_PIXEL_FORMAT_TYPE_UNORM,      { true,   false, false},           {1, 1}},
    { AGPU_PIXEL_FORMAT_D24_UNORM_S8,       "D24_UNORMS8",      4,  2,  AGPU_PIXEL_FORMAT_TYPE_UNORM,      { true,   true,  false},           {1, 1}},
    { AGPU_PIXEL_FORMAT_D32_FLOAT_S8,       "D32_FLOATS8",      8,  2,  AGPU_PIXEL_FORMAT_TYPE_FLOAT,      { true,   true,  false},           {1, 1}},

    { AGPU_PIXEL_FORMAT_BC1_UNORM,          "BC1_UNORM",        8,  3,  AGPU_PIXEL_FORMAT_TYPE_UNORM,      { false,  false, true },           {4, 4}},
    { AGPU_PIXEL_FORMAT_BC1_UNORM_SRGB,     "BC1_UNORMSrgb",    8,  3,  AGPU_PIXEL_FORMAT_TYPE_UNORM_SRGB,  { false,  false, true },           {4, 4}},
    { AGPU_PIXEL_FORMAT_BC2_UNORM,          "BC2_UNORM",        16, 4,  AGPU_PIXEL_FORMAT_TYPE_UNORM,      { false,  false, true },           {4, 4}},
    { AGPU_PIXEL_FORMAT_BC2_UNORM_SRGB,     "BC2_UNORMSrgb",    16, 4,  AGPU_PIXEL_FORMAT_TYPE_UNORM_SRGB,  { false,  false, true },           {4, 4}},
    { AGPU_PIXEL_FORMAT_BC3_UNORM,          "BC3_UNORM",        16, 4,  AGPU_PIXEL_FORMAT_TYPE_UNORM,      { false,  false, true },           {4, 4}},
    { AGPU_PIXEL_FORMAT_BC3_UNORM_SRGB,     "BC3_UNORMSrgb",    16, 4,  AGPU_PIXEL_FORMAT_TYPE_UNORM_SRGB,  { false,  false, true },           {4, 4}},
    { AGPU_PIXEL_FORMAT_BC4_UNORM,          "BC4_UNORM",        8,  1,  AGPU_PIXEL_FORMAT_TYPE_UNORM,      { false,  false, true },           {4, 4}},
    { AGPU_PIXEL_FORMAT_BC4_SNORM,          "BC4_SNORM",        8,  1,  AGPU_PIXEL_FORMAT_TYPE_SNORM,      { false,  false, true },           {4, 4}},
    { AGPU_PIXEL_FORMAT_BC5_UNORM,          "BC5_UNORM",        16, 2,  AGPU_PIXEL_FORMAT_TYPE_UNORM,      { false,  false, true },           {4, 4}},
    { AGPU_PIXEL_FORMAT_BC5_SNORM,          "BC5_SNORM",        16, 2,  AGPU_PIXEL_FORMAT_TYPE_SNORM,      { false,  false, true },           {4, 4}},

    { AGPU_PIXEL_FORMAT_BC6HS16,            "BC6HS16",          16, 3,  AGPU_PIXEL_FORMAT_TYPE_FLOAT,      { false,  false, true, },          { 4, 4 }},
    { AGPU_PIXEL_FORMAT_BC6HU16,            "BC6HU16",          16, 3,  AGPU_PIXEL_FORMAT_TYPE_FLOAT,      { false,  false, true, },          { 4, 4 } },
    { AGPU_PIXEL_FORMAT_BC7_UNORM,          "BC7_UNORM",        16, 4,  AGPU_PIXEL_FORMAT_TYPE_UNORM,      { false,  false, true, },          { 4, 4 } },
    { AGPU_PIXEL_FORMAT_BC7_UNORM_SRGB,     "BC7_UNORMSrgb",    16, 4,  AGPU_PIXEL_FORMAT_TYPE_UNORM_SRGB,  { false,  false, true, },          { 4, 4 } },
};

AgpuBool32 agpuIsDepthFormat(AgpuPixelFormat format)
{
    ALIMER_ASSERT(FormatDesc[format].format == format);
    return FormatDesc[format].isDepth;
}

AgpuBool32 agpuIsStencilFormat(AgpuPixelFormat format)
{
    ALIMER_ASSERT(FormatDesc[format].format == format);
    return FormatDesc[format].isStencil;
}

AgpuBool32 agpuIsDepthStencilFormat(AgpuPixelFormat format)
{
    return agpuIsDepthFormat(format) || agpuIsStencilFormat(format);
}

AgpuBool32 agpuIsCompressed(AgpuPixelFormat format)
{
    ALIMER_ASSERT(FormatDesc[format].format == format);
    return FormatDesc[format].isCompressed;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
