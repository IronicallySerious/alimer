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

#include "core/platform.h"
#include <stdint.h>   

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

#ifdef __cplusplus
extern "C" {
#endif

    typedef uint32_t VgpuFlags;
    typedef uint32_t VgpuBool32;

    enum {
        VGPU_MAX_COLOR_ATTACHMENTS = 8,
        VGPU_MAX_VERTEX_BUFFER_BINDINGS = 4,
        VGPU_MAX_VERTEX_ATTRIBUTES = 16
    };

    typedef enum vgpu_result {
        VGPU_SUCCESS = 0,
        VGPU_NOT_READY = 1,
        VGPU_TIMEOUT = 2,
        VGPU_INCOMPLETE = 3,
        VGPU_ALREADY_INITIALIZED = 4,
        VGPU_ERROR_GENERIC                          = -1,
        VGPU_ERROR_OUT_OF_HOST_MEMORY               = -2,
        VGPU_ERROR_OUT_OF_DEVICE_MEMORY             = -3,
        VGPU_ERROR_INITIALIZATION_FAILED            = -4,
        VGPU_ERROR_DEVICE_LOST                      = -5,
        VGPU_ERROR_TOO_MANY_OBJECTS                 = -6,
        VGPU_ERROR_BEGIN_FRAME_FAILED               = -7,
        VGPU_ERROR_END_FRAME_FAILED                 = -8,
        VGPU_ERROR_COMMAND_BUFFER_ALREADY_RECORDING = -9,
        VGPU_ERROR_COMMAND_BUFFER_NOT_RECORDING     = -10,
    } vgpu_result;

    typedef enum vgpu_device_preference {
        /// No GPU preference.
        VGPU_DEVICE_PREFERENCE_DONT_CARE,
        /// Prefer integrated GPU.
        VGPU_DEVICE_PREFERENCE_LOW_POWER,
        /// Prefer high performance/discrete GPU.
        VGPU_DEVICE_PREFERENCE_HIGH_PERFORMANCE,
    } vgpu_device_preference;

    /// Defines pixel format.
    typedef enum vgpu_pixel_format {
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

        VGPU_PIXEL_FORMAT_COUNT
    } vgpu_pixel_format;

    typedef enum vgpu_sample_count {
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
    } vgpu_sample_count;

    typedef struct vgpu_clear_value {
        union {
            struct {
                float                       r;
                float                       g;
                float                       b;
                float                       a;
            };
            struct {
                float                       depth;
                uint32_t                    stencil;
            };
        };
    } vgpu_clear_value;

    typedef struct vgpu_platform_handle {
#if defined(_WIN32)
        HINSTANCE                           hinstance;
        HWND                                hwnd;
#elif defined(__linux__)
        xcb_connection_t*                   connection;
        xcb_window_t                        window;      
#endif
    } vgpu_platform_handle;

    typedef struct vgpu_swapchain_descriptor {
        uint32_t            image_count;
        VgpuBool32          srgb;
        vgpu_clear_value    color_clear_value;
        vgpu_pixel_format   depth_stencil_pixel_format;
        vgpu_clear_value    depth_stencil_clear_value;
        vgpu_sample_count   samples;
        VgpuBool32          vsync;
    } vgpu_swapchain_descriptor;

    typedef struct vgpu_renderer_settings {
        vgpu_device_preference      device_preference;
        VgpuBool32                  validation;
        vgpu_platform_handle        handle;
        uint32_t                    width;
        uint32_t                    height;
        vgpu_swapchain_descriptor   swapchain;
    } vgpu_renderer_settings;

    VORTICE_API vgpu_result vgpu_initialize(const char* app_name, const vgpu_renderer_settings* settings);
    VORTICE_API void vgpu_shutdown();
    VORTICE_API vgpu_result vgpu_begin_frame();
    VORTICE_API vgpu_result vgpu_end_frame();

#ifdef __cplusplus
}
#endif
