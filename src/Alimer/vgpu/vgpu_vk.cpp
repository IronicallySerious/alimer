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

#include "vgpu.h"

#if defined(VGPU_VK) || defined(ALIMER_VULKAN)
#if defined(_WIN32)
#    ifndef VK_USE_PLATFORM_WIN32_KHR
#        define VK_USE_PLATFORM_WIN32_KHR 1
#    endif
#elif defined(__ANDROID__)
#    define VK_USE_PLATFORM_ANDROID_KHR 1
#elif defined(__linux__)
#    ifdef ALIMER_LINUX_WAYLAND)
#        define VK_USE_PLATFORM_WAYLAND_KHR 1
#    else
#        define VK_USE_PLATFORM_XCB_KHR 1
#    endif
#endif
#include "volk/volk.h"
#include <vk_mem_alloc.h>
//#include <spirv-cross/spirv_hlsl.hpp>
#include <vector>

#ifndef VGPU_DEBUG && !defined(NDEBUG)
#   define VGPU_DEBUG 1
#endif

/* Handle declaration */
typedef struct VGpuTexture_T {
    VkImage             image;
    VkFormat            format;
} VGpuTexture_T;

/* Conversion functions */
static VgpuResult vgpuVkConvertResult(VkResult value) 
{
    switch (value)
    {
    case VK_SUCCESS:                        return VGPU_SUCCESS;
    case VK_NOT_READY:                      return VGPU_NOT_READY;
    case VK_TIMEOUT:                        return VGPU_TIMEOUT;
    case VK_INCOMPLETE:                     return VGPU_INCOMPLETE;
    case VK_ERROR_OUT_OF_HOST_MEMORY:       return VGPU_ERROR_OUT_OF_HOST_MEMORY;
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:     return VGPU_ERROR_OUT_OF_DEVICE_MEMORY;
    case VK_ERROR_INITIALIZATION_FAILED:    return VGPU_ERROR_INITIALIZATION_FAILED;
    case VK_ERROR_DEVICE_LOST:              return VGPU_ERROR_DEVICE_LOST;
    default:
        if (value < VK_SUCCESS) {
            return VGPU_ERROR_GENERIC;
        }

        return VGPU_SUCCESS;
    }
}

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
            //ALIMER_LOGERROR("[Vulkan]: Validation Error: {}", pCallbackData->pMessage);
            //device->NotifyValidationError(pCallbackData->pMessage);
        }
        else
        {
            //ALIMER_LOGERROR("[Vulkan]: Other Error: {}", pCallbackData->pMessage);
        }

        break;

    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
        {
            //ALIMER_LOGWARN("[Vulkan]: Validation Warning:{}", pCallbackData->pMessage);
        }
        else
        {
            //ALIMER_LOGWARN("[Vulkan]: Other Warning: {}", pCallbackData->pMessage);
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

    /*bool log_object_names = false;
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
    }*/

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
        //ALIMER_LOGERROR("[Vulkan]: %s: %s", pLayerPrefix, pMessage);
        //device->NotifyValidationError(pMessage);
    }
    else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
    {
        //ALIMER_LOGWARN("[Vulkan]: %s: %s", pLayerPrefix, pMessage);
    }
    else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
    {
        //ALIMER_LOGWARN("[Vulkan]: Performance warning: %s: %s", pLayerPrefix, pMessage);
    }
    else
    {
        //ALIMER_LOGINFO("[Vulkan]: {}: {}", pLayerPrefix, pMessage);
    }

    return VK_FALSE;
}

struct VGpuRendererVk
{
    VGpuRendererVk();
    ~VGpuRendererVk();
    VgpuResult initialize(const char* applicationName, const VGpuDescriptor* descriptor);
    VgpuResult waitIdle();

    /* Extensions */
    bool                            KHR_get_physical_device_properties2;    /* VK_KHR_get_physical_device_properties2 */
    bool                            KHR_external_memory_capabilities;       /* VK_KHR_external_memory_capabilities */
    bool                            KHR_external_semaphore_capabilities;    /* VK_KHR_external_semaphore_capabilities */

    bool                            EXT_debug_report;       /* VK_EXT_debug_report */
    bool                            EXT_debug_utils;        /* VK_EXT_debug_utils */

    bool                            KHR_surface;            /* VK_KHR_surface */
#if defined(_WIN32) || defined(_WIN64)
    bool                            KHR_win32_surface;      /* VK_KHR_win32_surface*/
#elif defined(__ANDROID__)
    bool                            KHR_android_surface;
#elif defined(__APPLE__)
    bool                            MVK_macos_surface;    /* VK_MVK_macos_surface */
#elif defined(__linux__)
    bool                            KHR_xlib_surface;
    bool                            KHR_xcb_surface;
    bool                            KHR_wayland_surface;
#endif

    /* Layers */
    bool                            VK_LAYER_LUNARG_standard_validation;
    bool                            VK_LAYER_RENDERDOC_Capture;

