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

#define VGPU_INCLUDED (1)
#include <stdint.h>
#include <stdbool.h>

#ifndef VGPU_BUILD_SHARED
#   define VGPU_BUILD_SHARED 0
#endif

#if VGPU_BUILD_SHARED
#   if defined(_MSC_VER)
#       ifdef VGPU_EXPORTS
#           define _VGPU_API_DECL __declspec(dllexport)
#       else
#           define _VGPU_API_DECL __declspec(dllimport)
#       endif
#   else
#       define _VGPU_API_DECL __attribute__((visibility("default")))
#   endif
#else
#   define _VGPU_API_DECL
#endif

#ifdef __cplusplus
#    define VGPU_API extern "C" _VGPU_API_DECL
#else
#    define VGPU_API extern _VGPU_API_DECL
#endif

#ifndef VGPU_DEFINE_HANDLE
#   define VGPU_DEFINE_HANDLE(object) typedef struct object##_T* object
#endif

#define VGPU_VERSION_MAJOR 0
#define VGPU_VERSION_MINOR 1
#define VGPU_VERSION_PATCH 0

#if defined(_WIN32)
#   if !defined(NOMINMAX)
#       define NOMINMAX
#   endif
#   if !defined(WIN32_LEAN_AND_MEAN)
#       define WIN32_LEAN_AND_MEAN
#   endif
#   include <Windows.h>
#elif defined(__linux__)
#   include <X11/Xlib-xcb.h>
#endif

typedef uint32_t VgpuFlags;
typedef uint32_t VgpuBool32;
VGPU_DEFINE_HANDLE(VGpuTexture);
VGPU_DEFINE_HANDLE(VGpuFramebuffer);
VGPU_DEFINE_HANDLE(VGpuBuffer);
VGPU_DEFINE_HANDLE(VGpuShader);
VGPU_DEFINE_HANDLE(VGpuCommandBuffer);

enum {
    VGPU_MAX_COLOR_ATTACHMENTS = 8u,
    VGPU_MAX_VERTEX_BUFFER_BINDINGS = 4u,
    VGPU_MAX_VERTEX_ATTRIBUTES = 16u
};

typedef enum vgpu_log_type {
    vgpu_log_type_info = 0,
    vgpu_log_type_warn,
    vgpu_log_type_debug,
    vgpu_log_type_error
} vgpu_log_type;

typedef void(*vgpu_log_fn)(void *userdata, vgpu_log_type type, const char *message);

typedef enum VGpuResult {
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
    VGPU_ERROR_TOO_MANY_OBJECTS = -6,
    VGPU_ERROR_SWAPCHAIN_CREATION_FAILED = -7,
    VGPU_ERROR_BEGIN_FRAME_FAILED = -8,
    VGPU_ERROR_END_FRAME_FAILED = -9,
    VGPU_ERROR_COMMAND_BUFFER_ALREADY_RECORDING = -10,
    VGPU_ERROR_COMMAND_BUFFER_NOT_RECORDING = -11,
} VGpuResult;

typedef enum VGpuBackend {
    VGPU_BACKEND_INVALID = 0,
    VGPU_BACKEND_NULL,
    VGPU_BACKEND_VULKAN,
    VGPU_BACKEND_D3D12,
    VGPU_BACKEND_D3D11,
    VGPU_BACKEND_OPENGL,
    VGPU_BACKEND_COUNT
} VGpuBackend;

typedef enum VGpuDevicePreference {
    /// No GPU preference.
    VGPU_DEVICE_PREFERENCE_DONT_CARE,
    /// Prefer integrated GPU.
    VGPU_DEVICE_PREFERENCE_LOW_POWER,
    /// Prefer high performance/discrete GPU.
    VGPU_DEVICE_PREFERENCE_HIGH_PERFORMANCE,
} VGpuDevicePreference;

