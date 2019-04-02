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

#include "graphics/GraphicsDevice.h"
#include "BackendVk.h"
#include "graphics/Types.h"
#include <vector>
VK_DEFINE_HANDLE(VmaAllocator);

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

    /// Vulkan backend.
    class GraphicsDeviceVk final : public GraphicsDevice
    {
    public:
        GraphicsDeviceVk(GpuPowerPreference powerPreference);
        ~GraphicsDeviceVk();
        void Destroy();
        void NotifyValidationError(const char* message);

        VkCommandBuffer CreateCommandBuffer(VkCommandBufferLevel level, bool begin);
        void FlushCommandBuffer(VkCommandBuffer commandBuffer, bool free);
        void FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free);

        VkInstance GetVkInstance() const { return _instance; }
        VkPhysicalDevice GetVkPhysicalDevice() const { return _physicalDevice; }
        VkDevice GetVkDevice() const { return device; }

    private:
        // Backend methods
        SwapChain* CreateSwapChainImpl(const SwapChainSurface* surface, const SwapChainDescriptor* descriptor) override;
        CommandBuffer* CreateCommandBufferImpl() override;

    private:
        void InitializeCaps();
        bool InitializeAllocator();

    private:
        DeviceFeatures _features;
        VkInstance _instance = VK_NULL_HANDLE;

#ifdef VULKAN_DEBUG
        VkDebugReportCallbackEXT _debugCallback = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT _debugMessenger = VK_NULL_HANDLE;
#endif

        VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties _deviceProperties;
        VkPhysicalDeviceFeatures _deviceFeatures;
        VkPhysicalDeviceMemoryProperties _deviceMemoryProperties;

        uint32_t graphicsQueueFamily = VK_QUEUE_FAMILY_IGNORED;
        uint32_t computeQueueFamily = VK_QUEUE_FAMILY_IGNORED;
        uint32_t transferQueueFamily = VK_QUEUE_FAMILY_IGNORED;
        VkQueue _graphicsQueue = VK_NULL_HANDLE;
        VkQueue _computeQueue = VK_NULL_HANDLE;
        VkQueue _transferQueue = VK_NULL_HANDLE;
        VkDevice device = VK_NULL_HANDLE;
        VmaAllocator _memoryAllocator = VK_NULL_HANDLE;

        VkCommandPool _graphicsCommandPool = VK_NULL_HANDLE;

        uint32_t maxInflightFrames = 0;
        uint32_t frameNumber = 0;


        struct PerFrame
        {
            PerFrame(GraphicsDeviceVk* device_);
            ~PerFrame();
            void operator=(const PerFrame &) = delete;
            PerFrame(const PerFrame &) = delete;

            void Begin();

            GraphicsDeviceVk* device;
            VkFence fence;
            std::vector<VkCommandBuffer> submittedCommandBuffers;
            std::vector<VkSemaphore> waitSemaphores;
        };
        std::vector<std::unique_ptr<PerFrame>> perFrame;

        PerFrame &frame()
        {
            ALIMER_ASSERT(frameNumber < perFrame.size());
            ALIMER_ASSERT(perFrame[frameNumber]);
            return *perFrame[frameNumber];
        }

        const PerFrame &frame() const
        {
            ALIMER_ASSERT(frameNumber < perFrame.size());
            ALIMER_ASSERT(perFrame[frameNumber]);
            return *perFrame[frameNumber];
        }
    };
}
