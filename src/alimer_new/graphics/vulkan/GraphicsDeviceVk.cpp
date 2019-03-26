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

#include "engine/Window.h"
#include "GraphicsDeviceVk.h"
#include "SwapChainVk.h"
#define VMA_IMPLEMENTATION 1
#define VMA_STATIC_VULKAN_FUNCTION 0
#define VMA_STATS_STRING_ENABLED 0
#include "vk_mem_alloc.h"
#include <volk.h>
#include <vector>

#if defined(ALIMER_GLFW)
#   define GLFW_INCLUDE_NONE
#   include <GLFW/glfw3.h>
#endif /* ALIMER_GLFW */

using namespace std;

namespace alimer
{
#ifdef VULKAN_DEBUG
    static VKAPI_ATTR VkBool32 VKAPI_CALL vulkanMessengerCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT                  messageType,
        const VkDebugUtilsMessengerCallbackDataEXT*      pCallbackData,
        void *pUserData)
    {
        auto *context = static_cast<GraphicsDeviceVk*>(pUserData);

        switch (messageSeverity)
        {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
            {
                LOGE("[Vulkan]: Validation Error: %s", pCallbackData->pMessage);
                context->notifyValidationError(pCallbackData->pMessage);
            }
            else
                LOGE("[Vulkan]: Other Error: %s", pCallbackData->pMessage);
            break;

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
            {
                LOGW("[Vulkan]: Validation Warning: %s", pCallbackData->pMessage);
            }
            else
            {
                LOGW("[Vulkan]: Other Warning: %s", pCallbackData->pMessage);
            }
            break;

#if 0
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
                LOGI("[Vulkan]: Validation Info: %s", pCallbackData->pMessage);
            else
                LOGI("[Vulkan]: Other Info: %s", pCallbackData->pMessage);
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
                LOGI("  Object #%u: %s\n", i, name ? name : "N/A");
            }
        }

        return VK_FALSE;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL vulkanDebugCallback(VkDebugReportFlagsEXT flags,
        VkDebugReportObjectTypeEXT, uint64_t,
        size_t, int32_t messageCode, const char *pLayerPrefix,
        const char *pMessage, void *pUserData)
    {
        auto *context = static_cast<GraphicsDeviceVk*>(pUserData);

        // False positives about lack of srcAccessMask/dstAccessMask.
        if (strcmp(pLayerPrefix, "DS") == 0 && messageCode == 10)
            return VK_FALSE;

        // Demote to a warning, it's a false positive almost all the time for Granite.
        if (strcmp(pLayerPrefix, "DS") == 0 && messageCode == 6)
            flags = VK_DEBUG_REPORT_DEBUG_BIT_EXT;

        if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
        {
            LOGE("[Vulkan]: Error: %s: %s", pLayerPrefix, pMessage);
            context->notifyValidationError(pMessage);
        }
        else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
        {
            LOGW("[Vulkan]: Warning: %s: %s", pLayerPrefix, pMessage);
        }
        else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
        {
            //LOGW("[Vulkan]: Performance warning: %s: %s\n", pLayerPrefix, pMessage);
        }
        else
        {
            LOGI("[Vulkan]: Information: %s: %s", pLayerPrefix, pMessage);
        }

        return VK_FALSE;
    }