typedef enum VGpuFeature {
    VGPU_FEATURE_BLEND_INDEPENDENT = 0,
    VGPU_FEATURE_COMPUTE_SHADER,
    VGPU_FEATURE_GEOMETRY_SHADER,
    VGPU_FEATURE_TESSELLATION_SHADER,
    VGPU_FEATURE_STORAGE_BUFFERS,
    VGPU_FEATURE_MULTI_VIEWPORT,
    VGPU_FEATURE_INDEX_UINT32,
    VGPU_FEATURE_DRAW_INDIRECT,
    VGPU_FEATURE_FILL_MODE_NON_SOLID,
    VGPU_FEATURE_SAMPLER_ANISOTROPY,
    VGPU_FEATURE_TEXTURE_COMPRESSION_BC,
    VGPU_FEATURE_TEXTURE_COMPRESSION_PVRTC,
    VGPU_FEATURE_TEXTURE_COMPRESSION_ETC2,
    VGPU_FEATURE_TEXTURE_COMPRESSION_ATC,
    VGPU_FEATURE_TEXTURE_COMPRESSION_ASTC,
    VGPU_FEATURE_TEXTURE_3D,
    VGPU_FEATURE_TEXTURE_2D_ARRAY,
    VGPU_FEATURE_TEXTURE_CUBE_ARRAY,
    VGPU_FEATURE_RAYTRACING,
} VGpuFeature;

typedef enum VgpuSampleCount {
    /// 1 sample (no multi-sampling).
    VGPU_SAMPLE_COUNT1 = 1,
    /// 2 Samples.
    VGPU_SAMPLE_COUNT2 = 2,
    /// 4 Samples.
    VGPU_SAMPLE_COUNT4 = 4,
    /// 8 Samples.
    VGPU_SAMPLE_COUNT8 = 8,
    /// 16 Samples.
    VGPU_SAMPLE_COUNT16 = 16,
    /// 32 Samples.
    VGPU_SAMPLE_COUNT32 = 32,
    /// 64 Samples.
    VGPU_SAMPLE_COUNT64 = 64,
} VgpuSampleCount;

/// Defines pixel format.
typedef enum VGpuPixelFormat {
    VGPU_PIXEL_FORMAT_UNDEFINED = 0,
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
    VGPU_PIXEL_FORMAT_RGBA8_UNORM_SRGB,
    VGPU_PIXEL_FORMAT_RGBA8_SNORM,
    VGPU_PIXEL_FORMAT_RGBA8_UINT,
    VGPU_PIXEL_FORMAT_RGBA8_SINT,
    VGPU_PIXEL_FORMAT_BGRA8_UNORM,
    VGPU_PIXEL_FORMAT_BGRA8_UNORM_SRGB,

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
    VGPU_PIXEL_FORMAT_D16_UNORM,
    VGPU_PIXEL_FORMAT_D32_FLOAT,
    VGPU_PIXEL_FORMAT_D24_UNORM_S8_UINT,
    VGPU_PIXEL_FORMAT_D32_FLOAT_S8_UINT,
    VGPU_PIXEL_FORMAT_S8,

    // Compressed formats
    VGPU_PIXEL_FORMAT_BC1_UNORM,        // DXT1
    VGPU_PIXEL_FORMAT_BC1_UNORM_SRGB,   // DXT1
    VGPU_PIXEL_FORMAT_BC2_UNORM,        // DXT3
    VGPU_PIXEL_FORMAT_BC2_UNORM_SRGB,   // DXT3
    VGPU_PIXEL_FORMAT_BC3_UNORM,        // DXT5
    VGPU_PIXEL_FORMAT_BC3_UNORM_SRGB,   // DXT5
    VGPU_PIXEL_FORMAT_BC4_UNORM,        // RGTC Unsigned Red
    VGPU_PIXEL_FORMAT_BC4_SNORM,        // RGTC Signed Red
    VGPU_PIXEL_FORMAT_BC5_UNORM,        // RGTC Unsigned RG
    VGPU_PIXEL_FORMAT_BC5_SNORM,        // RGTC Signed RG
    VGPU_PIXEL_FORMAT_BC6HS16,
    VGPU_PIXEL_FORMAT_BC6HU16,
    VGPU_PIXEL_FORMAT_BC7_UNORM,
    VGPU_PIXEL_FORMAT_BC7_UNORM_SRGB,

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
    /// Pixel format count.
    VGPU_PIXEL_FORMAT_COUNT
} VGpuPixelFormat;

