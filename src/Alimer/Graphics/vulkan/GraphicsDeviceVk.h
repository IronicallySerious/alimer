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

#include "BackendVk.h"
#include "../GraphicsDevice.h"

namespace alimer
{
    class CommandContextVk;

    /// Vulkan gpu backend.
    class GraphicsImpl final 
    {
    public:
        /// Constructor.
        GraphicsImpl(const char* applicationName, const GraphicsDeviceDescriptor* descriptor);

        /// Destructor.
        ~GraphicsImpl();

        GraphicsBackend GetBackend() const { return GraphicsBackend::Vulkan; }

        void WaitIdle();
        
        void CreateInstance(const char* applicationName);
        void SelectPhysicalDevice(GpuPreference devicePreference);
        void CreateLogicalDevice();
        void InitializeFeatures();

        bool BeginFrame();
        void EndFrame();

        VkSurfaceKHR CreateSurface(uint64_t nativeHandle);

        bool ImageFormatIsSupported(VkFormat format, VkFormatFeatureFlags required, VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL) const;
        PixelFormat GetDefaultDepthStencilFormat() const;
        PixelFormat GetDefaultDepthFormat() const;

        VkSemaphore RequestSemaphore();
        void RecycleSemaphore(VkSemaphore semaphore);

        VkFence RequestFence();
        void RecycleFence(VkFence fence);

        void DestroySampler(VkSampler handle);
        void DestroyPipeline(VkPipeline handle);
        void DestroyImage(VkImage handle);

        VkInstance GetVkInstance() const { return _instance; }
        VkPhysicalDevice GetVkPhysicalDevice() const { return _physicalDevice; }
        VkDevice GetVkDevice() const { return _device; }
        VmaAllocator GetVmaAllocator() const { return _memoryAllocator; }

        uint32_t GetGraphicsQueueFamily() const { return _graphicsQueueFamily; }
        uint32_t GetComputeQueueFamily() const { return _computeQueueFamily; }
        uint32_t GetTransferQueueFamily() const { return _transferQueueFamily; }
        const DeviceFeaturesVk& GetFeaturesVk() const { return _featuresVk; }

        /// Get the device features.
        inline const GraphicsDeviceFeatures& GetFeatures() const { return _features; }

        /// Get the device limits
        inline const GraphicsDeviceLimits& GetLimits() const { return _limits; }

        VkQueue GetQueue(QueueType type) const
        {
            switch (type)
            {
            default:
            case QueueType::Direct:
                return _graphicsQueue;

            case QueueType::Compute:
                return _computeQueue;

            case QueueType::Copy:
                return _transferQueue;
            }
        }
        
    private:
        bool HasExtension(const std::string& extension)
        {
            return (std::find(_physicalDeviceExtensions.begin(), _physicalDeviceExtensions.end(), extension) != _physicalDeviceExtensions.end());
        }

        bool HasLayer(const std::string& extension)
        {
            return (std::find(_physicalDeviceLayers.begin(), _physicalDeviceLayers.end(), extension) != _physicalDeviceLayers.end());
        }

    private:
        bool                                    _headless = false;
        bool                                    _validation = false;
        GraphicsDeviceFeatures                  _features = {};
        GraphicsDeviceLimits                    _limits = {};

        VkInstance                              _instance = VK_NULL_HANDLE;
        VkDebugReportCallbackEXT                _debugCallback = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT                _debugMessenger = VK_NULL_HANDLE;
        VkPhysicalDevice                        _physicalDevice = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties              _physicalDeviceProperties;
        VkPhysicalDeviceMemoryProperties        _physicalDeviceMemoryProperties;
        VkPhysicalDeviceFeatures                _physicalDeviceFeatures;
        std::vector<VkQueueFamilyProperties>    _physicalDeviceQueueFamilyProperties;
        std::vector<std::string>                _physicalDeviceExtensions;
        std::vector<std::string>                _physicalDeviceLayers;
        uint32_t                                _presentQueueFamily = VK_QUEUE_FAMILY_IGNORED;
        uint32_t                                _graphicsQueueFamily = VK_QUEUE_FAMILY_IGNORED;
        uint32_t                                _computeQueueFamily = VK_QUEUE_FAMILY_IGNORED;
        uint32_t                                _transferQueueFamily = VK_QUEUE_FAMILY_IGNORED;
        VkDevice                                _device = VK_NULL_HANDLE;
        
        VkQueue                                 _graphicsQueue = VK_NULL_HANDLE;
        VkQueue                                 _computeQueue = VK_NULL_HANDLE;
        VkQueue                                 _transferQueue = VK_NULL_HANDLE;
        VmaAllocator                            _memoryAllocator = VK_NULL_HANDLE;
        DeviceFeaturesVk                        _featuresVk = {};
        VkPipelineCache                         _pipelineCache = VK_NULL_HANDLE;
        
        std::vector<VkFence>                    _fences;
        std::vector<VkSemaphore>                _semaphores;
    };
}
