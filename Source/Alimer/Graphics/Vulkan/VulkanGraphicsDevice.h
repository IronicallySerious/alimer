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

#include "../GraphicsImpl.h"
#include "../GraphicsDevice.h"
#include "../../Base/HashMap.h"
#include "VulkanPrerequisites.h"
#include <queue>

namespace Alimer
{
    class VulkanCommandBuffer;
    class VulkanDescriptorSetAllocator;
    class VulkanPipelineLayout;

    /// Vulkan graphics backend.
    class VulkanGraphics final : public GraphicsImpl
    {
    public:
        static bool IsSupported();

        /// Construct. Set parent shader and defines but do not compile yet.
        VulkanGraphics(const RenderingSettings& settings);
        /// Destruct.
        ~VulkanGraphics() override;

        virtual uint32_t GetVendorID() const override { return _vendorID; }
        virtual GpuVendor GetVendor() const override { return _vendor; }

        bool Initialize() override;
        bool waitIdle() override;

        bool beginFrame(SwapchainImpl* swapchain) override;
        void endFrame(SwapchainImpl* swapchain) override;

        SwapchainImpl* CreateSwapchain(void* windowHandle, const uvec2& size) override;

        // CommandContext
        void BeginRenderPass() override;
        void EndRenderPass() override;

        /*
        RenderPass* CreateRenderPassImpl(const RenderPassDescription* descriptor) override;
        GpuBuffer* CreateBufferImpl(const BufferDescriptor* descriptor, const void* initialData) override;
        VertexInputFormat* CreateVertexInputFormatImpl(const VertexInputFormatDescriptor* descriptor) override;
        ShaderModule* CreateShaderModuleImpl(const std::vector<uint32_t>& spirv) override;
        ShaderProgram* CreateShaderProgramImpl(const ShaderProgramDescriptor* descriptor) override;

        Texture* CreateTextureImpl(const TextureDescriptor* descriptor, const ImageLevel* initialData) override;
        */

        VkInstance GetInstance() const { return _instance; }
        VkPhysicalDevice GetPhysicalDevice() const { return _physicalDevice; }
        VkDevice GetDevice() const { return _device; }
        VmaAllocator GetAllocator() const { return _allocator; }
        VkPipelineCache GetPipelineCache() const { return _pipelineCache; }

        VkCommandBuffer CreateCommandBuffer(VkCommandBufferLevel level, bool begin = false);
        void FlushCommandBuffer(VkCommandBuffer commandBuffer, bool free = true);
        void FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free = true);
        void ClearImageWithColor(VkCommandBuffer commandBuffer, VkImage image, VkImageSubresourceRange range, VkImageAspectFlags aspect, VkImageLayout sourceLayout, VkImageLayout destLayout, VkAccessFlagBits srcAccessMask, VkClearColorValue *clearValue);

        VkRenderPass GetVkRenderPass(const RenderPassDescription* descriptor);

        //VulkanDescriptorSetAllocator* RequestDescriptorSetAllocator(const DescriptorSetLayout &layout);
        //VulkanPipelineLayout* RequestPipelineLayout(const ResourceLayout &layout);

        VkCommandPool CreateCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags);

        // Fence
        VkFence AcquireFence();
        void ReleaseFence(VkFence fence);

        // Semaphore
        VkSemaphore AcquireSemaphore();
        void ReleaseSemaphore(VkSemaphore semaphore);

    private:
        void CreateAllocator();

        VkInstance _instance = VK_NULL_HANDLE;
        VkDebugReportCallbackEXT _debugCallback = VK_NULL_HANDLE;

        // PhysicalDevice
        VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties _deviceProperties;
        VkPhysicalDeviceMemoryProperties _deviceMemoryProperties;
        VkPhysicalDeviceFeatures _deviceFeatures;
        std::vector<VkQueueFamilyProperties> _queueFamilyProperties;
        uint32_t _vendorID = 0;
        GpuVendor _vendor = GpuVendor::Unknown;
        uint32_t _deviceID = 0;
        String _deviceName;

        // LogicalDevice
        VkDevice _device = VK_NULL_HANDLE;
        VmaAllocator _allocator = VK_NULL_HANDLE;

        // Queue's.
        VkQueue _graphicsQueue = VK_NULL_HANDLE;
        VkQueue _computeQueue = VK_NULL_HANDLE;
        VkQueue _transferQueue = VK_NULL_HANDLE;
        uint32_t _graphicsQueueFamily = VK_QUEUE_FAMILY_IGNORED;
        uint32_t _computeQueueFamily = VK_QUEUE_FAMILY_IGNORED;
        uint32_t _transferQueueFamily = VK_QUEUE_FAMILY_IGNORED;

        // Pipeline cache.
        VkPipelineCache _pipelineCache = VK_NULL_HANDLE;

        // Default command pool.
        VkCommandPool _commandPool = VK_NULL_HANDLE;

        // Primary/Default command buffer
        VulkanCommandBuffer* _defaultCommandBuffer;

        uint32_t _swapchainImageIndex = 0;
        VkSemaphore _swapchainImageAcquiredSemaphore = VK_NULL_HANDLE;

        // Fence pool.
        std::mutex _fenceLock;
        std::set<VkFence> _allFences;
        std::queue<VkFence> _availableFences;

        // Semaphore pool
        std::mutex _semaphoreLock;
        std::set<VkSemaphore> _allSemaphores;
        std::queue<VkSemaphore> _availableSemaphores;

        // Cache
        std::unordered_map<uint64_t, VkRenderPass> _renderPassCache;

        //HashMap<std::unique_ptr<VulkanDescriptorSetAllocator>> _descriptorSetAllocators;
        //HashMap<std::unique_ptr<VulkanPipelineLayout>> _pipelineLayouts;
    };
}