/// Defines pixel format type.
typedef enum VGpuPixelFormatType {
    /// Unknown format Type
    VGPU_PIXEL_FORMAT_TYPE_UNKNOWN = 0,
    /// _FLOATing-point formats.
    VGPU_PIXEL_FORMAT_TYPE_FLOAT,
    /// Unsigned normalized formats.
    VGPU_PIXEL_FORMAT_TYPE_UNORM,
    /// Unsigned normalized SRGB formats
    VGPU_PIXEL_FORMAT_TYPE_UNORM_SRGB,
    /// Signed normalized formats.
    VGPU_PIXEL_FORMAT_TYPE_SNORM,
    /// Unsigned integer formats.
    VGPU_PIXEL_FORMAT_TYPE_UINT,
    /// Signed integer formats.
    VGPU_PIXEL_FORMAT_TYPE_SINT,
    /// PixelFormat type count.
    VGPU_PIXEL_FORMAT_TYPE_COUNT
} VGpuPixelFormatType;

typedef enum VGpuTextureType {
    VGPU_TEXTURE_TYPE_2D = 0,
    VGPU_TEXTURE_TYPE_3D,
    VGPU_TEXTURE_TYPE_CUBE,
    VGPU_TEXTURE_TYPE_COUNT,
} VGpuTextureType;

typedef enum VGpuTextureUsageFlagBits {
    VGPU_TEXTURE_USAGE_NONE = 0,
    VGPU_TEXTURE_USAGE_SHADER_READ = 1 << 0,
    VGPU_TEXTURE_USAGE_SHADER_WRITE = 1 << 1,
    VGPU_TEXTURE_USAGE_RENDER_TARGET = 1 << 2
} VGpuTextureUsageFlagBits;
typedef VgpuFlags VGpuTextureUsageFlags;

typedef enum VGpuAttachmentLoadOp {
    VGPU_ATTACHMENT_LOAD_OP_LOAD = 0,
    VGPU_ATTACHMENT_LOAD_OP_CLEAR = 1,
    VGPU_ATTACHMENT_LOAD_OP_DONT_CARE = 2,
} VGpuAttachmentLoadOp;

typedef enum VGpuAttachmentStoreOp {
    VGPU_ATTACHMENT_STORE_OP_STORE = 0,
    VGPU_ATTACHMENT_STORE_OP_DONT_CARE = 1,
} VGpuAttachmentStoreOp;

typedef enum VGpuBufferUsage {
    VGPU_BUFFER_USAGE_STATIC = 0,
    VGPU_BUFFER_USAGE_IMMUTABLE,
    VGPU_BUFFER_USAGE_DYNAMIC,
    VGPU_BUFFER_USAGE_STREAM
} VGpuBufferUsage;

typedef enum VGpuBufferType {
    VGPU_BUFFER_TYPE_VERTEX = 0,
    VGPU_BUFFER_TYPE_INDEX = 1,
    VGPU_BUFFER_TYPE_COUNT
} VGpuBufferType;

typedef enum VGpuShaderStageFlagBits {
    VGPU_SHADER_STAGE_NONE              = 0,
    VGPU_SHADER_STAGE_VERTEX_BIT        = 1 << 0,
    VGPU_SHADER_STAGE_TESS_CONTROL_BIT  = 1 << 1,
    VGPU_SHADER_STAGE_TESS_EVAL_BIT     = 1 << 2,
    VGPU_SHADER_STAGE_GEOMETRY_BIT      = 1 << 3,
    VGPU_SHADER_STAGE_FRAGMENT_BIT      = 1 << 4,
    VGPU_SHADER_STAGE_COMPUTE_BIT       = 1 << 5
} VGpuShaderStageFlagBits;
typedef VgpuFlags VGpuShaderStageFlags;

typedef enum VGpuVertexFormat {
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
    VGPU_VERTEX_FORMAT_COUNT
} VGpuVertexFormat;

