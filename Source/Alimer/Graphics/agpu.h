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

#pragma once

#include "AlimerConfig.h"

#ifndef AGPU_DEFINE_HANDLE
#   define AGPU_DEFINE_HANDLE(object) typedef struct object##_T* object
#endif

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    typedef uint32_t AgpuFlags;
    typedef uint32_t AgpuBool32;

    AGPU_DEFINE_HANDLE(AgpuSwapchain);
    AGPU_DEFINE_HANDLE(AgpuTexture);
    AGPU_DEFINE_HANDLE(AgpuFramebuffer);
    AGPU_DEFINE_HANDLE(AgpuBuffer);
    AGPU_DEFINE_HANDLE(AgpuShaderModule);
    AGPU_DEFINE_HANDLE(AgpuShader);
    AGPU_DEFINE_HANDLE(AgpuPipeline);
    AGPU_DEFINE_HANDLE(AgpuCommandBuffer);

#define AGPU_TRUE                           1
#define AGPU_FALSE                          0

    enum {
        AGPU_MAX_BACK_BUFFER_COUNT          = 3,
        AGPU_MAX_COLOR_ATTACHMENTS          = 8,
        AGPU_MAX_VERTEX_BUFFER_BINDINGS     = 4,
        AGPU_MAX_VERTEX_ATTRIBUTES          = 16
    };

    typedef enum AgpuResult {
        AGPU_OK = 0,
        AGPU_ERROR = -1,
        AGPU_ALREADY_INITIALIZED = -2,
        AGPU_NOT_READY = -3
    } AgpuResult;

    typedef enum AgpuBackend {
        AGPU_BACKEND_DEFAULT    = 0,
        AGPU_BACKEND_EMPTY      = 1,
        AGPU_BACKEND_VULKAN     = 2,
        AGPU_BACKEND_D3D11      = 3,
        AGPU_BACKEND_D3D12      = 4,
        AGPU_BACKEND_METAL      = 5,
        AGPU_BACKEND_OPENGL     = 6,
        AGPU_BACKEND_COUNT      = (AGPU_BACKEND_OPENGL - AGPU_BACKEND_DEFAULT + 1),
    } AgpuBackend;

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
    typedef enum AgpuPixelFormat {
        AGPU_PIXEL_FORMAT_UNKNOWN           = 0,
        AGPU_PIXEL_FORMAT_R8_UNORM          = 1,
        AGPU_PIXEL_FORMAT_R8_SNORM          = 2,
        AGPU_PIXEL_FORMAT_R16_UNORM         = 3,
        AGPU_PIXEL_FORMAT_R16_SNORM         = 4,
        AGPU_PIXEL_FORMAT_RG8_UNORM         = 5,
        AGPU_PIXEL_FORMAT_RG8_SNORM         = 6,
        AGPU_PIXEL_FORMAT_RG16_UNORM        = 7,
        AGPU_PIXEL_FORMAT_RG16_SNORM        = 8,
        AGPU_PIXEL_FORMAT_RGB16_UNORM       = 9,
        AGPU_PIXEL_FORMAT_RGB16_SNORM       = 10,

        AGPU_PIXEL_FORMAT_RGBA8_UNORM       = 11,
        AGPU_PIXEL_FORMAT_RGBA8_UNORM_SRGB  = 12,
        AGPU_PIXEL_FORMAT_RGBA8_SNORM       = 13,

        AGPU_PIXEL_FORMAT_BGRA8_UNORM       = 14,
        AGPU_PIXEL_FORMAT_BGRA8_UNORM_SRGB  = 15,

        // Depth-stencil
        AGPU_PIXEL_FORMAT_D32_FLOAT         = 16,
        AGPU_PIXEL_FORMAT_D16_UNORM         = 17,
        AGPU_PIXEL_FORMAT_D24_UNORM_S8      = 18,
        AGPU_PIXEL_FORMAT_D32_FLOAT_S8      = 19,

        // Compressed formats
        AGPU_PIXEL_FORMAT_BC1_UNORM         = 20,   // DXT1
        AGPU_PIXEL_FORMAT_BC1_UNORM_SRGB    = 21,
        AGPU_PIXEL_FORMAT_BC2_UNORM         = 22,   // DXT3
        AGPU_PIXEL_FORMAT_BC2_UNORM_SRGB    = 23,
        AGPU_PIXEL_FORMAT_BC3_UNORM         = 24,   // DXT5
        AGPU_PIXEL_FORMAT_BC3_UNORM_SRGB    = 25,
        AGPU_PIXEL_FORMAT_BC4_UNORM         = 26,   // RGTC Unsigned Red
        AGPU_PIXEL_FORMAT_BC4_SNORM         = 27,   // RGTC Signed Red
        AGPU_PIXEL_FORMAT_BC5_UNORM         = 28,   // RGTC Unsigned RG
        AGPU_PIXEL_FORMAT_BC5_SNORM         = 29,   // RGTC Signed RG

        AGPU_PIXEL_FORMAT_BC6HS16           = 30,
        AGPU_PIXEL_FORMAT_BC6HU16           = 31,
        AGPU_PIXEL_FORMAT_BC7_UNORM         = 32,
        AGPU_PIXEL_FORMAT_BC7_UNORM_SRGB    = 33,

        AGPU_PIXEL_FORMAT_COUNT             = (AGPU_PIXEL_FORMAT_BC7_UNORM_SRGB - AGPU_PIXEL_FORMAT_UNKNOWN + 1),
    } AgpuPixelFormat;

    typedef enum AgpuTextureType {
        AGPU_TEXTURE_TYPE_UNKNOWN   = 0,
        AGPU_TEXTURE_TYPE_1D        = 1,
        AGPU_TEXTURE_TYPE_2D        = 2,
        AGPU_TEXTURE_TYPE_3D        = 3,
        AGPU_TEXTURE_TYPE_CUBE      = 4,
    } AgpuTextureType;

    typedef enum AgpuTextureUsage {
        AGPU_TEXTURE_USAGE_NONE                 = 0,
        AGPU_TEXTURE_USAGE_TRANSFER_SRC         = 1,
        AGPU_TEXTURE_USAGE_TRANSFER_DEST        = 2,
        AGPU_TEXTURE_USAGE_SAMPLED              = 4,
        AGPU_TEXTURE_USAGE_STORAGE              = 8,
        AGPU_TEXTURE_USAGE_OUTPUT_ATTACHMENT    = 16,
        AGPU_TEXTURE_USAGE_PRESENT              = 32,
    } AgpuTextureUsage;

    typedef enum AgpuBufferUsage {
        AGPU_BUFFER_USAGE_NONE              = 0,
        AGPU_BUFFER_USAGE_VERTEX            = 1,
        AGPU_BUFFER_USAGE_INDEX             = 2,
        AGPU_BUFFER_USAGE_UNIFORM           = 4,
        AGPU_BUFFER_USAGE_STORAGE           = 8,
        AGPU_BUFFER_USAGE_INDIRECT          = 16,
        AGPU_BUFFER_USAGE_DYNAMIC           = 32,
        AGPU_BUFFER_USAGE_CPU_ACCESSIBLE    = 64,
    } AgpuBufferUsage;

    typedef enum AgpuShaderStageFlagBits {
        AGPU_SHADER_STAGE_NONE              = 0,
        AGPU_SHADER_STAGE_VERTEX_BIT        = 0x00000001,
        AGPU_SHADER_STAGE_TESS_CONTROL_BIT  = 0x00000002,
        AGPU_SHADER_STAGE_TESS_EVAL_BIT     = 0x00000004,
        AGPU_SHADER_STAGE_GEOMETRY_BIT      = 0x00000008,
        AGPU_SHADER_STAGE_FRAGMENT_BIT      = 0x00000010,
        AGPU_SHADER_STAGE_COMPUTE_BIT       = 0x00000020,
        AGPU_SHADER_STAGE_ALL_GRAPHICS      = 0x0000001F,
        AGPU_SHADER_STAGE_ALL               = 0x7FFFFFFF,
    } AgpuShaderStageFlagBits;

    typedef AgpuFlags AgpuShaderStageFlags;

    typedef enum AgpuVertexFormat {
        AGPU_VERTEX_FORMAT_UNKNOWN      = 0,
        AGPU_VERTEX_FORMAT_FLOAT        = 1,
        AGPU_VERTEX_FORMAT_FLOAT2       = 2,
        AGPU_VERTEX_FORMAT_FLOAT3       = 3,
        AGPU_VERTEX_FORMAT_FLOAT4       = 4,
        AGPU_VERTEX_FORMAT_BYTE4        = 5,
        AGPU_VERTEX_FORMAT_BYTE4N       = 6,
        AGPU_VERTEX_FORMAT_UBYTE4       = 7,
        AGPU_VERTEX_FORMAT_UBYTE4N      = 8,
        AGPU_VERTEX_FORMAT_SHORT2       = 9,
        AGPU_VERTEX_FORMAT_SHORT2N      = 10,
        AGPU_VERTEX_FORMAT_SHORT4       = 11,
        AGPU_VERTEX_FORMAT_SHORT4N      = 12,
        AGPU_VERTEX_FORMAT_COUNT        = (AGPU_VERTEX_FORMAT_SHORT2N - AGPU_VERTEX_FORMAT_UNKNOWN + 1),
        AGPU_VERTEX_FORMAT_MAX_ENUM     = 0x7FFFFFFF
    } AgpuVertexFormat;

    typedef enum AgpuVertexInputRate {
        AGPU_VERTEX_INPUT_RATE_VERTEX = 0,
        AGPU_VERTEX_INPUT_RATE_INSTANCE = 1,
    } AgpuVertexInputRate;

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
        AgpuTexture                 texture;
        uint32_t                    mipLevel;
        uint32_t                    slice;
    } AgpuFramebufferAttachment;

    typedef struct AgpuBufferDescriptor {
        AgpuBufferUsage             usage;
        uint64_t                    size;
        uint32_t                    stride;
        const char*                 name;
    } AgpuBufferDescriptor;

    typedef struct AgpuTextureDescriptor {
        AgpuTextureType             type;
        uint32_t                    width;
        uint32_t                    height;
        uint32_t                    depthOrArraySize;
        uint32_t                    mipLevels;
        AgpuPixelFormat             format;
        AgpuTextureUsage            usage;
        AgpuSampleCount             samples;
    } AgpuTextureDescriptor;

    typedef struct AgpuFramebufferDescriptor {
        AgpuFramebufferAttachment   colorAttachments[AGPU_MAX_COLOR_ATTACHMENTS];
        AgpuFramebufferAttachment   depthStencilAttachment;
    } AgpuFramebufferDescriptor;

    typedef struct AgpuVertexBufferLayoutDescriptor {
        uint32_t                    stride;
        AgpuVertexInputRate         inputRate;
    } AgpuVertexBufferLayoutDescriptor;

    typedef struct AgpuVertexAttributeDescriptor {
        AgpuVertexFormat            format;
        uint32_t                    offset;
        uint32_t                    bufferIndex;
    } AgpuVertexAttributeDescriptor;

    typedef struct AgpuVertexDescriptor {
        AgpuVertexBufferLayoutDescriptor    layouts[AGPU_MAX_VERTEX_BUFFER_BINDINGS];
        AgpuVertexAttributeDescriptor       attributes[AGPU_MAX_VERTEX_ATTRIBUTES];
    } AgpuVertexDescriptor;

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
        AgpuVertexDescriptor        vertexDescriptor;
        AgpuPrimitiveTopology       primitiveTopology;
    } AgpuRenderPipelineDescriptor;

    typedef struct AgpuComputePipelineDescriptor {
        AgpuShader                  shader;
    } AgpuComputePipelineDescriptor;

    typedef struct AgpuSwapchainDescriptor {
        /// Native connection, display or instance type.
        void*                       display;
        /// Native window handle.
        void*                       windowHandle;
        /// Preferred color format.
        AgpuPixelFormat             preferredColorFormat;
        /// Preferred depth stencil format.
        AgpuPixelFormat             preferredDepthStencilFormat;
        uint32_t                    width;
        uint32_t                    height;
        uint32_t                    bufferCount;
        /// Preferred samples.
        AgpuSampleCount             preferredSamples;
    } AgpuSwapchainDescriptor;

    typedef struct AgpuDescriptor {
        AgpuBackend                 preferredBackend;
        AgpuBool32                  validation;
        AgpuBool32                  headless;
        AgpuSwapchainDescriptor     swapchain;
    } AgpuDescriptor;

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

    ALIMER_API AgpuBackend agpuGetDefaultPlatformBackend();
    ALIMER_API AgpuBool32 agpuIsBackendSupported(AgpuBackend backend);
    ALIMER_API uint32_t agpuGetAvailableBackendsCount();
    ALIMER_API AgpuBackend agpuGetAvailableBackend(uint32_t index);

    ALIMER_API AgpuResult agpuInitialize(const AgpuDescriptor* descriptor);
    ALIMER_API void agpuShutdown();
    ALIMER_API uint64_t agpuFrame();

    /* Buffer */
    ALIMER_API AgpuBuffer agpuCreateBuffer(const AgpuBufferDescriptor* descriptor, const void* initialData);
    ALIMER_API AgpuBuffer agpuCreateExternalBuffer(const AgpuBufferDescriptor* descriptor, void* handle);
    ALIMER_API void agpuDestroyBuffer(AgpuBuffer buffer);

    /* Texture */
    ALIMER_API AgpuTexture agpuCreateTexture(const AgpuTextureDescriptor* descriptor);
    ALIMER_API AgpuTexture agpuCreateExternalTexture(const AgpuTextureDescriptor* descriptor, void* handle);
    ALIMER_API void agpuDestroyTexture(AgpuTexture texture);

    /* Framebuffer */
    ALIMER_API AgpuFramebuffer agpuCreateFramebuffer(const AgpuFramebufferDescriptor* descriptor);
    ALIMER_API void agpuDestroyFramebuffer(AgpuFramebuffer framebuffer);

    /* ShaderModule */
    ALIMER_API AgpuShaderModule agpuCreateShaderModule(const AgpuShaderModuleDescriptor* descriptor);
    ALIMER_API void agpuDestroyShaderModule(AgpuShaderModule shaderModule);
    ALIMER_API AgpuShaderStageFlagBits agpuGetShaderModuleState(AgpuShaderModule shaderModule);

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
    ALIMER_API void agpuCmdSetVertexBuffer(uint32_t binding, AgpuBuffer buffer, uint64_t offset, AgpuVertexInputRate inputRate);
    ALIMER_API void agpuCmdSetIndexBuffer(AgpuBuffer buffer, uint64_t offset, AgpuIndexType indexType);

    ALIMER_API void agpuCmdSetViewport(AgpuViewport viewport);
    ALIMER_API void agpuCmdSetViewports(uint32_t viewportCount, const AgpuViewport* pViewports);
    ALIMER_API void agpuCmdSetScissor(AgpuRect2D scissor);
    ALIMER_API void agpuCmdSetScissors(uint32_t scissorCount, const AgpuRect2D* pScissors);

    ALIMER_API void CmdSetPrimitiveTopology(AgpuPrimitiveTopology topology);
    ALIMER_API void agpuCmdDraw(uint32_t vertexCount, uint32_t firstVertex);
    ALIMER_API void agpuCmdDrawIndexed(uint32_t indexCount, uint32_t firstIndex, int32_t vertexOffset);

    /* Helper methods */
    ALIMER_API uint32_t agpuGetTextureWidth(AgpuTexture texture);
    ALIMER_API uint32_t agpuGetTextureLevelWidth(AgpuTexture texture, uint32_t mipLevel);
    ALIMER_API uint32_t agpuGetTextureHeight(AgpuTexture texture);
    ALIMER_API uint32_t agpuGetTextureLevelHeight(AgpuTexture texture, uint32_t mipLevel);
    ALIMER_API uint32_t agpuGetTextureDepth(AgpuTexture texture);
    ALIMER_API uint32_t agpuGetTextureLevelDepth(AgpuTexture texture, uint32_t mipLevel);

    ALIMER_API AgpuBool32 agpuIsDepthFormat(AgpuPixelFormat format);
    ALIMER_API AgpuBool32 agpuIsStencilFormat(AgpuPixelFormat format);
    ALIMER_API AgpuBool32 agpuIsDepthStencilFormat(AgpuPixelFormat format);
    ALIMER_API AgpuBool32 agpuIsCompressed(AgpuPixelFormat format);

#ifdef __cplusplus
}
#endif // __cplusplus
