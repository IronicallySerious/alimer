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

#include "gpu.h"

#if defined(VGPU_VK)
#include <stdio.h>
#include <stdint.h>
#include "volk.h"
/*#define VMA_IMPLEMENTATION 1
#define VMA_STATIC_VULKAN_FUNCTION 0
#define VMA_STATS_STRING_ENABLED 0
#include "vk_mem_alloc.h"
*/

#if defined(_MSC_VER)
#ifndef WIN32_LEAN_AND_MEAN
#   define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#define VGPU_LOGE(...) do { \
    fprintf(stderr, "[ERROR]: " __VA_ARGS__); \
    fflush(stderr); \
    char buffer[4096]; \
    sprintf(buffer, "[ERROR]: " __VA_ARGS__); \
    OutputDebugStringA(buffer); \
} while(false)

#define VGPU_LOGW(...) do { \
    fprintf(stderr, "[WARN]: " __VA_ARGS__); \
    fflush(stderr); \
    char buffer[4096]; \
    sprintf(buffer, "[WARN]: " __VA_ARGS__); \
    OutputDebugStringA(buffer); \
} while(false)

#define VGPU_LOGI(...) do { \
    fprintf(stderr, "[INFO]: " __VA_ARGS__); \
    fflush(stderr); \
    char buffer[4096]; \
    sprintf(buffer, "[INFO]: " __VA_ARGS__); \
    OutputDebugStringA(buffer); \
} while(false)
#elif defined(ANDROID)
#   include <android/log.h>
#   define VGPU_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "vgpu", __VA_ARGS__)
#   define VGPU_LOGW(...) __android_log_print(ANDROID_LOG_WARN, "vgpu", __VA_ARGS__)
#   define VGPU_LOGI(...) __android_log_print(ANDROID_LOG_INFO, "vgpu", __VA_ARGS__)
#else
#   define VGPU_LOGE(...)                     \
    do                                \
    {                                 \
        fprintf(stderr, "[ERROR]: " __VA_ARGS__); \
        fflush(stderr); \
    } while (false)

#define VGPU_LOGW(...)                     \
    do                                \
    {                                 \
        fprintf(stderr, "[WARN]: " __VA_ARGS__); \
        fflush(stderr); \
    } while (false)

#define VGPU_LOGI(...)                     \
    do                                \
    {                                 \
        fprintf(stderr, "[INFO]: " __VA_ARGS__); \
        fflush(stderr); \
    } while (false)
#endif

#include <vector>
#include <algorithm>

#ifndef VULKAN_DEBUG
#   ifdef _DEBUG
#       define VULKAN_DEBUG 1
#   else
#       define VULKAN_DEBUG 0
#   endif
#endif

/* Handle declaration */
typedef struct vgpu_swapchain_T {
    VkSwapchainKHR          vk_handle;
    uint32_t                width;
    uint32_t                height;
    VkFormat                format;
    std::vector<VkImage>    images;
    uint32_t                image_index; 
} vgpu_swapchain_T;

struct {
    bool                        headless = false;
    bool                        supports_physical_device_properties2 = false;   /* VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME */
    bool                        supports_debug_utils = false; /* VK_EXT_DEBUG_UTILS_EXTENSION_NAME */
    VkInstance                  instance = VK_NULL_HANDLE;
#ifdef VULKAN_DEBUG
    VkDebugReportCallbackEXT    debug_callback = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT    debug_messenger = VK_NULL_HANDLE;
#endif
    VkSurfaceKHR                surface = VK_NULL_HANDLE;

    VkPhysicalDevice            physical_device;
    uint32_t                    graphics_queue_family = VK_QUEUE_FAMILY_IGNORED;
    uint32_t                    compute_queue_family = VK_QUEUE_FAMILY_IGNORED;
    uint32_t                    transfer_queue_family = VK_QUEUE_FAMILY_IGNORED;
    VkDevice                    device = VK_NULL_HANDLE;
    VkQueue                     graphics_queue = VK_NULL_HANDLE;
    VkQueue                     compute_queue = VK_NULL_HANDLE;
    VkQueue                     transfer_queue = VK_NULL_HANDLE;
    vgpu_swapchain_T*           swapchain = nullptr;
} _vk;

