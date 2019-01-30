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

#define VMA_STATS_STRING_ENABLED 0
#define VMA_IMPLEMENTATION
#include "GPUDeviceVk.h"
#include "SwapChainVk.h"
#include "CommandQueueVk.h"
#include "CommandBufferVk.h"
#include "TextureVk.h"
#include "SamplerVk.h"
#include "BufferVk.h"

namespace alimer
{
    struct VGpuVkData
    {
        bool    available;

        bool    KHR_get_physical_device_properties2;    /* VK_KHR_get_physical_device_properties2 */
        bool    KHR_external_memory_capabilities;       /* VK_KHR_external_memory_capabilities */
        bool    KHR_external_semaphore_capabilities;    /* VK_KHR_external_semaphore_capabilities */

        bool    EXT_debug_report;       /* VK_EXT_debug_report */
        bool    EXT_debug_utils;        /* VK_EXT_debug_utils */

        bool    KHR_surface;            /* VK_KHR_surface */
#if defined(_WIN32) || defined(_WIN64)
        bool    KHR_win32_surface;      /* VK_KHR_win32_surface*/
#elif defined(__ANDROID__)
        bool    KHR_android_surface;
#elif defined(__APPLE__)
        bool    MVK_macos_surface;    /* VK_MVK_macos_surface */
#elif defined(__linux__)
        bool    KHR_xlib_surface;
        bool    KHR_xcb_surface;
        bool    KHR_wayland_surface;

#endif
    } _s_gpu_vk_data;

    bool GPUDeviceVk::IsSupported()
    {
        return true;
    }

    GPUDeviceVk::GPUDeviceVk(bool validation, bool headless)
        : GraphicsDevice(GraphicsBackend::Vulkan, validation, headless)
    {
    }

    GPUDeviceVk::~GPUDeviceVk()
    {
        vkDeviceWaitIdle(_device);

        // Delete swap chain if created.
        SafeDelete(_mainSwapChain);

        for (uint32_t i = 0; i < RenderLatency; i++)
        {
            vkDestroyFence(_device, _waitFences[i], nullptr);
            vkDestroySemaphore(_device, _renderCompleteSemaphores[i], nullptr);
            vkDestroySemaphore(_device, _presentCompleteSemaphores[i], nullptr);
        }

        _commandBuffers.clear();
        _graphicsCommandQueue.reset();
        _computeCommandQueue.reset();
        _copyCommandQueue.reset();

        if (_device != VK_NULL_HANDLE)
        {
            vkDestroyDevice(_device, nullptr);
            _device = VK_NULL_HANDLE;
        }

        if (_memoryAllocator != VK_NULL_HANDLE)
        {
            vmaDestroyAllocator(_memoryAllocator);
            _memoryAllocator = VK_NULL_HANDLE;
        }

        if (_debugCallback != VK_NULL_HANDLE)
        {
            vkDestroyDebugReportCallbackEXT(_instance, _debugCallback, nullptr);
            _debugCallback = VK_NULL_HANDLE;
        }

        if (_debugMessenger != VK_NULL_HANDLE)
        {
            vkDestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);
            _debugMessenger = VK_NULL_HANDLE;
        }

