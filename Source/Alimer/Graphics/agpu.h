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

#include "../AlimerConfig.h"

#ifndef AGPU_API
#   define AGPU_API
#endif

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
    AGPU_DEFINE_HANDLE(AgpuShader);
    AGPU_DEFINE_HANDLE(AgpuPipeline);
    AGPU_DEFINE_HANDLE(AgpuCommandBuffer);

#define AGPU_TRUE                           1
#define AGPU_FALSE                          0

    enum {
        AGPU_MAX_BACK_BUFFER_COUNT = 3,
        AGPU_MAX_COLOR_ATTACHMENTS = 8
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

    typedef enum AgpuShaderStage {
        AGPU_SHADER_STAGE_VERTEX    = 0,
        AGPU_SHADER_STAGE_HULL      = 1,
        AGPU_SHADER_STAGE_DOMAIN    = 2,
        AGPU_SHADER_STAGE_GEOMETRY  = 3,
        AGPU_SHADER_STAGE_FRAGMENT  = 4,
        AGPU_SHADER_STAGE_COMPUTE   = 5,
        AGPU_SHADER_STAGE_COUNT     = 6,
    } AgpuShaderStage;

    typedef enum AgpuShaderLanguage {
        AGPU_SHADER_LANGUAGE_DEFAULT    = 0,
        AGPU_SHADER_LANGUAGE_HLSL       = 1,
        AGPU_SHADER_LANGUAGE_GLSL       = 2,
    } AgpuShaderLanguage;

    typedef struct AgpuFramebufferAttachment
    {
        AgpuTexture                 texture;
        uint32_t                    mipLevel;
        uint32_t                    slice;
    } AgpuFramebufferAttachment;

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

    typedef struct AgpuShaderBlob
    {
        uint64_t size;
        uint8_t *data;
    } AgpuShaderBlob;

    typedef struct AgpuShaderDescriptor {
        AgpuShaderStage             stage;
        uint64_t                    codeSize;
        uint8_t*                    pCode;
        const char*                 source;
        const char*                 entryPoint;
        AgpuShaderLanguage          language;
    } AgpuShaderDescriptor;

    typedef struct AgpuRenderPipelineDescriptor {
        AgpuShader                  vertex;
        AgpuShader                  domain;
        AgpuShader                  hull;
        AgpuShader                  geometry;
        AgpuShader                  fragment;
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

    AGPU_API AgpuBackend agpuGetDefaultPlatformBackend();
    AGPU_API AgpuBool32 agpuIsBackendSupported(AgpuBackend backend);
    AGPU_API uint32_t agpuGetAvailableBackendsCount();
    AGPU_API AgpuBackend agpuGetAvailableBackend(uint32_t index);

    AGPU_API AgpuResult agpuInitialize(const AgpuDescriptor* descriptor);
    AGPU_API void agpuShutdown();
    AGPU_API uint64_t agpuFrame();

    AGPU_API AgpuTexture agpuCreateTexture(const AgpuTextureDescriptor* descriptor);
    AGPU_API AgpuTexture agpuCreateExternalTexture(const AgpuTextureDescriptor* descriptor, void* handle);
    AGPU_API void agpuDestroyTexture(AgpuTexture texture);

    AGPU_API AgpuFramebuffer agpuCreateFramebuffer(const AgpuFramebufferDescriptor* descriptor);
    AGPU_API void agpuDestroyFramebuffer(AgpuFramebuffer framebuffer);

    AGPU_API AgpuShader agpuCreateShader(const AgpuShaderDescriptor* descriptor);
    AGPU_API void agpuDestroyShader(AgpuShader shader);

    AGPU_API AgpuPipeline agpuCreateRenderPipeline(const AgpuRenderPipelineDescriptor* descriptor);
    AGPU_API AgpuPipeline agpuCreateComputePipeline(const AgpuComputePipelineDescriptor* descriptor);
    AGPU_API void agpuDestroyPipeline(AgpuPipeline pipeline);

    AGPU_API AgpuBool32 agpuIsDepthFormat(AgpuPixelFormat format);
    AGPU_API AgpuBool32 agpuIsStencilFormat(AgpuPixelFormat format);
    AGPU_API AgpuBool32 agpuIsDepthStencilFormat(AgpuPixelFormat format);
    AGPU_API AgpuBool32 agpuIsCompressed(AgpuPixelFormat format);

#ifdef __cplusplus
}
#endif // __cplusplus


#if !defined(AGPU_D3D11) && !defined(AGPU_DISABLE_D3D11)
#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP || ALIMER_PLATFORM_XBOX_ONE
#   define AGPU_D3D11 1
#endif
#endif

#if !defined(AGPU_D3D12) && !defined(AGPU_DISABLE_D3D12)
#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP || ALIMER_PLATFORM_XBOX_ONE
#   define AGPU_D3D12 1
#endif
#endif

#ifndef AGPU_D3D11
#   define AGPU_D3D11 0
#endif

#ifndef AGPU_D3D12
#   define AGPU_D3D12 0
#endif
