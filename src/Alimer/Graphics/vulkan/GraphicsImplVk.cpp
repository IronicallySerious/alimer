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
#include "../GraphicsDevice.h"

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
        GraphicsDevice* device = static_cast<GraphicsDevice*>(pUserData);

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
        GraphicsDevice* device = static_cast<GraphicsDevice*>(pUserData);

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

    bool GraphicsDevice::IsSupported()
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
   
    bool GraphicsDevice::PlatformInitialize(const GraphicsDeviceDescriptor* descriptor)
    {
        _validation = descriptor->validation;
        _headless = descriptor->headless;

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
            uint32_t layer_count = 0;
            vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
            std::vector<VkLayerProperties> queried_layers(layer_count);
            if (layer_count)
            {
                vkEnumerateInstanceLayerProperties(&layer_count, queried_layers.data());
            }

            const auto has_layer = [&](const char *name) -> bool {
                auto itr = find_if(begin(queried_layers), end(queried_layers), [name](const VkLayerProperties &e) -> bool {
                    return strcmp(e.layerName, name) == 0;
                    });
                return itr != end(queried_layers);
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
        instanceCreateInfo.enabledLayerCount = (uint32_t)instanceLayers.size();
        instanceCreateInfo.ppEnabledLayerNames = instanceLayers.data();
        instanceCreateInfo.enabledExtensionCount = (uint32_t)instanceExtensions.size();
        instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();

        VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &_instance);
        if (result != VK_SUCCESS)
        {
            ALIMER_LOGCRITICAL("Vulkan: Failed to create instance", vkGetVulkanResultString(result));
            return false;
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
            return false;
        }

        _physicalDevices.resize(gpuCount);
        std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
        vkEnumeratePhysicalDevices(_instance, &gpuCount, physicalDevices.data());
        uint32_t bestDeviceScore = 0U;
        uint32_t bestDeviceIndex = ~0u;
        const bool preferDiscrete = true;
        for (uint32_t i = 0; i < gpuCount; ++i)
        {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(physicalDevices[i], &properties);
            uint32_t score = 0U;
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
            }
        }
    }
}