typedef enum VGpuVertexInputRate {
    VGPU_VERTEX_INPUT_RATE_VERTEX = 0,
    VGPU_VERTEX_INPUT_RATE_INSTANCE,
} VGpuVertexInputRate;

typedef enum VGpuPrimitiveTopology {
    VGPU_PRIMITIVE_TOPOLOGY_POINT_LIST = 0,
    VGPU_PRIMITIVE_TOPOLOGY_LINE_LIST,
    VGPU_PRIMITIVE_TOPOLOGY_LINE_STRIP,
    VGPU_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    VGPU_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
    VGPU_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY,
    VGPU_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY,
    VGPU_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY,
    VGPU_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY,
    VGPU_PRIMITIVE_TOPOLOGY_PATCH_LIST,
    VGPU_PRIMITIVE_TOPOLOGY_COUNT
} VGpuPrimitiveTopology;

typedef enum VGpuIndexType {
    VGPU_INDEX_TYPE_UINT16 = 0,
    VGPU_INDEX_TYPE_UINT32 = 1
} VGpuIndexType;

typedef enum VGpuCompareFunction {
    VGPU_COMPARE_FUNCTION_NEVER = 0,
    VGPU_COMPARE_FUNCTION_LESS,
    VGPU_COMPARE_FUNCTION_EQUAL,
    VGPU_COMPARE_FUNCTION_LESS_EQUAL,
    VGPU_COMPARE_FUNCTION_GREATER,
    VGPU_COMPARE_FUNCTION_NOT_EQUAL,
    VGPU_COMPARE_FUNCTION_GREATER_EQUAL,
    VGPU_COMPARE_FUNCTION_ALWAYS,
    VGPU_COMPARE_FUNCTION_COUNT
} VGpuCompareFunction;

typedef enum VGpuStencilOperation {
    VGPU_STENCIL_OPERATION_KEEP = 0,
    VGPU_STENCIL_OPERATION_ZERO,
    VGPU_STENCIL_OPERATION_REPLACE,
    VGPU_STENCIL_OPERATION_INCREMENT_CLAMP,
    VGPU_STENCIL_OPERATION_INVERT,
    VGPU_STENCIL_OPERATION_DECREMENT_CLAMP,
    VGPU_STENCIL_OPERATION_INCREMENT_WRAP,
    VGPU_STENCIL_OPERATION_DECREMENT_WRAP,
} VGpuStencilOperation;

typedef struct VGpuExtent2D {
    uint32_t    width;
    uint32_t    height;
} VGpuExtent2D;

typedef struct VGpuExtent3D {
    uint32_t    width;
    uint32_t    height;
    uint32_t    depth;
} VGpuExtent3D;

typedef struct VGpuColor {
    float r;
    float g;
    float b;
    float a;
} VGpuColor;

typedef struct VGpuRect2D {
    int32_t     x;
    int32_t     y;
    uint32_t    width;
    uint32_t    height;
} VGpuRect2D;

typedef struct VGpuLimits {
    uint32_t        maxTextureDimension2D;
    uint32_t        maxTextureDimension3D;
    uint32_t        maxTextureDimensionCube;
    uint32_t        maxTextureArrayLayers;
    uint32_t        maxColorAttachments;
    uint32_t        maxUniformBufferSize;
    uint64_t        minUniformBufferOffsetAlignment;
    uint32_t        maxStorageBufferSize;
    uint64_t        minStorageBufferOffsetAlignment;
    uint32_t        maxSamplerAnisotropy;
    uint32_t        maxViewports;
    uint32_t        maxViewportDimensions[2];
    uint32_t        maxPatchVertices;
    float           pointSizeRange[2];
    float           lineWidthRange[2];
    uint32_t        maxComputeSharedMemorySize;
    uint32_t        maxComputeWorkGroupCount[3];
    uint32_t        maxComputeWorkGroupInvocations;
    uint32_t        maxComputeWorkGroupSize[3];
} VGpuLimits;

