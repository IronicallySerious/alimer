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

#include "GPUDeviceVk.h"
#include "SwapChainVk.h"
#include "CommandQueueVk.h"
#include "CommandBufferVk.h"
#include "TextureVk.h"
#include "SamplerVk.h"
#include "BufferVk.h"

#if defined(_WIN32) || defined(_WIN64)
#   ifndef NOMINMAX
#       define NOMINMAX
#   endif
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#   define VK_SURFACE_EXT               "VK_KHR_win32_surface"
#   define VK_CREATE_SURFACE_FN         vkCreateWin32SurfaceKHR
#   define VK_USE_PLATFORM_WIN32_KHR    1
#elif defined(__ANDROID__)
#   define VK_SURFACE_EXT               "VK_KHR_android_surface"
#   define VK_CREATE_SURFACE_FN         vkCreateAndroidSurfaceKHR
#   define VK_USE_PLATFORM_ANDROID_KHR  1
#elif defined(__linux__)
#   include <xcb/xcb.h>
#   include <dlfcn.h>
#   include <X11/Xlib-xcb.h>
#   define VK_SURFACE_EXT               "VK_KHR_xcb_surface"
#   define VK_CREATE_SURFACE_FN         vkCreateXcbSurfaceKHR
#   define VK_USE_PLATFORM_XCB_KHR      1
#endif

namespace alimer
{
    struct {
        bool    available;

        /* Extensions */
        bool    KHR_get_physical_device_properties2;    /* VK_KHR_get_physical_device_properties2 */
        bool    KHR_external_memory_capabilities;       /* VK_KHR_external_memory_capabilities */
        bool    KHR_external_semaphore_capabilities;    /* VK_KHR_external_semaphore_capabilities */

        bool    EXT_debug_report;       /* VK_EXT_debug_report */
        bool    EXT_debug_utils;        /* VK_EXT_debug_utils */

        bool    KHR_surface;            /* VK_KHR_surface */

        /* Layers */
        bool    VK_LAYER_LUNARG_standard_validation;
        bool    VK_LAYER_RENDERDOC_Capture;
    } _vk;

