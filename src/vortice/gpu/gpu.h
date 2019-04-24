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
        VGPU_ERROR_COMMAND_BUFFER_ALREADY_RECORDING = -7,
        VGPU_ERROR_COMMAND_BUFFER_NOT_RECORDING = -8,
    } VGpuResult;

    typedef enum VGpuDevicePreference {
        /// No GPU preference.
        VGPU_DEVICE_PREFERENCE_DONT_CARE,
        /// Prefer integrated GPU.
        VGPU_DEVICE_PREFERENCE_LOW_POWER,
        /// Prefer high performance/discrete GPU.
        VGPU_DEVICE_PREFERENCE_HIGH_PERFORMANCE,
    } VGpuDevicePreference;

    typedef struct VGpuDescriptor {
        VGpuDevicePreference            devicePreference;
        VgpuBool32                      validation;
        /// Main swap chain descriptor or null for headless.
        //const VgpuSwapchainDescriptor*  swapchain;
    } VGpuDescriptor;

    VORTICE_API VGpuResult vgpu_initialize(const char* applicationName, const VGpuDescriptor* descriptor);
    VORTICE_API void vgpu_shutdown();

#ifdef __cplusplus
}
#endif