#endif

    GraphicsDeviceVk::GraphicsDeviceVk()
    {
        VkResult result = volkInitialize();
        if (result)
        {
            throw VulkanException(result, "Failed to initialize volk.");
        }

        // Enumerate supported extenions.
        uint32_t instance_extension_count;
        vkThrowIfFailed(vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, nullptr));

        std::vector<VkExtensionProperties> instance_extensions(instance_extension_count);
        vkThrowIfFailed(vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, instance_extensions.data()));

        // Enumerate supported layers.
        uint32_t instance_layer_count;
        vkThrowIfFailed(vkEnumerateInstanceLayerProperties(&instance_layer_count, nullptr));

        std::vector<VkLayerProperties> instance_layers(instance_layer_count);
        vkThrowIfFailed(vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layers.data()));

        const auto has_extension = [&](const char *name) -> bool {
            auto itr = find_if(begin(instance_extensions), end(instance_extensions), [name](const VkExtensionProperties &e) -> bool {
                return strcmp(e.extensionName, name) == 0;
                });
            return itr != end(instance_extensions);
        };

        vector<const char*> enabled_extensions({ VK_KHR_SURFACE_EXTENSION_NAME });
        vector<const char*> enabled_layers;

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
        enabled_extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
        enabled_extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
        enabled_extensions.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#elif defined(enabled_extensions)
        active_instance_extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif

        if (has_extension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
        {
            _features.supports_physical_device_properties2 = true;
            enabled_extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        }

        if (_features.supports_physical_device_properties2 &&
            has_extension(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME) &&
            has_extension(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME))
        {
            enabled_extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
            enabled_extensions.push_back(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME);
            enabled_extensions.push_back(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME);
            _features.supports_external = true;
        }

        if (has_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
        {
            enabled_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            _features.supports_debug_utils = true;
        }

#ifdef VULKAN_DEBUG
        const auto has_layer = [&](const char *name) -> bool {
            auto itr = find_if(begin(instance_layers), end(instance_layers), [name](const VkLayerProperties &e) -> bool {
                return strcmp(e.layerName, name) == 0;
                });
            return itr != end(instance_layers);
        };

        if (!_features.supports_debug_utils && has_extension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
        {
            enabled_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        }

        if (has_layer("VK_LAYER_LUNARG_standard_validation"))
        {
            enabled_layers.push_back("VK_LAYER_LUNARG_standard_validation");
        }
#endif

        // Application info
        VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
        appInfo.pApplicationName = "alimer";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "alimer";
        appInfo.engineVersion = VK_MAKE_VERSION(ALIMER_VERSION_MAJOR, ALIMER_VERSION_MINOR, ALIMER_VERSION_PATCH);
        appInfo.apiVersion = volkGetInstanceVersion();

        if (appInfo.apiVersion >= VK_API_VERSION_1_1)
        {
            _features.supports_vulkan_11_instance = true;
        }

        VkInstanceCreateInfo instanceCreateInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
        instanceCreateInfo.pApplicationInfo = &appInfo;
        instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(enabled_extensions.size());
        instanceCreateInfo.ppEnabledExtensionNames = enabled_extensions.empty() ? nullptr : enabled_extensions.data();
        instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(enabled_layers.size());
        instanceCreateInfo.ppEnabledLayerNames = enabled_layers.empty() ? nullptr : enabled_layers.data();

        result = vkCreateInstance(&instanceCreateInfo, nullptr, &_instance);
        if (result != VK_SUCCESS)
        {
            throw VulkanException(result, "Could not create Vulkan instance");
        }

        volkLoadInstance(_instance);

#ifdef VULKAN_DEBUG
        if (_features.supports_debug_utils)
        {
            VkDebugUtilsMessengerCreateInfoEXT info = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
            info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
            info.pfnUserCallback = vulkanMessengerCallback;
            info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
            info.pUserData = this;

            vkCreateDebugUtilsMessengerEXT(_instance, &info, nullptr, &_debugMessenger);
        }
        else if (has_extension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
        {
            VkDebugReportCallbackCreateInfoEXT info = { VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT };
            info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
                VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
            info.pfnCallback = vulkanDebugCallback;
            info.pUserData = this;
            vkCreateDebugReportCallbackEXT(_instance, &info, nullptr, &_debugCallback);
        }
#endif
    }

    GraphicsDeviceVk::~GraphicsDeviceVk()
    {
        Destroy();
    }

    void GraphicsDeviceVk::Destroy()
    {
        if (device != VK_NULL_HANDLE)
        {
            vkDeviceWaitIdle(device);
        }

        // Destroy swap chain.
        _swapChain.reset();

        // Clear per frame data.
        perFrame.clear();

        if (_graphicsCommandPool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(device, _graphicsCommandPool, nullptr);
            _graphicsCommandPool = VK_NULL_HANDLE;
        }

        if (_memoryAllocator != VK_NULL_HANDLE)
        {
            VmaStats stats;
            vmaCalculateStats(_memoryAllocator, &stats);

            LOGI("Total device memory leaked: %llu bytes.", stats.total.usedBytes);

            vmaDestroyAllocator(_memoryAllocator);
            _memoryAllocator = VK_NULL_HANDLE;
        }

        if (device != VK_NULL_HANDLE)
        {
            vkDestroyDevice(device, nullptr);
            device = VK_NULL_HANDLE;
        }

#ifdef VULKAN_DEBUG
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
#endif

        if (_instance != VK_NULL_HANDLE)
        {
            vkDestroyInstance(_instance, nullptr);
            _instance = VK_NULL_HANDLE;
        }
    }

    bool GraphicsDeviceVk::Initialize(const std::shared_ptr<Window>& window, const GraphicsDeviceDescriptor& desc)
    {
        // Create surface first.
        VkSurfaceKHR surface = VK_NULL_HANDLE;

#if defined(ALIMER_GLFW)
        vkThrowIfFailed(
            glfwCreateWindowSurface(_instance, window->getApiHandle(), NULL, &surface)
        );
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
#endif

        // Obtain a list of available physical devices.
        uint32_t physical_device_count = 0u;
        vkEnumeratePhysicalDevices(_instance, &physical_device_count, NULL);

        vector<VkPhysicalDevice> gpus(physical_device_count);
        vkEnumeratePhysicalDevices(_instance, &physical_device_count, gpus.data());

        // Pick a suitable physical device based on user's preference.
        uint32_t best_device_score = 0U;
        uint32_t best_device_index = static_cast<uint32_t>(-1);
        for (uint32_t i = 0; i < physical_device_count; ++i)
        {
            VkPhysicalDeviceProperties dev_props;
            vkGetPhysicalDeviceProperties(gpus[i], &dev_props);

            uint32_t score = 0u;
            switch (dev_props.deviceType)
            {
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                score += 100U;
                if (desc.powerPreference == PowerPreference::HighPerformance) {
                    score += 1000u;
                }
                break;
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                score += 90U;
                if (desc.powerPreference == PowerPreference::LowPower) {
                    score += 1000u;
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

            if (score > best_device_score)
            {
                best_device_index = i;
                best_device_score = score;
            }
        }

        if (best_device_index == static_cast<uint32_t>(-1))
        {
            return false;
        }

        _physicalDevice = gpus[best_device_index];
        vkGetPhysicalDeviceProperties(_physicalDevice, &_deviceProperties);
        vkGetPhysicalDeviceFeatures(_physicalDevice, &_deviceFeatures);
        vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &_deviceMemoryProperties);

        LOGI("Selected Vulkan GPU: %s", _deviceProperties.deviceName);

        uint32_t ext_count = 0;
        vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &ext_count, nullptr);
        vector<VkExtensionProperties> queried_extensions(ext_count);
        if (ext_count) {
            vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &ext_count, queried_extensions.data());
        }

        const auto has_extension = [&](const char *name) -> bool {
            auto itr = find_if(begin(queried_extensions), end(queried_extensions), [name](const VkExtensionProperties &e) -> bool {
                return strcmp(e.extensionName, name) == 0;
                });
            return itr != end(queried_extensions);
        };

        if (_deviceProperties.apiVersion >= VK_API_VERSION_1_1)
        {
            _features.supports_vulkan_11_device = _features.supports_vulkan_11_instance;
            LOGI("GPU supports Vulkan 1.1.");
        }
        else if (_deviceProperties.apiVersion >= VK_API_VERSION_1_0)
        {
            _features.supports_vulkan_11_device = false;
            LOGI("GPU supports Vulkan 1.0.");
        }

        // Only need GetPhysicalDeviceProperties2 for Vulkan 1.1-only code, so don't bother getting KHR variant.
        _features.subgroup_properties = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES };
        VkPhysicalDeviceProperties2 props = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
        void **ppNext = &props.pNext;

        if (_features.supports_vulkan_11_instance && _features.supports_vulkan_11_device)
        {
            *ppNext = &_features.subgroup_properties;
            ppNext = &_features.subgroup_properties.pNext;
        }

        if (_features.supports_vulkan_11_instance && _features.supports_vulkan_11_device)
        {
            vkGetPhysicalDeviceProperties2(_physicalDevice, &props);
        }

        uint32_t queue_count;
        vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queue_count, nullptr);
        vector<VkQueueFamilyProperties> queue_props(queue_count);
        vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queue_count, queue_props.data());

        for (uint32_t i = 0; i < queue_count; i++)
        {
            VkBool32 supported = surface == VK_NULL_HANDLE;
            if (surface != VK_NULL_HANDLE)
                vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, i, surface, &supported);

            static const VkQueueFlags required = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT;
            if (supported && ((queue_props[i].queueFlags & required) == required))
            {
                graphicsQueueFamily = i;
                break;
            }
        }

        for (uint32_t i = 0; i < queue_count; i++)
        {
            static const VkQueueFlags required = VK_QUEUE_COMPUTE_BIT;
            if (i != graphicsQueueFamily && (queue_props[i].queueFlags & required) == required)
            {
                computeQueueFamily = i;
                break;
            }
        }

        for (uint32_t i = 0; i < queue_count; i++)
        {
            static const VkQueueFlags required = VK_QUEUE_TRANSFER_BIT;
            if (i != graphicsQueueFamily && i != computeQueueFamily && (queue_props[i].queueFlags & required) == required)
            {
                transferQueueFamily = i;
                break;
            }
        }

        if (transferQueueFamily == VK_QUEUE_FAMILY_IGNORED)
        {
            for (unsigned i = 0; i < queue_count; i++)
            {
                static const VkQueueFlags required = VK_QUEUE_TRANSFER_BIT;
                if (i != graphicsQueueFamily && (queue_props[i].queueFlags & required) == required)
                {
                    transferQueueFamily = i;
                    break;
                }
            }
        }

        if (graphicsQueueFamily == VK_QUEUE_FAMILY_IGNORED)
        {
            return false;
        }

        uint32_t universal_queue_index = 1;
        uint32_t graphics_queue_index = 0;
        uint32_t compute_queue_index = 0;
        uint32_t transfer_queue_index = 0;

        if (computeQueueFamily == VK_QUEUE_FAMILY_IGNORED)
        {
            computeQueueFamily = graphicsQueueFamily;
            compute_queue_index = std::min(queue_props[graphicsQueueFamily].queueCount - 1, universal_queue_index);
            universal_queue_index++;
        }

        if (transferQueueFamily == VK_QUEUE_FAMILY_IGNORED)
        {
            transferQueueFamily = graphicsQueueFamily;
            transfer_queue_index = std::min(queue_props[graphicsQueueFamily].queueCount - 1, universal_queue_index);
            universal_queue_index++;
        }
        else if (transferQueueFamily == computeQueueFamily)
        {
            transfer_queue_index = std::min(queue_props[computeQueueFamily].queueCount - 1, 1u);
        }

        static const float graphics_queue_prio = 0.5f;
        static const float compute_queue_prio = 1.0f;
        static const float transfer_queue_prio = 1.0f;
        float prio[3] = { graphics_queue_prio, compute_queue_prio, transfer_queue_prio };

        unsigned queue_family_count = 0;
        VkDeviceQueueCreateInfo queue_info[3] = {};

        VkDeviceCreateInfo deviceCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
        deviceCreateInfo.pQueueCreateInfos = queue_info;

        queue_info[queue_family_count].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info[queue_family_count].queueFamilyIndex = graphicsQueueFamily;
        queue_info[queue_family_count].queueCount = std::min(universal_queue_index,
            queue_props[graphicsQueueFamily].queueCount);
        queue_info[queue_family_count].pQueuePriorities = prio;
        queue_family_count++;

        if (computeQueueFamily != graphicsQueueFamily)
        {
            queue_info[queue_family_count].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_info[queue_family_count].queueFamilyIndex = computeQueueFamily;
            queue_info[queue_family_count].queueCount = std::min(transferQueueFamily == computeQueueFamily ? 2u : 1u,
                queue_props[computeQueueFamily].queueCount);
            queue_info[queue_family_count].pQueuePriorities = prio + 1;
            queue_family_count++;
        }

        if (transferQueueFamily != graphicsQueueFamily
            && transferQueueFamily != computeQueueFamily)
        {
            queue_info[queue_family_count].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_info[queue_family_count].queueFamilyIndex = transferQueueFamily;
            queue_info[queue_family_count].queueCount = 1;
            queue_info[queue_family_count].pQueuePriorities = prio + 2;
            queue_family_count++;
        }

        deviceCreateInfo.queueCreateInfoCount = queue_family_count;

        vector<const char*> required_device_extensions({ VK_KHR_SWAPCHAIN_EXTENSION_NAME });
        // VK_KHR_maintenance1 to support nevagive viewport and match DirectX coordinate system.
        required_device_extensions.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);

        vector<const char*> enabled_extensions;
        for (size_t i = 0; i < required_device_extensions.size(); i++)
        {
            if (!has_extension(required_device_extensions[i]))
            {
                LOGE("Vulkan: required extension is not supported '%s'", required_device_extensions[i]);
                return false;
            }

            enabled_extensions.push_back(required_device_extensions[i]);
        }

        if (has_extension(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME) &&
            has_extension(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME))
        {
            _features.supports_dedicated = true;
            enabled_extensions.push_back(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
            enabled_extensions.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        }

        if (has_extension(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME))
        {
            _features.supports_image_format_list = true;
            enabled_extensions.push_back(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
        }

        if (has_extension(VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
        {
            _features.supports_debug_marker = true;
            enabled_extensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
        }

        if (has_extension(VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME))
        {
            _features.supports_mirror_clamp_to_edge = true;
            enabled_extensions.push_back(VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME);
        }

        if (has_extension(VK_GOOGLE_DISPLAY_TIMING_EXTENSION_NAME))
        {
            _features.supports_google_display_timing = true;
            enabled_extensions.push_back(VK_GOOGLE_DISPLAY_TIMING_EXTENSION_NAME);
        }

#ifdef VULKAN_DEBUG
        if (has_extension(VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME))
        {
            _features.supports_nv_device_diagnostic_checkpoints = true;
            enabled_extensions.push_back(VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME);
        }
#endif

#ifdef _WIN32
        _features.supports_external = false;
#else
        if (_features.supports_external && features.supports_dedicated &&
            has_extension(VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME) &&
            has_extension(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME) &&
            has_extension(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME) &&
            has_extension(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME))
        {
            _features.supports_external = true;
            enabled_extensions.push_back(VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME);
            enabled_extensions.push_back(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME);
            enabled_extensions.push_back(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
            enabled_extensions.push_back(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
        }
        else
        {
            _features.supports_external = false;
        }
#endif

        VkPhysicalDeviceFeatures2KHR deviceFeatures2 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR };
        _features.storage_8bit_features = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES_KHR };
        _features.storage_16bit_features = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES_KHR };
        _features.float16_int8_features = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT16_INT8_FEATURES_KHR };
        ppNext = &deviceFeatures2.pNext;

        if (has_extension(VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME))
            enabled_extensions.push_back(VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME);

        if (_features.supports_physical_device_properties2 && has_extension(VK_KHR_8BIT_STORAGE_EXTENSION_NAME))
        {
            enabled_extensions.push_back(VK_KHR_8BIT_STORAGE_EXTENSION_NAME);
            *ppNext = &_features.storage_8bit_features;
            ppNext = &_features.storage_8bit_features.pNext;
        }

        if (_features.supports_physical_device_properties2 && has_extension(VK_KHR_16BIT_STORAGE_EXTENSION_NAME))
        {
            enabled_extensions.push_back(VK_KHR_16BIT_STORAGE_EXTENSION_NAME);
            *ppNext = &_features.storage_16bit_features;
            ppNext = &_features.storage_16bit_features.pNext;
        }

        if (_features.supports_physical_device_properties2
            && has_extension(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME))
        {
            enabled_extensions.push_back(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
            *ppNext = &_features.float16_int8_features;
            ppNext = &_features.float16_int8_features.pNext;
        }

        if (_features.supports_physical_device_properties2)
            vkGetPhysicalDeviceFeatures2KHR(_physicalDevice, &deviceFeatures2);
        else
            vkGetPhysicalDeviceFeatures(_physicalDevice, &deviceFeatures2.features);

        // Enable device features we might care about.
        {
            VkPhysicalDeviceFeatures enabled_features = {};
            if (deviceFeatures2.features.textureCompressionETC2)
                enabled_features.textureCompressionETC2 = VK_TRUE;
            if (deviceFeatures2.features.textureCompressionBC)
                enabled_features.textureCompressionBC = VK_TRUE;
            if (deviceFeatures2.features.textureCompressionASTC_LDR)
                enabled_features.textureCompressionASTC_LDR = VK_TRUE;
            if (deviceFeatures2.features.fullDrawIndexUint32)
                enabled_features.fullDrawIndexUint32 = VK_TRUE;
            if (deviceFeatures2.features.imageCubeArray)
                enabled_features.imageCubeArray = VK_TRUE;
            if (deviceFeatures2.features.fillModeNonSolid)
                enabled_features.fillModeNonSolid = VK_TRUE;
            if (deviceFeatures2.features.independentBlend)
                enabled_features.independentBlend = VK_TRUE;
            if (deviceFeatures2.features.sampleRateShading)
                enabled_features.sampleRateShading = VK_TRUE;
            if (deviceFeatures2.features.fragmentStoresAndAtomics)
                enabled_features.fragmentStoresAndAtomics = VK_TRUE;
            if (deviceFeatures2.features.shaderStorageImageExtendedFormats)
                enabled_features.shaderStorageImageExtendedFormats = VK_TRUE;
            if (deviceFeatures2.features.shaderStorageImageMultisample)
                enabled_features.shaderStorageImageMultisample = VK_TRUE;
            if (deviceFeatures2.features.largePoints)
                enabled_features.largePoints = VK_TRUE;

            if (deviceFeatures2.features.shaderSampledImageArrayDynamicIndexing)
                enabled_features.shaderSampledImageArrayDynamicIndexing = VK_TRUE;
            if (deviceFeatures2.features.shaderUniformBufferArrayDynamicIndexing)
                enabled_features.shaderUniformBufferArrayDynamicIndexing = VK_TRUE;
            if (deviceFeatures2.features.shaderStorageBufferArrayDynamicIndexing)
                enabled_features.shaderStorageBufferArrayDynamicIndexing = VK_TRUE;
            if (deviceFeatures2.features.shaderStorageImageArrayDynamicIndexing)
                enabled_features.shaderStorageImageArrayDynamicIndexing = VK_TRUE;

            deviceFeatures2.features = enabled_features;
            _features.enabled_features = enabled_features;
        }

        if (_features.supports_physical_device_properties2)
            deviceCreateInfo.pNext = &deviceFeatures2;
        else
            deviceCreateInfo.pEnabledFeatures = &deviceFeatures2.features;

        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(enabled_extensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = enabled_extensions.data();
        // Deprecated and ignored
        deviceCreateInfo.enabledLayerCount = 0;
        deviceCreateInfo.ppEnabledLayerNames = nullptr;

        if (vkCreateDevice(_physicalDevice, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS)
        {
            return false;
        }

        volkLoadDevice(device);
        vkGetDeviceQueue(device, graphicsQueueFamily, graphics_queue_index, &_graphicsQueue);
        vkGetDeviceQueue(device, computeQueueFamily, compute_queue_index, &_computeQueue);
        vkGetDeviceQueue(device, transferQueueFamily, transfer_queue_index, &_transferQueue);

        // Init info and caps.
        InitializeCaps();

        if (!InitializeAllocator())
        {
            return false;
        }

        // Create default graphics command pool.
        VkCommandPoolCreateInfo cmdPoolInfo = {};
        cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmdPoolInfo.queueFamilyIndex = graphicsQueueFamily;
        cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        vkThrowIfFailed(vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &_graphicsCommandPool));

        // Init swap chain.
        auto swapChainVk = new SwapChainVk(this, surface, &desc.swapChainDescriptor);
        _swapChain.reset(swapChainVk);

        const bool headless = false;
        maxInflightFrames = headless ? 3u : swapChainVk->GetImageCount();
        perFrame.clear();

        for (uint32_t i = 0; i < maxInflightFrames; i++)
        {
            auto frame = unique_ptr<PerFrame>(new PerFrame(this));
            perFrame.emplace_back(move(frame));
        }

        return GraphicsDevice::Initialize(window, desc);
    }

    void GraphicsDeviceVk::InitializeCaps()
    {
        _info.backend = GraphicsBackend::Vulkan;
        _info.backendName = "Vulkan " + vkGetVersionToString(_deviceProperties.apiVersion);
        _info.deviceName = _deviceProperties.deviceName;
        _info.vendorName = vkGetVendorByID(_deviceProperties.vendorID);
        _info.vendorId = _deviceProperties.vendorID;
        //_info.shadingLanguageName = "SPIR-V";

        _caps.features.instancing = true;
        _caps.features.alphaToCoverage = true;
        _caps.features.independentBlend = _deviceFeatures.independentBlend;
        _caps.features.computeShader = true;
        _caps.features.geometryShader = _deviceFeatures.geometryShader;
        _caps.features.tessellationShader = _deviceFeatures.tessellationShader;
        _caps.features.sampleRateShading = _deviceFeatures.sampleRateShading;
        _caps.features.dualSrcBlend = _deviceFeatures.dualSrcBlend;
        _caps.features.logicOp = _deviceFeatures.logicOp;
        _caps.features.multiViewport = _deviceFeatures.multiViewport;
        _caps.features.indexUInt32 = _deviceFeatures.fullDrawIndexUint32;
        _caps.features.drawIndirect = _deviceFeatures.multiDrawIndirect;
        _caps.features.alphaToOne = _deviceFeatures.alphaToOne;
        _caps.features.fillModeNonSolid = _deviceFeatures.fillModeNonSolid;
        _caps.features.samplerAnisotropy = _deviceFeatures.samplerAnisotropy;
        _caps.features.textureCompressionBC = _deviceFeatures.textureCompressionBC;
        _caps.features.textureCompressionPVRTC = false;
        _caps.features.textureCompressionETC2 = _deviceFeatures.textureCompressionETC2;
        _caps.features.textureCompressionATC = false;
        _caps.features.textureCompressionASTC = _deviceFeatures.textureCompressionASTC_LDR;
        _caps.features.pipelineStatisticsQuery = _deviceFeatures.pipelineStatisticsQuery;
        _caps.features.texture1D = true;
        _caps.features.texture3D = true;
        _caps.features.texture2DArray = true;
        _caps.features.textureCubeArray = _deviceFeatures.imageCubeArray;

        // Limits
        _caps.limits.maxTextureDimension1D = _deviceProperties.limits.maxImageDimension1D;
        _caps.limits.maxTextureDimension2D = _deviceProperties.limits.maxImageDimension2D;
        _caps.limits.maxTextureDimension3D = _deviceProperties.limits.maxImageDimension3D;
        _caps.limits.maxTextureDimensionCube = _deviceProperties.limits.maxImageDimensionCube;
        _caps.limits.maxTextureArrayLayers = _deviceProperties.limits.maxImageArrayLayers;
        _caps.limits.maxColorAttachments = _deviceProperties.limits.maxColorAttachments;
        _caps.limits.maxUniformBufferSize = _deviceProperties.limits.maxUniformBufferRange;
        _caps.limits.minUniformBufferOffsetAlignment = _deviceProperties.limits.minUniformBufferOffsetAlignment;
        _caps.limits.maxStorageBufferSize = _deviceProperties.limits.maxStorageBufferRange;
        _caps.limits.minStorageBufferOffsetAlignment = _deviceProperties.limits.minStorageBufferOffsetAlignment;
        _caps.limits.maxSamplerAnisotropy = static_cast<uint32_t>(_deviceProperties.limits.maxSamplerAnisotropy);
        _caps.limits.maxViewports = _deviceProperties.limits.maxViewports;

        _caps.limits.maxViewportDimensions[0] = _deviceProperties.limits.maxViewportDimensions[0];
        _caps.limits.maxViewportDimensions[1] = _deviceProperties.limits.maxViewportDimensions[0];
        _caps.limits.maxPatchVertices = _deviceProperties.limits.maxTessellationPatchSize;
        _caps.limits.pointSizeRange[0] = _deviceProperties.limits.pointSizeRange[0];
        _caps.limits.pointSizeRange[1] = _deviceProperties.limits.pointSizeRange[1];
        _caps.limits.lineWidthRange[0] = _deviceProperties.limits.lineWidthRange[0];
        _caps.limits.lineWidthRange[1] = _deviceProperties.limits.lineWidthRange[0];
        _caps.limits.maxComputeSharedMemorySize = _deviceProperties.limits.maxComputeSharedMemorySize;
        _caps.limits.maxComputeWorkGroupCount[0] = _deviceProperties.limits.maxComputeWorkGroupCount[0];
        _caps.limits.maxComputeWorkGroupCount[1] = _deviceProperties.limits.maxComputeWorkGroupCount[1];
        _caps.limits.maxComputeWorkGroupCount[2] = _deviceProperties.limits.maxComputeWorkGroupCount[2];
        _caps.limits.maxComputeWorkGroupInvocations = _deviceProperties.limits.maxComputeWorkGroupInvocations;
        _caps.limits.maxComputeWorkGroupSize[0] = _deviceProperties.limits.maxComputeWorkGroupSize[0];
        _caps.limits.maxComputeWorkGroupSize[1] = _deviceProperties.limits.maxComputeWorkGroupSize[1];
        _caps.limits.maxComputeWorkGroupSize[2] = _deviceProperties.limits.maxComputeWorkGroupSize[2];
    }

    bool GraphicsDeviceVk::InitializeAllocator()
    {
        VmaVulkanFunctions vmaFunctions = {};
        vmaFunctions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
        vmaFunctions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
        vmaFunctions.vkAllocateMemory = vkAllocateMemory;
        vmaFunctions.vkFreeMemory = vkFreeMemory;
        vmaFunctions.vkMapMemory = vkMapMemory;
        vmaFunctions.vkUnmapMemory = vkUnmapMemory;
        vmaFunctions.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
        vmaFunctions.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
        vmaFunctions.vkBindBufferMemory = vkBindBufferMemory;
        vmaFunctions.vkBindImageMemory = vkBindImageMemory;
        vmaFunctions.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
        vmaFunctions.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
        vmaFunctions.vkCreateBuffer = vkCreateBuffer;
        vmaFunctions.vkCreateImage = vkCreateImage;
        vmaFunctions.vkDestroyBuffer = vkDestroyBuffer;
        vmaFunctions.vkDestroyImage = vkDestroyImage;
        vmaFunctions.vkCmdCopyBuffer = vkCmdCopyBuffer;
#if VMA_DEDICATED_ALLOCATION
        vmaFunctions.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR;
        vmaFunctions.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2KHR;
#endif

        VmaAllocatorCreateInfo allocatorCreateInfo = {};
        allocatorCreateInfo.physicalDevice = _physicalDevice;
        allocatorCreateInfo.device = device;
        allocatorCreateInfo.pVulkanFunctions = &vmaFunctions;

        if (vmaCreateAllocator(&allocatorCreateInfo, &_memoryAllocator) != VK_SUCCESS)
        {
            LOGE("Cannot create vma memory allocator");
            return false;
        }

        return true;
    }

    void GraphicsDeviceVk::notifyValidationError(const char* message)
    {
        ALIMER_UNUSED(message);
    }

    bool GraphicsDeviceVk::BeginFrame()
    {
        vkThrowIfFailed(vkWaitForFences(device, 1u, &frame().fence, VK_TRUE, std::numeric_limits<uint64_t>::max()));
        vkThrowIfFailed(vkResetFences(device, 1u, &frame().fence));

        /*const VkResult result =
            vkAcquireNextImageKHR(device,
                swapchain,
                std::numeric_limits<uint64_t>::max(),
                swapchainImageSemaphores[frameNumber],
                VK_NULL_HANDLE,
                &swapchainImageIndex);

        if (result != VK_SUCCESS)
        {
            return false;
        }*/

        return true;
    }

    void GraphicsDeviceVk::EndFrame()
    {
        const uint32_t frameIndex = frameNumber;

        // Submit the pending command buffers.
        const VkPipelineStageFlags colorAttachmentStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        uint32_t waitSemaphoreCount = 0u;
        VkSemaphore *waitSemaphores = NULL;
        const VkPipelineStageFlags *waitStageMasks = nullptr;
        /*if (swapchain != VK_NULL_HANDLE)
        {
            waitSemaphoreCount = 1u;
            waitSemaphores = &swapchainImageSemaphores[frameIndex];
            waitStageMasks = &colorAttachmentStage;
        }*/

        VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.waitSemaphoreCount = waitSemaphoreCount;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStageMasks;
        submitInfo.commandBufferCount = (uint32_t)frame().submittedCommandBuffers.size();
        submitInfo.pCommandBuffers = frame().submittedCommandBuffers.data();
        submitInfo.signalSemaphoreCount = (uint32_t)frame().waitSemaphores.size();
        submitInfo.pSignalSemaphores = frame().waitSemaphores.data();

        vkQueueSubmit(_graphicsQueue, 1u, &submitInfo, frame().fence);

        /*if (swapchain != VK_NULL_HANDLE)
        {
            VkResult presentResult = VK_SUCCESS;
            VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
            presentInfo.waitSemaphoreCount = (uint32_t)frame().waitSemaphores.size();
            presentInfo.pWaitSemaphores = frame().waitSemaphores.data();
            presentInfo.swapchainCount = 1u;
            presentInfo.pSwapchains = &swapchain;
            presentInfo.pImageIndices = &swapchainImageIndex;
            presentInfo.pResults = &presentResult;
            vkQueuePresentKHR(graphicsQueue, &presentInfo);
            if (presentResult != VK_SUCCESS)
            {
            }
        }*/

        // Advance to next frame.
        frameNumber = (frameNumber + 1u) % maxInflightFrames;
    }

    VkCommandBuffer GraphicsDeviceVk::CreateCommandBuffer(VkCommandBufferLevel level, bool begin)
    {
        VkCommandBufferAllocateInfo cmdBufAllocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO  };
        cmdBufAllocateInfo.commandPool = _graphicsCommandPool;
        cmdBufAllocateInfo.level = level;
        cmdBufAllocateInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkThrowIfFailed(vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, &commandBuffer));

        // If requested, also start the new command buffer
        if (begin)
        {
            VkCommandBufferBeginInfo cmdBufInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
            vkThrowIfFailed(vkBeginCommandBuffer(commandBuffer, &cmdBufInfo));
        }

        return commandBuffer;
    }
    void GraphicsDeviceVk::FlushCommandBuffer(VkCommandBuffer commandBuffer, bool free)
    {
        FlushCommandBuffer(commandBuffer, _graphicsQueue, free);
    }

    void GraphicsDeviceVk::FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free)
    {
        vkThrowIfFailed(vkEndCommandBuffer(commandBuffer));

        VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        // Create fence to ensure that the command buffer has finished executing
        VkFenceCreateInfo fenceCreateInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
        VkFence fence;
        vkThrowIfFailed(vkCreateFence(device, &fenceCreateInfo, nullptr, &fence));

        vkThrowIfFailed(vkQueueSubmit(queue, 1, &submitInfo, fence));
        // Wait for the fence to signal that command buffer has finished executing
        vkThrowIfFailed(vkWaitForFences(device, 1, &fence, VK_TRUE, 100000000000));
        vkDestroyFence(device, fence, nullptr);

        if (free)
        {
            vkFreeCommandBuffers(device, _graphicsCommandPool, 1, &commandBuffer);
        }
    }

    GraphicsDeviceVk::PerFrame::PerFrame(GraphicsDeviceVk* device_)
        : device(device_)
    {
        const VkFenceCreateInfo fenceCreateInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, VK_FENCE_CREATE_SIGNALED_BIT };
        vkCreateFence(device->device, &fenceCreateInfo, nullptr, &fence);
    }

    GraphicsDeviceVk::PerFrame::~PerFrame()
    {
        Begin();
        vkDestroyFence(device->device, fence, nullptr);
    }

    void GraphicsDeviceVk::PerFrame::Begin()
    {

    }
}