    VkInstance                              instance = VK_NULL_HANDLE;
    VkDebugReportCallbackEXT                debugCallback = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT                debugMessenger = VK_NULL_HANDLE;
    VkPhysicalDevice                        physicalDevice = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties              physicalDeviceProperties;
    VkPhysicalDeviceMemoryProperties        physicalDeviceMemoryProperties;
    VkPhysicalDeviceFeatures                physicalDeviceFeatures;
    std::vector<VkQueueFamilyProperties>    physicalDeviceQueueFamilyProperties;
    std::vector<std::string>                physicalDeviceExtensions;
    uint32_t                                graphicsQueueFamily = VK_QUEUE_FAMILY_IGNORED;
    uint32_t                                computeQueueFamily = VK_QUEUE_FAMILY_IGNORED;
    uint32_t                                transferQueueFamily = VK_QUEUE_FAMILY_IGNORED;
    VkDevice                                device = VK_NULL_HANDLE;
    VkQueue                                 graphicsQueue = VK_NULL_HANDLE;
    VkQueue                                 computeQueue = VK_NULL_HANDLE;
    VkQueue                                 transferQueue = VK_NULL_HANDLE;
    VmaAllocator                            memoryAllocator = VK_NULL_HANDLE;
};

VGpuRendererVk::VGpuRendererVk()
{

}
VGpuRendererVk::~VGpuRendererVk()
{

}

