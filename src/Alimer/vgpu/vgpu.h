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

#include "AlimerConfig.h"

#ifndef VGPU_DEFINE_HANDLE
#   define VGPU_DEFINE_HANDLE(object) typedef struct object##_T* object
#endif

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    typedef uint32_t VGpuFlags;
    typedef uint32_t VGpuBool32;

    VGPU_DEFINE_HANDLE(AgpuSwapchain);
    VGPU_DEFINE_HANDLE(VGpuTexture);
    VGPU_DEFINE_HANDLE(AgpuFramebuffer);
    VGPU_DEFINE_HANDLE(AgpuBuffer);
    VGPU_DEFINE_HANDLE(AgpuShaderModule);
    VGPU_DEFINE_HANDLE(AgpuShader);
    VGPU_DEFINE_HANDLE(AgpuPipeline);
    VGPU_DEFINE_HANDLE(AgpuCommandBuffer);

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

    typedef enum VGpuBackend {
        VGPU_BACKEND_INVALID = 0,
        VGPU_BACKEND_NULL = 1,
        VGPU_BACKEND_VULKAN = 2,
        VGPU_BACKEND_D3D11 = 3,
        VGPU_BACKEND_D3D12 = 4,
        VGPU_BACKEND_OPENGL = 5,
        VGPU_BACKEND_COUNT = (VGPU_BACKEND_OPENGL - VGPU_BACKEND_INVALID + 1),
    } VGpuBackend;

    typedef enum VGpuDevicePreference {
        /// Prefer discrete GPU.
        VGPU_DEVICE_PREFERENCE_DISCRETE,
        /// Prefer integrated GPU.
        VGPU_DEVICE_PREFERENCE_INTEGRATED,
        /// No GPU preference.
        VGPU_DEVICE_PREFERENCE_DONT_CARE
    } VGpuDevicePreference;

    typedef enum AgpuSampleCount {
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
    } AgpuSampleCount;

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
        VGPU_PIXEL_FORMAT_MAX_ENUM = 0x7FFFFFFF
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
        VGPU_PIXEL_FORMAT_TYPE_MAX_ENUM = 0x7FFFFFFF
    } VgpuPixelFormatType;

    typedef enum AgpuTextureType {
        AGPU_TEXTURE_TYPE_UNKNOWN = 0,
        AGPU_TEXTURE_TYPE_1D = 1,
        AGPU_TEXTURE_TYPE_2D = 2,
        AGPU_TEXTURE_TYPE_3D = 3,
        AGPU_TEXTURE_TYPE_CUBE = 4,
    } AgpuTextureType;

    typedef enum AgpuTextureUsage {
        AGPU_TEXTURE_USAGE_NONE = 0,
        AGPU_TEXTURE_USAGE_TRANSFER_SRC = 1,
        AGPU_TEXTURE_USAGE_TRANSFER_DEST = 2,
        AGPU_TEXTURE_USAGE_SAMPLED = 4,
        AGPU_TEXTURE_USAGE_STORAGE = 8,
        AGPU_TEXTURE_USAGE_OUTPUT_ATTACHMENT = 16,
        AGPU_TEXTURE_USAGE_PRESENT = 32,
    } AgpuTextureUsage;

    typedef enum AgpuBufferUsage {
        AGPU_BUFFER_USAGE_NONE = 0,
        AGPU_BUFFER_USAGE_VERTEX = 1,
        AGPU_BUFFER_USAGE_INDEX = 2,
        AGPU_BUFFER_USAGE_UNIFORM = 4,
        AGPU_BUFFER_USAGE_STORAGE = 8,
        AGPU_BUFFER_USAGE_INDIRECT = 16,
        AGPU_BUFFER_USAGE_DYNAMIC = 32,
        AGPU_BUFFER_USAGE_CPU_ACCESSIBLE = 64,
    } AgpuBufferUsage;

    typedef enum AgpuShaderStageFlagBits {
        AGPU_SHADER_STAGE_NONE = 0,
        AGPU_SHADER_STAGE_VERTEX_BIT = 0x00000001,
        AGPU_SHADER_STAGE_TESS_CONTROL_BIT = 0x00000002,
        AGPU_SHADER_STAGE_TESS_EVAL_BIT = 0x00000004,
        AGPU_SHADER_STAGE_GEOMETRY_BIT = 0x00000008,
        AGPU_SHADER_STAGE_FRAGMENT_BIT = 0x00000010,
        AGPU_SHADER_STAGE_COMPUTE_BIT = 0x00000020,
        AGPU_SHADER_STAGE_ALL_GRAPHICS = 0x0000001F,
        AGPU_SHADER_STAGE_ALL = 0x7FFFFFFF,
    } AgpuShaderStageFlagBits;

    typedef VGpuFlags AgpuShaderStageFlags;

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
        VGPU_VERTEX_FORMAT_MAX_ENUM = 0x7FFFFFFF
    } VgpuVertexFormat;

    typedef enum VgpuVertexInputRate {
        VGPU_VERTEX_INPUT_RATE_VERTEX = 0,
        VGPU_VERTEX_INPUT_RATE_INSTANCE = 1,
    } VgpuVertexInputRate;

    typedef enum AgpuPrimitiveTopology {
        AGPU_PRIMITIVE_TOPOLOGY_POINT_LIST = 0,
        AGPU_PRIMITIVE_TOPOLOGY_LINE_LIST = 1,
        AGPU_PRIMITIVE_TOPOLOGY_LINE_STRIP = 2,
        AGPU_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST = 3,
        AGPU_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP = 4,
        AGPU_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY = 5,
        AGPU_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY = 6,
        AGPU_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY = 7,
        AGPU_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY = 8,
        AGPU_PRIMITIVE_TOPOLOGY_PATCH_LIST = 9,
        AGPU_PRIMITIVE_TOPOLOGY_COUNT = (AGPU_PRIMITIVE_TOPOLOGY_PATCH_LIST - AGPU_PRIMITIVE_TOPOLOGY_POINT_LIST + 1),
        AGPU_PRIMITIVE_TOPOLOGY_MAX_ENUM = 0x7FFFFFFF
    } AgpuPrimitiveTopology;

    typedef enum AgpuIndexType {
        AGPU_INDEX_TYPE_UINT16 = 0,
        AGPU_INDEX_TYPE_UINT32 = 1,
        AGPU_INDEX_TYPE_COUNT = (AGPU_INDEX_TYPE_UINT32 - AGPU_INDEX_TYPE_UINT16 + 1),
        AGPU_INDEX_TYPE_MAX_ENUM = 0x7FFFFFFF
    } AgpuIndexType;

    typedef struct AgpuFramebufferAttachment
    {
        VGpuTexture                 texture;
        uint32_t                    mipLevel;
        uint32_t                    slice;
    } AgpuFramebufferAttachment;

    typedef struct AgpuBufferDescriptor {
        AgpuBufferUsage             usage;
        uint64_t                    size;
        uint32_t                    stride;
        const char*                 name;
    } AgpuBufferDescriptor;

    typedef struct VgpuTextureDescriptor {
        AgpuTextureType             type;
        uint32_t                    width;
        uint32_t                    height;
        uint32_t                    depthOrArraySize;
        uint32_t                    mipLevels;
        VgpuPixelFormat             format;
        AgpuTextureUsage            usage;
        AgpuSampleCount             samples;
        VGpuBool32                  sRGB;
    } VgpuTextureDescriptor;

    typedef struct AgpuFramebufferDescriptor {
        AgpuFramebufferAttachment   colorAttachments[VGPU_MAX_COLOR_ATTACHMENTS];
        AgpuFramebufferAttachment   depthStencilAttachment;
    } AgpuFramebufferDescriptor;

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

    typedef struct AgpuShaderModuleDescriptor {
        AgpuShaderStageFlagBits     stage;
        size_t                      codeSize;
        const uint8_t*              pCode;
        const char*                 source;
        const char*                 entryPoint;
    } AgpuShaderModuleDescriptor;

    typedef struct AgpuShaderStageDescriptor {
        AgpuShaderModule            shaderModule;
        const char*                 entryPoint;
        /*const AgpuSpecializationInfo* specializationInfo;*/
    } AgpuShaderStageDescriptor;

    typedef struct AgpuShaderDescriptor {
        uint32_t                            stageCount;
        const AgpuShaderStageDescriptor*    stages;
    } AgpuShaderDescriptor;

    typedef struct AgpuRenderPipelineDescriptor {
        AgpuShader                  shader;
        VgpuVertexDescriptor        vertexDescriptor;
        AgpuPrimitiveTopology       primitiveTopology;
    } AgpuRenderPipelineDescriptor;

    typedef struct AgpuComputePipelineDescriptor {
        AgpuShader                  shader;
    } AgpuComputePipelineDescriptor;

    typedef struct VgpuSwapchainDescriptor {
        uint32_t                    width;
        uint32_t                    height;
        VGpuBool32                  depthStencil;
        VGpuBool32                  multisampling;
        VGpuBool32                  tripleBuffer;
        VGpuBool32                  vsync;
        /// Native window handle (HWND, ANativeWindow, NSWindow).
        uint64_t                    nativeHandle;
        /// Native display (HINSTANCE, X11Display, WaylandDisplay).
        uint64_t                    nativeDisplay;
    } VgpuSwapchainDescriptor;

    typedef struct VGpuDescriptor {
        VGpuDevicePreference            devicePreference;
        VGpuBool32                      validation;
        /// Main swap chain descriptor or null for headless.
        const VgpuSwapchainDescriptor*  swapchain;
    } VGpuDescriptor;

    typedef struct AgpuViewport {
        float       x;
        float       y;
        float       width;
        float       height;
        float       minDepth;
        float       maxDepth;
    } AgpuViewport;

    typedef struct AgpuRect2D {
        int32_t     x;
        int32_t     y;
        uint32_t    width;
        uint32_t    height;
    } AgpuRect2D;

    /// Get the gpu backend.
    ALIMER_API VGpuBackend vgpuGetBackend();

    ALIMER_API VgpuResult vgpuInitialize(const char* applicationName, const VGpuDescriptor* descriptor);
    ALIMER_API void agpuShutdown();
    ALIMER_API uint64_t agpuFrame();

    /* Buffer */
    ALIMER_API AgpuBuffer agpuCreateBuffer(const AgpuBufferDescriptor* descriptor, const void* initialData);
    ALIMER_API AgpuBuffer agpuCreateExternalBuffer(const AgpuBufferDescriptor* descriptor, void* handle);
    ALIMER_API void agpuDestroyBuffer(AgpuBuffer buffer);

    /* Texture */
    ALIMER_API VGpuTexture agpuCreateTexture(const VgpuTextureDescriptor* descriptor);
    ALIMER_API VGpuTexture agpuCreateExternalTexture(const VgpuTextureDescriptor* descriptor, void* handle);
    ALIMER_API void agpuDestroyTexture(VGpuTexture texture);

    /* Framebuffer */
    ALIMER_API AgpuFramebuffer agpuCreateFramebuffer(const AgpuFramebufferDescriptor* descriptor);
    ALIMER_API void agpuDestroyFramebuffer(AgpuFramebuffer framebuffer);

    /* ShaderModule */
    ALIMER_API AgpuShaderModule agpuCreateShaderModule(const AgpuShaderModuleDescriptor* descriptor);
    ALIMER_API void agpuDestroyShaderModule(AgpuShaderModule shaderModule);

    /* Shader */
    ALIMER_API AgpuShader agpuCreateShader(const AgpuShaderDescriptor* descriptor);
    ALIMER_API void agpuDestroyShader(AgpuShader shader);

    /* Pipeline */
    ALIMER_API AgpuPipeline agpuCreateRenderPipeline(const AgpuRenderPipelineDescriptor* descriptor);
    ALIMER_API AgpuPipeline agpuCreateComputePipeline(const AgpuComputePipelineDescriptor* descriptor);
    ALIMER_API void agpuDestroyPipeline(AgpuPipeline pipeline);

    /* Command buffer */
    ALIMER_API void agpuBeginRenderPass(AgpuFramebuffer framebuffer);
    ALIMER_API void agpuEndRenderPass();
    ALIMER_API void agpuCmdSetShader(AgpuShader shader);
    ALIMER_API void agpuCmdSetVertexBuffer(uint32_t binding, AgpuBuffer buffer, uint64_t offset, VgpuVertexInputRate inputRate);
    ALIMER_API void agpuCmdSetIndexBuffer(AgpuBuffer buffer, uint64_t offset, AgpuIndexType indexType);

    ALIMER_API void agpuCmdSetViewport(AgpuViewport viewport);
    ALIMER_API void agpuCmdSetViewports(uint32_t viewportCount, const AgpuViewport* pViewports);
    ALIMER_API void agpuCmdSetScissor(AgpuRect2D scissor);
    ALIMER_API void agpuCmdSetScissors(uint32_t scissorCount, const AgpuRect2D* pScissors);

    ALIMER_API void CmdSetPrimitiveTopology(AgpuPrimitiveTopology topology);
    ALIMER_API void agpuCmdDraw(uint32_t vertexCount, uint32_t firstVertex);
    ALIMER_API void agpuCmdDrawIndexed(uint32_t indexCount, uint32_t firstIndex, int32_t vertexOffset);

    /* Helper methods */
    /// Get the number of bits per format
    ALIMER_API uint32_t vgpuGetFormatBitsPerPixel(VgpuPixelFormat format);
    ALIMER_API uint32_t vgpuGetFormatBlockSize(VgpuPixelFormat format);

    /// Get the format compression ration along the x-axis.
    ALIMER_API uint32_t vgpuGetFormatBlockWidth(VgpuPixelFormat format);
    /// Get the format compression ration along the y-axis.
    ALIMER_API uint32_t vgpuGetFormatBlockHeight(VgpuPixelFormat format);
    /// Get the format Type.
    ALIMER_API VgpuPixelFormatType vgpuGetFormatType(VgpuPixelFormat format);

    /// Check if the format has a depth component.
    ALIMER_API VGpuBool32 vgpuIsDepthFormat(VgpuPixelFormat format);
    /// Check if the format has a stencil component.
    ALIMER_API VGpuBool32 vgpuIsStencilFormat(VgpuPixelFormat format);
    /// Check if the format has depth or stencil components.
    ALIMER_API VGpuBool32 vgpuIsDepthStencilFormat(VgpuPixelFormat format);
    /// Check if the format is a compressed format.
    ALIMER_API VGpuBool32 vgpuIsCompressedFormat(VgpuPixelFormat format);
    /// Get format string name.
    ALIMER_API const char* vgpuGetFormatName(VgpuPixelFormat format);

#ifdef __cplusplus
}
#endif // __cplusplus
