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

#include "../Backend.h"

#if defined(_WIN32) || defined(_WIN64)
#   ifndef NOMINMAX
#       define NOMINMAX
#   endif
#   ifndef WIN32_LEAN_AND_MEAN
#       define WIN32_LEAN_AND_MEAN
#   endif
#   define VK_SURFACE_EXT               "VK_KHR_win32_surface"
#   define VK_CREATE_SURFACE_FN         vkCreateWin32SurfaceKHR
#   define VK_USE_PLATFORM_WIN32_KHR    1
#elif defined(__ANDROID__)
#   define VK_SURFACE_EXT               "VK_KHR_android_surface"
#   define VK_CREATE_SURFACE_FN         vkCreateAndroidSurfaceKHR
#   define VK_USE_PLATFORM_ANDROID_KHR  1
#elif defined(__linux__)
#   include <dlfcn.h>
#   include <xcb/xcb.h>
#   include <X11/Xlib-xcb.h>
#   define VK_SURFACE_EXT               "VK_KHR_xcb_surface"
#   define VK_CREATE_SURFACE_FN         vkCreateXcbSurfaceKHR
#   define VK_USE_PLATFORM_XCB_KHR      1
#endif

#include "volk.h"
#include "vk_mem_alloc.h"

namespace alimer
{
    class GPUDeviceVk;

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
                "Fatal Vulkan result is \"{}\" in {} at line {}",
                vkGetVulkanResultString(result),
                __FILE__,
                __LINE__);
        }
    }

    struct DeviceFeaturesVk
    {
        bool supportsPhysicalDeviceProperties2 = false;
        bool supportsExternal = false;
        bool supportsDedicated = false;
        bool supportsImageFormatList = false;
        bool supportsDebugMarker = false;
        bool supportsDebugUtils = false;
        bool supportsMirrorClampToEdge = false;
        bool supportsGoogleDisplayTiming = false;
        bool supportsVulkan11Instance = false;
        bool supportsVulkan11Device = false;
        VkPhysicalDeviceSubgroupProperties subgroupProperties = {};
        VkPhysicalDevice8BitStorageFeaturesKHR storage8BitFeatures = {};
        VkPhysicalDevice16BitStorageFeaturesKHR storage16BitFeatures = {};
        VkPhysicalDeviceFloat16Int8FeaturesKHR float16Int8Features = {};
        VkPhysicalDeviceFeatures enabledFeatures = {};
    };

    struct VkFormatDesc
    {
        PixelFormat format;
        VkFormat    vkFormat;
    };

    extern ALIMER_API const VkFormatDesc s_vkFormatDesc[];

    inline VkFormat GetVkFormat(PixelFormat format)
    {
        assert(s_vkFormatDesc[(uint32_t)format].format == format);
        assert(s_vkFormatDesc[(uint32_t)format].vkFormat != VK_FORMAT_UNDEFINED);
        return s_vkFormatDesc[(uint32_t)format].vkFormat;
    }

    // Put an image memory barrier for setting an image layout on the sub resource into the given command buffer
    void vkTransitionImageLayout(
        VkCommandBuffer commandBuffer,
        VkImage image,
        VkImageLayout oldImageLayout,
        VkImageLayout newImageLayout,
        VkImageSubresourceRange subresourceRange,
        VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    // Uses a fixed sub resource layout with first mip level and layer
    void vkTransitionImageLayout(
        VkCommandBuffer commandBuffer,
        VkImage image,
        VkImageAspectFlags aspectMask,
        VkImageLayout oldImageLayout,
        VkImageLayout newImageLayout,
        VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

    void vkClearImageWithColor(
        VkCommandBuffer commandBuffer,
        VkImage image,
        VkImageSubresourceRange range,
        VkImageAspectFlags aspect,
        VkImageLayout sourceLayout,
        VkImageLayout destLayout,
        VkClearColorValue *clearValue);
}
