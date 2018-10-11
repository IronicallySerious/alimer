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

#if defined(_WIN32)
#   define VK_USE_PLATFORM_WIN32_KHR 1
#elif defined(__ANDROID__)
#   define VK_USE_PLATFORM_ANDROID_KHR 1
#elif defined(__linux__)
#   ifdef ALIMER_LINUX_WAYLAND)
#       define VK_USE_PLATFORM_WAYLAND_KHR 1
#   else
#       define VK_USE_PLATFORM_XCB_KHR 1
#   endif
#endif

#define VK_NO_PROTOTYPES
#include "volk/volk.h"
#include "vkmemalloc/vk_mem_alloc.h"

#include "../../Base/HashMap.h"
#include "../../Graphics/Types.h"
#include "../../Graphics/GpuDeviceFeatures.h"
#include "../../Core/Ptr.h"
#include "../../Core/Log.h"
#include <queue>
#include <set>
#include <mutex>

namespace Alimer
{
    class CommandBuffer;
    class TextureView;
    class VulkanSwapchain;
    class VulkanFramebuffer;
    class VulkanRenderPass;
    struct RenderPassDescriptor;

    class GraphicsImpl final
    {
        friend class Graphics;

    public:
        /// Construct.
        GraphicsImpl(bool validation);

        ~GraphicsImpl();

        bool Initialize(const RenderingSettings& settings);
        bool WaitIdle();

        bool BeginFrame();
        void EndFrame();

        SharedPtr<TextureView> GetSwapchainView() const;

        GraphicsBackend GetBackend() const
        {
            return GraphicsBackend::Vulkan;
        }

        const GraphicsDeviceFeatures& GetFeatures() const
        {
            return _features;
        }

        SharedPtr<CommandBuffer> GetMainCommandBuffer() const;

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

        VulkanFramebuffer* RequestFramebuffer(const RenderPassDescriptor* descriptor);
        VulkanRenderPass* RequestRenderPass(const RenderPassDescriptor* descriptor);

        //VulkanDescriptorSetAllocator* RequestDescriptorSetAllocator(const DescriptorSetLayout &layout);
        //VulkanPipelineLayout* RequestPipelineLayout(const ResourceLayout &layout);

        VkInstance GetInstance() const { return _instance; }
        VkPhysicalDevice GetPhysicalDevice() const { return _physicalDevice; }
        VkDevice GetDevice() const { return _device; }
        VmaAllocator GetVmaAllocator() const { return _allocator; }

        VkPipelineCache GetPipelineCache() const { return _pipelineCache; }
        VkCommandPool GetCommandPool() const { return _commandPool; }
        uint32_t GetGraphicsQueueFamily() const { return _graphicsQueueFamily; }
        uint32_t GetComputeQueueFamily() const { return _computeQueueFamily; }
        uint32_t GetTransferQueueFamily() const { return _transferQueueFamily; }

    private:
        void CreateMemoryAllocator();
        VkSurfaceKHR CreateSurface(const RenderingSettings& settings);

    private:
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
        VmaAllocator _allocator = nullptr;

        // Queue's.
        VkQueue _graphicsQueue = VK_NULL_HANDLE;
        VkQueue _computeQueue = VK_NULL_HANDLE;
        VkQueue _transferQueue = VK_NULL_HANDLE;
        uint32_t _graphicsQueueFamily = VK_QUEUE_FAMILY_IGNORED;
        uint32_t _computeQueueFamily = VK_QUEUE_FAMILY_IGNORED;
        uint32_t _transferQueueFamily = VK_QUEUE_FAMILY_IGNORED;

        // Frame fences
        std::vector<VkFence> _waitFences;
        uint32_t _swapchainImageIndex = 0;
        VkSemaphore _swapchainImageAcquiredSemaphore = VK_NULL_HANDLE;

        // Pipeline cache.
        VkPipelineCache _pipelineCache = VK_NULL_HANDLE;

        // Default command pool.
        VkCommandPool _commandPool = VK_NULL_HANDLE;
        SharedPtr<CommandBuffer> _mainCommandBuffer;

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

        GraphicsDeviceFeatures _features = {};

        template <typename T>
        class Cache
        {
        public:
            T* Find(uint64_t hash) const
            {
                auto itr = _hashMap.find(hash);
                auto *ret = itr != _hashMap.end() ? itr->second.get() : nullptr;
                return ret;
            }

            T* Insert(uint64_t hash, std::unique_ptr<T> value)
            {
                auto &cache = _hashMap[hash];
                if (!cache)
                    cache = std::move(value);

                auto *ret = cache.get();
                return ret;
            }

            void Clear()
            {
                _hashMap.clear();
            }

            Util::HashMap<std::unique_ptr<T>> &GetHashMap()
            {
                return _hashMap;
            }

            const Util::HashMap<std::unique_ptr<T>>& GetHashMap() const
            {
                return _hashMap;
            }

        private:
            Util::HashMap<std::unique_ptr<T>> _hashMap;
        };

        // Cache
        Cache<VulkanRenderPass> _renderPasses;
        Cache<VulkanFramebuffer> _framebuffers;

        //HashMap<std::unique_ptr<VulkanDescriptorSetAllocator>> _descriptorSetAllocators;
        //HashMap<std::unique_ptr<VulkanPipelineLayout>> _pipelineLayouts;
    };

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
            ALIMER_LOGCRITICALF(
                "Fatal Vulkan result is \"%s\" in %u at line %u",
                vkGetVulkanResultString(result),
                __FILE__,
                __LINE__);
        }
    }
}