#if VULKAN_DEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_messenger_cb(
    VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT                  messageType,
    const VkDebugUtilsMessengerCallbackDataEXT*      pCallbackData,
    void *pUserData)
{
    switch (messageSeverity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
        {
            VGPU_LOGE("[Vulkan]: Validation Error: %s\n", pCallbackData->pMessage);
            //vgpu_notify_validation_error(pCallbackData->pMessage);
        }
        else {
            VGPU_LOGE("[Vulkan]: Other Error: %s\n", pCallbackData->pMessage);
        }

        break;

    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
            VGPU_LOGW("[Vulkan]: Validation Warning: %s\n", pCallbackData->pMessage);
        else
            VGPU_LOGW("[Vulkan]: Other Warning: %s\n", pCallbackData->pMessage);
        break;

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
            VGPU_LOGI("  Object #%u: %s\n", i, name ? name : "N/A");
        }
    }

    return VK_FALSE;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_cb(VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT, uint64_t,
    size_t, int32_t messageCode, const char *pLayerPrefix,
    const char *pMessage, void *pUserData)
{
    // False positives about lack of srcAccessMask/dstAccessMask.
    if (strcmp(pLayerPrefix, "DS") == 0 && messageCode == 10)
        return VK_FALSE;

    // False positive almost all the time and threat as warning.
    if (strcmp(pLayerPrefix, "DS") == 0 && messageCode == 6)
        flags = VK_DEBUG_REPORT_DEBUG_BIT_EXT;

    if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
    {
        VGPU_LOGE("[Vulkan]: Error: %s: %s\n", pLayerPrefix, pMessage);
        //vgpu_notify_validation_error(pMessage);
    }
    else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
    {
        VGPU_LOGW("[Vulkan]: Warning: %s: %s\n", pLayerPrefix, pMessage);
    }
    else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
    {
        VGPU_LOGW("[Vulkan]: Performance warning: %s: %s\n", pLayerPrefix, pMessage);
    }
    else
    {
        VGPU_LOGI("[Vulkan]: Information: %s: %s\n", pLayerPrefix, pMessage);
    }

    return VK_FALSE;
}
#endif

using namespace std;

vgpu_result _vgpu_vk_create_swapchain(
    uint32_t width, uint32_t height,
    VkSurfaceKHR surface,
    vgpu_swapchain_T* swapchain,
    const vgpu_swapchain_descriptor* descriptor);

vgpu_result vgpu_initialize(const char* app_name, const vgpu_renderer_settings* settings)
{
    if (_vk.instance != VK_NULL_HANDLE) {
        return VGPU_ALREADY_INITIALIZED;
    }

    VkResult result = volkInitialize();
    if (result != VK_SUCCESS)
    {
        return VGPU_ERROR_INITIALIZATION_FAILED;
    }

    uint32_t ext_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, nullptr);
    vector<VkExtensionProperties> queried_extensions(ext_count);
    if (ext_count)
        vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, queried_extensions.data());

    uint32_t layer_count = 0;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
    std::vector<VkLayerProperties> queried_layers(layer_count);
    if (layer_count)
        vkEnumerateInstanceLayerProperties(&layer_count, queried_layers.data());

    const auto has_extension = [&](const char *name) -> bool {
        auto itr = find_if(begin(queried_extensions), end(queried_extensions), [name](const VkExtensionProperties &e) -> bool {
            return strcmp(e.extensionName, name) == 0;
            });
        return itr != end(queried_extensions);
    };

    VkApplicationInfo appInfo;
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = nullptr;
    appInfo.pApplicationName = app_name;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "vortice";
    appInfo.engineVersion = VK_MAKE_VERSION(VGPU_VERSION_MAJOR, VGPU_VERSION_MINOR, VGPU_VERSION_PATCH);

    if (volkGetInstanceVersion() >= VK_API_VERSION_1_1)
    {
        appInfo.apiVersion = VK_API_VERSION_1_1;
    }
    else
    {
        appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 57);
    }

