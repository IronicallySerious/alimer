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

    AGPU_DEFINE_HANDLE(AgpuBuffer);

#define AGPU_TRUE                           1
#define AGPU_FALSE                          0

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

    typedef struct AgpuPlatformHandle {
        /// Native connection, display or instance type.
        void* connection;
        /// Native window handle.
        void* handle;
    } AgpuPlatformHandle;

    typedef struct AgpuSwapchainDescriptor {
        AgpuPlatformHandle  handle;
        uint32_t            width;
        uint32_t            height;
    } AgpuSwapchainDescriptor;

    typedef struct AgpuDescriptor {
        AgpuBool32                  validation;
        AgpuBackend                 preferredBackend;
        AgpuSwapchainDescriptor     swapchain;
    } AgpuDescriptor;

    AGPU_API AgpuBackend agpuGetDefaultPlatformBackend();
    AGPU_API AgpuBool32 agpuIsBackendSupported(AgpuBackend backend);
    AGPU_API uint32_t agpuGetAvailableBackendsCount();
    AGPU_API AgpuBackend agpuGetAvailableBackend(uint32_t index);

    AGPU_API AgpuResult agpuInitialize(const AgpuDescriptor* descriptor);
    AGPU_API void agpuShutdown();
    AGPU_API AgpuResult agpuBeginFrame();
    AGPU_API uint64_t agpuEndFrame();

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
