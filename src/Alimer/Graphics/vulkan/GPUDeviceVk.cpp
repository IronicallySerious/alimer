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
#include "CommandQueueVk.h"
#include "CommandContextVk.h"
#include "TextureVk.h"
#include "BufferVk.h"
#include "SwapChainVk.h"

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

    static VkBool32 QueryPresentationSupport(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex)
    {
        // TODO: @see: glfwGetPhysicalDevicePresentationSupport
#if defined(_WIN32) || defined(_WIN64)
        return vkGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex);
#elif defined(__ANDROID__)
        return VK_TRUE;
#else
        // TODO:
        return VK_TRUE;
#endif
    }

    bool GraphicsDeviceVk::IsSupported()
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

    GraphicsDeviceVk::GraphicsDeviceVk(const char* applicationName, PhysicalDevicePreference devicePreference, bool validation)
        : GraphicsDevice(GraphicsBackend::Vulkan, devicePreference, validation)
        , _headless(false)
    {
        CreateInstance(applicationName);
        SelectPhysicalDevice(devicePreference);
        InitializeFeatures();
    }

    GraphicsDeviceVk::~GraphicsDeviceVk()
    {
        WaitIdle();

        // Delete swap chain if created.
        SafeDelete(_swapChain);

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
        //_frameData.clear();

        for (uint32_t i = 0u; i < _maxInflightFrames; i++) {
            vkDestroyFence(_device, _waitFences[i], nullptr);
            vkDestroySemaphore(_device, _renderCompleteSemaphores[i], nullptr);
            vkDestroySemaphore(_device, _presentCompleteSemaphores[i], nullptr);
        }
        vkDestroyPipelineCache(_device, _pipelineCache, nullptr);
        vkDestroyCommandPool(_device, _graphicsCommandPool, nullptr);

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
    }

    void GraphicsDeviceVk::Finalize()
    {
        WaitIdle();

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

    void GraphicsDeviceVk::CreateInstance(const char* applicationName)
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

#if !defined(NDEBUG)
        if (_validation)
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

        if (_validation)
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

    void GraphicsDeviceVk::SelectPhysicalDevice(PhysicalDevicePreference devicePreference)
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

            ALIMER_LOGTRACE("Found Vulkan GPU: {}", properties.deviceName);
            ALIMER_LOGTRACE("    API: {}.{}.{}",
                VK_VERSION_MAJOR(properties.apiVersion),
                VK_VERSION_MINOR(properties.apiVersion),
                VK_VERSION_PATCH(properties.apiVersion));
            ALIMER_LOGTRACE("    Driver: {}.{}.{}",
                VK_VERSION_MAJOR(properties.driverVersion),
                VK_VERSION_MINOR(properties.driverVersion),
                VK_VERSION_PATCH(properties.driverVersion));

            switch (properties.deviceType)
            {
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                score += 100u;
                if (devicePreference == PhysicalDevicePreference::Discrete) {
                    score += 1000u;
                }
                break;
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                score += 90u;
                if (devicePreference == PhysicalDevicePreference::Integrated) {
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

        ALIMER_LOGDEBUG("Selected Vulkan GPU: {}", _physicalDeviceProperties.deviceName);
    }

    void GraphicsDeviceVk::CreateLogicalDevice(VkSurfaceKHR surface)
    {
        /// Create surface and logical device,
        /// some code adapted from https://github.com/Themaister/Granite
        if (_physicalDeviceProperties.apiVersion >= VK_API_VERSION_1_1)
        {
            _featuresVk.supportsVulkan11Device = _featuresVk.supportsVulkan11Instance;
            ALIMER_LOGDEBUG("GPU supports Vulkan 1.1.");
        }
        else if (_physicalDeviceProperties.apiVersion >= VK_API_VERSION_1_0)
        {
            _featuresVk.supportsVulkan11Device = false;
            ALIMER_LOGDEBUG("GPU supports Vulkan 1.0.");
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
            //VkBool32 supported = QueryPresentationSupport(_physicalDevice, i);
            VkBool32 supported = surface == VK_NULL_HANDLE;
            if (surface != VK_NULL_HANDLE)
            {
                vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, i, surface, &supported);
            }

            static const VkQueueFlags required = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT;
            if (supported && ((_physicalDeviceQueueFamilyProperties[i].queueFlags & required) == required))
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

        if (_graphicsQueueFamily == VK_QUEUE_FAMILY_IGNORED)
        {
            return;
        }

        // Setup queues
        uint32_t universalQueueIndex = 1;
        const uint32_t graphicsQueueIndex = 0;
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
        vector<const char*> enabledLayers;

        for (size_t i = 0; i < requiredDeviceExtensions.size(); i++)
        {
            if (!HasExtension(requiredDeviceExtensions[i]))
            {
                ALIMER_LOGERROR("Vulkan: required extension is not supported '{}'", requiredDeviceExtensions[i]);
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

#if !defined(NDEBUG)
        if (_validation
            && HasLayer("VK_LAYER_LUNARG_standard_validation"))
        {
            enabledLayers.push_back("VK_LAYER_LUNARG_standard_validation");
        }
#endif

        device_info.enabledExtensionCount = (uint32_t)enabledExtensions.size();
        device_info.ppEnabledExtensionNames = enabledExtensions.data();
        device_info.enabledLayerCount = (uint32_t)enabledLayers.size();
        device_info.ppEnabledLayerNames = enabledLayers.data();

        if (vkCreateDevice(_physicalDevice, &device_info, nullptr, &_device) != VK_SUCCESS)
        {
            ALIMER_LOGERROR("Vulkan: Failed to create device");
            return;
        }

        volkLoadDevice(_device);
        vkGetDeviceQueue(_device, _graphicsQueueFamily, graphicsQueueIndex, &_graphicsQueue);
        vkGetDeviceQueue(_device, _computeQueueFamily, computeQueueIndex, &_computeQueue);
        vkGetDeviceQueue(_device, _transferQueueFamily, transferQueueIndex, &_transferQueue);
    }

    void GraphicsDeviceVk::InitializeFeatures()
    {
        _features.SetBackend(GraphicsBackend::Vulkan);
        _features.SetVendorId(_physicalDeviceProperties.vendorID);
        _features.SetDeviceId(_physicalDeviceProperties.deviceID);
        _features.SetDeviceName(_physicalDeviceProperties.deviceName);
        _features.SetMultithreading(true);
        _features.SetMaxColorAttachments(_physicalDeviceProperties.limits.maxColorAttachments);
        _features.SetMaxBindGroups(_physicalDeviceProperties.limits.maxBoundDescriptorSets);
        _features.SetMinUniformBufferOffsetAlignment(static_cast<uint32_t>(_physicalDeviceProperties.limits.minUniformBufferOffsetAlignment));
        _features.SetMinStorageBufferOffsetAlignment(static_cast<uint32_t>(_physicalDeviceProperties.limits.minStorageBufferOffsetAlignment));
    }

    void GraphicsDeviceVk::WaitIdle()
    {
        vkThrowIfFailed(
            vkDeviceWaitIdle(_device)
        );
    }


    VkSurfaceKHR GraphicsDeviceVk::CreateSurface(const SwapChainDescriptor* descriptor)
    {
#if defined(_WIN32) || defined(_WIN64)
        VkWin32SurfaceCreateInfoKHR surfaceCreateInfo;
        surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        surfaceCreateInfo.pNext = nullptr;
        surfaceCreateInfo.flags = 0;
        if (descriptor->nativeDisplay == 0) {
            surfaceCreateInfo.hinstance = GetModuleHandleW(nullptr);
        }
        else {
            surfaceCreateInfo.hinstance = reinterpret_cast<HINSTANCE>(descriptor->nativeDisplay);
        }

        surfaceCreateInfo.hwnd = reinterpret_cast<HWND>(descriptor->nativeHandle);
#elif defined(__ANDROID__)
        VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo = {};
        surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
        surfaceCreateInfo.window = reinterpret_cast<ANativeWindow>(descriptor->nativeHandle);
#elif defined(__linux__)
        VkXcbSurfaceCreateInfoKHR surfaceCreateInfo;
        surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
        surfaceCreateInfo.pNext = nullptr;
        surfaceCreateInfo.flags = 0;

        if (descriptor->nativeDisplay == 0) {
            static xcb_connection_t* XCB_CONNECTION = nullptr;
            if (XCB_CONNECTION == nullptr) {
                int screenIndex = 0;
                xcb_screen_t* screen = nullptr;
                xcb_connection_t* connection = xcb_connect(NULL, &screenIndex);
                const xcb_setup_t *setup = xcb_get_setup(connection);
                for (xcb_screen_iterator_t it = xcb_setup_roots_iterator(setup);
                    screen >= 0 && it.rem;
                    xcb_screen_next(&it)) {
                    if (screenIndex-- == 0) {
                        screen = it.data;
                    }
                }
                assert(screen);
                XCB_CONNECTION = connection;
            }

            surfaceCreateInfo.connection = XCB_CONNECTION;
        }
        else
        {
            surfaceCreateInfo.connection = = (xcb_connection_t*)descriptor->nativeDisplay;
        }

        surfaceCreateInfo.window = (xcb_window_t)descriptor->nativeHandle;
#elif defined(VK_USE_PLATFORM_IOS_MVK)
        VkIOSSurfaceCreateInfoMVK surfaceCreateInfo = {};
        surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_IOS_SURFACE_CREATE_INFO_MVK;
        surfaceCreateInfo.pView = nativeHandle;
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
        VkMacOSSurfaceCreateInfoMVK surfaceCreateInfo = {};
        surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
        surfaceCreateInfo.pView = nativeHandle;
#endif

        VkSurfaceKHR surface = VK_NULL_HANDLE;
        VkResult result = VK_CREATE_SURFACE_FN(_instance, &surfaceCreateInfo, nullptr, &surface);
        if (result != VK_SUCCESS)
        {
            ALIMER_LOGERROR("Vulkan: Failed to create platform surface");
            return VK_NULL_HANDLE;
        }

        return surface;
    }

    bool GraphicsDeviceVk::InitializeImpl(const SwapChainDescriptor* descriptor)
    {
        VkSurfaceKHR surface = _headless ? VK_NULL_HANDLE : CreateSurface(descriptor);
        CreateLogicalDevice(surface);

        VkCommandPoolCreateInfo cmdPoolInfo = {};
        cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmdPoolInfo.queueFamilyIndex = _graphicsQueueFamily;
        cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        vkThrowIfFailed(vkCreateCommandPool(_device, &cmdPoolInfo, nullptr, &_graphicsCommandPool));

        // Pipeline cache
        VkPipelineCacheCreateInfo pipelineCacheCreateInfo{};
        pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        vkThrowIfFailed(
            vkCreatePipelineCache(_device, &pipelineCacheCreateInfo, nullptr, &_pipelineCache)
        );

        // Create swap chain.
        _swapChain = new SwapChainVk(this, surface, descriptor);

        _frameIndex = _swapchainImageIndex = 0u;
        _maxInflightFrames = _swapChain != nullptr ? _swapChain->GetImageCount() : 3u;
        _waitFences.resize(_maxInflightFrames);
        _presentCompleteSemaphores.resize(_maxInflightFrames);
        _renderCompleteSemaphores.resize(_maxInflightFrames);
        _commandBuffers.resize(_maxInflightFrames);

        // Command buffer execution fences
        for (uint32_t i = 0u; i < _maxInflightFrames; i++)
        {
            VkFenceCreateInfo fenceCreateInfo;
            fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceCreateInfo.pNext = nullptr;
            fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            vkThrowIfFailed(vkCreateFence(_device, &fenceCreateInfo, nullptr, &_waitFences[i]));
        }

        // Queue ordering semaphores
        for (auto &semaphore : _presentCompleteSemaphores) {
            VkSemaphoreCreateInfo semaphoreCreateInfo;
            semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            semaphoreCreateInfo.pNext = nullptr;
            semaphoreCreateInfo.flags = 0;
            vkThrowIfFailed(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &semaphore));
        }

        for (auto &semaphore : _renderCompleteSemaphores) {
            VkSemaphoreCreateInfo semaphoreCreateInfo;
            semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            semaphoreCreateInfo.pNext = nullptr;
            semaphoreCreateInfo.flags = 0;
            vkThrowIfFailed(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &semaphore));
        }

        // Command buffers
        {
            std::vector<VkCommandBuffer> vkCommandBuffers(_maxInflightFrames);

            VkCommandBufferAllocateInfo cmdBufAllocateInfo;
            cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            cmdBufAllocateInfo.pNext = nullptr;
            cmdBufAllocateInfo.commandPool = _graphicsCommandPool;
            cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            cmdBufAllocateInfo.commandBufferCount = _maxInflightFrames;
            vkThrowIfFailed(vkAllocateCommandBuffers(_device, &cmdBufAllocateInfo, vkCommandBuffers.data()));

            for (uint32_t i = 0u; i < _maxInflightFrames; i++)
            {
                _commandBuffers[i] = new CommandContextVk(this, vkCommandBuffers[i]);
            }
        }

        OnAfterCreated();
        return true;
    }

    bool GraphicsDeviceVk::BeginFrameImpl()
    {
        // Wait to end previous frame.
        VkFence fence = _waitFences[_frameIndex];
        vkThrowIfFailed(vkWaitForFences(_device, 1, &fence, VK_TRUE, UINT64_MAX));
        vkThrowIfFailed(vkResetFences(_device, 1, &fence));

        // Delete all pending/deferred resources
        //frame().ProcessDeferredDelete();

        // Acquire the next image from the swap chain
        VkResult result =  _swapChain->AcquireNextImage(_presentCompleteSemaphores[_frameIndex], &_swapchainImageIndex);
        if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR)) {
            _swapChain->Resize();
        }
        else {
            vkThrowIfFailed(result);
        }

        _commandBuffers[_swapchainImageIndex]->Begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

        return true;
    }

    void GraphicsDeviceVk::EndFrame(uint32_t frameId)
    {
        _commandBuffers[_swapchainImageIndex]->End();

        VkCommandBuffer commandBuffer = _commandBuffers[_swapchainImageIndex]->GetVkCommandBuffer();
        const VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pWaitDstStageMask = &waitDstStageMask;
        submitInfo.pWaitSemaphores = &_presentCompleteSemaphores[_frameIndex];
        submitInfo.waitSemaphoreCount = 1u;
        submitInfo.pSignalSemaphores = &_renderCompleteSemaphores[_frameIndex];
        submitInfo.signalSemaphoreCount = 1u;
        submitInfo.pCommandBuffers = &commandBuffer;
        submitInfo.commandBufferCount = 1u;
        vkThrowIfFailed(vkQueueSubmit(_graphicsQueue, 1, &submitInfo, _waitFences[_frameIndex]));

        VkResult result = _swapChain->QueuePresent(_graphicsQueue, _swapchainImageIndex, _renderCompleteSemaphores[_frameIndex]);
        if (!((result == VK_SUCCESS) || (result == VK_SUBOPTIMAL_KHR))) {
            if (result == VK_ERROR_OUT_OF_DATE_KHR) {
                _swapChain->Resize();
                return;
            }
            else {
                vkThrowIfFailed(result);
            }
        }


        // Advance frame index.
        _frameIndex = (_frameIndex + 1u) % _maxInflightFrames;
    }

    SharedPtr<CommandContext> GraphicsDeviceVk::GetContext() const
    {
        return _commandBuffers[_swapchainImageIndex];
    }

    bool GraphicsDeviceVk::ImageFormatIsSupported(VkFormat format, VkFormatFeatureFlags required, VkImageTiling tiling) const
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(_physicalDevice, format, &props);
        VkFormatFeatureFlags flags = tiling == VK_IMAGE_TILING_OPTIMAL ? props.optimalTilingFeatures : props.linearTilingFeatures;
        return (flags & required) == required;
    }
    
    PixelFormat GraphicsDeviceVk::GetDefaultDepthStencilFormat() const
    {
        if (ImageFormatIsSupported(VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL)) {
            return PixelFormat::Depth24UNormStencil8;
        }

        if (ImageFormatIsSupported(VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL)) {
            return PixelFormat::Depth32FloatStencil8;
        }

        return PixelFormat::Undefined;
    }

    PixelFormat GraphicsDeviceVk::GetDefaultDepthFormat() const
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

    VkCommandBuffer GraphicsDeviceVk::CreateCommandBuffer(VkCommandBufferLevel level, bool begin)
    {
        VkCommandBufferAllocateInfo info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
        info.commandPool = _graphicsCommandPool;
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

    void GraphicsDeviceVk::FlushCommandBuffer(VkCommandBuffer commandBuffer, bool free)
    {
        FlushCommandBuffer(commandBuffer, _graphicsQueue, free);
    }

    void GraphicsDeviceVk::FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free)
    {
        if (commandBuffer == VK_NULL_HANDLE) {
            return;
        }

        vkThrowIfFailed(vkEndCommandBuffer(commandBuffer));

        VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        VkFenceCreateInfo fenceCreateInfo = {};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = 0;
        VkFence fence;
        vkThrowIfFailed(vkCreateFence(_device, &fenceCreateInfo, nullptr, &fence));

        // Submit to the queue
        vkThrowIfFailed(vkQueueSubmit(queue, 1, &submitInfo, fence));

        // Wait for the fence to signal that command buffer has finished executing.
        vkThrowIfFailed(vkWaitForFences(_device, 1, &fence, VK_TRUE, UINT64_MAX));
        vkDestroyFence(_device, fence, nullptr);

        if (free)
        {
            vkFreeCommandBuffers(_device, _graphicsCommandPool, 1, &commandBuffer);
        }
    }

    VkSemaphore GraphicsDeviceVk::RequestSemaphore()
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

    void GraphicsDeviceVk::RecycleSemaphore(VkSemaphore semaphore)
    {
        _semaphores.push_back(semaphore);
    }

    VkFence GraphicsDeviceVk::RequestFence()
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

    void GraphicsDeviceVk::RecycleFence(VkFence fence)
    {
        _fences.push_back(fence);
    }

    void GraphicsDeviceVk::DestroySampler(VkSampler handle)
    {
#if !defined(NDEBUG)
        //ALIMER_ASSERT(std::find(frame().destroyedSamplers.begin(), frame().destroyedSamplers.end(), handle) == frame().destroyedSamplers.end());
#endif
        //frame().destroyedSamplers.push_back(handle);
    }

    void GraphicsDeviceVk::DestroyPipeline(VkPipeline handle)
    {
#if !defined(NDEBUG)
        //ALIMER_ASSERT(std::find(frame().destroyedPipelines.begin(), frame().destroyedPipelines.end(), handle) == frame().destroyedPipelines.end());
#endif
        //frame().destroyedPipelines.push_back(handle);
    }

    void GraphicsDeviceVk::DestroyImage(VkImage handle)
    {
#if !defined(NDEBUG)
        //ALIMER_ASSERT(std::find(frame().destroyedImages.begin(), frame().destroyedImages.end(), handle) == frame().destroyedImages.end());
#endif
        //frame().destroyedImages.push_back(handle);
    }

    /*GraphicsDeviceVk::FrameData::FrameData(GraphicsDeviceVk* device_)
        : device(device_)
        , logicalDevice(device_->GetVkDevice())
    {
        VkFenceCreateInfo fenceCreateInfo = {};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.pNext = nullptr;
        fenceCreateInfo.flags = 0u;
        vkThrowIfFailed(vkCreateFence(logicalDevice, &fenceCreateInfo, nullptr, &fence));
    }

    GraphicsDeviceVk::FrameData::~FrameData()
    {
        ProcessDeferredDelete();
    }

    void GraphicsDeviceVk::FrameData::ProcessDeferredDelete()
    {
        for (auto &sampler : destroyedSamplers)
            vkDestroySampler(logicalDevice, sampler, nullptr);

        for (auto &pipeline : destroyedPipelines)
            vkDestroyPipeline(logicalDevice, pipeline, nullptr);

        for (auto &image : destroyedImages)
            vkDestroyImage(logicalDevice, image, nullptr);

        destroyedSamplers.clear();
        destroyedPipelines.clear();
        destroyedImages.clear();
    }*/
}
