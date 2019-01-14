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

#include "../Debug/Debug.h"

#define AGPU_IMPLEMENTATION
#include "agpu_backend.h"
#include "../Core/Log.h"

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

#if defined(ALIMER_D3D12)
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

    case AGPU_BACKEND_D3D12:
#if defined(ALIMER_D3D12)
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

    AGpuRendererI* renderer = nullptr;
    switch (backend)
    {
    case AGPU_BACKEND_EMPTY:
        break;
    case AGPU_BACKEND_VULKAN:
        break;
    case AGPU_BACKEND_D3D12:
#if defined(ALIMER_D3D12)
        renderer = agpuCreateD3D12Backend(descriptor->validation);
#else
        ALIMER_LOGERROR("D3D12 backend is not supported");
#endif
        break;
    case AGPU_BACKEND_METAL:
        break;
    case AGPU_BACKEND_OPENGL:
        break;
    default:
        break;
    }

    if (renderer != nullptr)
    {
        s_renderer = renderer;
        return renderer->Initialize(descriptor);
    }

    return AGPU_ERROR;
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

static void agpuBufferInitialize(AgpuBuffer buffer, const AgpuBufferDescriptor* descriptor)
{
    if (buffer != nullptr)
    {
        // Set properties
        uint64_t size = descriptor->size;
        //size = alimer::AlignTo(size, uint64_t(descriptor->stride));

        buffer->usage = descriptor->usage;
        buffer->size = size;
        buffer->stride = descriptor->stride;

#if defined(ALIMER_DEV)
        const char* usageStr = "";
        if (buffer->usage & AGPU_BUFFER_USAGE_VERTEX)
        {
            usageStr = "vertex";
        }
        else if (buffer->usage & AGPU_BUFFER_USAGE_INDEX)
        {
            usageStr = "index";
        }
        else if (buffer->usage & AGPU_BUFFER_USAGE_UNIFORM)
        {
            usageStr = "uniform";
        }

        ALIMER_LOGDEBUG("Created {} buffer [size: {}, stride: {}]",
            usageStr,
            buffer->size,
            buffer->stride
        );
#endif
    }
    else
    {
        ALIMER_LOGERROR("Failed to create buffer");
    }
}

AgpuBuffer agpuCreateBuffer(const AgpuBufferDescriptor* descriptor, const void* initialData)
{
    AgpuBuffer buffer = s_renderer->CreateBuffer(descriptor, initialData, NULL);
    agpuBufferInitialize(buffer, descriptor);
    return buffer;
}

AgpuBuffer agpuCreateExternalBuffer(const AgpuBufferDescriptor* descriptor, void* handle)
{
    AgpuBuffer buffer = s_renderer->CreateBuffer(descriptor, NULL, handle);
    agpuBufferInitialize(buffer, descriptor);
    return buffer;
}

void agpuDestroyBuffer(AgpuBuffer buffer)
{
    s_renderer->DestroyBuffer(buffer);
}

static void agpuTextureInitialize(AgpuTexture texture, const AgpuTextureDescriptor* descriptor)
{
    if (texture != nullptr)
    {
        // Set properties
        texture->width = descriptor->width;
        texture->height = descriptor->height;
        texture->depthOrArraySize = descriptor->depthOrArraySize;
        texture->mipLevels = descriptor->mipLevels;
    }
}

AgpuTexture agpuCreateTexture(const AgpuTextureDescriptor* descriptor)
{
    AgpuTexture texture = s_renderer->CreateTexture(descriptor, NULL);
    agpuTextureInitialize(texture, descriptor);
    return texture;
}

