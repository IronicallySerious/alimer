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
#include "../../Base/HashMap.h"
#include "../GraphicsDevice.h"
#include <queue>
#include <set>

#ifdef ALIMER_THREADING
#   include <atomic>
#   include <mutex>
#   include <condition_variable>
#endif

namespace alimer
{
    class VulkanBuffer;
    class VulkanCommandBuffer;
    class VulkanSwapchain;
    class VulkanFramebuffer;
    class VulkanRenderPass;
    struct VulkanResourceLayout;
    class VulkanPipelineLayout;
    struct RenderPassDescriptor;

    class VulkanGraphicsDevice final : public GraphicsDevice
    {
        ALIMER_OBJECT(VulkanGraphicsDevice, GraphicsDevice);

    public:
        static bool IsSupported();

        /// Construct.
        VulkanGraphicsDevice(bool validation);
        ~VulkanGraphicsDevice() override;

        uint64_t AllocateCookie();

        //bool Initialize(const RenderingSettings& settings) override;
        //void Shutdown() override;
        //bool WaitIdle() override;

        //void PresentImpl() override;

        void AddWaitSemaphore(VkSemaphore semaphore, VkPipelineStageFlags flags);
        void SubmitCommandBuffer(VkCommandBuffer commandBuffer);
        void SubmitCommandBuffer(VkCommandBuffer commandBuffer, VkFence fence);

        //Framebuffer* GetSwapchainFramebuffer() const override;
        //GpuBuffer* CreateBufferImpl(const BufferDescriptor* descriptor, const void* initialData) override;
        //Texture* CreateTextureImpl(const TextureDescriptor* descriptor, const ImageLevel* initialData) override;
        //Framebuffer* CreateFramebufferImpl(const FramebufferDescriptor* descriptor) override;
        //Shader* CreateShaderImpl(const ShaderDescriptor* descriptor) override;
        //Pipeline* CreateRenderPipelineImpl(const RenderPipelineDescriptor* descriptor) override;

        // Fence
        VkFence AcquireFence();
        void ReleaseFence(VkFence fence);

        // Semaphore
        VkSemaphore AcquireSemaphore();
        void ReleaseSemaphore(VkSemaphore semaphore);


        VkCommandPool CreateCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags);
        VkCommandBuffer CreateCommandBuffer(VkCommandBufferLevel level, bool begin = false);
        void FlushCommandBuffer(VkCommandBuffer commandBuffer, bool free = true);
        void FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free = true);
        void ClearImageWithColor(VkCommandBuffer commandBuffer, VkImage image, VkImageSubresourceRange range, VkImageAspectFlags aspect, VkImageLayout sourceLayout, VkImageLayout destLayout, VkClearColorValue *clearValue);

        VkResult BufferSubData(VulkanBuffer* buffer, VkDeviceSize offset, VkDeviceSize size, const void* pData);
        VkResult MapBuffer(VulkanBuffer* buffer, VkDeviceSize offset, VkDeviceSize size, void** ppData);
        void UnmapBuffer(VulkanBuffer* buffer);

        VulkanFramebuffer* RequestFramebuffer(const RenderPassDescriptor* descriptor);
        VulkanRenderPass* RequestRenderPass(const RenderPassDescriptor* descriptor);

        //VulkanDescriptorSetAllocator* RequestDescriptorSetAllocator(const DescriptorSetLayout &layout);
        VulkanPipelineLayout* RequestPipelineLayout(const VulkanResourceLayout* layout);

        VkInstance GetInstance() const { return _instance; }
        VkPhysicalDevice GetPhysicalDevice() const { return _physicalDevice; }
        VkDevice GetDevice() const { return _device; }
        VmaAllocator GetVmaAllocator() const { return _memoryAllocator; }

        VkPipelineCache GetPipelineCache() const { return _pipelineCache; }
        VkCommandPool GetCommandPool() const { return _commandPool; }
        uint32_t GetGraphicsQueueFamily() const { return _graphicsQueueFamily; }
        uint32_t GetComputeQueueFamily() const { return _computeQueueFamily; }
        uint32_t GetTransferQueueFamily() const { return _transferQueueFamily; }

    private:
        int32_t ScorePhysicalDevice(VkPhysicalDevice physicalDevice);

        void InitializeCaps();
        void CreateMemoryAllocator();
        VkSurfaceKHR CreateSurface(const SwapChainDescriptor& descriptor);

#ifdef ALIMER_THREADING
        std::atomic<uint64_t> _cookie;
#else
        uint64_t _cookie = 0;
#endif

        bool _supportsExternal = false;
        bool _supportsDedicated = false;
        bool _supportsImageFormatList = false;
        bool _supportsDebugMarker = false;
        bool _supportsDebugUtils = false;

        VkInstance _instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT _debugMessenger = VK_NULL_HANDLE;
        VkDebugReportCallbackEXT _debugCallback = VK_NULL_HANDLE;

        // PhysicalDevice
        VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties _deviceProperties;
        VkPhysicalDeviceMemoryProperties _deviceMemoryProperties;
        VkPhysicalDeviceFeatures _deviceFeatures;
        std::vector<VkQueueFamilyProperties> _queueFamilyProperties;

        // Logical device.
        VkDevice _device = nullptr;
        VmaAllocator _memoryAllocator = nullptr;
        VulkanBuffer* _pinnedMemoryBuffer = nullptr;
        void* _pinnedMemoryPtr = nullptr;

        // Queue's.
        VkQueue _graphicsQueue = VK_NULL_HANDLE;
        VkQueue _computeQueue = VK_NULL_HANDLE;
        VkQueue _transferQueue = VK_NULL_HANDLE;
        uint32_t _graphicsQueueFamily = VK_QUEUE_FAMILY_IGNORED;
        uint32_t _computeQueueFamily = VK_QUEUE_FAMILY_IGNORED;
        uint32_t _transferQueueFamily = VK_QUEUE_FAMILY_IGNORED;

        std::vector<VkSemaphore> _waitSemaphores;
        std::vector<VkPipelineStageFlags> _waitStageFlags;
        std::vector<VkCommandBuffer> _submittedCommandBuffers;

        // Pipeline cache.
        VkPipelineCache _pipelineCache = VK_NULL_HANDLE;

        // Default command pool.
        VkCommandPool _commandPool = VK_NULL_HANDLE;

        // Main swap chain.
        UniquePtr<VulkanSwapchain> _mainSwapchain;

        // Fence pool.
        std::mutex _fenceLock;
        std::set<VkFence> _allFences;
        std::queue<VkFence> _availableFences;

        // Semaphore pool
        std::mutex _semaphoreLock;
        std::set<VkSemaphore> _allSemaphores;
        std::queue<VkSemaphore> _availableSemaphores;

        // Cache
        //Cache<VulkanRenderPass> _renderPasses;
        //Cache<VulkanFramebuffer> _framebuffers;

        //HashMap<std::unique_ptr<VulkanDescriptorSetAllocator>> _descriptorSetAllocators;
        //Cache<VulkanPipelineLayout> _pipelineLayouts;
    };
}
