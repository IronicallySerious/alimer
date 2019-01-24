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

#ifdef _WIN32
#   ifndef NOMINMAX
#       define NOMINMAX
#   endif
#   ifndef WIN32_LEAN_AND_MEAN
#       define WIN32_LEAN_AND_MEAN
#   endif
#endif

#if defined(ALIMER_D3D11)
#   include <dxgi.h>
#   define D3D11_NO_HELPERS
#   include <d3d11_1.h>
#   include <d3dcompiler.h>
#   ifdef _DEBUG
#       include <dxgidebug.h>
#   endif
#   define MAKE_SMART_COM_PTR(_a) _COM_SMARTPTR_TYPEDEF(_a, __uuidof(_a))
#   define GET_COM_INTERFACE(base, type, var) MAKE_SMART_COM_PTR(type); concat_strings(type, Ptr) var; d3d_call(base->QueryInterface(IID_PPV_ARGS(&var)));
#elif defined(ALIMER_D3D12)
#   include <dxgi.h>
#   if defined(NTDDI_WIN10_RS2)
#       include <dxgi1_5.h>
#   else
#   include <dxgi1_4.h>
#   endif
#   include <d3d12.h>
#   include <d3dcompiler.h>
#   ifdef _DEBUG
#       include <dxgidebug.h>
#   endif
#   define MAKE_SMART_COM_PTR(_a) _COM_SMARTPTR_TYPEDEF(_a, __uuidof(_a))
#   define GET_COM_INTERFACE(base, type, var) MAKE_SMART_COM_PTR(type); concat_strings(type, Ptr) var; d3d_call(base->QueryInterface(IID_PPV_ARGS(&var)));
#elif defined(ALIMER_VULKAN)
#   if defined(_WIN32)
#       ifndef VK_USE_PLATFORM_WIN32_KHR
#           define VK_USE_PLATFORM_WIN32_KHR 1
#       endif
#   elif defined(__ANDROID__)
#       define VK_USE_PLATFORM_ANDROID_KHR 1
#   elif defined(__linux__)
#       ifdef ALIMER_LINUX_WAYLAND)
#           define VK_USE_PLATFORM_WAYLAND_KHR 1
#       else
#           define VK_USE_PLATFORM_XCB_KHR 1
#       endif
#   endif
#   include "volk/volk.h"
#   include <vk_mem_alloc.h>
#elif defined(ALIMER_OPENGL)
#endif

#include "../Core/Log.h"

namespace alimer
{
#if defined(ALIMER_D3D11)
#define BACKEND_INVALID_HANDLE  nullptr
    using PhysicalDeviceHandle  = IDXGIAdapter1*;
    using BufferHandle          = ID3D11Buffer*;
#elif defined(ALIMER_D3D12)
#define BACKEND_INVALID_HANDLE  nullptr
    using PhysicalDeviceHandle  = IDXGIAdapter1*;
    using BufferHandle          = ID3D11Resource*;
#elif defined(ALIMER_VULKAN)
#define BACKEND_INVALID_HANDLE  0
    using PhysicalDeviceHandle  = VkPhysicalDevice;
    using BufferHandle          = VkBuffer;


    inline const char* vkGetVulkanResultString(VkResult result)
    {
        switch (result)
        {
        case VK_SUCCESS:
            return "Success";
        case VK_NOT_READY:
            return "A fence or query has not yet completed";
        case VK_TIMEOUT:
            return "A wait operation has not completed in the specified time";
        case VK_EVENT_SET:
            return "An event is signaled";
        case VK_EVENT_RESET:
            return "An event is unsignaled";
        case VK_INCOMPLETE:
            return "A return array was too small for the result";
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            return "A host memory allocation has failed";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            return "A device memory allocation has failed";
        case VK_ERROR_INITIALIZATION_FAILED:
            return "Initialization of an object could not be completed for implementation-specific reasons";
        case VK_ERROR_DEVICE_LOST:
            return "The logical or physical device has been lost";
        case VK_ERROR_MEMORY_MAP_FAILED:
            return "Mapping of a memory object has failed";
        case VK_ERROR_LAYER_NOT_PRESENT:
            return "A requested layer is not present or could not be loaded";
        case VK_ERROR_EXTENSION_NOT_PRESENT:
            return "A requested extension is not supported";
        case VK_ERROR_FEATURE_NOT_PRESENT:
            return "A requested feature is not supported";
        case VK_ERROR_INCOMPATIBLE_DRIVER:
            return "The requested version of Vulkan is not supported by the driver or is otherwise incompatible";
        case VK_ERROR_TOO_MANY_OBJECTS:
            return "Too many objects of the type have already been created";
        case VK_ERROR_FORMAT_NOT_SUPPORTED:
            return "A requested format is not supported on this device";
        case VK_ERROR_SURFACE_LOST_KHR:
            return "A surface is no longer available";
        case VK_SUBOPTIMAL_KHR:
            return "A swapchain no longer matches the surface properties exactly, but can still be used";
        case VK_ERROR_OUT_OF_DATE_KHR:
            return "A surface has changed in such a way that it is no longer compatible with the swapchain";
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
            return "The display used by a swapchain does not use the same presentable image layout";
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
            return "The requested window is already connected to a VkSurfaceKHR, or to some other non-Vulkan API";
        case VK_ERROR_VALIDATION_FAILED_EXT:
            return "A validation layer found an error";
        default:
            return "ERROR: UNKNOWN VULKAN ERROR";
        }
}

    // Helper utility converts Vulkan API failures into exceptions.
    inline void vkThrowIfFailed(VkResult result)
    {
        if (result < VK_SUCCESS)
        {
            ALIMER_LOGCRITICAL(
                "Fatal Vulkan result is \"%s\" in %u at line %u",
                vkGetVulkanResultString(result),
                __FILE__,
                __LINE__);
        }
    }
#elif defined(ALIMER_OPENGL)
#define BACKEND_INVALID_HANDLE  GL_INVALID_VALUE
    using PhysicalDeviceHandle  = unsigned;
    using BufferHandle          = unsigned;
#endif
};