typedef struct VGpuClearValue {
    union {
        struct {
            VGpuColor   color;
        };
        struct {
            float       depth;
            uint32_t    stencil;
        };
    };
} VGpuClearValue;

typedef struct VGpuPlatformHandle {
#if defined(_WIN32)
    HINSTANCE                           hinstance;
    HWND                                hwnd;
#elif defined(__linux__)
    xcb_connection_t*                   connection;
    xcb_window_t                        window;
#endif
} VGpuPlatformHandle;

typedef struct VGpuSwapchainDescriptor {
    uint32_t            imageCount;
    VgpuBool32          srgb;
    VGpuClearValue      colorClearValue;
    VGpuPixelFormat     depthStencilFormat;
    VGpuClearValue      depthStencilClearValue;
    VgpuSampleCount     sampleCount;
    VgpuBool32          vsync;
} VGpuSwapchainDescriptor;

typedef struct VGpuRendererSettings {
    VGpuDevicePreference    devicePreference;
    VgpuBool32              validation;
    VGpuPlatformHandle      handle;
    uint32_t                width;
    uint32_t                height;
    VGpuSwapchainDescriptor swapchain;
} VGpuRendererSettings;

typedef struct VGpuTextureDescriptor {
    VGpuTextureType         textureType;
    VGpuPixelFormat         pixelFormat;
    VGpuExtent3D            size;
    uint32_t                mipLevels;
    uint32_t                arrayLayers;
    VgpuSampleCount         samples;
    VGpuTextureUsageFlags   usage;
    const char*             label;
} VGpuTextureDescriptor;

typedef struct VGpuFramebufferAttachment {
    /// The texture attachment.
    VGpuTexture texture;
    /// The mipmap level of the texture used for rendering to the attachment.
    uint32_t level;
    union {
        /// Cubemap face
        uint32_t face;
        /// The slice of the texture used for rendering to the attachment.
        uint32_t slice;
        /// The 3D texture layer.
        uint32_t layer;
    };
} VGpuFramebufferAttachment;

typedef struct VGpuFramebufferDescriptor {
    VGpuFramebufferAttachment   colorAttachments[VGPU_MAX_COLOR_ATTACHMENTS];
    VGpuFramebufferAttachment   depthStencilAttachment;
    uint32_t                    width;
    uint32_t                    height;
    uint32_t                    layers;
} VGpuFramebufferDescriptor;

typedef struct VGpuColorAttachmentAction {
    VGpuAttachmentLoadOp loadOp;
    VGpuAttachmentStoreOp storeOp;
    VGpuColor clearColor;
} VGpuColorAttachmentAction;

typedef struct VGpuDepthStencilAttachmentAction {
    VGpuAttachmentLoadOp depthLoadOp;
    VGpuAttachmentStoreOp depthStoreOp;
    float clearDepth;

    VGpuAttachmentLoadOp stencilLoadOp;
    VGpuAttachmentStoreOp stencilStoreOp;
    uint8_t clearStencil;
} VGpuDepthStencilAttachmentAction;

typedef struct VGpuRenderPassBeginDescriptor {
    VGpuFramebuffer                     framebuffer;
    VGpuColorAttachmentAction           colors[VGPU_MAX_COLOR_ATTACHMENTS];
    VGpuDepthStencilAttachmentAction    depthStencil;
} VGpuRenderPassBeginDescriptor;

typedef struct VGpuStencilDescriptor {
    VGpuStencilOperation      failOperation;
    VGpuStencilOperation      passOperation;
    VGpuStencilOperation      depthFailOperation;
    VGpuCompareFunction       compareFunction;
} VGpuStencilDescriptor;

typedef struct VGpuDepthStencilDescriptor {
    VGpuCompareFunction     depthCompareFunction;
    bool                    depthWriteEnabled;
    bool                    stencilTestEnable;
    uint8_t                 stencilReadMask;
    uint8_t                 stencilWriteMask;
    VGpuStencilDescriptor   frontFace;
    VGpuStencilDescriptor   backFace;
} VGpuDepthStencilDescriptor;

VGPU_API void vgpu_set_log_callback(vgpu_log_fn callback, void *userdata);

