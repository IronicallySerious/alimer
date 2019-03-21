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

#include "core/Log.h"
#include "vulkan/vulkan.h"
#include <string>

#if !defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG)
#	define VULKAN_DEBUG
#endif

namespace alimer
{
    inline const char* GetVkResultString(VkResult result)
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
            LOGE("Fatal Vulkan result is \"%s\" in %s at line %d", GetVkResultString(result), __FILE__, __LINE__);
            abort();
        }
    }

    inline static std::string vkGetVendorByID(uint32_t id)
    {
        switch (id)
        {
        case 0x1002: return "Advanced Micro Devices, Inc.";
        case 0x10de: return "NVIDIA Corporation";
        case 0x102b: return "Matrox Electronic Systems Ltd.";
        case 0x1414: return "Microsoft Corporation";
        case 0x5333: return "S3 Graphics Co., Ltd.";
        case 0x8086: return "Intel Corporation";
        case 0x80ee: return "Oracle Corporation";
        case 0x15ad: return "VMware Inc.";
        }
        return "";
    }

    // see https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#fundamentals-versionnum
    inline std::string vkGetVersionToString(std::uint32_t version)
    {
        std::string s;

        s += std::to_string(VK_VERSION_MAJOR(version));
        s += '.';
        s += std::to_string(VK_VERSION_MINOR(version));
        s += ".";
        s += std::to_string(VK_VERSION_PATCH(version));

        return s;
    }

    /// Vulkan exception structure
    class VulkanException : std::runtime_error
    {
    public:
        VulkanException(const VkResult result, const std::string &msg = "Vulkan error");
        const char* what() const noexcept override;

    private:
        std::string _errorMessage;
    };
}
