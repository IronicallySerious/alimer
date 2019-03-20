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

#ifdef VULKAN_DEBUG
        if (debugCallback)
            vkDestroyDebugReportCallbackEXT(instance, debugCallback, nullptr);
        if (debugMessenger)
            vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        debugCallback = VK_NULL_HANDLE;
        debugMessenger = VK_NULL_HANDLE;
#endif

        if (ownedDevice && device != VK_NULL_HANDLE)
        {
            vkDestroyDevice(device, nullptr);
            device = VK_NULL_HANDLE;
        }

        if (ownedInstance && instance != VK_NULL_HANDLE)
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
            uint32_t score = 0U;
            switch (dev_props.deviceType) {
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                score += 100U;
                if (desc.powerPreference == PowerPreference::HighPerformance) { score += 1000U; }
                break;
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                score += 90U;
                if (desc.powerPreference == PowerPreference::LowPower) { score += 1000U; }
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
        if (best_device_index == static_cast<uint32_t>(-1)) {
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

        for (unsigned i = 0; i < queue_count; i++)
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

        // Init info and caps.
        initializeCaps();

        return true;
    }

    static std::string getVendorByID(uint32_t id)
    {
        switch (id)
        {
        case 0x1002: return "Advanced Micro Devices, Inc.";
        case 0x10de: return "NVIDIA Corporation";
        case 0x102b: return "Matrox Electronic Systems Ltd.";
        case 0x1414: return "Microsoft Corporation";
        case 0x5333: return "S3 Graphics Co., Ltd.";
        case 0x8086: return "Intel Corporation";
        case 0x80ee: return "Oracle Corporation";
        case 0x15ad: return "VMware Inc.";
        }
        return "";
    }

    // see https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#fundamentals-versionnum
    std::string getVKApiVersionToString(std::uint32_t version)
    {
        std::string s;

        s += std::to_string(VK_VERSION_MAJOR(version));
        s += '.';
        s += std::to_string(VK_VERSION_MINOR(version));
        s += ".";
        s += std::to_string(VK_VERSION_PATCH(version));

        return s;
    }

    void GraphicsImpl::initializeCaps()
    {
        info.backend = GraphicsBackend::Vulkan;
        info.backendName = "Vulkan " + getVKApiVersionToString(deviceProperties.apiVersion);
        info.deviceName = deviceProperties.deviceName;
        info.vendorName = getVendorByID(deviceProperties.vendorID);
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

    void GraphicsImpl::notifyValidationError(const char* message)
    {
        ALIMER_UNUSED(message);
    }
}
