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

#include "UtilsVk.h"
#include "graphics/Types.h"
namespace alimer
{
    class Window;

    struct DeviceFeatures
    {
        bool supports_physical_device_properties2 = false;
        bool supports_external = false;
        bool supports_dedicated = false;
        bool supports_image_format_list = false;
        bool supports_debug_marker = false;
        bool supports_debug_utils = false;
        bool supports_mirror_clamp_to_edge = false;
        bool supports_google_display_timing = false;
        bool supports_nv_device_diagnostic_checkpoints = false;
        bool supports_vulkan_11_instance = false;
        bool supports_vulkan_11_device = false;
        VkPhysicalDeviceSubgroupProperties subgroup_properties = {};
        VkPhysicalDevice8BitStorageFeaturesKHR storage_8bit_features = {};
        VkPhysicalDevice16BitStorageFeaturesKHR storage_16bit_features = {};
        VkPhysicalDeviceFloat16Int8FeaturesKHR float16_int8_features = {};
        VkPhysicalDeviceFeatures enabled_features = {};
    };

    class GraphicsImpl final
    {
    public:
        GraphicsImpl();
        ~GraphicsImpl();
        void destroy();
        bool initialize(Window* window, const GraphicsDeviceDescriptor& desc);
        void notifyValidationError(const char* message);

        const GraphicsDeviceInfo& getInfo() const { return info; }
        const GraphicsDeviceCapabilities& getCaps() const { return caps; }

    private:
        void initializeCaps();

    private:
        DeviceFeatures features;
        bool ownedInstance = true;
        bool ownedDevice = true;
        VkInstance instance = VK_NULL_HANDLE;
#ifdef VULKAN_DEBUG
        VkDebugReportCallbackEXT debugCallback = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
#endif
        VkSurfaceKHR surface = VK_NULL_HANDLE;

        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        VkPhysicalDeviceMemoryProperties deviceMemoryProperties;

        uint32_t graphicsQueueFamily = VK_QUEUE_FAMILY_IGNORED;
        uint32_t computeQueueFamily = VK_QUEUE_FAMILY_IGNORED;
        uint32_t transferFueueFamily = VK_QUEUE_FAMILY_IGNORED;
        VkQueue graphicsQueue = VK_NULL_HANDLE;
        VkQueue computeQueue = VK_NULL_HANDLE;
        VkQueue transferQueue = VK_NULL_HANDLE;
        VkDevice device = VK_NULL_HANDLE;

        GraphicsDeviceInfo          info = {};
        GraphicsDeviceCapabilities  caps = {};
    };
}