VGPU_API VGpuBackend vgpuGetBackend();

VGPU_API bool vgpuInitialize(const char* appName, const VGpuRendererSettings* settings);
VGPU_API void vgpuShutdown();
VGPU_API bool vgpuQueryFeature(VGpuFeature feature);
VGPU_API void vgpuQueryLimits(VGpuLimits* pLimits);
VGPU_API uint32_t vgpuFrame();

/* Texture */
VGPU_API VGpuTexture vgpuCreateTexture(const VGpuTextureDescriptor* descriptor);
VGPU_API VGpuTexture vgpuCreateExternalTexture(const VGpuTextureDescriptor* descriptor, void* handle);
VGPU_API void vgpuDestroyTexture(VGpuTexture texture);

/* Framebuffer */
VGPU_API VGpuFramebuffer vgpuCreateFramebuffer(const VGpuFramebufferDescriptor* descriptor);
VGPU_API void vgpuDestroyFramebuffer(VGpuFramebuffer framebuffer);

/* Buffer */
VGPU_API VGpuBuffer vgpuCreateBuffer(uint64_t size, VGpuBufferType type, VGpuBufferUsage usage, const void* data);
VGPU_API void vgpuDestroyBuffer(VGpuBuffer buffer);

/* Shader */
VGPU_API VGpuShader vgpuCreateShader(const char* vertexSource, const char* fragmentSource);
VGPU_API VGpuShader vgpuCreateComputeShader(const char* source);
VGPU_API void vgpuDestroyShader(VGpuShader shader);

/// Get frame command buffer for recording.
VGPU_API VGpuCommandBuffer vgpuGetCommandBuffer();
VGPU_API VGpuResult vgpuBeginCommandBuffer(VGpuCommandBuffer commandBuffer);
VGPU_API VGpuResult vgpuEndCommandBuffer(VGpuCommandBuffer commandBuffer);
VGPU_API void vgpuCmdBeginDefaultRenderPass(VGpuCommandBuffer commandBuffer, VGpuColor clearColor, float clearDepth, uint8_t clearStencil);
VGPU_API void vgpuCmdBeginRenderPass(VGpuCommandBuffer commandBuffer, const VGpuRenderPassBeginDescriptor* descriptor);
VGPU_API void vgpuCmdEndRenderPass(VGpuCommandBuffer commandBuffer);
VGPU_API void vgpuCmdSetViewport(VGpuCommandBuffer commandBuffer, float x, float y, float width, float height);
VGPU_API void vgpuCmdSetScissor(VGpuCommandBuffer commandBuffer, int32_t x, int32_t y, uint32_t width, uint32_t height);
VGPU_API void vgpuSubmitCommandBuffer(VGpuCommandBuffer commandBuffer);

/// Get the number of bits per format
VGPU_API uint32_t vgpuGetFormatBitsPerPixel(VGpuPixelFormat format);
VGPU_API uint32_t vgpuGetFormatBlockSize(VGpuPixelFormat format);

/// Get the format compression ration along the x-axis.
VGPU_API uint32_t vgpuGetFormatBlockWidth(VGpuPixelFormat format);
/// Get the format compression ration along the y-axis.
VGPU_API uint32_t vgpuGetFormatBlockHeight(VGpuPixelFormat format);
/// Get the format Type.
VGPU_API VGpuPixelFormatType vgpuGetFormatType(VGpuPixelFormat format);

/// Check if the format has a depth component.
VGPU_API VgpuBool32 vgpuIsDepthFormat(VGpuPixelFormat format);
/// Check if the format has a stencil component.
VGPU_API VgpuBool32 vgpuIsStencilFormat(VGpuPixelFormat format);
/// Check if the format has depth or stencil components.
VGPU_API VgpuBool32 vgpuIsDepthStencilFormat(VGpuPixelFormat format);
/// Check if the format is a compressed format.
VGPU_API VgpuBool32 vgpuIsCompressedFormat(VGpuPixelFormat format);
/// Get format string name.
VGPU_API const char* vgpuGetFormatName(VGpuPixelFormat format);