VgpuResult VGpuRendererVk::initialize(const char* applicationName, const VGpuDescriptor* descriptor)
{
    VkResult result;
    uint32_t i, count, layerCount;
    VkExtensionProperties* queriedExtensions;
    VkLayerProperties* queriedLayers;

    result = volkInitialize();
    if (result != VK_SUCCESS)
    {
        return vgpuVkConvertResult(result);
    }

    result = vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    if (result < VK_SUCCESS)
    {
        return vgpuVkConvertResult(result);
    }

    queriedExtensions = (VkExtensionProperties*)calloc(count, sizeof(VkExtensionProperties));
    result = vkEnumerateInstanceExtensionProperties(nullptr, &count, queriedExtensions);
    if (result < VK_SUCCESS)
    {
        free(queriedExtensions);
        return vgpuVkConvertResult(result);
    }

    result = vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    if (result < VK_SUCCESS)
    {
        return vgpuVkConvertResult(result);
    }

    queriedLayers = (VkLayerProperties*)calloc(layerCount, sizeof(VkLayerProperties));
    result = vkEnumerateInstanceLayerProperties(&layerCount, queriedLayers);
    if (result < VK_SUCCESS)
    {
        free(queriedLayers);
        return vgpuVkConvertResult(result);
    }

    // Initialize extensions.
    for (i = 0; i < count; i++)
    {
        if (strcmp(queriedExtensions[i].extensionName, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) == 0)
        {
            KHR_get_physical_device_properties2 = true;
        }
        else if (strcmp(queriedExtensions[i].extensionName, VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME) == 0)
        {
            KHR_external_memory_capabilities = true;
        }
        else if (strcmp(queriedExtensions[i].extensionName, VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME) == 0)
        {
            KHR_external_semaphore_capabilities = true;
        }
        else if (strcmp(queriedExtensions[i].extensionName, VK_EXT_DEBUG_REPORT_EXTENSION_NAME) == 0)
        {
            EXT_debug_report = true;
        }
        else if (strcmp(queriedExtensions[i].extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
        {
            EXT_debug_utils = true;
        }
        else if (strcmp(queriedExtensions[i].extensionName, VK_KHR_SURFACE_EXTENSION_NAME) == 0)
        {
            KHR_surface = true;
        }
#if defined(_WIN32) || defined(_WIN64)
        else if (strcmp(queriedExtensions[i].extensionName, "VK_KHR_win32_surface") == 0)
        {
            KHR_win32_surface = true;
        }
#elif defined(__ANDROID__)
        else if (strcmp(queriedExtensions[i].extensionName, "VK_KHR_android_surface") == 0)
        {
            KHR_android_surface = true;
        }
#elif defined(__APPLE__)
        else if (strcmp(queriedExtensions[i].extensionName, "VK_MVK_macos_surface") == 0)
        {
            MVK_macos_surface = true;
        }
#elif defined(__linux__)
        else if (strcmp(queriedExtensions[i].extensionName, "VK_KHR_xlib_surface") == 0)
        {
            KHR_xlib_surface = true;
        }
        else if (strcmp(queriedExtensions[i].extensionName, "VK_KHR_xcb_surface") == 0)
        {
            KHR_xcb_surface = true;
        }
        else if (strcmp(queriedExtensions[i].extensionName, "VK_KHR_wayland_surface") == 0)
        {
            KHR_wayland_surface = true;
        }
#endif
    }

    // Initialize layers.
    for (i = 0; i < layerCount; i++)
    {
        if (strcmp(queriedLayers[i].layerName, "VK_LAYER_LUNARG_standard_validation") == 0)
        {
            VK_LAYER_LUNARG_standard_validation = true;
        }
        else if (strcmp(queriedLayers[i].layerName, "VK_LAYER_RENDERDOC_Capture") == 0)
        {
            VK_LAYER_RENDERDOC_Capture = true;
        }
    }

    VkApplicationInfo appInfo;
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = nullptr;
    appInfo.pApplicationName = applicationName;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
    appInfo.pEngineName = "vgpu";
    appInfo.engineVersion = VK_MAKE_VERSION(VGPU_VERSION_MAJOR, VGPU_VERSION_MINOR, 0),
    appInfo.apiVersion = volkGetInstanceVersion();

    if (appInfo.apiVersion >= VK_API_VERSION_1_1)
    {
        appInfo.apiVersion = VK_API_VERSION_1_1;
    }

    std::vector<const char*> instanceLayers;
    std::vector<const char*> instanceExtensions;

    if (KHR_get_physical_device_properties2)
    {
        instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    if (KHR_get_physical_device_properties2
        && KHR_external_memory_capabilities
        && KHR_external_semaphore_capabilities)
    {
        instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        instanceExtensions.push_back(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME);
        instanceExtensions.push_back(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME);
    }

    if (EXT_debug_utils)
    {
        instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

#ifdef VGPU_DEBUG
    if (descriptor->validation)
    {
        if (!EXT_debug_utils && EXT_debug_report)
        {
            instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        }

        bool force_no_validation = false;
        /*if (getenv("VGPU_VULKAN_NO_VALIDATION"))
        {
            force_no_validation = true;
        }*/

        if (!force_no_validation && VK_LAYER_LUNARG_standard_validation)
        {
            instanceLayers.push_back("VK_LAYER_LUNARG_standard_validation");
        }
    }
#endif

    if (descriptor->swapchain != nullptr
        && KHR_surface)
    {
        instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

#if defined(_WIN32) || defined(_WIN64)
        if (KHR_win32_surface)
        {
            instanceExtensions.push_back("VK_KHR_win32_surface");
        }
#elif defined(__ANDROID__)
        if (KHR_android_surface)
        {
            instanceExtensions.push_back("VK_KHR_android_surface")
        }
#elif defined(__APPLE__)
        if (MVK_macos_surface)
        {
            instanceExtensions.push_back("VK_MVK_macos_surface");
        }
#elif defined(__linux__)
        if (strcmp(KHR_xlib_surface)
        {
            instanceExtensions.push_back("VK_KHR_xlib_surface");
        }
        else if (KHR_xcb_surface)
        {
            instanceExtensions.push_back("VK_KHR_xcb_surface");
        }
        else if (KHR_wayland_surface)
        {
            instanceExtensions.push_back("VK_KHR_wayland_surface");
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

    result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
    if (result != VK_SUCCESS)
    {
        return vgpuVkConvertResult(result);
    }

    volkLoadInstance(instance);

    // Free allocated stuff
    free(queriedExtensions);
    free(queriedLayers);

    if (descriptor->validation)
    {
        if (EXT_debug_utils)
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
            vkCreateDebugUtilsMessengerEXT(instance, &info, nullptr, &debugMessenger);
        }
        else if (EXT_debug_report)
        {
            VkDebugReportCallbackCreateInfoEXT info = { VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT };
            info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
                VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
            info.pfnCallback = vgpuVkDebugCallback;
            info.pUserData = this;
            vkCreateDebugReportCallbackEXT(instance, &info, nullptr, &debugCallback);
        }
    }

    uint32_t gpuCount = 0;
    vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr);
    if (gpuCount == 0)
    {
        //ALIMER_LOGCRITICAL("Vulkan: No physical device detected");
        return VGPU_ERROR_INITIALIZATION_FAILED;
    }

    std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
    vkEnumeratePhysicalDevices(instance, &gpuCount, physicalDevices.data());
    uint32_t bestDeviceScore = 0U;
    uint32_t bestDeviceIndex = ~0u;
    for (uint32_t i = 0; i < gpuCount; ++i)
    {
        uint32_t score = 0U;
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physicalDevices[i], &properties);
        switch (properties.deviceType)
        {
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            score += 100U;
            if (descriptor->devicePreference == VGPU_DEVICE_PREFERENCE_DISCRETE) {
                score += 1000U;
            }
            break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            score += 90U;
            if (descriptor->devicePreference == VGPU_DEVICE_PREFERENCE_INTEGRATED) {
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
        //ALIMER_LOGCRITICAL("Vulkan: No physical device supported.");
        return VGPU_ERROR_INITIALIZATION_FAILED;
    }

    physicalDevice = physicalDevices[bestDeviceIndex];
    //InitializeFeatures();
    return VGPU_SUCCESS;
}

VgpuResult VGpuRendererVk::waitIdle()
{
    VkResult result = vkDeviceWaitIdle(device);
    if (result < VK_SUCCESS)
    {
        return vgpuVkConvertResult(result);
    }

    return VGPU_SUCCESS;
}

/* Implementation */
static VGpuRendererVk* s_renderer = nullptr;

VgpuResult vgpuInitialize(const char* applicationName, const VGpuDescriptor* descriptor)
{
    if (s_renderer != nullptr)
        return VGPU_ALREADY_INITIALIZED;

    s_renderer = new VGpuRendererVk();
    return s_renderer->initialize(applicationName, descriptor);
}

#endif /* defined(VGPU_VK) || defined(ALIMER_VULKAN) */
