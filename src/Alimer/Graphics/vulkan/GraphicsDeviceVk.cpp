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

#include "GraphicsDeviceVk.h"
#include "SwapChainVk.h"
#include "../Window.h"

#if defined(_WIN32) || defined(_WIN64)
#   define VK_SURFACE_EXT               "VK_KHR_win32_surface"
#elif defined(__ANDROID__)
#   define VK_SURFACE_EXT               "VK_KHR_android_surface"
#elif defined(__linux__)
#   define VK_SURFACE_EXT               "VK_KHR_xcb_surface"
#endif

#include "volk.h"
#define VMA_IMPLEMENTATION 1
#define VMA_STATIC_VULKAN_FUNCTION 0
#define VMA_STATS_STRING_ENABLED 0
#include "vk_mem_alloc.h"

using namespace std;

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

#if VULKAN_DEBUG
    static VKAPI_ATTR VkBool32 VKAPI_CALL vgpuVkMessengerCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT                  messageType,
        const VkDebugUtilsMessengerCallbackDataEXT*      pCallbackData,
        void *pUserData)
    {
        //auto device = static_cast<GraphicsDeviceVk*>(pUserData);

        switch (messageSeverity)
        {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
            {
                ALIMER_LOGERROR("[Vulkan]: Validation Error: %s", pCallbackData->pMessage);
                //device->NotifyValidationError(pCallbackData->pMessage);
            }
            else
            {
                ALIMER_LOGERROR("[Vulkan]: Other Error: %s", pCallbackData->pMessage);
            }

            break;

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
            {
                ALIMER_LOGWARN("[Vulkan]: Validation Warning: %s", pCallbackData->pMessage);
            }
            else
            {
                ALIMER_LOGWARN("[Vulkan]: Other Warning: %s", pCallbackData->pMessage);
            }
            break;

#if 0
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
                ALIMER_LOGINFO("[Vulkan]: Validation Info: %s", pCallbackData->pMessage);
            else
                ALIMER_LOGINFO("[Vulkan]: Other Info: %s", pCallbackData->pMessage);
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
                ALIMER_LOGINFO("  Object #%u: %s", i, name ? name : "N/A");
            }
        }

        return VK_FALSE;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL vgpuVkDebugCallback(VkDebugReportFlagsEXT flags,
        VkDebugReportObjectTypeEXT, uint64_t,
        size_t, int32_t messageCode, const char *pLayerPrefix,
        const char *pMessage, void *pUserData)
    {
        //auto device = static_cast<GraphicsDeviceVk*>(pUserData);

        // False positives about lack of srcAccessMask/dstAccessMask.
        if (strcmp(pLayerPrefix, "DS") == 0 && messageCode == 10)
        {
            return VK_FALSE;
        }

        if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
        {
            ALIMER_LOGERROR("[Vulkan]: %s: %s", pLayerPrefix, pMessage);
            //device->NotifyValidationError(pMessage);
        }
        else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
        {
            ALIMER_LOGWARN("[Vulkan]: %s: %s", pLayerPrefix, pMessage);
        }
        else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
        {
            ALIMER_LOGWARN("[Vulkan]: Performance warning: %s: %s", pLayerPrefix, pMessage);
        }
        else
        {
            ALIMER_LOGINFO("[Vulkan]: %s: %s", pLayerPrefix, pMessage);
        }

        return VK_FALSE;
    }
