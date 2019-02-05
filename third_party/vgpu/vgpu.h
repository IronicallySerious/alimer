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

#pragma once

#include <stddef.h>

#if !defined(VGPU_NO_STDINT_H)
    #if defined(_MSC_VER) && (_MSC_VER < 1600)
        typedef signed   __int8  int8_t;
        typedef unsigned __int8  uint8_t;
        typedef signed   __int16 int16_t;
        typedef unsigned __int16 uint16_t;
        typedef signed   __int32 int32_t;
        typedef unsigned __int32 uint32_t;
        typedef signed   __int64 int64_t;
        typedef unsigned __int64 uint64_t;
    #else
        #include <stdint.h>
    #endif
#endif // !defined(VGPU_NO_STDINT_H)

#if defined(_WIN32) && defined(VGPU_SHARED_LIBRARY)
#   define VGPU_API __declspec(dllexport)
#elif defined(_WIN32) && defined(VGPU_USE_SHARED_LIBRARY)
#   define VGPU_API __declspec(dllimport)
#else
#   define VGPU_API
#endif


#ifndef VGPU_DEFINE_HANDLE
#   define VGPU_DEFINE_HANDLE(object) typedef struct object##_T* object
#endif

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    typedef uint32_t VgpuFlags;
    typedef uint32_t VgpuBool32;

    VGPU_DEFINE_HANDLE(VgpuSwapchain);
    VGPU_DEFINE_HANDLE(VgpuTexture);
    VGPU_DEFINE_HANDLE(VgpuFramebuffer);
    VGPU_DEFINE_HANDLE(VgpuBuffer);
    VGPU_DEFINE_HANDLE(VgpuShaderModule);
    VGPU_DEFINE_HANDLE(VgpuShader);
    VGPU_DEFINE_HANDLE(VgpuPipeline);
    VGPU_DEFINE_HANDLE(VgpuCommandBuffer);