#if defined(_WIN32)
    _vk.headless =
        settings->width == 0
        || settings->height == 0
        || !IsWindow(settings->handle.hwnd);
#else
    _vk.headless = false;
#endif

    vector<const char*> enabled_instance_extensions;
    vector<const char*> enabled_instance_layers;
    if (!_vk.headless)
    {
        enabled_instance_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

        // Enable surface extensions depending on os
#if defined(VK_USE_PLATFORM_WIN32_KHR)
        enabled_instance_extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
        enabled_instance_extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
        enabled_instance_extensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
        enabled_instance_extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_IOS_MVK)
        enabled_instance_extensions.push_back(VK_MVK_IOS_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
        enabled_instance_extensions.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#endif
    }

    if (has_extension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
    {
        _vk.supports_physical_device_properties2 = true;
        enabled_instance_extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    if (has_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
    {
        _vk.supports_debug_utils = true;
        enabled_instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }


#if VULKAN_DEBUG
    if (settings->validation)
    {
        enabled_instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

        const auto has_layer = [&](const char *name) -> bool {
            auto itr = find_if(begin(queried_layers), end(queried_layers), [name](const VkLayerProperties &e) -> bool {
                return strcmp(e.layerName, name) == 0;
                });
            return itr != end(queried_layers);
        };

        if (!_vk.supports_debug_utils
            && has_extension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
        {
            enabled_instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        }

        bool force_no_validation = false;
        if (getenv("VGPU_NO_VALIDATION")) {
            force_no_validation = true;
        }

        if (!force_no_validation && has_layer("VK_LAYER_LUNARG_standard_validation")) {
            enabled_instance_layers.push_back("VK_LAYER_LUNARG_standard_validation");
        }
    }
#endif

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = NULL;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledExtensionCount = (uint32_t)enabled_instance_extensions.size();
    instanceCreateInfo.ppEnabledExtensionNames = enabled_instance_extensions.data();
    instanceCreateInfo.enabledLayerCount = (uint32_t)enabled_instance_layers.size();
    instanceCreateInfo.ppEnabledLayerNames = enabled_instance_layers.data();

    result = vkCreateInstance(&instanceCreateInfo, NULL, &_vk.instance);
    if (result != VK_SUCCESS) {
        return VGPU_ERROR_INITIALIZATION_FAILED;
    }

    volkLoadInstance(_vk.instance);

#if VULKAN_DEBUG
    if (_vk.supports_debug_utils)
    {
        VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo = {};
        debugUtilsMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugUtilsMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        debugUtilsMessengerCreateInfo.pfnUserCallback = vulkan_messenger_cb;
        debugUtilsMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;

        vkCreateDebugUtilsMessengerEXT(_vk.instance, &debugUtilsMessengerCreateInfo, nullptr, &_vk.debug_messenger);
    }
    else if (has_extension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
    {
        VkDebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo = {};
        debugReportCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
        debugReportCallbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT
            | VK_DEBUG_REPORT_WARNING_BIT_EXT
            | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
        debugReportCallbackCreateInfo.pfnCallback = vulkan_debug_cb;
        vkCreateDebugReportCallbackEXT(_vk.instance, &debugReportCallbackCreateInfo, nullptr, &_vk.debug_callback);
    }
#endif

    // Create platform surface.
    if (!_vk.headless)
    {
#if defined(__linux__)
        VkXcbSurfaceCreateInfoKHR surfaceCreateInfo = {};
        surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
        surfaceCreateInfo.connection = settings->handle.connection;
        surfaceCreateInfo.window = settings->handle.window;
        result = vkCreateXcbSurfaceKHR(_vk.instance, &surfaceCreateInfo, NULL, &_vk.surface);
        assert(VK_SUCCESS == vk_res);
#elif defined(_WIN32)
        VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
        surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        surfaceCreateInfo.hinstance = settings->handle.hinstance;
        surfaceCreateInfo.hwnd = settings->handle.hwnd;
        result = vkCreateWin32SurfaceKHR(_vk.instance, &surfaceCreateInfo, NULL, &_vk.surface);
#endif
        if (result != VK_SUCCESS)
        {
            return VGPU_ERROR_INITIALIZATION_FAILED;
        }
    }

    // Enumerate physical device and create logical device.
    uint32_t gpu_count = 0u;
    if (vkEnumeratePhysicalDevices(_vk.instance, &gpu_count, nullptr) != VK_SUCCESS) {
        return VGPU_ERROR_INITIALIZATION_FAILED;
    }

    vector<VkPhysicalDevice> gpus(gpu_count);
    if (vkEnumeratePhysicalDevices(_vk.instance, &gpu_count, gpus.data()) != VK_SUCCESS) {
        return VGPU_ERROR_INITIALIZATION_FAILED;
    }

    // Pick a suitable physical device based on user's preference.
    uint32_t best_device_score = 0u;
    uint32_t best_device_index = (uint32_t)-1;
    for (uint32_t i = 0; i < gpu_count; ++i) {
        VkPhysicalDeviceProperties device_props;
        vkGetPhysicalDeviceProperties(gpus[i], &device_props);
        uint32_t score = 0u;
        switch (device_props.deviceType) {
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            score += 100U;
            if (settings->device_preference == VGPU_DEVICE_PREFERENCE_HIGH_PERFORMANCE) {
                score += 1000u;
            }
            break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            score += 90U;
            if (settings->device_preference == VGPU_DEVICE_PREFERENCE_LOW_POWER) {
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
        if (score > best_device_score) {
            best_device_index = i;
            best_device_score = score;
        }
    }
    if (best_device_index == (uint32_t)-1) {
        return VGPU_ERROR_INITIALIZATION_FAILED;
    }

    _vk.physical_device = gpus[best_device_index];
    VkPhysicalDeviceProperties device_props;
    vkGetPhysicalDeviceProperties(_vk.physical_device, &device_props);

    VGPU_LOGI("Using Vulkan GPU: %s\n", device_props.deviceName);
    VGPU_LOGI("    API: %u.%u.%u\n",
        VK_VERSION_MAJOR(device_props.apiVersion),
        VK_VERSION_MINOR(device_props.apiVersion),
        VK_VERSION_PATCH(device_props.apiVersion));
    VGPU_LOGI("    Driver: %u.%u.%u\n",
        VK_VERSION_MAJOR(device_props.driverVersion),
        VK_VERSION_MINOR(device_props.driverVersion),
        VK_VERSION_PATCH(device_props.driverVersion));


    uint32_t queue_count;
    vkGetPhysicalDeviceQueueFamilyProperties(_vk.physical_device, &queue_count, nullptr);
    vector<VkQueueFamilyProperties> queue_props(queue_count);
    vkGetPhysicalDeviceQueueFamilyProperties(_vk.physical_device, &queue_count, queue_props.data());

    for (uint32_t i = 0; i < queue_count; i++)
    {
        VkBool32 supported = _vk.surface == VK_NULL_HANDLE;
        if (_vk.surface != VK_NULL_HANDLE) {
            vkGetPhysicalDeviceSurfaceSupportKHR(_vk.physical_device, i, _vk.surface, &supported);
        }

        static const VkQueueFlags required = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT;
        if (supported && ((queue_props[i].queueFlags & required) == required))
        {
            _vk.graphics_queue_family = i;
            break;
        }
    }

    for (uint32_t i = 0; i < queue_count; i++)
    {
        static const VkQueueFlags required = VK_QUEUE_COMPUTE_BIT;
        if (i != _vk.graphics_queue_family && (queue_props[i].queueFlags & required) == required)
        {
            _vk.compute_queue_family = i;
            break;
        }
    }

    for (uint32_t i = 0; i < queue_count; i++)
    {
        static const VkQueueFlags required = VK_QUEUE_TRANSFER_BIT;
        if (i != _vk.graphics_queue_family && i != _vk.compute_queue_family && (queue_props[i].queueFlags & required) == required)
        {
            _vk.transfer_queue_family = i;
            break;
        }
    }

    if (_vk.transfer_queue_family == VK_QUEUE_FAMILY_IGNORED)
    {
        for (uint32_t i = 0; i < queue_count; i++)
        {
            static const VkQueueFlags required = VK_QUEUE_TRANSFER_BIT;
            if (i != _vk.graphics_queue_family && (queue_props[i].queueFlags & required) == required)
            {
                _vk.transfer_queue_family = i;
                break;
            }
        }
    }

    if (_vk.graphics_queue_family == VK_QUEUE_FAMILY_IGNORED)
    {
        return VGPU_ERROR_INITIALIZATION_FAILED;
    }

    uint32_t universal_queue_index = 1;
    uint32_t graphics_queue_index = 0;
    uint32_t compute_queue_index = 0;
    uint32_t transfer_queue_index = 0;

    if (_vk.compute_queue_family == VK_QUEUE_FAMILY_IGNORED)
    {
        _vk.compute_queue_family = _vk.graphics_queue_family;
        compute_queue_index = std::min(queue_props[_vk.graphics_queue_family].queueCount - 1, universal_queue_index);
        universal_queue_index++;
    }

    if (_vk.transfer_queue_family == VK_QUEUE_FAMILY_IGNORED)
    {
        _vk.transfer_queue_family = _vk.graphics_queue_family;
        transfer_queue_index = std::min(queue_props[_vk.graphics_queue_family].queueCount - 1, universal_queue_index);
        universal_queue_index++;
    }
    else if (_vk.transfer_queue_family == _vk.compute_queue_family)
    {
        transfer_queue_index = std::min(queue_props[_vk.compute_queue_family].queueCount - 1, 1u);
    }

    static const float graphics_queue_prio = 0.5f;
    static const float compute_queue_prio = 1.0f;
    static const float transfer_queue_prio = 1.0f;
    float prio[3] = { graphics_queue_prio, compute_queue_prio, transfer_queue_prio };

    unsigned queue_family_count = 0;
    VkDeviceQueueCreateInfo queue_create_info[3] = {};

    queue_create_info[queue_family_count].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info[queue_family_count].queueFamilyIndex = _vk.graphics_queue_family;
    queue_create_info[queue_family_count].queueCount = std::min(universal_queue_index,
        queue_props[_vk.graphics_queue_family].queueCount);
    queue_create_info[queue_family_count].pQueuePriorities = prio;
    queue_family_count++;

    if (_vk.compute_queue_family != _vk.graphics_queue_family)
    {
        queue_create_info[queue_family_count].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info[queue_family_count].queueFamilyIndex = _vk.compute_queue_family;
        queue_create_info[queue_family_count].queueCount = std::min(_vk.transfer_queue_family == _vk.compute_queue_family ? 2u : 1u,
            queue_props[_vk.compute_queue_family].queueCount);
        queue_create_info[queue_family_count].pQueuePriorities = prio + 1;
        queue_family_count++;
    }

    if (_vk.transfer_queue_family != _vk.graphics_queue_family && _vk.transfer_queue_family != _vk.compute_queue_family)
    {
        queue_create_info[queue_family_count].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info[queue_family_count].queueFamilyIndex = _vk.transfer_queue_family;
        queue_create_info[queue_family_count].queueCount = 1;
        queue_create_info[queue_family_count].pQueuePriorities = prio + 2;
        queue_family_count++;
    }

    // Create the logical device representation
    vector<const char*> enabled_device_extensions;
    if (!_vk.headless)
    {
        enabled_device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }

    VkPhysicalDeviceFeatures2KHR features = {};
    features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR;

    if (_vk.supports_physical_device_properties2)
        vkGetPhysicalDeviceFeatures2KHR(_vk.physical_device, &features);
    else
        vkGetPhysicalDeviceFeatures(_vk.physical_device, &features.features);

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

        if (features.features.shaderSampledImageArrayDynamicIndexing)
            enabled_features.shaderSampledImageArrayDynamicIndexing = VK_TRUE;
        if (features.features.shaderUniformBufferArrayDynamicIndexing)
            enabled_features.shaderUniformBufferArrayDynamicIndexing = VK_TRUE;
        if (features.features.shaderStorageBufferArrayDynamicIndexing)
            enabled_features.shaderStorageBufferArrayDynamicIndexing = VK_TRUE;
        if (features.features.shaderStorageImageArrayDynamicIndexing)
            enabled_features.shaderStorageImageArrayDynamicIndexing = VK_TRUE;

        features.features = enabled_features;
    }

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = queue_family_count;
    deviceCreateInfo.pQueueCreateInfos = queue_create_info;
    deviceCreateInfo.enabledLayerCount = 0;
    deviceCreateInfo.ppEnabledLayerNames = nullptr;
    deviceCreateInfo.enabledExtensionCount = (uint32_t)enabled_device_extensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = enabled_device_extensions.data();

    if (_vk.supports_physical_device_properties2)
        deviceCreateInfo.pNext = &features;
    else
        deviceCreateInfo.pEnabledFeatures = &features.features;

    result = vkCreateDevice(_vk.physical_device, &deviceCreateInfo, nullptr, &_vk.device);
    if (result != VK_SUCCESS)
    {
        return VGPU_ERROR_INITIALIZATION_FAILED;
    }

    volkLoadDevice(_vk.device);
    vkGetDeviceQueue(_vk.device, _vk.graphics_queue_family, graphics_queue_index, &_vk.graphics_queue);
    vkGetDeviceQueue(_vk.device, _vk.compute_queue_family, compute_queue_index, &_vk.compute_queue);
    vkGetDeviceQueue(_vk.device, _vk.transfer_queue_family, transfer_queue_index, &_vk.transfer_queue);

    return _vgpu_vk_create_swapchain(settings->width, settings->height, _vk.surface, _vk.swapchain, &settings->swapchain);
}

vgpu_result _vgpu_vk_create_swapchain(
    uint32_t width, uint32_t height,
    VkSurfaceKHR surface,
    vgpu_swapchain_T* swapchain,
    const vgpu_swapchain_descriptor* descriptor)
{
    VkSurfaceCapabilitiesKHR surface_properties;
    if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_vk.physical_device, surface, &surface_properties) != VK_SUCCESS)
    {
        return VGPU_ERROR_INITIALIZATION_FAILED;
    }

    // Happens on nVidia Windows when you minimize a window.
    if (surface_properties.maxImageExtent.width == 0
        && surface_properties.maxImageExtent.height == 0)
    {
        return VGPU_ERROR_INITIALIZATION_FAILED;
    }

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(_vk.physical_device, surface, &format_count, nullptr);
    vector<VkSurfaceFormatKHR> formats(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(_vk.physical_device, surface, &format_count, formats.data());

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
            VGPU_LOGE("Surface has no formats.\n");
            return VGPU_ERROR_INITIALIZATION_FAILED;
        }

        bool found = false;
        for (uint32_t i = 0; i < format_count; i++)
        {
            if (descriptor->srgb)
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
    if (surface_properties.currentExtent.width == ~0u)
    {
        swapchain_size.width = width;
        swapchain_size.height = height;
    }
    else
    {
        swapchain_size.width = max(min(width, surface_properties.maxImageExtent.width), surface_properties.minImageExtent.width);
        swapchain_size.height = max(min(height, surface_properties.maxImageExtent.height), surface_properties.minImageExtent.height);
    }

    uint32_t num_present_modes;
    vkGetPhysicalDeviceSurfacePresentModesKHR(_vk.physical_device, surface, &num_present_modes, nullptr);
    vector<VkPresentModeKHR> present_modes(num_present_modes);
    vkGetPhysicalDeviceSurfacePresentModesKHR(_vk.physical_device, surface, &num_present_modes, present_modes.data());

    VkPresentModeKHR swapchain_present_mode = VK_PRESENT_MODE_FIFO_KHR;
    if (!descriptor->vsync)
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

    uint32_t desired_swapchain_images = descriptor->image_count;
    if (desired_swapchain_images == 0)
    {
        desired_swapchain_images = 3;
    }

    VGPU_LOGI("Targeting %u swapchain images.\n", desired_swapchain_images);

    if (desired_swapchain_images < surface_properties.minImageCount)
        desired_swapchain_images = surface_properties.minImageCount;

    VkSurfaceTransformFlagBitsKHR pre_transform;
    if (surface_properties.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
        pre_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    else
        pre_transform = surface_properties.currentTransform;

    VkCompositeAlphaFlagBitsKHR composite_mode = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    if (surface_properties.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR)
        composite_mode = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    if (surface_properties.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
        composite_mode = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    if (surface_properties.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR)
        composite_mode = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
    if (surface_properties.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR)
        composite_mode = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;

    VkSwapchainKHR old_swapchain = swapchain != nullptr ? swapchain->vk_handle : VK_NULL_HANDLE;

    VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.pNext = nullptr;
    swapchainCreateInfo.surface = surface;
    swapchainCreateInfo.minImageCount = desired_swapchain_images;
    swapchainCreateInfo.imageFormat = format.format;
    swapchainCreateInfo.imageColorSpace = format.colorSpace;
    swapchainCreateInfo.imageExtent.width = swapchain_size.width;
    swapchainCreateInfo.imageExtent.height = swapchain_size.height;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfo.preTransform = pre_transform;
    swapchainCreateInfo.compositeAlpha = composite_mode;
    swapchainCreateInfo.presentMode = swapchain_present_mode;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = old_swapchain;

    // Enable transfer source on swap chain images if supported
    if (surface_properties.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
        swapchainCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }

    // Enable transfer destination on swap chain images if supported
    if (surface_properties.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
        swapchainCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    swapchain = new vgpu_swapchain_T();
    VkResult result = vkCreateSwapchainKHR(_vk.device, &swapchainCreateInfo, nullptr, &swapchain->vk_handle);
    if (old_swapchain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(_vk.device, old_swapchain, nullptr);
    }

    swapchain->width = swapchain_size.width;
    swapchain->height = swapchain_size.height;
    swapchain->format = format.format;
    swapchain->image_index = 0;

    uint32_t image_count;
    if (vkGetSwapchainImagesKHR(_vk.device, swapchain->vk_handle, &image_count, nullptr) != VK_SUCCESS)
    {
        return VGPU_ERROR_INITIALIZATION_FAILED;
    }

    swapchain->images.resize(image_count);
    if (vkGetSwapchainImagesKHR(_vk.device, swapchain->vk_handle, &image_count, swapchain->images.data()) != VK_SUCCESS)
    {
        return VGPU_ERROR_INITIALIZATION_FAILED;
    }

    return VGPU_SUCCESS;
}

void vgpu_shutdown()
{
    if (_vk.instance == VK_NULL_HANDLE) {
        return;
    }

#if VULKAN_DEBUG
    if (_vk.debug_callback != VK_NULL_HANDLE) {
        vkDestroyDebugReportCallbackEXT(_vk.instance, _vk.debug_callback, nullptr);
        _vk.debug_callback = VK_NULL_HANDLE;
    }

    if (_vk.debug_messenger != VK_NULL_HANDLE) {
        vkDestroyDebugUtilsMessengerEXT(_vk.instance, _vk.debug_messenger, nullptr);
        _vk.debug_messenger = VK_NULL_HANDLE;
    }
#endif

    if (_vk.device != VK_NULL_HANDLE)
    {
        vkDestroyDevice(_vk.device, nullptr);
        _vk.device = VK_NULL_HANDLE;
    }

    if (_vk.instance != VK_NULL_HANDLE)
        vkDestroyInstance(_vk.instance, nullptr);

    _vk.instance = VK_NULL_HANDLE;
}

#endif