#endif /* VULKAN_DEBUG */

    bool GraphicsImpl::IsSupported()
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

    GraphicsImpl::GraphicsImpl(const char* applicationName, GpuPreference devicePreference)
    {
        ALIMER_ASSERT_MSG(IsSupported(), "Vulkan backend is not supported");

        CreateInstance(applicationName);
        SelectPhysicalDevice(devicePreference);
        ALIMER_LOGINFO("Vulkan backend created with success.");
    }

    GraphicsImpl::~GraphicsImpl()
    {
        WaitIdle();

        for (auto &fence : _fences)
        {
            vkDestroyFence(_device, fence, nullptr);
        }
        for (auto &semaphore : _semaphores)
        {
            vkDestroySemaphore(_device, semaphore, nullptr);
        }
        _fences.clear();
        _semaphores.clear();
        vkDestroyPipelineCache(_device, _pipelineCache, nullptr);

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

    void GraphicsImpl::CreateInstance(const char* applicationName)
    {
        VkApplicationInfo appInfo;
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pNext = nullptr;
        appInfo.pApplicationName = applicationName;
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "ALIMER_VERSION_PATCH";
        appInfo.engineVersion = VK_MAKE_VERSION(ALIMER_VERSION_MAJOR, ALIMER_VERSION_MINOR, ALIMER_VERSION_PATCH);
        appInfo.apiVersion = volkGetInstanceVersion();

        if (appInfo.apiVersion >= VK_API_VERSION_1_1)
        {
            _featuresVk.supportsVulkan11Instance = true;
            appInfo.apiVersion = VK_API_VERSION_1_1;
        }

        vector<const char*> instanceLayers;
        vector<const char*> instanceExtensions;

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

#if VULKAN_DEBUG
        if (!_vk.EXT_debug_utils && _vk.EXT_debug_report)
        {
            instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        }

        const bool force_no_validation = false;
        /*if (getenv("VGPU_VULKAN_NO_VALIDATION"))
        {
            force_no_validation = true;
        }*/

        if (!force_no_validation && _vk.VK_LAYER_LUNARG_standard_validation)
        {
            instanceLayers.push_back("VK_LAYER_LUNARG_standard_validation");
        }
#endif

        if (!_headless
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

#if VULKAN_DEBUG
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
#endif /* VULKAN_DEBUG */
    }

    void GraphicsImpl::SelectPhysicalDevice(GpuPreference devicePreference)
    {
        uint32_t gpuCount = 0;
        vkEnumeratePhysicalDevices(_instance, &gpuCount, nullptr);
        if (gpuCount == 0)
        {
            ALIMER_LOGCRITICAL("Vulkan: No physical device detected");
            return;
        }

        VkPhysicalDevice* physicalDevices = (VkPhysicalDevice*)alloca(gpuCount * sizeof(VkPhysicalDevice));
        vkEnumeratePhysicalDevices(_instance, &gpuCount, physicalDevices);
        uint32_t bestDeviceScore = 0U;
        uint32_t bestDeviceIndex = static_cast<uint32_t>(-1);
        for (uint32_t i = 0; i < gpuCount; ++i)
        {
            uint32_t score = 0U;
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(physicalDevices[i], &properties);

            ALIMER_LOGTRACE("Found Vulkan GPU: %s", properties.deviceName);
            ALIMER_LOGTRACE("    API: %d.%d.%d",
                VK_VERSION_MAJOR(properties.apiVersion),
                VK_VERSION_MINOR(properties.apiVersion),
                VK_VERSION_PATCH(properties.apiVersion));
            ALIMER_LOGTRACE("    Driver: %d.%d.%d",
                VK_VERSION_MAJOR(properties.driverVersion),
                VK_VERSION_MINOR(properties.driverVersion),
                VK_VERSION_PATCH(properties.driverVersion));

            switch (properties.deviceType)
            {
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                score += 100u;
                if (devicePreference == GpuPreference::HighPerformance) {
                    score += 1000u;
                }
                break;
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                score += 90u;
                if (devicePreference == GpuPreference::LowPower) {
                    score += 1000u;
                }
                break;
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                score += 80u;
                break;
            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                score += 70u;
                break;
            default:
                score += 10u;
                break;
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
        vkGetPhysicalDeviceProperties(_physicalDevice, &_physicalDeviceProperties);
        vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &_physicalDeviceMemoryProperties);
        vkGetPhysicalDeviceFeatures(_physicalDevice, &_physicalDeviceFeatures);

        // Queue family properties, used for setting up requested queues upon device creation
        uint32_t count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &count, nullptr);
        ALIMER_ASSERT(count > 0);
        _physicalDeviceQueueFamilyProperties.resize(count);
        vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &count, _physicalDeviceQueueFamilyProperties.data());

        // Get list of supported extensions
        vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &count, nullptr);
        if (vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &count, nullptr) == VK_SUCCESS &&
            count > 0)
        {
            _physicalDeviceExtensions.resize(count);
            std::vector<VkExtensionProperties> extensions(count);
            if (vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &count, &extensions.front()) == VK_SUCCESS)
            {
                for (auto ext : extensions)
                {
                    _physicalDeviceExtensions.push_back(ext.extensionName);
                }
            }
        }

        if (vkEnumerateDeviceLayerProperties(_physicalDevice, &count, nullptr) == VK_SUCCESS &&
            count > 0)
        {
            vector<VkLayerProperties> queriedLayers(count);
            if (vkEnumerateDeviceLayerProperties(_physicalDevice, &count, &queriedLayers.front()) == VK_SUCCESS)
            {
                for (auto layer : queriedLayers)
                {
                    _physicalDeviceLayers.push_back(layer.layerName);
                }
            }
        }

        ALIMER_LOGDEBUG("Selected Vulkan GPU: %s", _physicalDeviceProperties.deviceName);
    }

    bool GraphicsImpl::Initialize(Window* window, SampleCount samples)
    {
        _surface = CreateSurface(window);
        CreateLogicalDevice();
        InitializeInfoAndCaps();

        // Create default graphics command pool
        VkCommandPoolCreateInfo cmdPoolCreateInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO  };
        cmdPoolCreateInfo.queueFamilyIndex = _graphicsQueueFamily;
        cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        vkThrowIfFailed(vkCreateCommandPool(_device, &cmdPoolCreateInfo, nullptr, &_graphicsCommandPool));

        // Create swap chain.
        SwapChainDescriptor swapChainDescriptor = {};
        swapChainDescriptor.width = window->GetWidth();
        swapChainDescriptor.height = window->GetHeight();
        _swapChain.Reset(new SwapChainVk(this, _surface, &swapChainDescriptor));

        // Pipeline cache
        VkPipelineCacheCreateInfo pipelineCacheCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO };
        vkThrowIfFailed(
            vkCreatePipelineCache(_device, &pipelineCacheCreateInfo, nullptr, &_pipelineCache)
        );


        return true;
    }

    void GraphicsImpl::CreateLogicalDevice()
    {
        /// Create surface and logical device,
        /// some code adapted from https://github.com/Themaister/Granite
        if (_physicalDeviceProperties.apiVersion >= VK_API_VERSION_1_1)
        {
            _featuresVk.supportsVulkan11Device = _featuresVk.supportsVulkan11Instance;
        }
        else if (_physicalDeviceProperties.apiVersion >= VK_API_VERSION_1_0)
        {
            _featuresVk.supportsVulkan11Device = false;
        }

        // Only need GetPhysicalDeviceProperties2 for Vulkan 1.1-only code, so don't bother getting KHR variant.
        _featuresVk.subgroupProperties = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES };
        VkPhysicalDeviceProperties2 props = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
        void **ppNext = &props.pNext;

        if (_featuresVk.supportsVulkan11Instance
            && _featuresVk.supportsVulkan11Device)
        {
            *ppNext = &_featuresVk.subgroupProperties;
            ppNext = &_featuresVk.subgroupProperties.pNext;
        }

        if (_featuresVk.supportsVulkan11Instance
            && _featuresVk.supportsVulkan11Device)
        {
            vkGetPhysicalDeviceProperties2(_physicalDevice, &props);
        }

        // Find queue's family
        const uint32_t queueCount = (uint32_t)_physicalDeviceQueueFamilyProperties.size();
        for (uint32_t i = 0u; i < queueCount; i++)
        {
            VkBool32 supportPresent;
            vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, i, _surface, &supportPresent);

            if (_presentQueueFamily == VK_QUEUE_FAMILY_IGNORED &&
                supportPresent)
            {
                _presentQueueFamily = i;
            }

            static const VkQueueFlags required = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT;
            if (supportPresent && ((_physicalDeviceQueueFamilyProperties[i].queueFlags & required) == required))
            {
                _graphicsQueueFamily = i;
                break;
            }
        }

        for (uint32_t i = 0u; i < queueCount; i++)
        {
            static const VkQueueFlags required = VK_QUEUE_COMPUTE_BIT;
            if (i != _graphicsQueueFamily
                && (_physicalDeviceQueueFamilyProperties[i].queueFlags & required) == required)
            {
                _computeQueueFamily = i;
                break;
            }
        }

        for (uint32_t i = 0u; i < queueCount; i++)
        {
            static const VkQueueFlags required = VK_QUEUE_TRANSFER_BIT;
            if (i != _graphicsQueueFamily
                && i != _computeQueueFamily
                && (_physicalDeviceQueueFamilyProperties[i].queueFlags & required) == required)
            {
                _transferQueueFamily = i;
                break;
            }
        }

        if (_transferQueueFamily == VK_QUEUE_FAMILY_IGNORED)
        {
            for (uint32_t i = 0u; i < queueCount; i++)
            {
                static const VkQueueFlags required = VK_QUEUE_TRANSFER_BIT;
                if (i != _graphicsQueueFamily
                    && (_physicalDeviceQueueFamilyProperties[i].queueFlags & required) == required)
                {
                    _transferQueueFamily = i;
                    break;
                }
            }
        }

        if (_presentQueueFamily == VK_QUEUE_FAMILY_IGNORED && _graphicsQueueFamily == VK_QUEUE_FAMILY_IGNORED)
        {
            return;
        }

        // Setup queues
        uint32_t universalQueueIndex = 1;
        uint32_t computeQueueIndex = 0;
        uint32_t transferQueueIndex = 0;

        if (_computeQueueFamily == VK_QUEUE_FAMILY_IGNORED)
        {
            _computeQueueFamily = _graphicsQueueFamily;
            computeQueueIndex = std::min(_physicalDeviceQueueFamilyProperties[_graphicsQueueFamily].queueCount - 1, universalQueueIndex);
            universalQueueIndex++;
        }

        if (_transferQueueFamily == VK_QUEUE_FAMILY_IGNORED)
        {
            _transferQueueFamily = _graphicsQueueFamily;
            transferQueueIndex = std::min(_physicalDeviceQueueFamilyProperties[_graphicsQueueFamily].queueCount - 1, universalQueueIndex);
            universalQueueIndex++;
        }
        else if (_transferQueueFamily == _computeQueueFamily)
        {
            transferQueueIndex = std::min(_physicalDeviceQueueFamilyProperties[_computeQueueFamily].queueCount - 1, 1u);
        }

        static const float graphicsQueuePriority = 0.5f;
        static const float computeQueuePriority = 1.0f;
        static const float transferQueuePriority = 1.0f;
        float queuePriorities[3] = { graphicsQueuePriority, computeQueuePriority, transferQueuePriority };

        uint32_t queueFamilyCount = 0;
        VkDeviceQueueCreateInfo queue_info[3] = {};

        VkDeviceCreateInfo device_info = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
        device_info.pQueueCreateInfos = queue_info;

        queue_info[queueFamilyCount].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info[queueFamilyCount].queueFamilyIndex = _graphicsQueueFamily;
        queue_info[queueFamilyCount].queueCount = std::min(universalQueueIndex,
            _physicalDeviceQueueFamilyProperties[_graphicsQueueFamily].queueCount);
        queue_info[queueFamilyCount].pQueuePriorities = queuePriorities;
        queueFamilyCount++;

        if (_computeQueueFamily != _graphicsQueueFamily)
        {
            queue_info[queueFamilyCount].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_info[queueFamilyCount].queueFamilyIndex = _computeQueueFamily;
            queue_info[queueFamilyCount].queueCount = std::min(_transferQueueFamily == _computeQueueFamily ? 2u : 1u,
                _physicalDeviceQueueFamilyProperties[_computeQueueFamily].queueCount);
            queue_info[queueFamilyCount].pQueuePriorities = queuePriorities + 1;
            queueFamilyCount++;
        }

        if (_transferQueueFamily != _graphicsQueueFamily
            && _transferQueueFamily != _computeQueueFamily)
        {
            queue_info[queueFamilyCount].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_info[queueFamilyCount].queueFamilyIndex = _transferQueueFamily;
            queue_info[queueFamilyCount].queueCount = 1;
            queue_info[queueFamilyCount].pQueuePriorities = queuePriorities + 2;
            queueFamilyCount++;
        }

        device_info.queueCreateInfoCount = queueFamilyCount;

        vector<const char*> requiredDeviceExtensions;
        if (!_headless)
        {
            requiredDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        }

        // VK_KHR_maintenance1 to support nevagive viewport and match DirectX coordinate system.
        requiredDeviceExtensions.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);

        vector<const char*> enabledExtensions;

        for (size_t i = 0; i < requiredDeviceExtensions.size(); i++)
        {
            if (!HasExtension(requiredDeviceExtensions[i]))
            {
                ALIMER_LOGERROR("Vulkan: required extension is not supported '%d'", requiredDeviceExtensions[i]);
                return;
            }

            enabledExtensions.push_back(requiredDeviceExtensions[i]);
        }

        if (HasExtension(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME) &&
            HasExtension(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME))
        {
            _featuresVk.supportsDedicated = true;
            enabledExtensions.push_back(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
            enabledExtensions.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        }

        if (HasExtension(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME))
        {
            _featuresVk.supportsImageFormatList = true;
            enabledExtensions.push_back(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
        }

        if (HasExtension(VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
        {
            _featuresVk.supportsDebugMarker = true;
            enabledExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
        }

        if (HasExtension(VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME))
        {
            _featuresVk.supportsMirrorClampToEdge = true;
            enabledExtensions.push_back(VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME);
        }

        if (HasExtension(VK_GOOGLE_DISPLAY_TIMING_EXTENSION_NAME))
        {
            _featuresVk.supportsGoogleDisplayTiming = true;
            enabledExtensions.push_back(VK_GOOGLE_DISPLAY_TIMING_EXTENSION_NAME);
        }

#ifdef _WIN32
        _featuresVk.supportsExternal = false;
#else
        if (_featuresVk.supportsExternal
            && _featuresVk.supportsDedicated
            && HasExtension(VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME)
            && HasExtension(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME)
            && HasExtension(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME)
            && HasExtension(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME))
        {
            _featuresVk.supportsExternal = true;
            enabledExtensions.push_back(VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME);
            enabledExtensions.push_back(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME);
            enabledExtensions.push_back(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
            enabledExtensions.push_back(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
        }
        else {
            _featuresVk.supportsExternal = false;
        }
#endif

        VkPhysicalDeviceFeatures2KHR features = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR };
        _featuresVk.storage8BitFeatures = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES_KHR };
        _featuresVk.storage16BitFeatures = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES_KHR };
        _featuresVk.float16Int8Features = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT16_INT8_FEATURES_KHR };
        ppNext = &features.pNext;

        if (HasExtension(VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME))
        {
            enabledExtensions.push_back(VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME);
        }

        if (_featuresVk.supportsPhysicalDeviceProperties2
            && HasExtension(VK_KHR_8BIT_STORAGE_EXTENSION_NAME))
        {
            enabledExtensions.push_back(VK_KHR_8BIT_STORAGE_EXTENSION_NAME);
            *ppNext = &_featuresVk.storage8BitFeatures;
            ppNext = &_featuresVk.storage8BitFeatures.pNext;
        }

        if (_featuresVk.supportsPhysicalDeviceProperties2
            && HasExtension(VK_KHR_16BIT_STORAGE_EXTENSION_NAME))
        {
            enabledExtensions.push_back(VK_KHR_16BIT_STORAGE_EXTENSION_NAME);
            *ppNext = &_featuresVk.storage16BitFeatures;
            ppNext = &_featuresVk.storage16BitFeatures.pNext;
        }

        if (_featuresVk.supportsPhysicalDeviceProperties2
            && HasExtension(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME))
        {
            enabledExtensions.push_back(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
            *ppNext = &_featuresVk.float16Int8Features;
            ppNext = &_featuresVk.float16Int8Features.pNext;
        }

        if (_featuresVk.supportsPhysicalDeviceProperties2)
        {
            vkGetPhysicalDeviceFeatures2KHR(_physicalDevice, &features);
        }
        else
        {
            vkGetPhysicalDeviceFeatures(_physicalDevice, &features.features);
        }

        // Enable device features we use.
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
            _featuresVk.enabledFeatures = enabled_features;
        }

        if (_featuresVk.supportsPhysicalDeviceProperties2)
        {
            device_info.pNext = &features;
        }
        else
        {
            device_info.pEnabledFeatures = &features.features;
        }

        device_info.enabledExtensionCount = (uint32_t)enabledExtensions.size();
        device_info.ppEnabledExtensionNames = enabledExtensions.data();
        // Deprecated and ignored.
        device_info.enabledLayerCount = 0;
        device_info.ppEnabledLayerNames = nullptr;

        if (vkCreateDevice(_physicalDevice, &device_info, nullptr, &_device) != VK_SUCCESS)
        {
            ALIMER_LOGERROR("Vulkan: Failed to create device");
            return;
        }

        volkLoadDevice(_device);
        vkGetDeviceQueue(_device, _graphicsQueueFamily, 0, &_graphicsQueue);
        vkGetDeviceQueue(_device, _computeQueueFamily, computeQueueIndex, &_computeQueue);
        vkGetDeviceQueue(_device, _transferQueueFamily, transferQueueIndex, &_transferQueue);

        // Initialize vma memory allocator
        VmaVulkanFunctions vma_vulkan_func = {};
        vma_vulkan_func.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
        vma_vulkan_func.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
        vma_vulkan_func.vkAllocateMemory = vkAllocateMemory;
        vma_vulkan_func.vkFreeMemory = vkFreeMemory;
        vma_vulkan_func.vkMapMemory = vkMapMemory;
        vma_vulkan_func.vkUnmapMemory = vkUnmapMemory;
        vma_vulkan_func.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
        vma_vulkan_func.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
        vma_vulkan_func.vkBindBufferMemory = vkBindBufferMemory;
        vma_vulkan_func.vkBindImageMemory = vkBindImageMemory;
        vma_vulkan_func.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
        vma_vulkan_func.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
        vma_vulkan_func.vkCreateBuffer = vkCreateBuffer;
        vma_vulkan_func.vkCreateImage = vkCreateImage;
        vma_vulkan_func.vkDestroyBuffer = vkDestroyBuffer;
        vma_vulkan_func.vkDestroyImage = vkDestroyImage;
        vma_vulkan_func.vkCmdCopyBuffer = vkCmdCopyBuffer;

#if VMA_DEDICATED_ALLOCATION
        vma_vulkan_func.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR;
        vma_vulkan_func.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2KHR;
#endif

        VmaAllocatorCreateInfo allocatorCreateInfo = {};
        allocatorCreateInfo.physicalDevice = _physicalDevice;
        allocatorCreateInfo.device = _device;
        allocatorCreateInfo.pVulkanFunctions = &vma_vulkan_func;
        VkResult result = vmaCreateAllocator(&allocatorCreateInfo, &_memoryAllocator);

        if (result != VK_SUCCESS)
        {
            ALIMER_LOGERROR("Vulkan: Failed to create vma allocator");
            return;
        }
    }

    void GraphicsImpl::InitializeInfoAndCaps()
    {
        _info.backend = GraphicsBackend::Vulkan;
        _info.backendName = "Vulkan " + VKApiVersionToString(_physicalDeviceProperties.apiVersion);
        _info.deviceName = _physicalDeviceProperties.deviceName;
        _info.vendorName = GetVendorByID(_physicalDeviceProperties.vendorID);
        _info.vendorId = _physicalDeviceProperties.vendorID;

        _caps.features.instancing = true;
        _caps.features.alphaToCoverage = true;
        _caps.features.independentBlend = _physicalDeviceFeatures.independentBlend;
        _caps.features.computeShader = true;
        _caps.features.geometryShader = _physicalDeviceFeatures.geometryShader;
        _caps.features.tessellationShader = _physicalDeviceFeatures.tessellationShader;
        _caps.features.sampleRateShading = _physicalDeviceFeatures.sampleRateShading;
        _caps.features.dualSrcBlend = _physicalDeviceFeatures.dualSrcBlend;
        _caps.features.logicOp = _physicalDeviceFeatures.logicOp;
        _caps.features.multiViewport = _physicalDeviceFeatures.multiViewport;
        _caps.features.indexUInt32 = _physicalDeviceFeatures.fullDrawIndexUint32;
        _caps.features.drawIndirect = _physicalDeviceFeatures.multiDrawIndirect;
        _caps.features.alphaToOne = _physicalDeviceFeatures.alphaToOne;
        _caps.features.fillModeNonSolid = _physicalDeviceFeatures.fillModeNonSolid;
        _caps.features.samplerAnisotropy = _physicalDeviceFeatures.samplerAnisotropy;
        _caps.features.textureCompressionBC = _physicalDeviceFeatures.textureCompressionBC;
        _caps.features.textureCompressionPVRTC = false;
        _caps.features.textureCompressionETC2 = _physicalDeviceFeatures.textureCompressionETC2;
        _caps.features.textureCompressionATC = false;
        _caps.features.textureCompressionASTC = _physicalDeviceFeatures.textureCompressionASTC_LDR;
        _caps.features.pipelineStatisticsQuery = _physicalDeviceFeatures.pipelineStatisticsQuery;
        _caps.features.texture1D = true;
        _caps.features.texture3D = true;
        _caps.features.texture2DArray = true;
        _caps.features.textureCubeArray = _physicalDeviceFeatures.imageCubeArray;

        // TODO: Raytracing
        _caps.features.raytracing = false;

        // Limits
        _caps.limits.maxTextureDimension1D = _physicalDeviceProperties.limits.maxImageDimension1D;
        _caps.limits.maxTextureDimension2D = _physicalDeviceProperties.limits.maxImageDimension2D;
        _caps.limits.maxTextureDimension3D = _physicalDeviceProperties.limits.maxImageDimension3D;
        _caps.limits.maxTextureDimensionCube = _physicalDeviceProperties.limits.maxImageDimensionCube;
        _caps.limits.maxTextureArrayLayers = _physicalDeviceProperties.limits.maxImageArrayLayers;
        _caps.limits.maxColorAttachments = _physicalDeviceProperties.limits.maxColorAttachments;
        _caps.limits.maxUniformBufferSize = _physicalDeviceProperties.limits.maxUniformBufferRange;
        _caps.limits.minUniformBufferOffsetAlignment = _physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;
        _caps.limits.maxStorageBufferSize = _physicalDeviceProperties.limits.maxStorageBufferRange;
        _caps.limits.minStorageBufferOffsetAlignment = _physicalDeviceProperties.limits.minStorageBufferOffsetAlignment;
        _caps.limits.maxSamplerAnisotropy = static_cast<uint32_t>(_physicalDeviceProperties.limits.maxSamplerAnisotropy);
        _caps.limits.maxViewports = _physicalDeviceProperties.limits.maxViewports;
        _caps.limits.maxViewportDimensions[0] = _physicalDeviceProperties.limits.maxViewportDimensions[0];
        _caps.limits.maxViewportDimensions[1] = _physicalDeviceProperties.limits.maxViewportDimensions[1];
        _caps.limits.maxPatchVertices = _physicalDeviceProperties.limits.maxTessellationPatchSize;
        _caps.limits.pointSizeRange[0] = _physicalDeviceProperties.limits.pointSizeRange[0];
        _caps.limits.pointSizeRange[1] = _physicalDeviceProperties.limits.pointSizeRange[1];
        _caps.limits.lineWidthRange[0] = _physicalDeviceProperties.limits.lineWidthRange[0];
        _caps.limits.lineWidthRange[1] = _physicalDeviceProperties.limits.lineWidthRange[0];
        _caps.limits.maxComputeSharedMemorySize = _physicalDeviceProperties.limits.maxComputeSharedMemorySize;
        _caps.limits.maxComputeWorkGroupCount[0] = _physicalDeviceProperties.limits.maxComputeWorkGroupCount[0];
        _caps.limits.maxComputeWorkGroupCount[1] = _physicalDeviceProperties.limits.maxComputeWorkGroupCount[1];
        _caps.limits.maxComputeWorkGroupCount[2] = _physicalDeviceProperties.limits.maxComputeWorkGroupCount[2];
        _caps.limits.maxComputeWorkGroupInvocations = _physicalDeviceProperties.limits.maxComputeWorkGroupInvocations;
        _caps.limits.maxComputeWorkGroupSize[0] = _physicalDeviceProperties.limits.maxComputeWorkGroupSize[0];
        _caps.limits.maxComputeWorkGroupSize[1] = _physicalDeviceProperties.limits.maxComputeWorkGroupSize[1];
        _caps.limits.maxComputeWorkGroupSize[2] = _physicalDeviceProperties.limits.maxComputeWorkGroupSize[2];
    }

    void GraphicsImpl::WaitIdle()
    {
        vkThrowIfFailed(
            vkDeviceWaitIdle(_device)
        );
    }

    VkSurfaceKHR GraphicsImpl::CreateSurface(Window* window)
    {
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        VkResult result = VK_SUCCESS;

#if defined(_WIN32) || defined(_WIN64)
        VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
        surfaceCreateInfo.hinstance = window->GetNativeConnection()/* GetModuleHandleW(nullptr)*/;
        surfaceCreateInfo.hwnd = window->GetNativeHandle();
        result = vkCreateWin32SurfaceKHR(_instance, &surfaceCreateInfo, nullptr, &surface);
#elif defined(__ANDROID__)
        VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo = { VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR  };
        surfaceCreateInfo.window = window->GetNativeHandle();
        result = vkCreateAndroidSurfaceKHR(_instance, &surfaceCreateInfo, nullptr, &surface);
#elif defined(__linux__)
        VkXcbSurfaceCreateInfoKHR surfaceCreateInfo = { VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR };
        surfaceCreateInfo.connection = window->GetNativeConnection();
        surfaceCreateInfo.window = window->GetNativeHandle();
        result = vkCreateXcbSurfaceKHR(_instance, &surfaceCreateInfo, nullptr, &surface);
#elif defined(VK_USE_PLATFORM_IOS_MVK)
        VkIOSSurfaceCreateInfoMVK surfaceCreateInfo = { VK_STRUCTURE_TYPE_IOS_SURFACE_CREATE_INFO_MVK };
        surfaceCreateInfo.pView = nativeHandle;
        result = vkCreateIOSSurfaceMVK(_instance, &surfaceCreateInfo, nullptr, &surface);
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
        VkMacOSSurfaceCreateInfoMVK surfaceCreateInfo = { VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK  };
        surfaceCreateInfo.pView = nativeHandle;
        result = vkCreateMacOSSurfaceMVK(_instance, &surfaceCreateInfo, nullptr, &surface);
#endif
       
        if (result != VK_SUCCESS)
        {
            ALIMER_LOGERROR("Vulkan: Failed to create platform surface");
            return VK_NULL_HANDLE;
        }

        return surface;
    }

    bool GraphicsImpl::BeginFrame()
    {
        // Wait to end previous frame.
        //VkFence fence = _waitFences[_frameIndex];
        //vkThrowIfFailed(vkWaitForFences(_device, 1, &fence, VK_TRUE, UINT64_MAX));
        //vkThrowIfFailed(vkResetFences(_device, 1, &fence));

        // Delete all pending/deferred resources
        //frame().ProcessDeferredDelete();

        // Acquire the next image from the swap chain
        _swapChain->AcquireNextTexture();

        return true;
    }

    void GraphicsImpl::EndFrame()
    {
        VkResult result = _swapChain->QueuePresent(_graphicsQueue);
        if (!((result == VK_SUCCESS) || (result == VK_SUBOPTIMAL_KHR)))
        {
            if (result == VK_ERROR_OUT_OF_DATE_KHR)
            {
                //_swapChain->Resize();
                return;
            }
            else {
                vkThrowIfFailed(result);
            }
        }

        // Advance frame index.
        //_frameIndex = (_frameIndex + 1u) % _maxInflightFrames;
    }

    bool GraphicsImpl::ImageFormatIsSupported(VkFormat format, VkFormatFeatureFlags required, VkImageTiling tiling) const
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(_physicalDevice, format, &props);
        VkFormatFeatureFlags flags = tiling == VK_IMAGE_TILING_OPTIMAL ? props.optimalTilingFeatures : props.linearTilingFeatures;
        return (flags & required) == required;
    }

    PixelFormat GraphicsImpl::GetDefaultDepthStencilFormat() const
    {
        if (ImageFormatIsSupported(VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL)) {
            return PixelFormat::Depth24UNormStencil8;
        }

        if (ImageFormatIsSupported(VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL)) {
            return PixelFormat::Depth32FloatStencil8;
        }

        return PixelFormat::Undefined;
    }

    PixelFormat GraphicsImpl::GetDefaultDepthFormat() const
    {
        if (ImageFormatIsSupported(VK_FORMAT_D32_SFLOAT, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL)) {
            return PixelFormat::Depth32Float;
        }

        if (ImageFormatIsSupported(VK_FORMAT_X8_D24_UNORM_PACK32, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL)) {
            return PixelFormat::Depth24UNormStencil8;
        }

        if (ImageFormatIsSupported(VK_FORMAT_D16_UNORM, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL)) {
            return PixelFormat::Depth16UNorm;
        }

        return PixelFormat::Undefined;
    }

    VkCommandBuffer GraphicsImpl::CreateCommandBuffer(VkCommandBufferLevel level, bool begin)
    {
        VkCommandBufferAllocateInfo commandBufferAllocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO  };
        commandBufferAllocateInfo.commandPool = _graphicsCommandPool;
        commandBufferAllocateInfo.level = level;
        commandBufferAllocateInfo.commandBufferCount = 1u;

        VkCommandBuffer commandBuffer;
        vkThrowIfFailed(vkAllocateCommandBuffers(_device, &commandBufferAllocateInfo, &commandBuffer));

        // If requested, also start the new command buffer
        if (begin)
        {
            VkCommandBufferBeginInfo cmdBufferBeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
            vkThrowIfFailed(vkBeginCommandBuffer(commandBuffer, &cmdBufferBeginInfo));
        }

        return commandBuffer;
    }

    void GraphicsImpl::FlushCommandBuffer(VkCommandBuffer commandBuffer, bool free)
    {
        vkThrowIfFailed(vkEndCommandBuffer(commandBuffer));

        VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO  };
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        // Create fence to ensure that the command buffer has finished executing
        VkFenceCreateInfo fenceCreateInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
        VkFence fence;
        vkThrowIfFailed(vkCreateFence(_device, &fenceCreateInfo, nullptr, &fence));

        vkThrowIfFailed(vkQueueSubmit(_graphicsQueue, 1, &submitInfo, fence));
        // Wait for the fence to signal that command buffer has finished executing
        vkThrowIfFailed(vkWaitForFences(_device, 1, &fence, VK_TRUE, UINT64_MAX));

        vkDestroyFence(_device, fence, nullptr);

        if (free)
        {
            vkFreeCommandBuffers(_device, _graphicsCommandPool, 1, &commandBuffer);
        }
    }

    VkSemaphore GraphicsImpl::RequestSemaphore()
    {
        VkSemaphore semaphore;
        if (_semaphores.empty())
        {
            VkSemaphoreCreateInfo createInfo;
            createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            createInfo.pNext = nullptr;
            createInfo.flags = 0u;
            vkCreateSemaphore(_device, &createInfo, nullptr, &semaphore);
        }
        else
        {
            semaphore = _semaphores.back();
            _semaphores.pop_back();
        }

        return semaphore;
    }

    void GraphicsImpl::RecycleSemaphore(VkSemaphore semaphore)
    {
        _semaphores.push_back(semaphore);
    }

    VkFence GraphicsImpl::RequestFence()
    {
        VkFence fence;
        if (_fences.empty())
        {
            VkFenceCreateInfo createInfo;
            createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            createInfo.pNext = nullptr;
            createInfo.flags = 0;

            vkCreateFence(_device, &createInfo, nullptr, &fence);
        }
        else
        {
            fence = _fences.back();
            _fences.pop_back();
        }

        return fence;
    }

    void GraphicsImpl::RecycleFence(VkFence fence)
    {
        _fences.push_back(fence);
    }

    void GraphicsImpl::DestroySampler(VkSampler handle)
    {
#if !defined(NDEBUG)
        //ALIMER_ASSERT(std::find(frame().destroyedSamplers.begin(), frame().destroyedSamplers.end(), handle) == frame().destroyedSamplers.end());
#endif
        //frame().destroyedSamplers.push_back(handle);
    }

    void GraphicsImpl::DestroyPipeline(VkPipeline handle)
    {
#if !defined(NDEBUG)
        //ALIMER_ASSERT(std::find(frame().destroyedPipelines.begin(), frame().destroyedPipelines.end(), handle) == frame().destroyedPipelines.end());
#endif
        //frame().destroyedPipelines.push_back(handle);
    }

    void GraphicsImpl::DestroyImage(VkImage handle)
    {
#if !defined(NDEBUG)
        //ALIMER_ASSERT(std::find(frame().destroyedImages.begin(), frame().destroyedImages.end(), handle) == frame().destroyedImages.end());
#endif
        //frame().destroyedImages.push_back(handle);
    }
}