#define VGPU_TRUE                           1
#define VGPU_FALSE                          0
#define VGPU_VERSION_MAJOR                  0
#define VGPU_VERSION_MINOR                  1

    enum {
        VGPU_MAX_BACK_BUFFER_COUNT = 3,
        VGPU_MAX_COLOR_ATTACHMENTS = 8,
        VGPU_MAX_VERTEX_BUFFER_BINDINGS = 4,
        VGPU_MAX_VERTEX_ATTRIBUTES = 16
    };

    typedef enum VgpuResult {
        VGPU_SUCCESS = 0,
        VGPU_NOT_READY = 1,
        VGPU_TIMEOUT = 2,
        VGPU_INCOMPLETE = 3,
        VGPU_ALREADY_INITIALIZED = 4,
        VGPU_ERROR_GENERIC = -1,
        VGPU_ERROR_OUT_OF_HOST_MEMORY = -2,
        VGPU_ERROR_OUT_OF_DEVICE_MEMORY = -3,
        VGPU_ERROR_INITIALIZATION_FAILED = -4,
        VGPU_ERROR_DEVICE_LOST = -5,
    } VgpuResult;

    typedef enum VgpuBackend {
        VGPU_BACKEND_INVALID    = 0,
        VGPU_BACKEND_NULL       = 1,
        VGPU_BACKEND_VULKAN     = 2,
        VGPU_BACKEND_D3D12      = 3,
        VGPU_BACKEND_D3D11      = 4,
        VGPU_BACKEND_OPENGL     = 5,
        VGPU_BACKEND_COUNT      = (VGPU_BACKEND_OPENGL - VGPU_BACKEND_INVALID + 1),
    } VgpuBackend;

    typedef enum VgpuDevicePreference {
        /// Prefer discrete GPU.
        VGPU_DEVICE_PREFERENCE_DISCRETE,
        /// Prefer integrated GPU.
        VGPU_DEVICE_PREFERENCE_INTEGRATED,
        /// No GPU preference.
        VGPU_DEVICE_PREFERENCE_DONT_CARE
    } VgpuDevicePreference;

    typedef enum VgpuSampleCount {
        /// 1 sample (no multi-sampling).
        AGPU_SAMPLE_COUNT1 = 1,
        /// 2 Samples.
        AGPU_SAMPLE_COUNT2 = 2,
        /// 4 Samples.
        AGPU_SAMPLE_COUNT4 = 4,
        /// 8 Samples.
        AGPU_SAMPLE_COUNT8 = 8,
        /// 16 Samples.
        AGPU_SAMPLE_COUNT16 = 16,
        /// 32 Samples.
        AGPU_SAMPLE_COUNT32 = 32,
    } VgpuSampleCount;

    /// Defines pixel format.
    typedef enum VgpuPixelFormat {
        VGPU_PIXEL_FORMAT_UNKNOWN = 0,
        // 8-bit pixel formats
        VGPU_PIXEL_FORMAT_A8_UNORM,
        VGPU_PIXEL_FORMAT_R8_UNORM,
        VGPU_PIXEL_FORMAT_R8_SNORM,
        VGPU_PIXEL_FORMAT_R8_UINT,
        VGPU_PIXEL_FORMAT_R8_SINT,

        // 16-bit pixel formats
        VGPU_PIXEL_FORMAT_R16_UNORM,
        VGPU_PIXEL_FORMAT_R16_SNORM,
        VGPU_PIXEL_FORMAT_R16_UINT,
        VGPU_PIXEL_FORMAT_R16_SINT,
        VGPU_PIXEL_FORMAT_R16_FLOAT,
        VGPU_PIXEL_FORMAT_RG8_UNORM,
        VGPU_PIXEL_FORMAT_RG8_SNORM,
        VGPU_PIXEL_FORMAT_RG8_UINT,
        VGPU_PIXEL_FORMAT_RG8_SINT,

        // Packed 16-bit pixel formats
        VGPU_PIXEL_FORMAT_R5G6B5_UNORM,
        VGPU_PIXEL_FORMAT_RGBA4_UNORM,

        // 32-bit pixel formats
        VGPU_PIXEL_FORMAT_R32_UINT,
        VGPU_PIXEL_FORMAT_R32_SINT,
        VGPU_PIXEL_FORMAT_R32_FLOAT,
        VGPU_PIXEL_FORMAT_RG16_UNORM,
        VGPU_PIXEL_FORMAT_RG16_SNORM,
        VGPU_PIXEL_FORMAT_RG16_UINT,
        VGPU_PIXEL_FORMAT_RG16_SINT,
        VGPU_PIXEL_FORMAT_RG16_FLOAT,
        VGPU_PIXEL_FORMAT_RGBA8_UNORM,
        VGPU_PIXEL_FORMAT_RGBA8_SNORM,
        VGPU_PIXEL_FORMAT_RGBA8_UINT,
        VGPU_PIXEL_FORMAT_RGBA8_SINT,
        VGPU_PIXEL_FORMAT_BGRA8_UNORM,

        // Packed 32-Bit Pixel formats
        VGPU_PIXEL_FORMAT_RGB10A2_UNORM,
        VGPU_PIXEL_FORMAT_RGB10A2_UINT,
        VGPU_PIXEL_FORMAT_RG11B10_FLOAT,
        VGPU_PIXEL_FORMAT_RGB9E5_FLOAT,

        // 64-Bit Pixel Formats
        VGPU_PIXEL_FORMAT_RG32_UINT,
        VGPU_PIXEL_FORMAT_RG32_SINT,
        VGPU_PIXEL_FORMAT_RG32_FLOAT,
        VGPU_PIXEL_FORMAT_RGBA16_UNORM,
        VGPU_PIXEL_FORMAT_RGBA16_SNORM,
        VGPU_PIXEL_FORMAT_RGBA16_UINT,
        VGPU_PIXEL_FORMAT_RGBA16_SINT,
        VGPU_PIXEL_FORMAT_RGBA16_FLOAT,

        // 128-Bit Pixel Formats
        VGPU_PIXEL_FORMAT_RGBA32_UINT,
        VGPU_PIXEL_FORMAT_RGBA32_SINT,
        VGPU_PIXEL_FORMAT_RGBA32_FLOAT,

        // Depth-stencil
        VGPU_PIXEL_FORMAT_D16,
        VGPU_PIXEL_FORMAT_D24,
        VGPU_PIXEL_FORMAT_D24S8,
        VGPU_PIXEL_FORMAT_D32,
        VGPU_PIXEL_FORMAT_D16F,
        VGPU_PIXEL_FORMAT_D24F,
        VGPU_PIXEL_FORMAT_D32F,
        VGPU_PIXEL_FORMAT_D32FS8,
        VGPU_PIXEL_FORMAT_D0S8,

        // Compressed formats
        VGPU_PIXEL_FORMAT_BC1_UNORM,   // DXT1
        VGPU_PIXEL_FORMAT_BC2_UNORM,   // DXT3
        VGPU_PIXEL_FORMAT_BC3_UNORM,   // DXT5
        VGPU_PIXEL_FORMAT_BC4_UNORM,   // RGTC Unsigned Red
        VGPU_PIXEL_FORMAT_BC4_SNORM,   // RGTC Signed Red
        VGPU_PIXEL_FORMAT_BC5_UNORM,   // RGTC Unsigned RG
        VGPU_PIXEL_FORMAT_BC5_SNORM,   // RGTC Signed RG
        VGPU_PIXEL_FORMAT_BC6HS16,
        VGPU_PIXEL_FORMAT_BC6HU16,
        VGPU_PIXEL_FORMAT_BC7_UNORM,

        // Compressed PVRTC Pixel Formats
        VGPU_PIXEL_FORMAT_PVRTC_RGB2,
        VGPU_PIXEL_FORMAT_PVRTC_RGBA2,
        VGPU_PIXEL_FORMAT_PVRTC_RGB4,
        VGPU_PIXEL_FORMAT_PVRTC_RGBA4,

        // Compressed ETC Pixel Formats
        VGPU_PIXEL_FORMAT_ETC2_RGB8,
        VGPU_PIXEL_FORMAT_ETC2_RGB8A1,

        // Compressed ASTC Pixel Formats
        VGPU_PIXEL_FORMAT_ASTC4x4,
        VGPU_PIXEL_FORMAT_ASTC5x5,
        VGPU_PIXEL_FORMAT_ASTC6x6,
        VGPU_PIXEL_FORMAT_ASTC8x5,
        VGPU_PIXEL_FORMAT_ASTC8x6,
        VGPU_PIXEL_FORMAT_ASTC8x8,
        VGPU_PIXEL_FORMAT_ASTC10x10,
        VGPU_PIXEL_FORMAT_ASTC12x12,

        VGPU_PIXEL_FORMAT_COUNT = (VGPU_PIXEL_FORMAT_ASTC12x12 - VGPU_PIXEL_FORMAT_UNKNOWN + 1),
        _VGPU_PIXEL_FORMAT_MAX_ENUM = 0x7FFFFFFF
    } VgpuPixelFormat;

    /// Defines pixel format type.
    typedef enum VgpuPixelFormatType {
        /// Unknown format Type
        VGPU_PIXEL_FORMAT_TYPE_UNKNOWN = 0,
        /// _FLOATing-point formats.
        VGPU_PIXEL_FORMAT_TYPE_FLOAT = 1,
        /// Unsigned normalized formats.
        VGPU_PIXEL_FORMAT_TYPE_UNORM = 2,
        /// Signed normalized formats.
        VGPU_PIXEL_FORMAT_TYPE_SNORM = 3,
        /// Unsigned integer formats.
        VGPU_PIXEL_FORMAT_TYPE_UINT = 4,
        /// Signed integer formats.
        VGPU_PIXEL_FORMAT_TYPE_SINT = 5,

        VGPU_PIXEL_FORMAT_TYPE_COUNT = (VGPU_PIXEL_FORMAT_TYPE_SINT - VGPU_PIXEL_FORMAT_TYPE_UNKNOWN + 1),
        _VGPU_PIXEL_FORMAT_TYPE_MAX_ENUM = 0x7FFFFFFF
    } VgpuPixelFormatType;

    typedef enum VgpuTextureType {
        VGPU_TEXTURE_TYPE_UNKNOWN = 0,
        VGPU_TEXTURE_TYPE_1D = 1,
        VGPU_TEXTURE_TYPE_2D = 2,
        VGPU_TEXTURE_TYPE_3D = 3,
        VGPU_TEXTURE_TYPE_CUBE = 4,
    } VgpuTextureType;

    typedef enum VgpuTextureUsage {
        VGPU_TEXTURE_USAGE_NONE             = 0,
        VGPU_TEXTURE_USAGE_TRANSFER_SRC     = 1 << 0,
        VGPU_TEXTURE_USAGE_TRANSFER_DEST    = 1 << 1,
        VGPU_TEXTURE_USAGE_SAMPLED          = 1 << 2,
        VGPU_TEXTURE_USAGE_STORAGE          = 1 << 3,
        VGPU_TEXTURE_USAGE_RENDER_TARGET    = 1 << 4
    } VgpuTextureUsage;

    typedef enum VgpuBufferUsage {
        AGPU_BUFFER_USAGE_NONE = 0,
        AGPU_BUFFER_USAGE_VERTEX = 1,
        AGPU_BUFFER_USAGE_INDEX = 2,
        AGPU_BUFFER_USAGE_UNIFORM = 4,
        AGPU_BUFFER_USAGE_STORAGE = 8,
        AGPU_BUFFER_USAGE_INDIRECT = 16,
        AGPU_BUFFER_USAGE_DYNAMIC = 32,
        AGPU_BUFFER_USAGE_CPU_ACCESSIBLE = 64,
    } VgpuBufferUsage;

    typedef enum VgpuShaderStageFlagBits {
        VGPU_SHADER_STAGE_NONE = 0,
        VGPU_SHADER_STAGE_VERTEX_BIT = 0x00000001,
        VGPU_SHADER_STAGE_TESS_CONTROL_BIT = 0x00000002,
        VGPU_SHADER_STAGE_TESS_EVAL_BIT = 0x00000004,
        VGPU_SHADER_STAGE_GEOMETRY_BIT = 0x00000008,
        VGPU_SHADER_STAGE_FRAGMENT_BIT = 0x00000010,
        VGPU_SHADER_STAGE_COMPUTE_BIT = 0x00000020,
        VGPU_SHADER_STAGE_ALL_GRAPHICS = 0x0000001F,
        VGPU_SHADER_STAGE_ALL = 0x7FFFFFFF,
    } VgpuShaderStageFlagBits;

    typedef VgpuFlags VgpuShaderStageFlags;

    typedef enum VgpuVertexFormat {
        VGPU_VERTEX_FORMAT_UNKNOWN = 0,
        VGPU_VERTEX_FORMAT_FLOAT = 1,
        VGPU_VERTEX_FORMAT_FLOAT2 = 2,
        VGPU_VERTEX_FORMAT_FLOAT3 = 3,
        VGPU_VERTEX_FORMAT_FLOAT4 = 4,
        VGPU_VERTEX_FORMAT_BYTE4 = 5,
        VGPU_VERTEX_FORMAT_BYTE4N = 6,
        VGPU_VERTEX_FORMAT_UBYTE4 = 7,
        VGPU_VERTEX_FORMAT_UBYTE4N = 8,
        VGPU_VERTEX_FORMAT_SHORT2 = 9,
        VGPU_VERTEX_FORMAT_SHORT2N = 10,
        VGPU_VERTEX_FORMAT_SHORT4 = 11,
        VGPU_VERTEX_FORMAT_SHORT4N = 12,
        VGPU_VERTEX_FORMAT_COUNT = (VGPU_VERTEX_FORMAT_SHORT2N - VGPU_VERTEX_FORMAT_UNKNOWN + 1),
        _VGPU_VERTEX_FORMAT_MAX_ENUM = 0x7FFFFFFF
    } VgpuVertexFormat;

    typedef enum VgpuVertexInputRate {
        VGPU_VERTEX_INPUT_RATE_VERTEX = 0,
        VGPU_VERTEX_INPUT_RATE_INSTANCE = 1,
    } VgpuVertexInputRate;

    typedef enum VgpuPrimitiveTopology {
        VGPU_PRIMITIVE_TOPOLOGY_POINT_LIST = 0,
        VGPU_PRIMITIVE_TOPOLOGY_LINE_LIST = 1,
        VGPU_PRIMITIVE_TOPOLOGY_LINE_STRIP = 2,
        VGPU_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST = 3,
        VGPU_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP = 4,
        VGPU_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY = 5,
        VGPU_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY = 6,
        VGPU_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY = 7,
        VGPU_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY = 8,
        VGPU_PRIMITIVE_TOPOLOGY_PATCH_LIST = 9,
        VGPU_PRIMITIVE_TOPOLOGY_COUNT = (VGPU_PRIMITIVE_TOPOLOGY_PATCH_LIST - VGPU_PRIMITIVE_TOPOLOGY_POINT_LIST + 1),
        _VGPU_PRIMITIVE_TOPOLOGY_MAX_ENUM = 0x7FFFFFFF
    } VgpuPrimitiveTopology;

    typedef enum VgpuIndexType {
        VGPU_INDEX_TYPE_UINT16 = 0,
        VGPU_INDEX_TYPE_UINT32 = 1,
        _VGPU_INDEX_TYPE_MAX_ENUM = 0x7FFFFFFF
    } VgpuIndexType;

    typedef struct VgpuFramebufferAttachment
    {
        VgpuTexture                 texture;
        uint32_t                    mipLevel;
        uint32_t                    slice;
    } VgpuFramebufferAttachment;

    typedef struct VgpuBufferDescriptor {
        VgpuBufferUsage             usage;
        uint64_t                    size;
        uint32_t                    stride;
        const char*                 name;
    } VgpuBufferDescriptor;

    typedef struct VgpuTextureDescriptor {
        VgpuTextureType             type;
        uint32_t                    width;
        uint32_t                    height;
        uint32_t                    depthOrArraySize;
        uint32_t                    mipLevels;
        VgpuPixelFormat             format;
        VgpuTextureUsage            usage;
        VgpuSampleCount             samples;
        VgpuBool32                  sRGB;
    } VgpuTextureDescriptor;

    typedef struct VgpuFramebufferDescriptor {
        VgpuFramebufferAttachment   colorAttachments[VGPU_MAX_COLOR_ATTACHMENTS];
        VgpuFramebufferAttachment   depthStencilAttachment;
    } VgpuFramebufferDescriptor;

    typedef struct VgpuVertexBufferLayoutDescriptor {
        uint32_t                    stride;
        VgpuVertexInputRate         inputRate;
    } VgpuVertexBufferLayoutDescriptor;

    typedef struct VgpuVertexAttributeDescriptor {
        VgpuVertexFormat            format;
        uint32_t                    offset;
        uint32_t                    bufferIndex;
    } VgpuVertexAttributeDescriptor;

    typedef struct VgpuVertexDescriptor {
        VgpuVertexBufferLayoutDescriptor    layouts[VGPU_MAX_VERTEX_BUFFER_BINDINGS];
        VgpuVertexAttributeDescriptor       attributes[VGPU_MAX_VERTEX_ATTRIBUTES];
    } VgpuVertexDescriptor;

    typedef struct VgpuShaderModuleDescriptor {
        VgpuShaderStageFlagBits     stage;
        size_t                      codeSize;
        const uint8_t*              pCode;
        const char*                 source;
        const char*                 entryPoint;
    } VgpuShaderModuleDescriptor;

    typedef struct VgpuShaderStageDescriptor {
        VgpuShaderModule            shaderModule;
        const char*                 entryPoint;
        /*const VgpuSpecializationInfo* specializationInfo;*/
    } VgpuShaderStageDescriptor;

    typedef struct VgpuShaderDescriptor {
        uint32_t                            stageCount;
        const VgpuShaderStageDescriptor*    stages;
    } VgpuShaderDescriptor;

    typedef struct VgpuRenderPipelineDescriptor {
        VgpuShader                  shader;
        VgpuVertexDescriptor        vertexDescriptor;
        VgpuPrimitiveTopology       primitiveTopology;
    } VgpuRenderPipelineDescriptor;

    typedef struct VgpuComputePipelineDescriptor {
        VgpuShader                  shader;
    } VgpuComputePipelineDescriptor;

    typedef struct VgpuSwapchainDescriptor {
        uint32_t                    width;
        uint32_t                    height;
        VgpuBool32                  depthStencil;
        VgpuBool32                  multisampling;
        VgpuBool32                  tripleBuffer;
        VgpuBool32                  vsync;
        /// Native window handle (HWND, ANativeWindow, NSWindow).
        uint64_t                    nativeHandle;
        /// Native display (HINSTANCE, X11Display, WaylandDisplay).
        uint64_t                    nativeDisplay;
    } VgpuSwapchainDescriptor;

    typedef struct VgpuDescriptor {
        VgpuDevicePreference            devicePreference;
        VgpuBool32                      validation;
        /// Main swap chain descriptor or null for headless.
        const VgpuSwapchainDescriptor*  swapchain;
    } VgpuDescriptor;

    typedef struct VgpuViewport {
        float       x;
        float       y;
        float       width;
        float       height;
        float       minDepth;
        float       maxDepth;
    } VgpuViewport;

    typedef struct VgpuRect2D {
        int32_t     x;
        int32_t     y;
        uint32_t    width;
        uint32_t    height;
    } VgpuRect2D;

    /// Get the gpu backend.
    VGPU_API VgpuBackend vgpuGetBackend();

    VGPU_API VgpuResult vgpuInitialize(const char* applicationName, const VgpuDescriptor* descriptor);
    VGPU_API void vgpuShutdown();
    VGPU_API VgpuResult agpuBeginFrame();
    VGPU_API VgpuResult agpuEndFrame();
    VGPU_API VgpuResult vgpuWaitIdle();

    /* Buffer */
    VGPU_API VgpuBuffer agpuCreateBuffer(const VgpuBufferDescriptor* descriptor, const void* initialData);
    VGPU_API VgpuBuffer agpuCreateExternalBuffer(const VgpuBufferDescriptor* descriptor, void* handle);
    VGPU_API void agpuDestroyBuffer(VgpuBuffer buffer);

    /* Texture */
    VGPU_API VgpuTexture agpuCreateTexture(const VgpuTextureDescriptor* descriptor);
    VGPU_API VgpuTexture agpuCreateExternalTexture(const VgpuTextureDescriptor* descriptor, void* handle);
    VGPU_API void agpuDestroyTexture(VgpuTexture texture);

    /* Framebuffer */
    VGPU_API VgpuFramebuffer agpuCreateFramebuffer(const VgpuFramebufferDescriptor* descriptor);
    VGPU_API void agpuDestroyFramebuffer(VgpuFramebuffer framebuffer);

    /* ShaderModule */
    VGPU_API VgpuShaderModule agpuCreateShaderModule(const VgpuShaderModuleDescriptor* descriptor);
    VGPU_API void agpuDestroyShaderModule(VgpuShaderModule shaderModule);

    /* Shader */
    VGPU_API VgpuShader agpuCreateShader(const VgpuShaderDescriptor* descriptor);
    VGPU_API void agpuDestroyShader(VgpuShader shader);

    /* Pipeline */
    VGPU_API VgpuPipeline agpuCreateRenderPipeline(const VgpuRenderPipelineDescriptor* descriptor);
    VGPU_API VgpuPipeline agpuCreateComputePipeline(const VgpuComputePipelineDescriptor* descriptor);
    VGPU_API void agpuDestroyPipeline(VgpuPipeline pipeline);

    /* Command buffer */
    VGPU_API void agpuBeginRenderPass(VgpuFramebuffer framebuffer);
    VGPU_API void agpuEndRenderPass();
    VGPU_API void agpuCmdSetShader(VgpuShader shader);
    VGPU_API void agpuCmdSetVertexBuffer(uint32_t binding, VgpuBuffer buffer, uint64_t offset, VgpuVertexInputRate inputRate);
    VGPU_API void agpuCmdSetIndexBuffer(VgpuBuffer buffer, uint64_t offset, VgpuIndexType indexType);

    VGPU_API void agpuCmdSetViewport(VgpuViewport viewport);
    VGPU_API void agpuCmdSetViewports(uint32_t viewportCount, const VgpuViewport* pViewports);
    VGPU_API void agpuCmdSetScissor(VgpuRect2D scissor);
    VGPU_API void agpuCmdSetScissors(uint32_t scissorCount, const VgpuRect2D* pScissors);

    VGPU_API void CmdSetPrimitiveTopology(VgpuPrimitiveTopology topology);
    VGPU_API void agpuCmdDraw(uint32_t vertexCount, uint32_t firstVertex);
    VGPU_API void agpuCmdDrawIndexed(uint32_t indexCount, uint32_t firstIndex, int32_t vertexOffset);

    /* Helper methods */
    /// Get the number of bits per format
    VGPU_API uint32_t vgpuGetFormatBitsPerPixel(VgpuPixelFormat format);
    VGPU_API uint32_t vgpuGetFormatBlockSize(VgpuPixelFormat format);

    /// Get the format compression ration along the x-axis.
    VGPU_API uint32_t vgpuGetFormatBlockWidth(VgpuPixelFormat format);
    /// Get the format compression ration along the y-axis.
    VGPU_API uint32_t vgpuGetFormatBlockHeight(VgpuPixelFormat format);
    /// Get the format Type.
    VGPU_API VgpuPixelFormatType vgpuGetFormatType(VgpuPixelFormat format);

    /// Check if the format has a depth component.
    VGPU_API VgpuBool32 vgpuIsDepthFormat(VgpuPixelFormat format);
    /// Check if the format has a stencil component.
    VGPU_API VgpuBool32 vgpuIsStencilFormat(VgpuPixelFormat format);
    /// Check if the format has depth or stencil components.
    VGPU_API VgpuBool32 vgpuIsDepthStencilFormat(VgpuPixelFormat format);
    /// Check if the format is a compressed format.
    VGPU_API VgpuBool32 vgpuIsCompressedFormat(VgpuPixelFormat format);
    /// Get format string name.
    VGPU_API const char* vgpuGetFormatName(VgpuPixelFormat format);

#ifdef __cplusplus
}
#endif // __cplusplus