        vkDestroyInstance(_instance, nullptr);
    }

    void GPUDeviceVk::NotifyValidationError(const char* message)
    {
        ALIMER_UNUSED(message);
    }

    void GPUDeviceVk::InitializeFeatures()
    {
        vkGetPhysicalDeviceProperties(_physicalDevice, &_physicalDeviceProperties);
        vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &_physicalDeviceMemoryProperties);
        vkGetPhysicalDeviceFeatures(_physicalDevice, &_physicalDeviceFeatures);

        // Queue family properties, used for setting up requested queues upon device creation
        uint32_t queueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, nullptr);
        assert(queueFamilyCount > 0);
        _physicalDeviceQueueFamilyProperties.resize(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, _physicalDeviceQueueFamilyProperties.data());

        // Get list of supported extensions
        uint32_t extCount = 0;
        vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &extCount, nullptr);
        if (extCount > 0)
        {
            std::vector<VkExtensionProperties> extensions(extCount);
            if (vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &extCount, &extensions.front()) == VK_SUCCESS)
            {
                for (auto ext : extensions)
                {
                    _physicalDeviceExtensions.push_back(ext.extensionName);
                }
            }
        }


        _features.SetBackend(GraphicsBackend::Vulkan);
        _features.SetVendorId(_physicalDeviceProperties.vendorID);
        // Find vendor
        switch (_physicalDeviceProperties.vendorID)
        {
        case 0x13B5:
            _features.SetVendor(GpuVendor::ARM);
            break;
        case 0x10DE:
            _features.SetVendor(GpuVendor::NVIDIA);
            break;
        case 0x1002:
        case 0x1022:
            _features.SetVendor(GpuVendor::AMD);
            break;
        case 0x163C:
        case 0x8086:
            _features.SetVendor(GpuVendor::INTEL);
            break;
        default:
            _features.SetVendor(GpuVendor::Unknown);
            break;
        }

        _features.SetDeviceId(_physicalDeviceProperties.deviceID);
        _features.SetDeviceName(_physicalDeviceProperties.deviceName);
        _features.SetMultithreading(true);
        _features.SetMaxColorAttachments(_physicalDeviceProperties.limits.maxColorAttachments);
        _features.SetMaxBindGroups(_physicalDeviceProperties.limits.maxBoundDescriptorSets);
        _features.SetMinUniformBufferOffsetAlignment(static_cast<uint32_t>(_physicalDeviceProperties.limits.minUniformBufferOffsetAlignment));
    }

    /*bool GPUDeviceVk::SetMode(const SwapChainHandle* handle, const SwapChainDescriptor* descriptor)
    {
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        if (!_headless)
        {
            surface = CreateSurface(handle);
        }

        uint32_t queueCount = (uint32_t)_physicalDeviceQueueFamilyProperties.size();
        std::vector<VkQueueFamilyProperties> queue_props = _physicalDeviceQueueFamilyProperties;

        for (uint32_t i = 0; i < queueCount; i++)
        {
            VkBool32 supported = surface == VK_NULL_HANDLE;
            if (surface != VK_NULL_HANDLE)
                vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, i, surface, &supported);

            static const VkQueueFlags required = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT;
            if (supported
                && ((queue_props[i].queueFlags & required) == required))
            {
                _graphicsQueueFamily = i;
                break;
            }
        }

        for (uint32_t i = 0; i < queueCount; i++)
        {
            static const VkQueueFlags required = VK_QUEUE_COMPUTE_BIT;
            if (i != _graphicsQueueFamily
                && (queue_props[i].queueFlags & required) == required)
            {
                _computeQueueFamily = i;
                break;
            }
        }

        for (uint32_t i = 0; i < queueCount; i++)
        {
            static const VkQueueFlags required = VK_QUEUE_TRANSFER_BIT;
            if (i != _graphicsQueueFamily
                && i != _computeQueueFamily
                && (queue_props[i].queueFlags & required) == required)
            {
                _transferQueueFamily = i;
                break;
            }
        }

        if (_transferQueueFamily == VK_QUEUE_FAMILY_IGNORED)
        {
            for (uint32_t i = 0; i < queueCount; i++)
            {
                static const VkQueueFlags required = VK_QUEUE_TRANSFER_BIT;
                if (i != _graphicsQueueFamily
                    && (queue_props[i].queueFlags & required) == required)
                {
                    _transferQueueFamily = i;
                    break;
                }
            }
        }

        if (_graphicsQueueFamily == VK_QUEUE_FAMILY_IGNORED)
        {
            ALIMER_LOGERROR("Vulkan: Invalid graphics queue family");
            return false;
        }

        uint32_t universalQueueIndex = 1;
        uint32_t graphicsQueueIndex = 0;
        uint32_t computeQueueIndex = 0;
        uint32_t transferQueueIndex = 0;

        if (_computeQueueFamily == VK_QUEUE_FAMILY_IGNORED)
        {
            _computeQueueFamily = _graphicsQueueFamily;
            computeQueueIndex = std::min(queue_props[_graphicsQueueFamily].queueCount - 1, universalQueueIndex);
            universalQueueIndex++;
        }

        if (_transferQueueFamily == VK_QUEUE_FAMILY_IGNORED)
        {
            _transferQueueFamily = _graphicsQueueFamily;
            transferQueueIndex = std::min(queue_props[_graphicsQueueFamily].queueCount - 1, universalQueueIndex);
            universalQueueIndex++;
        }
        else if (_transferQueueFamily == _computeQueueFamily)
        {
            transferQueueIndex = std::min(queue_props[_computeQueueFamily].queueCount - 1, 1u);
        }

        static const float graphicsQueuePriority = 0.5f;
        static const float computeQueuePriority = 1.0f;
        static const float transferQueuePriority = 1.0f;
        float queuePriorities[3] = { graphicsQueuePriority, computeQueuePriority, transferQueuePriority };

        uint32_t queueFamilyCount = 0;
        VkDeviceQueueCreateInfo queueCreateInfo[3] = {};

        VkDeviceCreateInfo deviceCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfo;

        queueCreateInfo[queueFamilyCount].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo[queueFamilyCount].queueFamilyIndex = _graphicsQueueFamily;
        queueCreateInfo[queueFamilyCount].queueCount = std::min(universalQueueIndex, queue_props[_graphicsQueueFamily].queueCount);
        queueCreateInfo[queueFamilyCount].pQueuePriorities = queuePriorities;
        queueFamilyCount++;

        if (_computeQueueFamily != _graphicsQueueFamily)
        {
            queueCreateInfo[queueFamilyCount].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo[queueFamilyCount].queueFamilyIndex = _computeQueueFamily;
            queueCreateInfo[queueFamilyCount].queueCount = std::min(_transferQueueFamily == _computeQueueFamily ? 2u : 1u,
                queue_props[_computeQueueFamily].queueCount);
            queueCreateInfo[queueFamilyCount].pQueuePriorities = queuePriorities + 1;
            queueFamilyCount++;
        }

        if (_transferQueueFamily != _graphicsQueueFamily
            && _transferQueueFamily != _computeQueueFamily)
        {
            queueCreateInfo[queueFamilyCount].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo[queueFamilyCount].queueFamilyIndex = _transferQueueFamily;
            queueCreateInfo[queueFamilyCount].queueCount = 1;
            queueCreateInfo[queueFamilyCount].pQueuePriorities = queuePriorities + 2;
            queueFamilyCount++;
        }

        std::vector<const char*> enabledExtensions;
        if (!_headless)
        {
            enabledExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        }

        // Enable VK_KHR_maintenance1 to support nevagive viewport and match DirectX coordinate system.
        if (IsExtensionSupported(VK_KHR_MAINTENANCE1_EXTENSION_NAME))
        {
            enabledExtensions.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        }

        if (IsExtensionSupported(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME)
            && IsExtensionSupported(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME))
        {
            _featuresVk.supportsDedicated = true;
            enabledExtensions.push_back(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
            enabledExtensions.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        }

        if (IsExtensionSupported(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME))
        {
            _featuresVk.supportsImageFormatList = true;
            enabledExtensions.push_back(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
        }

        if (IsExtensionSupported(VK_GOOGLE_DISPLAY_TIMING_EXTENSION_NAME))
        {
            _featuresVk.supportsGoogleDisplayTiming = true;
            enabledExtensions.push_back(VK_GOOGLE_DISPLAY_TIMING_EXTENSION_NAME);
        }

        if (_s_gpu_vk_data.EXT_debug_utils)
        {
            _featuresVk.supportsDebugUtils = true;
        }

        if (IsExtensionSupported(VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
        {
            _featuresVk.supportsDebugMarker = true;
            enabledExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
        }

        if (IsExtensionSupported(VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME))
        {
            _featuresVk.supportsMirrorClampToEdge = true;
            enabledExtensions.push_back(VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME);
        }

        VkPhysicalDeviceFeatures2KHR features = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR };
        if (_s_gpu_vk_data.KHR_get_physical_device_properties2)
        {
            vkGetPhysicalDeviceFeatures2KHR(_physicalDevice, &features);
        }
        else
        {
            vkGetPhysicalDeviceFeatures(_physicalDevice, &features.features);
        }

        // Enable device features we might care about.
        {
            VkPhysicalDeviceFeatures enabled_features = {};
            if (features.features.textureCompressionETC2)
                enabled_features.textureCompressionETC2 = VK_TRUE;
            if (features.features.textureCompressionBC)
                enabled_features.textureCompressionBC = VK_TRUE;
            if (features.features.textureCompressionASTC_LDR)
                enabled_features.textureCompressionASTC_LDR = VK_TRUE;
            if (features.features.fullDrawIndexUint32)
                enabled_features.fullDrawIndexUint32 = VK_TRUE;
            if (features.features.imageCubeArray)
                enabled_features.imageCubeArray = VK_TRUE;
            if (features.features.fillModeNonSolid)
                enabled_features.fillModeNonSolid = VK_TRUE;
            if (features.features.independentBlend)
                enabled_features.independentBlend = VK_TRUE;
            if (features.features.sampleRateShading)
                enabled_features.sampleRateShading = VK_TRUE;
            if (features.features.fragmentStoresAndAtomics)
                enabled_features.fragmentStoresAndAtomics = VK_TRUE;
            if (features.features.shaderStorageImageExtendedFormats)
                enabled_features.shaderStorageImageExtendedFormats = VK_TRUE;
            if (features.features.shaderStorageImageMultisample)
                enabled_features.shaderStorageImageMultisample = VK_TRUE;
            if (features.features.largePoints)
                enabled_features.largePoints = VK_TRUE;

            features.features = enabled_features;
        }

        if (_s_gpu_vk_data.KHR_get_physical_device_properties2)
        {
            deviceCreateInfo.pNext = &features;
        }
        else
        {
            deviceCreateInfo.pEnabledFeatures = &features.features;
        }


        deviceCreateInfo.queueCreateInfoCount = queueFamilyCount;
        deviceCreateInfo.enabledLayerCount = 0;
        deviceCreateInfo.ppEnabledLayerNames = nullptr;
        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();

        if (vkCreateDevice(_physicalDevice, &deviceCreateInfo, nullptr, &_device) != VK_SUCCESS)
        {
            ALIMER_LOGERROR("Vulkan: Failed to create device");
            return false;
        }

        volkLoadDevice(_device);
        vkGetDeviceQueue(_device, _graphicsQueueFamily, graphicsQueueIndex, &_graphicsQueue);
        vkGetDeviceQueue(_device, _computeQueueFamily, computeQueueIndex, &_computeQueue);
        vkGetDeviceQueue(_device, _transferQueueFamily, transferQueueIndex, &_transferQueue);

        // Set up VMA.
        VmaVulkanFunctions vulkanFunctions = {};
        vulkanFunctions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
        vulkanFunctions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
        vulkanFunctions.vkAllocateMemory = vkAllocateMemory;
        vulkanFunctions.vkFreeMemory = vkFreeMemory;
        vulkanFunctions.vkMapMemory = vkMapMemory;
        vulkanFunctions.vkUnmapMemory = vkUnmapMemory;
        vulkanFunctions.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
        vulkanFunctions.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
        vulkanFunctions.vkBindBufferMemory = vkBindBufferMemory;
        vulkanFunctions.vkBindImageMemory = vkBindImageMemory;
        vulkanFunctions.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
        vulkanFunctions.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
        vulkanFunctions.vkCreateBuffer = vkCreateBuffer;
        vulkanFunctions.vkDestroyBuffer = vkDestroyBuffer;
        vulkanFunctions.vkCreateImage = vkCreateImage;
        vulkanFunctions.vkDestroyImage = vkDestroyImage;
        vulkanFunctions.vkCmdCopyBuffer = vkCmdCopyBuffer;
#if VMA_DEDICATED_ALLOCATION
        vulkanFunctions.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR;
        vulkanFunctions.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2KHR;
#endif

        VmaAllocatorCreateInfo allocatorCreateInfo = {};
        allocatorCreateInfo.physicalDevice = _physicalDevice;
        allocatorCreateInfo.device = _device;
        allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;
        if (vmaCreateAllocator(&allocatorCreateInfo, &_memoryAllocator) != VK_SUCCESS)
        {
            return false;
        }

        // Create command queue's.
        _graphicsCommandQueue = std::make_unique<CommandQueueVk>(this, _graphicsQueue, _graphicsQueueFamily);
        _computeCommandQueue = std::make_unique<CommandQueueVk>(this, _computeQueue, _computeQueueFamily);
        _copyCommandQueue = std::make_unique<CommandQueueVk>(this, _transferQueue, _transferQueueFamily);

        // Create main swap chain.
        if (!_headless)
        {
            _mainSwapChain = new SwapChainVk(this, surface, descriptor);
        }

        _waitFences.resize(RenderLatency);
        _presentCompleteSemaphores.resize(RenderLatency);
        _renderCompleteSemaphores.resize(RenderLatency);
        _perFrame.clear();
        for (uint32_t i = 0; i < RenderLatency; ++i)
        {
            VkFenceCreateInfo fenceCreateInfo = {};
            fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            vkThrowIfFailed(vkCreateFence(_device, &fenceCreateInfo, nullptr, &_waitFences[i]));

            VkSemaphoreCreateInfo semaphoreCreateInfo = {};
            semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            vkThrowIfFailed(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_presentCompleteSemaphores[i]));
            vkThrowIfFailed(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_renderCompleteSemaphores[i]));

            auto frame = std::unique_ptr<PerFrame>(new PerFrame(this));
            _perFrame.emplace_back(std::move(frame));
        }

        // Frame command buffers.
        {
            std::vector<VkCommandBuffer> vkCommandBuffers(_mainSwapChain->GetTextureCount());
            VkCommandBufferAllocateInfo commandBufferAllocateInfo;
            commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            commandBufferAllocateInfo.pNext = nullptr;
            commandBufferAllocateInfo.commandPool = _graphicsCommandQueue->GetVkCommandPool();
            commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            commandBufferAllocateInfo.commandBufferCount = _mainSwapChain->GetTextureCount();
            vkThrowIfFailed(vkAllocateCommandBuffers(_device, &commandBufferAllocateInfo, vkCommandBuffers.data()));

            _commandBuffers.resize(_mainSwapChain->GetTextureCount());
            for (uint32_t i = 0; i < _mainSwapChain->GetTextureCount(); i++)
            {
                _commandBuffers[i] = std::make_unique<CommandBufferVk>(this, _graphicsCommandQueue.get(), vkCommandBuffers[i]);
            }
        }

        return true;
    }*/

    void GPUDeviceVk::WaitIdle()
    {
    }

    /*bool GPUDeviceVk::BeginFrame()
    {
        vkThrowIfFailed(vkWaitForFences(_device, 1, &_waitFences[_frameIndex], VK_TRUE, UINT64_MAX));
        vkThrowIfFailed(vkResetFences(_device, 1, &_waitFences[_frameIndex]));

        VkResult result = _mainSwapChain->AcquireNextImage(_presentCompleteSemaphores[_frameIndex], &_swapchainImageIndex);
        if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR))
        {
            _mainSwapChain->Resize();
        }
        else {
            vkThrowIfFailed(result);
        }

        _commandBuffers[_swapchainImageIndex]->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        return true;
    }

    void GPUDeviceVk::EndFrame()
    {
        _graphicsCommandQueue->ExecuteCommandBuffer(
            _commandBuffers[_swapchainImageIndex].get(),
            _presentCompleteSemaphores[_frameIndex],
            _renderCompleteSemaphores[_frameIndex],
            _waitFences[_frameIndex]
        );

        VkResult result = _mainSwapChain->QueuePresent(_graphicsQueue, _swapchainImageIndex, _renderCompleteSemaphores[_frameIndex]);
        if (!((result == VK_SUCCESS) || (result == VK_SUBOPTIMAL_KHR)))
        {
            if (result == VK_ERROR_OUT_OF_DATE_KHR) {
                _mainSwapChain->Resize();
                return;
            }
            else {
                vkThrowIfFailed(result);
            }
        }

        _frameIndex += 1;
        _frameIndex %= RenderLatency;
    }

    GPUTexture* GPUDeviceVk::CreateTexture(const TextureDescriptor* descriptor, void* nativeTexture, const void* pInitData)
    {
        return new TextureVk(this, descriptor, nativeTexture, pInitData);
    }

    GPUSampler* GPUDeviceVk::CreateSampler(const SamplerDescriptor* descriptor)
    {
        return new SamplerVk(this, descriptor);
    }

    GPUBuffer* GPUDeviceVk::CreateBuffer(const BufferDescriptor* descriptor, const void* pInitData)
    {
        return new BufferVk(this, descriptor, pInitData);
    }*/

    void GPUDeviceVk::DestroySampler(VkSampler sampler)
    {
#if !defined(NDEBUG)
        ALIMER_ASSERT(std::find(frame().destroyedSamplers.begin(), frame().destroyedSamplers.end(), sampler) == frame().destroyedSamplers.end());
#endif
        frame().destroyedSamplers.push_back(sampler);
    }

    VkCommandBuffer GPUDeviceVk::CreateCommandBuffer(VkCommandBufferLevel level, bool begin)
    {
        VkCommandBufferAllocateInfo info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
        info.commandPool = _graphicsCommandQueue->GetVkCommandPool();
        info.level = level;
        info.commandBufferCount = 1;

        VkCommandBuffer vkCommandBuffer;
        vkThrowIfFailed(vkAllocateCommandBuffers(_device, &info, &vkCommandBuffer));

        // If requested, also start recording for the new command buffer
        if (begin)
        {
            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            vkThrowIfFailed(vkBeginCommandBuffer(vkCommandBuffer, &beginInfo));
        }

        return vkCommandBuffer;
    }

    void GPUDeviceVk::FlushCommandBuffer(VkCommandBuffer commandBuffer, bool free)
    {
        FlushCommandBuffer(commandBuffer, _graphicsQueue, free);
    }

    void GPUDeviceVk::FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free)
    {
        if (commandBuffer == VK_NULL_HANDLE)
            return;

        vkThrowIfFailed(vkEndCommandBuffer(commandBuffer));

        VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        // Create fence to ensure that the command buffer has finished executing
        VkFenceCreateInfo fenceCreateInfo = {};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = 0;
        VkFence fence;
        vkThrowIfFailed(vkCreateFence(_device, &fenceCreateInfo, nullptr, &fence));

        // Submit to the queue
        vkThrowIfFailed(vkQueueSubmit(queue, 1, &submitInfo, fence));

        // Wait for the fence to signal that command buffer has finished executing.
        vkThrowIfFailed(vkWaitForFences(_device, 1, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max()));
        vkDestroyFence(_device, fence, nullptr);

        if (free)
        {
            vkFreeCommandBuffers(_device, _graphicsCommandQueue->GetVkCommandPool(), 1, &commandBuffer);
        }
    }

    void GPUDeviceVk::ClearImageWithColor(
        VkCommandBuffer commandBuffer,
        VkImage image,
        VkImageSubresourceRange range,
        VkImageAspectFlags aspect,
        VkImageLayout sourceLayout,
        VkImageLayout destLayout,
        VkClearColorValue *clearValue)
    {
        // Transition to destination layout.
        vk::SetImageLayout(commandBuffer, image, aspect, sourceLayout,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        // Clear the image
        range.aspectMask = aspect;
        vkCmdClearColorImage(
            commandBuffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            clearValue,
            1,
            &range);

        // Transition back to source layout.
        vk::SetImageLayout(commandBuffer, image, aspect, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, destLayout);
    }


    GPUDeviceVk::PerFrame::PerFrame(GPUDeviceVk* device_)
        : device(device_)
        , logicalDevice(device_->GetVkDevice())
    {

    }

    GPUDeviceVk::PerFrame::~PerFrame()
    {
        Begin();
    }

    void GPUDeviceVk::PerFrame::Begin()
    {
        for (auto &sampler : destroyedSamplers)
        {
            vkDestroySampler(logicalDevice, sampler, nullptr);
        }
    }
}