    static VKAPI_ATTR VkBool32 VKAPI_CALL vgpuVkMessengerCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT                  messageType,
        const VkDebugUtilsMessengerCallbackDataEXT*      pCallbackData,
        void *pUserData)
    {
        //GPUDeviceVk* device = static_cast<GPUDeviceVk*>(pUserData);

        switch (messageSeverity)
        {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
            {
                ALIMER_LOGERROR("[Vulkan]: Validation Error: {}", pCallbackData->pMessage);
                //device->NotifyValidationError(pCallbackData->pMessage);
            }
            else
            {
                ALIMER_LOGERROR("[Vulkan]: Other Error: {}", pCallbackData->pMessage);
            }

            break;

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
            {
                ALIMER_LOGWARN("[Vulkan]: Validation Warning: {}", pCallbackData->pMessage);
            }
            else
            {
                ALIMER_LOGWARN("[Vulkan]: Other Warning: {}", pCallbackData->pMessage);
            }
            break;

#if 0
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
                ALIMER_LOGINFO("[Vulkan]: Validation Info: {}", pCallbackData->pMessage);
            else
                ALIMER_LOGINFO("[Vulkan]: Other Info: {}", pCallbackData->pMessage);
            break;
#endif

        default:
            return VK_FALSE;
        }

        bool logObjectNames = false;
        for (uint32_t i = 0; i < pCallbackData->objectCount; i++)
        {
            auto *name = pCallbackData->pObjects[i].pObjectName;
            if (name)
            {
                logObjectNames = true;
                break;
            }
        }

        if (logObjectNames)
        {
            for (uint32_t i = 0; i < pCallbackData->objectCount; i++)
            {
                auto *name = pCallbackData->pObjects[i].pObjectName;
                ALIMER_LOGINFO("  Object #%u: {}", i, name ? name : "N/A");
            }
        }

        return VK_FALSE;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL vgpuVkDebugCallback(VkDebugReportFlagsEXT flags,
        VkDebugReportObjectTypeEXT, uint64_t,
        size_t, int32_t messageCode, const char *pLayerPrefix,
        const char *pMessage, void *pUserData)
    {
        //GPUDeviceVk* device = static_cast<GPUDeviceVk*>(pUserData);

        // False positives about lack of srcAccessMask/dstAccessMask.
        if (strcmp(pLayerPrefix, "DS") == 0 && messageCode == 10)
        {
            return VK_FALSE;
        }

        // Demote to a warning, it's a false positive almost all the time for Granite.
        if (strcmp(pLayerPrefix, "DS") == 0 && messageCode == 6)
        {
            flags = VK_DEBUG_REPORT_DEBUG_BIT_EXT;
        }

        if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
        {
            ALIMER_LOGERROR("[Vulkan]: {}: {}", pLayerPrefix, pMessage);
            //device->NotifyValidationError(pMessage);
        }
        else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
        {
            ALIMER_LOGWARN("[Vulkan]: {}: {}", pLayerPrefix, pMessage);
        }
        else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
        {
            ALIMER_LOGWARN("[Vulkan]: Performance warning: {}: {}", pLayerPrefix, pMessage);
        }
        else
        {
            ALIMER_LOGINFO("[Vulkan]: {}: {}", pLayerPrefix, pMessage);
        }

        return VK_FALSE;
    }

    bool GPUDeviceVk::IsSupported()
    {
        VkResult result;
        uint32_t extensionsCount;
        uint32_t layersCount;
        VkExtensionProperties* queriedExtensions;
        VkLayerProperties* queriedLayers;

        if (_vk.available) {
            return true;
        }

        result = volkInitialize();
        if (result != VK_SUCCESS)
        {
            _vk.available = false;
            return false;
        }

        result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, nullptr);
        if (result < VK_SUCCESS)
        {
            _vk.available = false;
            return false;
        }

        queriedExtensions = (VkExtensionProperties*)calloc(extensionsCount, sizeof(VkExtensionProperties));
        result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, queriedExtensions);
        if (result < VK_SUCCESS)
        {
            free(queriedExtensions);
            _vk.available = false;
            return false;
        }

        result = vkEnumerateInstanceLayerProperties(&layersCount, nullptr);
        if (result < VK_SUCCESS)
        {
            free(queriedExtensions);
            _vk.available = false;
            return false;
        }

        queriedLayers = (VkLayerProperties*)calloc(layersCount, sizeof(VkLayerProperties));
        result = vkEnumerateInstanceLayerProperties(&layersCount, queriedLayers);
        if (result < VK_SUCCESS)
        {
            free(queriedExtensions);
            free(queriedLayers);
            _vk.available = false;
            return false;
        }

        bool surface = false;
        bool platform_surface = false;
        for (uint32_t i = 0u; i < extensionsCount; i++)
        {
            if (strcmp(queriedExtensions[i].extensionName, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) == 0)
            {
                _vk.KHR_get_physical_device_properties2 = true;
            }
            else if (strcmp(queriedExtensions[i].extensionName, VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME) == 0)
            {
                _vk.KHR_external_memory_capabilities = true;
            }
            else if (strcmp(queriedExtensions[i].extensionName, VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME) == 0)
            {
                _vk.KHR_external_semaphore_capabilities = true;
            }
            else if (strcmp(queriedExtensions[i].extensionName, VK_EXT_DEBUG_REPORT_EXTENSION_NAME) == 0)
            {
                _vk.EXT_debug_report = true;
            }
            else if (strcmp(queriedExtensions[i].extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
            {
                _vk.EXT_debug_utils = true;
            }
            else if (strcmp(queriedExtensions[i].extensionName, VK_KHR_SURFACE_EXTENSION_NAME) == 0)
            {
                surface = true;
            }
            else if (strcmp(queriedExtensions[i].extensionName, VK_SURFACE_EXT) == 0)
            {
                platform_surface = true;
            }
        }

        _vk.KHR_surface = surface && platform_surface;

        // Initialize layers.
        for (uint32_t i = 0u; i < layersCount; i++)
        {
            if (strcmp(queriedLayers[i].layerName, "VK_LAYER_LUNARG_standard_validation") == 0)
            {
                _vk.VK_LAYER_LUNARG_standard_validation = true;
            }
            else if (strcmp(queriedLayers[i].layerName, "VK_LAYER_RENDERDOC_Capture") == 0)
            {
                _vk.VK_LAYER_RENDERDOC_Capture = true;
            }
        }

        free(queriedExtensions);
        free(queriedLayers);
        _vk.available = true;
        return true;
    }

    GPUDeviceVk::GPUDeviceVk(bool validation, bool headless)
        : GPUDevice(validation, headless)
    {
        VkApplicationInfo appInfo;
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pNext = nullptr;
        appInfo.pApplicationName = "alimer";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "ALIMER_VERSION_PATCH";
        appInfo.engineVersion = VK_MAKE_VERSION(ALIMER_VERSION_MAJOR, ALIMER_VERSION_MINOR, ALIMER_VERSION_PATCH);
        appInfo.apiVersion = volkGetInstanceVersion();

        if (appInfo.apiVersion >= VK_API_VERSION_1_1)
        {
            appInfo.apiVersion = VK_API_VERSION_1_1;
        }

        std::vector<const char*> instanceLayers;
        std::vector<const char*> instanceExtensions;

        if (_vk.KHR_get_physical_device_properties2)
        {
            instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        }

        if (_vk.KHR_get_physical_device_properties2
            && _vk.KHR_external_memory_capabilities
            && _vk.KHR_external_semaphore_capabilities)
        {
            instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
            instanceExtensions.push_back(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME);
            instanceExtensions.push_back(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME);
        }

        if (_vk.EXT_debug_utils)
        {
            instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

#if !defined(NDEBUG)
        if (validation)
        {
            if (!_vk.EXT_debug_utils && _vk.EXT_debug_report)
            {
                instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
            }

            bool force_no_validation = false;
            /*if (getenv("VGPU_VULKAN_NO_VALIDATION"))
            {
                force_no_validation = true;
            }*/

            if (!force_no_validation && _vk.VK_LAYER_LUNARG_standard_validation)
            {
                instanceLayers.push_back("VK_LAYER_LUNARG_standard_validation");
            }
        }
#endif

        if (!headless
            && _vk.KHR_surface)
        {
            instanceExtensions.push_back("VK_KHR_surface");
            instanceExtensions.push_back(VK_SURFACE_EXT);
        }

        VkInstanceCreateInfo instanceCreateInfo;
        instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pNext = nullptr;
        instanceCreateInfo.flags = 0;
        instanceCreateInfo.pApplicationInfo = &appInfo;
        instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(instanceLayers.size());
        instanceCreateInfo.ppEnabledLayerNames = instanceLayers.data();
        instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
        instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
        vkThrowIfFailed(
            vkCreateInstance(&instanceCreateInfo, nullptr, &_instance)
        );

        volkLoadInstance(_instance);

        if (validation)
        {
            if (_vk.EXT_debug_utils)
            {
                VkDebugUtilsMessengerCreateInfoEXT info = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
                info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
                info.pfnUserCallback = vgpuVkMessengerCallback;
                info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
                info.pUserData = this;
                vkCreateDebugUtilsMessengerEXT(_instance, &info, nullptr, &_debugMessenger);
            }
            else if (_vk.EXT_debug_report)
            {
                VkDebugReportCallbackCreateInfoEXT info = { VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT };
                info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
                    VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
                info.pfnCallback = vgpuVkDebugCallback;
                info.pUserData = this;
                vkCreateDebugReportCallbackEXT(_instance, &info, nullptr, &_debugCallback);
            }
        }

    }

    GPUDeviceVk::~GPUDeviceVk()
    {
        vkDeviceWaitIdle(_device);

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

    void GPUDeviceVk::WaitIdle()
    {
        vkThrowIfFailed(
            vkDeviceWaitIdle(_device)
        );
    }

    bool GPUDeviceVk::Initialize(const SwapChainDescriptor* descriptor)
    {
        /*VkSurfaceKHR surface = VK_NULL_HANDLE;
        if (!_headless)
        {
            surface = CreateSurface(handle);
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
        }*/

        return true;
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