AgpuTexture agpuCreateExternalTexture(const AgpuTextureDescriptor* descriptor, void* handle)
{
    AgpuTexture texture = s_renderer->CreateTexture(descriptor, handle);
    agpuTextureInitialize(texture, descriptor);
    return texture;
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

AgpuShaderModule agpuCreateShaderModule(const AgpuShaderModuleDescriptor* descriptor)
{
    return s_renderer->CreateShaderModule(descriptor);
}

void agpuDestroyShaderModule(AgpuShaderModule shaderModule)
{
    s_renderer->DestroyShaderModule(shaderModule);
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
#if defined(ALIMER_DEV)
    ALIMER_ASSERT_MSG(descriptor, "Invalid render pipeline descriptor.");
    ALIMER_ASSERT_MSG(descriptor->shader, "Invalid shader.");
    //ALIMER_ASSERT_MSG(descriptor->fragment, "Invalid fragment shader.");

    for (int i = 0; i < AGPU_MAX_VERTEX_BUFFER_BINDINGS; i++)
    {
        const AgpuVertexBufferLayoutDescriptor* desc = &descriptor->vertexDescriptor.layouts[i];
        if (desc->stride == 0) {
            continue;
        }

        ALIMER_ASSERT_MSG((desc->stride & 3) == 0, "AgpuVertexDescriptor buffer stride must be multiple of 4");
    }

    ALIMER_ASSERT_MSG(descriptor->vertexDescriptor.attributes[0].format != AGPU_VERTEX_FORMAT_UNKNOWN, "AgpuVertexDescriptor attributes is empty or not continuous");
    bool attrsContinuous = true;

    for (int i = 0; i < AGPU_MAX_VERTEX_ATTRIBUTES; i++)
    {
        const AgpuVertexAttributeDescriptor* desc = &descriptor->vertexDescriptor.attributes[i];
        if (desc->format == AGPU_VERTEX_FORMAT_UNKNOWN) {
            attrsContinuous = false;
            continue;
        }
        ALIMER_ASSERT_MSG(attrsContinuous, "AgpuVertexDescriptor buffer stride must be multiple of 4");
        ALIMER_ASSERT(desc->bufferIndex < AGPU_MAX_VERTEX_BUFFER_BINDINGS);
    }
#endif

    return s_renderer->CreateRenderPipeline(descriptor);
}

AgpuPipeline agpuCreateComputePipeline(const AgpuComputePipelineDescriptor* descriptor)
{
#if defined(ALIMER_DEV)
    ALIMER_ASSERT_MSG(descriptor, "Invalid compute pipeline descriptor.");
    ALIMER_ASSERT_MSG(descriptor->shader, "Invalid shader");
#endif

    return s_renderer->CreateComputePipeline(descriptor);
}

void agpuDestroyPipeline(AgpuPipeline pipeline)
{
    s_renderer->DestroyPipeline(pipeline);
}

void agpuBeginRenderPass(AgpuFramebuffer framebuffer)
{
    ALIMER_ASSERT_MSG(framebuffer, "Invalid framebuffer");
    s_renderer->CmdBeginRenderPass(framebuffer);
}

void agpuEndRenderPass()
{
    s_renderer->CmdEndRenderPass();
}

void agpuCmdSetShader(AgpuShader shader)
{
    ALIMER_ASSERT_MSG(shader, "Invalid shader");
    s_renderer->CmdSetShader(shader);
}

void agpuCmdSetVertexBuffer(uint32_t binding, AgpuBuffer buffer, uint64_t offset, AgpuVertexInputRate inputRate)
{
    ALIMER_ASSERT_MSG(buffer, "Invalid buffer");
    ALIMER_ASSERT(binding < AGPU_MAX_VERTEX_BUFFER_BINDINGS);
#ifdef ALIMER_DEV
    if ((buffer->usage & AGPU_BUFFER_USAGE_VERTEX) == 0)
    {
        //agpuNotifyValidationError("agpuCmdSetVertexBuffer need buffer with Vertex usage");
        return;
    }
#endif

    s_renderer->CmdSetVertexBuffer(binding, buffer, offset, buffer->stride, inputRate);
}

void agpuCmdSetIndexBuffer(AgpuBuffer buffer, uint64_t offset, AgpuIndexType indexType)
{
    ALIMER_ASSERT_MSG(buffer, "Invalid buffer");
#ifdef ALIMER_DEV
    if ((buffer->usage & AGPU_BUFFER_USAGE_INDEX) == 0)
    {
        //agpuNotifyValidationError("agpuCmdSetIndexBuffer need buffer with Index usage");
        return;
    }
#endif

    s_renderer->CmdSetIndexBuffer(buffer, offset, indexType);
}

void agpuCmdSetViewport(AgpuViewport viewport)
{
    s_renderer->CmdSetViewport(viewport);
}

void agpuCmdSetViewports(uint32_t viewportCount, const AgpuViewport* pViewports)
{
    s_renderer->CmdSetViewports(viewportCount, pViewports);
}

void agpuCmdSetScissor(AgpuRect2D scissor)
{
    s_renderer->CmdSetScissor(scissor);
}

void agpuCmdSetScissors(uint32_t scissorCount, const AgpuRect2D* pScissors)
{
    s_renderer->CmdSetScissors(scissorCount, pScissors);
}

void CmdSetPrimitiveTopology(AgpuPrimitiveTopology topology)
{
    s_renderer->CmdSetPrimitiveTopology(topology);
}

void agpuCmdDraw(uint32_t vertexCount, uint32_t firstVertex)
{
    s_renderer->CmdDraw(vertexCount, 1, firstVertex, 0);
}

void agpuCmdDrawIndexed(uint32_t indexCount, uint32_t firstIndex, int32_t vertexOffset)
{
    ALIMER_ASSERT(indexCount > 1);
    s_renderer->CmdDrawIndexed(indexCount, 1, firstIndex, vertexOffset, 0);
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

uint32_t agpuGetTextureWidth(AgpuTexture texture)
{
    return texture->width;
}

uint32_t agpuGetTextureLevelWidth(AgpuTexture texture, uint32_t mipLevel)
{
    return std::max(1u, texture->width >> mipLevel);
}

uint32_t agpuGetTextureHeight(AgpuTexture texture)
{
    return texture->height;
}

uint32_t agpuGetTextureLevelHeight(AgpuTexture texture, uint32_t mipLevel)
{
    return std::max(1u, texture->height >> mipLevel);
}

uint32_t agpuGetTextureDepth(AgpuTexture texture)
{
    return texture->depthOrArraySize;
}

uint32_t agpuGetTextureLevelDepth(AgpuTexture texture, uint32_t mipLevel)
{
    return std::max(1u, texture->depthOrArraySize >> mipLevel);
}

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
