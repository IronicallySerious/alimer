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
#if ALIMER_PLATFORM_WINDOWS
        bool    KHR_win32_surface;      /* VK_KHR_win32_surface*/
#elif ALIMER_PLATFORM_APPLE
        bool    MVK_macos_surface;    /* VK_MVK_macos_surface */
#elif ALIMER_PLATFORM_LINUX
        bool    KHR_xlib_surface;
        bool    KHR_xcb_surface;
        bool    KHR_wayland_surface;
#elif ALIMER_PLATFORM_ANDROID
        bool    KHR_android_surface;
#endif
    } _s_gpu_vk_data;

    static VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_messenger_cb(
        VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT                  messageType,
        const VkDebugUtilsMessengerCallbackDataEXT*      pCallbackData,
        void *pUserData)
    {
        GPUDeviceVk* device = static_cast<GPUDeviceVk*>(pUserData);

        switch (messageSeverity)
        {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
            {
                ALIMER_LOGERROR("[Vulkan]: Validation Error: {}", pCallbackData->pMessage);
                device->NotifyValidationError(pCallbackData->pMessage);
            }
            else
            {
                ALIMER_LOGERROR("[Vulkan]: Other Error: {}", pCallbackData->pMessage);
            }

            break;

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
                ALIMER_LOGWARN("[Vulkan]: Validation Warning:{}", pCallbackData->pMessage);
            else
                ALIMER_LOGWARN("[Vulkan]: Other Warning: {}", pCallbackData->pMessage);
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

        bool log_object_names = false;
        for (uint32_t i = 0; i < pCallbackData->objectCount; i++)
        {
            auto *name = pCallbackData->pObjects[i].pObjectName;
            if (name)
            {
                log_object_names = true;
                break;
            }
        }

        if (log_object_names)
        {
            for (uint32_t i = 0; i < pCallbackData->objectCount; i++)
            {
                auto *name = pCallbackData->pObjects[i].pObjectName;
                ALIMER_LOGINFO("  Object #%u: %s", i, name ? name : "N/A");
            }
        }

        return VK_FALSE;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_cb(VkDebugReportFlagsEXT flags,
        VkDebugReportObjectTypeEXT, uint64_t,
        size_t, int32_t messageCode, const char *pLayerPrefix,
        const char *pMessage, void *pUserData)
    {
        GPUDeviceVk* device = static_cast<GPUDeviceVk*>(pUserData);

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
            ALIMER_LOGERROR("[Vulkan]: %s: %s", pLayerPrefix, pMessage);
            device->NotifyValidationError(pMessage);
        }
        else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
        {
            ALIMER_LOGWARN("[Vulkan]: %s: %s", pLayerPrefix, pMessage);
        }
        else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
        {
            //ALIMER_LOGWARN("[Vulkan]: Performance warning: %s: %s", pLayerPrefix, pMessage);
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
        uint32_t i, count;
        VkExtensionProperties* queried_extensions;

        if (_s_gpu_vk_data.available) {
            return true;
        }

        result = volkInitialize();
        if (result != VK_SUCCESS)
        {
            _s_gpu_vk_data.available = false;
            return false;
        }

        result = vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
        if (result < VK_SUCCESS)
        {
            _s_gpu_vk_data.available = false;
            return false;
        }

        queried_extensions = (VkExtensionProperties*)calloc(count, sizeof(VkExtensionProperties));
        result = vkEnumerateInstanceExtensionProperties(nullptr, &count, queried_extensions);
        if (result < VK_SUCCESS)
        {
            free(queried_extensions);
            _s_gpu_vk_data.available = false;
            return false;
        }

        for (i = 0; i < count; i++)
        {
            if (strcmp(queried_extensions[i].extensionName, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) == 0)
            {
                _s_gpu_vk_data.KHR_get_physical_device_properties2 = true;
            }
            else if (strcmp(queried_extensions[i].extensionName, VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME) == 0)
            {
                _s_gpu_vk_data.KHR_external_memory_capabilities = true;
            }
            else if (strcmp(queried_extensions[i].extensionName, VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME) == 0)
            {
                _s_gpu_vk_data.KHR_external_semaphore_capabilities = true;
            }
            else if (strcmp(queried_extensions[i].extensionName, VK_EXT_DEBUG_REPORT_EXTENSION_NAME) == 0)
            {
                _s_gpu_vk_data.EXT_debug_report = true;
            }
            else if (strcmp(queried_extensions[i].extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
            {
                _s_gpu_vk_data.EXT_debug_utils = true;
            }
            else if (strcmp(queried_extensions[i].extensionName, VK_KHR_SURFACE_EXTENSION_NAME) == 0)
            {
                _s_gpu_vk_data.KHR_surface = true;
            }
#if ALIMER_PLATFORM_WINDOWS
            else if (strcmp(queried_extensions[i].extensionName, "VK_KHR_win32_surface") == 0)
            {
                _s_gpu_vk_data.KHR_win32_surface = true;
            }
#elif ALIMER_PLATFORM_APPLE
            else if (strcmp(queried_extensions[i].extensionName, "VK_MVK_macos_surface") == 0)
            {
                _s_gpu_vk_data.MVK_macos_surface = true;
            }
#elif ALIMER_PLATFORM_LINUX
            else if (strcmp(queried_extensions[i].extensionName, "VK_KHR_xlib_surface") == 0)
            {
                _s_gpu_vk_data.KHR_xlib_surface = true;
            }
            else if (strcmp(queried_extensions[i].extensionName, "VK_KHR_xcb_surface") == 0)
            {
                _s_gpu_vk_data.KHR_xcb_surface = true;
            }
            else if (strcmp(queried_extensions[i].extensionName, "VK_KHR_wayland_surface") == 0)
            {
                _s_gpu_vk_data.KHR_wayland_surface = true;
            }
#elif ALIMER_PLATFORM_ANDROID
            else if (strcmp(queried_extensions[i].extensionName, "VK_KHR_android_surface") == 0)
            {
                _s_gpu_vk_data.KHR_android_surface = true;
            }
#endif
        }


        free(queried_extensions);
        _s_gpu_vk_data.available = true;
        return true;
    }

    GPUDeviceVk::GPUDeviceVk(bool validation, bool headless)
        : GraphicsDevice(GraphicsBackend::Vulkan, validation, headless)
    {
        VkApplicationInfo appInfo;
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pNext = nullptr;
        appInfo.pApplicationName = "Alimer";
        appInfo.applicationVersion = 0;
        appInfo.pEngineName = "Alimer";
        appInfo.engineVersion = 0;
        appInfo.apiVersion = volkGetInstanceVersion();

        if (appInfo.apiVersion >= VK_API_VERSION_1_1)
        {
            appInfo.apiVersion = VK_API_VERSION_1_1;
        }

        std::vector<const char*> instanceLayers;
        std::vector<const char*> instanceExtensions;

        if (_s_gpu_vk_data.KHR_get_physical_device_properties2)
        {
            instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        }

        if (_s_gpu_vk_data.KHR_get_physical_device_properties2
            && _s_gpu_vk_data.KHR_external_memory_capabilities
            && _s_gpu_vk_data.KHR_external_semaphore_capabilities)
        {
            instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
            instanceExtensions.push_back(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME);
            instanceExtensions.push_back(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME);
        }

        if (_s_gpu_vk_data.EXT_debug_utils)
        {
            instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        if (_validation)
        {
            uint32_t layerCount = 0;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
            std::vector<VkLayerProperties> queriedLayers(layerCount);
            if (layerCount)
            {
                vkEnumerateInstanceLayerProperties(&layerCount, queriedLayers.data());
            }

            const auto has_layer = [&](const char *name) -> bool {
                auto itr = std::find_if(queriedLayers.begin(), queriedLayers.end(), [name](const VkLayerProperties &e) -> bool {
                    return strcmp(e.layerName, name) == 0;
                    });
                return itr != queriedLayers.end();
            };

            if (!_s_gpu_vk_data.EXT_debug_utils && _s_gpu_vk_data.EXT_debug_report)
            {
                instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
            }

            bool force_no_validation = false;
            /*if (getenv("VGPU_VULKAN_NO_VALIDATION"))
            {
                force_no_validation = true;
            }*/

            if (!force_no_validation && has_layer("VK_LAYER_LUNARG_standard_validation"))
            {
                instanceLayers.push_back("VK_LAYER_LUNARG_standard_validation");
            }
            else
            {
                _validation = false;
            }
        }

        if (!_headless
            && _s_gpu_vk_data.KHR_surface)
        {
            instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

#if ALIMER_PLATFORM_WINDOWS
            if (_s_gpu_vk_data.KHR_win32_surface)
            {
                instanceExtensions.push_back("VK_KHR_win32_surface");
            }
#elif ALIMER_PLATFORM_APPLE
            if (_s_gpu_vk_data.MVK_macos_surface)
            {
                instanceExtensions.push_back("VK_MVK_macos_surface");
            }
#elif ALIMER_PLATFORM_LINUX
            if (strcmp(_s_gpu_vk_data.KHR_xlib_surface)
            {
                instanceExtensions.push_back("VK_KHR_xlib_surface");
            }
            else if (_s_gpu_vk_data.KHR_xcb_surface)
            {
                instanceExtensions.push_back("VK_KHR_xcb_surface");
            }
            else if (_s_gpu_vk_data.KHR_wayland_surface)
            {
                instanceExtensions.push_back("VK_KHR_wayland_surface");
            }
#elif ALIMER_PLATFORM_ANDROID
            if (_s_gpu_vk_data.KHR_android_surface)
            {
                instanceExtensions.push_back("VK_KHR_android_surface")
            }
#endif
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

        VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &_instance);
        if (result != VK_SUCCESS)
        {
            ALIMER_LOGCRITICAL("Vulkan: Failed to create instance", vkGetVulkanResultString(result));
            return;
        }

        volkLoadInstance(_instance);

        if (_validation)
        {
            if (_s_gpu_vk_data.EXT_debug_utils)
            {
                VkDebugUtilsMessengerCreateInfoEXT info = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
                info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
                info.pfnUserCallback = vulkan_messenger_cb;
                info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
                info.pUserData = this;
                vkCreateDebugUtilsMessengerEXT(_instance, &info, nullptr, &_debugMessenger);
            }
            else if (_s_gpu_vk_data.EXT_debug_report)
            {
                VkDebugReportCallbackCreateInfoEXT info = { VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT };
                info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
                    VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
                info.pfnCallback = vulkan_debug_cb;
                info.pUserData = this;
                vkCreateDebugReportCallbackEXT(_instance, &info, nullptr, &_debugCallback);
            }
        }

        uint32_t gpuCount = 0;
        vkEnumeratePhysicalDevices(_instance, &gpuCount, nullptr);
        if (gpuCount == 0)
        {
            ALIMER_LOGCRITICAL("Vulkan: No physical device detected");
            return;
        }

        std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
        vkEnumeratePhysicalDevices(_instance, &gpuCount, physicalDevices.data());
        uint32_t bestDeviceScore = 0U;
        uint32_t bestDeviceIndex = ~0u;
        const bool preferDiscrete = true;
        for (uint32_t i = 0; i < gpuCount; ++i)
        {
            uint32_t score = 0U;
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(physicalDevices[i], &properties);
            switch (properties.deviceType)
            {
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                score += 100U;
                if (preferDiscrete) {
                    score += 1000U;
                }
                break;
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                score += 90U;
                if (!preferDiscrete) {
                    score += 1000U;
                }
                break;
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                score += 80U;
                break;
            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                score += 70U;
                break;
            default: score += 10U;
            }

            if (score > bestDeviceScore) {
                bestDeviceIndex = i;
                bestDeviceScore = score;
            }
        }

        if (bestDeviceIndex == ~0u)
        {
            ALIMER_LOGCRITICAL("Vulkan: No physical device supported.");
            return;
        }

        _physicalDevice = physicalDevices[bestDeviceIndex];
        InitializeFeatures();
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

    VkSurfaceKHR GPUDeviceVk::CreateSurface(const SwapChainHandle* handle)
    {
        VkResult result = VK_SUCCESS;
        VkSurfaceKHR surface = VK_NULL_HANDLE;

#if defined(VK_USE_PLATFORM_WIN32_KHR)
        VkWin32SurfaceCreateInfoKHR surfaceCreateInfo;
        memset(&surfaceCreateInfo, 0, sizeof(surfaceCreateInfo));
        surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        if (handle->nativeDisplay != nullptr)
        {
            surfaceCreateInfo.hinstance = (HINSTANCE)handle->nativeDisplay;
        }
        else
        {
            surfaceCreateInfo.hinstance = GetModuleHandleW(nullptr);
        }

        surfaceCreateInfo.hwnd = (HWND)handle->nativeHandle;
        PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHRProc = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(_instance, "vkCreateWin32SurfaceKHR");
        result = vkCreateWin32SurfaceKHRProc(_instance, &surfaceCreateInfo, nullptr, &surface);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
        VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo;
        memset(&surfaceCreateInfo, 0, sizeof(surfaceCreateInfo));
        surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
        surfaceCreateInfo.window = (ANativeWindow)handle->nativeHandle;
        PFN_vkCreateAndroidSurfaceKHR vkCreateAndroidSurfaceKHRProc = (PFN_vkCreateAndroidSurfaceKHR)vkGetInstanceProcAddr(_instance, "vkCreateAndroidSurfaceKHR");
        result = vkCreateAndroidSurfaceKHRProc(_instance, &surfaceCreateInfo, NULL, &surface);
#elif defined(VK_USE_PLATFORM_IOS_MVK)
        VkIOSSurfaceCreateInfoMVK surfaceCreateInfo;
        memset(&surfaceCreateInfo, 0, sizeof(surfaceCreateInfo));
        surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_IOS_SURFACE_CREATE_INFO_MVK;
        surfaceCreateInfo.pView = handle->nativeHandle;
        PFN_vkCreateIOSSurfaceMVK vkCreateIOSSurfaceMVKProc = (PFN_vkCreateIOSSurfaceMVK)vkGetInstanceProcAddr(_instance, "vkCreateIOSSurfaceMVK");
        result = vkCreateIOSSurfaceMVKProc(_instance, &surfaceCreateInfo, nullptr, &surface);
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
        VkMacOSSurfaceCreateInfoMVK surfaceCreateInfo;
        memset(&surfaceCreateInfo, 0, sizeof(surfaceCreateInfo));
        surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
        surfaceCreateInfo.pView = handle->nativeHandle;
        PFN_vkCreateMacOSSurfaceMVK vkCreateMacOSSurfaceMVKProc = (PFN_vkCreateMacOSSurfaceMVK)vkGetInstanceProcAddr(_instance, "vkCreateMacOSSurfaceMVK");
        result = vkCreateMacOSSurfaceMVKProc(_instance, &surfaceCreateInfo, NULL, &surface);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
        VkWaylandSurfaceCreateInfoKHR surfaceCreateInfo;
        memset(&surfaceCreateInfo, 0, sizeof(surfaceCreateInfo));
        surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
        surfaceCreateInfo.display = (wl_display*)handle->display;
        surfaceCreateInfo.surface = (wl_surface*)handle->windowHandle;
        PFN_vkCreateWaylandSurfaceKHR vkCreateWaylandSurfaceKHRProc = (PFN_vkCreateWaylandSurfaceKHR)vkGetInstanceProcAddr(_instance, "vkCreateWaylandSurfaceKHR");
        result = vkCreateWaylandSurfaceKHRProc(_instance, &surfaceCreateInfo, nullptr, &surface);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
        VkXcbSurfaceCreateInfoKHR surfaceCreateInfo;
        memset(&surfaceCreateInfo, 0, sizeof(surfaceCreateInfo));
        surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
        surfaceCreateInfo.connection = (xcb_connection_t*)handle->nativeDisplay;
        surfaceCreateInfo.window = (xcb_window_t)handle->nativeHandle;
        PFN_vkCreateXcbSurfaceKHR vkCreateXcbSurfaceKHRProc = (PFN_vkCreateXcbSurfaceKHR)vkGetInstanceProcAddr(_instance, "vkCreateXcbSurfaceKHR");
        result = vkCreateXcbSurfaceKHRProc(_instance, &surfaceCreateInfo, nullptr, &surface);
#endif

        if (result != VK_SUCCESS)
        {
            ALIMER_LOGERROR("Vulkan: Failed to create platform surface");
            return nullptr;
        }

        return surface;
    }

    void GPUDeviceVk::WaitIdle()
    {
        VkResult result = vkDeviceWaitIdle(_device);
        if (result < VK_SUCCESS)
        {
            ALIMER_LOGTRACE("Vulkan: vkDeviceWaitIdle failed : %s", vkGetVulkanResultString(result));
        }
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
