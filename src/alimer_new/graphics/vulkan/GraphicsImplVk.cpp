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

#include "alimer_config.h"
#include "engine/Window.h"
#include "GraphicsImplVk.h"
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
        auto *context = static_cast<GraphicsImpl*>(pUserData);

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
        auto *context = static_cast<GraphicsImpl*>(pUserData);

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

    GraphicsImpl::GraphicsImpl()
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
            features.supports_physical_device_properties2 = true;
            enabled_extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        }

        if (features.supports_physical_device_properties2 &&
            has_extension(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME) &&
            has_extension(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME))
        {
            enabled_extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
            enabled_extensions.push_back(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME);
            enabled_extensions.push_back(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME);
            features.supports_external = true;
        }

        if (has_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
        {
            enabled_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            features.supports_debug_utils = true;
        }

#ifdef VULKAN_DEBUG
        const auto has_layer = [&](const char *name) -> bool {
            auto itr = find_if(begin(instance_layers), end(instance_layers), [name](const VkLayerProperties &e) -> bool {
                return strcmp(e.layerName, name) == 0;
                });
            return itr != end(instance_layers);
        };

        if (!features.supports_debug_utils && has_extension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
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
            features.supports_vulkan_11_instance = true;
        }

        VkInstanceCreateInfo instanceCreateInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
        instanceCreateInfo.pApplicationInfo = &appInfo;
        instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(enabled_extensions.size());
        instanceCreateInfo.ppEnabledExtensionNames = enabled_extensions.empty() ? nullptr : enabled_extensions.data();
        instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(enabled_layers.size());
        instanceCreateInfo.ppEnabledLayerNames = enabled_layers.empty() ? nullptr : enabled_layers.data();

        result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
        if (result != VK_SUCCESS)
        {
            throw VulkanException(result, "Could not create Vulkan instance");
        }

        volkLoadInstance(instance);

#ifdef VULKAN_DEBUG
        if (features.supports_debug_utils)
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

            vkCreateDebugUtilsMessengerEXT(instance, &info, nullptr, &debugMessenger);
        }
        else if (has_extension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
        {
            VkDebugReportCallbackCreateInfoEXT info = { VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT };
            info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
                VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
            info.pfnCallback = vulkanDebugCallback;
            info.pUserData = this;
            vkCreateDebugReportCallbackEXT(instance, &info, nullptr, &debugCallback);
        }
#endif
    }

    GraphicsImpl::~GraphicsImpl()
    {
        destroy();
    }

    void GraphicsImpl::destroy()
    {
        if (device != VK_NULL_HANDLE)
            vkDeviceWaitIdle(device);

        if (swapchain != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(device, swapchain, nullptr);
            swapchain = VK_NULL_HANDLE;
        }

        if (surface != VK_NULL_HANDLE)
        {
            vkDestroySurfaceKHR(instance, surface, nullptr);
            surface = VK_NULL_HANDLE;
        }

        if (memoryAllocator != VK_NULL_HANDLE)
        {
            VmaStats stats;
            vmaCalculateStats(memoryAllocator, &stats);

            LOGI("Total device memory leaked: %llu bytes.", stats.total.usedBytes);

            vmaDestroyAllocator(memoryAllocator);
            memoryAllocator = VK_NULL_HANDLE;
        }

        if (device != VK_NULL_HANDLE)
        {
            vkDestroyDevice(device, nullptr);
            device = VK_NULL_HANDLE;
        }

#ifdef VULKAN_DEBUG
        if (debugCallback != VK_NULL_HANDLE)
        {
            vkDestroyDebugReportCallbackEXT(instance, debugCallback, nullptr);
            debugCallback = VK_NULL_HANDLE;
        }

        if (debugMessenger != VK_NULL_HANDLE)
        {
            vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
            debugMessenger = VK_NULL_HANDLE;
        }
#endif

        if (instance != VK_NULL_HANDLE)
        {
            vkDestroyInstance(instance, nullptr);
            instance = VK_NULL_HANDLE;
        }
    }

    bool GraphicsImpl::initialize(Window* window, const GraphicsDeviceDescriptor& desc)
    {
        // Create surface first.
#if defined(ALIMER_GLFW)
        vkThrowIfFailed(
            glfwCreateWindowSurface(instance, window->getApiHandle(), NULL, &surface)
        );
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
#endif

        // Obtain a list of available physical devices.
        uint32_t physical_device_count = 0u;
        vkEnumeratePhysicalDevices(instance, &physical_device_count, NULL);

        vector<VkPhysicalDevice> gpus(physical_device_count);
        vkEnumeratePhysicalDevices(instance, &physical_device_count, gpus.data());

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

        physicalDevice = gpus[best_device_index];
        vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
        vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);

        LOGI("Selected Vulkan GPU: %s", deviceProperties.deviceName);

        uint32_t ext_count = 0;
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &ext_count, nullptr);
        vector<VkExtensionProperties> queried_extensions(ext_count);
        if (ext_count) {
            vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &ext_count, queried_extensions.data());
        }

        const auto has_extension = [&](const char *name) -> bool {
            auto itr = find_if(begin(queried_extensions), end(queried_extensions), [name](const VkExtensionProperties &e) -> bool {
                return strcmp(e.extensionName, name) == 0;
                });
            return itr != end(queried_extensions);
        };

        if (deviceProperties.apiVersion >= VK_API_VERSION_1_1)
        {
            features.supports_vulkan_11_device = features.supports_vulkan_11_instance;
            LOGI("GPU supports Vulkan 1.1.");
        }
        else if (deviceProperties.apiVersion >= VK_API_VERSION_1_0)
        {
            features.supports_vulkan_11_device = false;
            LOGI("GPU supports Vulkan 1.0.");
        }

        // Only need GetPhysicalDeviceProperties2 for Vulkan 1.1-only code, so don't bother getting KHR variant.
        features.subgroup_properties = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES };
        VkPhysicalDeviceProperties2 props = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
        void **ppNext = &props.pNext;

        if (features.supports_vulkan_11_instance && features.supports_vulkan_11_device)
        {
            *ppNext = &features.subgroup_properties;
            ppNext = &features.subgroup_properties.pNext;
        }

        if (features.supports_vulkan_11_instance && features.supports_vulkan_11_device)
        {
            vkGetPhysicalDeviceProperties2(physicalDevice, &props);
        }

        uint32_t queue_count;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queue_count, nullptr);
        vector<VkQueueFamilyProperties> queue_props(queue_count);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queue_count, queue_props.data());

        for (uint32_t i = 0; i < queue_count; i++)
        {
            VkBool32 supported = surface == VK_NULL_HANDLE;
            if (surface != VK_NULL_HANDLE)
                vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &supported);

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
            features.supports_dedicated = true;
            enabled_extensions.push_back(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
            enabled_extensions.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        }

        if (has_extension(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME))
        {
            features.supports_image_format_list = true;
            enabled_extensions.push_back(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
        }

        if (has_extension(VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
        {
            features.supports_debug_marker = true;
            enabled_extensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
        }

        if (has_extension(VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME))
        {
            features.supports_mirror_clamp_to_edge = true;
            enabled_extensions.push_back(VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME);
        }

        if (has_extension(VK_GOOGLE_DISPLAY_TIMING_EXTENSION_NAME))
        {
            features.supports_google_display_timing = true;
            enabled_extensions.push_back(VK_GOOGLE_DISPLAY_TIMING_EXTENSION_NAME);
        }

#ifdef VULKAN_DEBUG
        if (has_extension(VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME))
        {
            features.supports_nv_device_diagnostic_checkpoints = true;
            enabled_extensions.push_back(VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME);
        }
#endif

#ifdef _WIN32
        features.supports_external = false;
#else
        if (features.supports_external && features.supports_dedicated &&
            has_extension(VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME) &&
            has_extension(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME) &&
            has_extension(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME) &&
            has_extension(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME))
        {
            features.supports_external = true;
            enabled_extensions.push_back(VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME);
            enabled_extensions.push_back(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME);
            enabled_extensions.push_back(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
            enabled_extensions.push_back(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
        }
        else
        {
            features.supports_external = false;
        }
#endif

        VkPhysicalDeviceFeatures2KHR deviceFeatures2 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR };
        features.storage_8bit_features = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES_KHR };
        features.storage_16bit_features = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES_KHR };
        features.float16_int8_features = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT16_INT8_FEATURES_KHR };
        ppNext = &deviceFeatures2.pNext;

        if (has_extension(VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME))
            enabled_extensions.push_back(VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME);

        if (features.supports_physical_device_properties2 && has_extension(VK_KHR_8BIT_STORAGE_EXTENSION_NAME))
        {
            enabled_extensions.push_back(VK_KHR_8BIT_STORAGE_EXTENSION_NAME);
            *ppNext = &features.storage_8bit_features;
            ppNext = &features.storage_8bit_features.pNext;
        }

        if (features.supports_physical_device_properties2 && has_extension(VK_KHR_16BIT_STORAGE_EXTENSION_NAME))
        {
            enabled_extensions.push_back(VK_KHR_16BIT_STORAGE_EXTENSION_NAME);
            *ppNext = &features.storage_16bit_features;
            ppNext = &features.storage_16bit_features.pNext;
        }

        if (features.supports_physical_device_properties2
            && has_extension(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME))
        {
            enabled_extensions.push_back(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
            *ppNext = &features.float16_int8_features;
            ppNext = &features.float16_int8_features.pNext;
        }

        if (features.supports_physical_device_properties2)
            vkGetPhysicalDeviceFeatures2KHR(physicalDevice, &deviceFeatures2);
        else
            vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures2.features);

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
            features.enabled_features = enabled_features;
        }

        if (features.supports_physical_device_properties2)
            deviceCreateInfo.pNext = &deviceFeatures2;
        else
            deviceCreateInfo.pEnabledFeatures = &deviceFeatures2.features;

        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(enabled_extensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = enabled_extensions.data();
        // Deprecated and ignored
        deviceCreateInfo.enabledLayerCount = 0;
        deviceCreateInfo.ppEnabledLayerNames = nullptr;

        if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS)
        {
            return false;
        }

        volkLoadDevice(device);
        vkGetDeviceQueue(device, graphicsQueueFamily, graphics_queue_index, &graphicsQueue);
        vkGetDeviceQueue(device, computeQueueFamily, compute_queue_index, &computeQueue);
        vkGetDeviceQueue(device, transferQueueFamily, transfer_queue_index, &transferQueue);

        // Init info and caps.
        initializeCaps();

        if (!initializeAllocator())
        {
            return false;
        }

        if (!initializeSwapChain(window, desc))
        {
            return false;
        }

        return true;
    }

    void GraphicsImpl::initializeCaps()
    {
        info.backend = GraphicsBackend::Vulkan;
        info.backendName = "Vulkan " + vkGetVersionToString(deviceProperties.apiVersion);
        info.deviceName = deviceProperties.deviceName;
        info.vendorName = vkGetVendorByID(deviceProperties.vendorID);
        info.vendorId = deviceProperties.vendorID;
        //info.shadingLanguageName = "SPIR-V";

        caps.features.instancing = true;
        caps.features.alphaToCoverage = true;
        caps.features.independentBlend = deviceFeatures.independentBlend;
        caps.features.computeShader = true;
        caps.features.geometryShader = deviceFeatures.geometryShader;
        caps.features.tessellationShader = deviceFeatures.tessellationShader;
        caps.features.sampleRateShading = deviceFeatures.sampleRateShading;
        caps.features.dualSrcBlend = deviceFeatures.dualSrcBlend;
        caps.features.logicOp = deviceFeatures.logicOp;
        caps.features.multiViewport = deviceFeatures.multiViewport;
        caps.features.indexUInt32 = deviceFeatures.fullDrawIndexUint32;
        caps.features.drawIndirect = deviceFeatures.multiDrawIndirect;
        caps.features.alphaToOne = deviceFeatures.alphaToOne;
        caps.features.fillModeNonSolid = deviceFeatures.fillModeNonSolid;
        caps.features.samplerAnisotropy = deviceFeatures.samplerAnisotropy;
        caps.features.textureCompressionBC = deviceFeatures.textureCompressionBC;
        caps.features.textureCompressionPVRTC = false;
        caps.features.textureCompressionETC2 = deviceFeatures.textureCompressionETC2;
        caps.features.textureCompressionATC = false;
        caps.features.textureCompressionASTC = deviceFeatures.textureCompressionASTC_LDR;
        caps.features.pipelineStatisticsQuery = deviceFeatures.pipelineStatisticsQuery;
        caps.features.texture1D = true;
        caps.features.texture3D = true;
        caps.features.texture2DArray = true;
        caps.features.textureCubeArray = deviceFeatures.imageCubeArray;

        // Limits
        caps.limits.maxTextureDimension1D = deviceProperties.limits.maxImageDimension1D;
        caps.limits.maxTextureDimension2D = deviceProperties.limits.maxImageDimension2D;
        caps.limits.maxTextureDimension3D = deviceProperties.limits.maxImageDimension3D;
        caps.limits.maxTextureDimensionCube = deviceProperties.limits.maxImageDimensionCube;
        caps.limits.maxTextureArrayLayers = deviceProperties.limits.maxImageArrayLayers;
        caps.limits.maxColorAttachments = deviceProperties.limits.maxColorAttachments;
        caps.limits.maxUniformBufferSize = deviceProperties.limits.maxUniformBufferRange;
        caps.limits.minUniformBufferOffsetAlignment = deviceProperties.limits.minUniformBufferOffsetAlignment;
        caps.limits.maxStorageBufferSize = deviceProperties.limits.maxStorageBufferRange;
        caps.limits.minStorageBufferOffsetAlignment = deviceProperties.limits.minStorageBufferOffsetAlignment;
        caps.limits.maxSamplerAnisotropy = static_cast<uint32_t>(deviceProperties.limits.maxSamplerAnisotropy);
        caps.limits.maxViewports = deviceProperties.limits.maxViewports;

        caps.limits.maxViewportDimensions[0] = deviceProperties.limits.maxViewportDimensions[0];
        caps.limits.maxViewportDimensions[1] = deviceProperties.limits.maxViewportDimensions[0];
        caps.limits.maxPatchVertices = deviceProperties.limits.maxTessellationPatchSize;
        caps.limits.pointSizeRange[0] = deviceProperties.limits.pointSizeRange[0];
        caps.limits.pointSizeRange[1] = deviceProperties.limits.pointSizeRange[1];
        caps.limits.lineWidthRange[0] = deviceProperties.limits.lineWidthRange[0];
        caps.limits.lineWidthRange[1] = deviceProperties.limits.lineWidthRange[0];
        caps.limits.maxComputeSharedMemorySize = deviceProperties.limits.maxComputeSharedMemorySize;
        caps.limits.maxComputeWorkGroupCount[0] = deviceProperties.limits.maxComputeWorkGroupCount[0];
        caps.limits.maxComputeWorkGroupCount[1] = deviceProperties.limits.maxComputeWorkGroupCount[1];
        caps.limits.maxComputeWorkGroupCount[2] = deviceProperties.limits.maxComputeWorkGroupCount[2];
        caps.limits.maxComputeWorkGroupInvocations = deviceProperties.limits.maxComputeWorkGroupInvocations;
        caps.limits.maxComputeWorkGroupSize[0] = deviceProperties.limits.maxComputeWorkGroupSize[0];
        caps.limits.maxComputeWorkGroupSize[1] = deviceProperties.limits.maxComputeWorkGroupSize[1];
        caps.limits.maxComputeWorkGroupSize[2] = deviceProperties.limits.maxComputeWorkGroupSize[2];
    }

    bool GraphicsImpl::initializeAllocator()
    {
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
        allocatorCreateInfo.physicalDevice = physicalDevice;
        allocatorCreateInfo.device = device;
        allocatorCreateInfo.pVulkanFunctions = &vma_vulkan_func;

        if (vmaCreateAllocator(&allocatorCreateInfo, &memoryAllocator) != VK_SUCCESS)
        {
            LOGE("Cannot create vma memory allocator");
            return false;
        }

        return true;
    }

    bool GraphicsImpl::initializeSwapChain(Window* window, const GraphicsDeviceDescriptor& desc)
    {
        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        vkThrowIfFailed(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities));

        uint32_t format_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &format_count, nullptr);
        std::vector<VkSurfaceFormatKHR> formats(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &format_count, formats.data());

        VkSurfaceFormatKHR format;
        if (format_count == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
        {
            format = formats[0];
            format.format = VK_FORMAT_B8G8R8A8_UNORM;
        }
        else
        {
            if (format_count == 0)
            {
                LOGE("Vulkan: Surface has no formats.");
                return false;
            }

            bool found = false;
            for (uint32_t i = 0; i < format_count; i++)
            {
                if (desc.srgb)
                {
                    if (formats[i].format == VK_FORMAT_R8G8B8A8_SRGB ||
                        formats[i].format == VK_FORMAT_B8G8R8A8_SRGB ||
                        formats[i].format == VK_FORMAT_A8B8G8R8_SRGB_PACK32)
                    {
                        format = formats[i];
                        found = true;
                    }
                }
                else
                {
                    if (formats[i].format == VK_FORMAT_R8G8B8A8_UNORM ||
                        formats[i].format == VK_FORMAT_B8G8R8A8_UNORM ||
                        formats[i].format == VK_FORMAT_A8B8G8R8_UNORM_PACK32)
                    {
                        format = formats[i];
                        found = true;
                    }
                }
            }

            if (!found)
                format = formats[0];
        }

        VkExtent2D swapchain_size;
        if (surfaceCapabilities.currentExtent.width == ~0u)
        {
            swapchain_size.width = window->getWidth();
            swapchain_size.height = window->getHeight();
        }
        else
        {
            swapchain_size.width = max(min(window->getWidth(), surfaceCapabilities.maxImageExtent.width), surfaceCapabilities.minImageExtent.width);
            swapchain_size.height = max(min(window->getHeight(), surfaceCapabilities.maxImageExtent.height), surfaceCapabilities.minImageExtent.height);
        }

        uint32_t num_present_modes;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &num_present_modes, nullptr);
        vector<VkPresentModeKHR> present_modes(num_present_modes);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &num_present_modes, present_modes.data());

        VkPresentModeKHR swapchain_present_mode = VK_PRESENT_MODE_FIFO_KHR;
        if (!desc.vSyncEnabled)
        {
            for (uint32_t i = 0; i < num_present_modes; i++)
            {
                if (present_modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR || present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
                {
                    swapchain_present_mode = present_modes[i];
                    break;
                }
            }
        }

        uint32_t desired_swapchain_images = 3;

        if (desired_swapchain_images < surfaceCapabilities.minImageCount)
            desired_swapchain_images = surfaceCapabilities.minImageCount;

        if ((surfaceCapabilities.maxImageCount > 0) && (desired_swapchain_images > surfaceCapabilities.maxImageCount))
            desired_swapchain_images = surfaceCapabilities.maxImageCount;

        VkSurfaceTransformFlagBitsKHR pre_transform;
        if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
            pre_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        else
            pre_transform = surfaceCapabilities.currentTransform;

        VkCompositeAlphaFlagBitsKHR composite_mode = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        if (surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR)
            composite_mode = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
        if (surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
            composite_mode = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        if (surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR)
            composite_mode = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
        if (surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR)
            composite_mode = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;

        VkSwapchainKHR old_swapchain = swapchain;

        VkSwapchainCreateInfoKHR info = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
        info.surface = surface;
        info.minImageCount = desired_swapchain_images;
        info.imageFormat = format.format;
        info.imageColorSpace = format.colorSpace;
        info.imageExtent.width = swapchain_size.width;
        info.imageExtent.height = swapchain_size.height;
        info.imageArrayLayers = 1;
        info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.preTransform = pre_transform;
        info.compositeAlpha = composite_mode;
        info.presentMode = swapchain_present_mode;
        info.clipped = VK_TRUE;
        info.oldSwapchain = old_swapchain;

        // Enable transfer source on swap chain images if supported
        if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
            info.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        }

        // Enable transfer destination on swap chain images if supported
        if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
            info.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        }

        if (vkCreateSwapchainKHR(device, &info, nullptr, &swapchain) != VK_SUCCESS)
        {
            return false;
        }

        if (old_swapchain != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(device, old_swapchain, nullptr);
        }

        return true;
    }

    void GraphicsImpl::notifyValidationError(const char* message)
    {
        ALIMER_UNUSED(message);
    }
}
