//
// Copyright (c) 2018 Amer Koleci and contributors.
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

#include "../../Core/Log.h"
#include "../../Base/String.h"
#include "../../Application/Window.h"
#include "VulkanGraphicsDevice.h"
#include "VulkanSwapchain.h"
#include "VulkanCommandBuffer.h"
#include "VulkanRenderPass.h"
#include "VulkanTexture.h"
#include "VulkanBuffer.h"
#include "VulkanShader.h"
#include "VulkanPipelineLayout.h"
#include "VulkanConvert.h"
#include "../../Math/Math.h"

#include "AlimerVersion.h"
using namespace std;

namespace Alimer
{
    /*
    * A layer can expose extensions, keep track of those
    * extensions here.
    */
    struct LayerProperties {
        VkLayerProperties properties;
        std::vector<VkExtensionProperties> instanceExtensions;
    };

    static inline VkResult InitGlobalExtensionProperties(LayerProperties &layerProps)
    {
        VkExtensionProperties *instanceExtensions;
        uint32_t instanceExtensionCount;
        char *layerName = layerProps.properties.layerName;

        VkResult result;

        do {
            result = vkEnumerateInstanceExtensionProperties(layerName, &instanceExtensionCount, nullptr);
            if (result)
                return result;

            if (instanceExtensionCount == 0)
                return VK_SUCCESS;

            layerProps.instanceExtensions.resize(instanceExtensionCount);
            instanceExtensions = layerProps.instanceExtensions.data();
            result = vkEnumerateInstanceExtensionProperties(layerName, &instanceExtensionCount, instanceExtensions);
        } while (result == VK_INCOMPLETE);

        return result;
    }

    static VkResult InitGlobalLayerProperties(std::vector<LayerProperties>& instance_layer_properties)
    {
        uint32_t instance_layer_count;
        VkLayerProperties *vk_props = nullptr;
        VkResult res;

        /*
        * It's possible, though very rare, that the number of
        * instance layers could change. For example, installing something
        * could include new layers that the loader would pick up
        * between the initial query for the count and the
        * request for VkLayerProperties. The loader indicates that
        * by returning a VK_INCOMPLETE status and will update the
        * the count parameter.
        * The count parameter will be updated with the number of
        * entries loaded into the data pointer - in case the number
        * of layers went down or is smaller than the size given.
        */
        do {
            res = vkEnumerateInstanceLayerProperties(&instance_layer_count, NULL);
            if (res) return res;

            if (instance_layer_count == 0) {
                return VK_SUCCESS;
            }

            vk_props = (VkLayerProperties *)realloc(vk_props, instance_layer_count * sizeof(VkLayerProperties));

            res = vkEnumerateInstanceLayerProperties(&instance_layer_count, vk_props);
        } while (res == VK_INCOMPLETE);

        /*
        * Now gather the extension list for each instance layer.
        */
        for (uint32_t i = 0; i < instance_layer_count; i++) {
            LayerProperties layer_props;
            layer_props.properties = vk_props[i];
            res = InitGlobalExtensionProperties(layer_props);
            if (res) return res;
            instance_layer_properties.push_back(layer_props);
        }
        free(vk_props);

        return res;
    }

    bool CheckLayers(
        const std::vector<LayerProperties> &layerProps,
        const std::vector<const char *> &layerNames)
    {
        size_t checkCount = layerNames.size();
        size_t layerCount = layerProps.size();
        for (size_t i = 0; i < checkCount; i++)
        {
            bool found = false;
            for (size_t j = 0; j < layerCount; j++)
            {
                if (!strcmp(layerNames[i], layerProps[j].properties.layerName))
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                ALIMER_LOGDEBUG("[Vulkan] - Cannot find layer: %s", layerNames[i]);
                return false;
            }
        }

        return true;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL VkDebugCallback(
        VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
        uint64_t object, size_t location, int32_t messageCode,
        const char *pLayerPrefix, const char *pMessage, void *pUserData)
    {
        (void)objectType;
        (void)object;
        (void)location;
        (void)pUserData;

        if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
        {
            ALIMER_LOGERROR("[Vulkan] - [%s] Code %d : %s", pLayerPrefix, messageCode, pMessage);
        }
        else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
        {
            ALIMER_LOGWARN("[Vulkan] - [%s] Code %d : %s", pLayerPrefix, messageCode, pMessage);
        }
        else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
        {
            ALIMER_LOGERROR("[Vulkan] - PERFORMANCE WARNING: [%s] Code %d : %s", pLayerPrefix, messageCode, pMessage);
        }
        else if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
        {
            ALIMER_LOGINFO("[Vulkan] - [%s] Code %d : %s", pLayerPrefix, messageCode, pMessage);
        }
        else if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
        {
            ALIMER_LOGDEBUG("[%s] Code %d : %s", pLayerPrefix, messageCode, pMessage);
        }
        else
        {
            ALIMER_LOGINFO("%s: %s", pLayerPrefix, pMessage);
        }

        return VK_FALSE;
    }

    enum class VkExtensionType
    {
        Optional,
        Required,
        Desired
    };

    struct VkExtension {
        const char* name;
        VkExtensionType type;
        bool supported;
    };


    bool VulkanGraphics::IsSupported()
    {
        static bool availableCheck = false;
        static bool isAvailable = false;

        if (availableCheck)
            return isAvailable;

        availableCheck = true;
        VkResult vkRes = volkInitialize();
        if (vkRes != VK_SUCCESS)
        {
            isAvailable = false;
            ALIMER_LOGERROR("Failed to initialize Vulkan");
            return false;
        }

        isAvailable = true;
        return true;
    }

    VulkanGraphics::VulkanGraphics(const RenderingSettings& settings)
    {
        std::vector<LayerProperties> instanceLayerProperties;
        VkResult result = InitGlobalLayerProperties(instanceLayerProperties);

        uint32_t apiVersion = VK_API_VERSION_1_0;
        // Determine if the new instance version command is available
        if (vkEnumerateInstanceVersion != nullptr)
        {
            uint32_t checkApiVersion = 0;
            if (vkEnumerateInstanceVersion(&checkApiVersion) == VK_SUCCESS)
            {
                // Translate the version into major/minor for easier comparison
                ALIMER_LOGDEBUG("Loader/Runtime support detected for Vulkan %u.%u.%u",
                    VK_VERSION_MAJOR(checkApiVersion),
                    VK_VERSION_MINOR(checkApiVersion),
                    VK_VERSION_PATCH(checkApiVersion));

                apiVersion = checkApiVersion;
            }
        }

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Alimer";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Alimer Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(ALIMER_VERSION_MAJOR, ALIMER_VERSION_MINOR, ALIMER_VERSION_PATCH);
        appInfo.apiVersion = apiVersion;

        vector<const char*> instanceExtensionNames = { VK_KHR_SURFACE_EXTENSION_NAME };

        // Enable surface extensions depending on os.
#if ALIMER_PLATFORM_WINDOWS
        instanceExtensionNames.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif ALIMER_PLATFORM_LINUX
        instanceExtensionNames.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#elif ALIMER_PLATFORM_APPLE_OSX
        instanceExtensionNames.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#elif ALIMER_PLATFORM_APPLE_IOS || ALIMER_PLATFORM_APPLE_TV
        instanceExtensionNames.push_back(VK_MVK_IOS_SURFACE_EXTENSION_NAME);
#elif ALIMER_PLATFORM_ANDROID
        instanceExtensionNames.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#else
        instanceExtensionNames.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#endif

        vector<const char*> instanceLayerNames;
        bool hasValidationLayer = false;
        if (settings.validation)
        {
            instanceExtensionNames.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

            instanceLayerNames.push_back("VK_LAYER_LUNARG_standard_validation");
            if (!CheckLayers(instanceLayerProperties, instanceLayerNames))
            {
                // If standard validation is not present, search instead for the
                // individual layers that make it up, in the correct order.
                instanceLayerNames.clear();
                instanceLayerNames.push_back("VK_LAYER_GOOGLE_threading");
                instanceLayerNames.push_back("VK_LAYER_LUNARG_parameter_validation");
                instanceLayerNames.push_back("VK_LAYER_LUNARG_object_tracker");
                instanceLayerNames.push_back("VK_LAYER_LUNARG_core_validation");
                instanceLayerNames.push_back("VK_LAYER_GOOGLE_unique_objects");

                if (!CheckLayers(instanceLayerProperties, instanceLayerNames))
                {
                    instanceLayerNames.clear();
                    ALIMER_LOGWARN("[Vulkan] - Set the environment variable VK_LAYER_PATH to point to the location of your layers");
                }
                else
                {
                    hasValidationLayer = true;
                }
            }
            else
            {
                hasValidationLayer = true;
            }
        }

        VkInstanceCreateInfo instanceCreateInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
        instanceCreateInfo.pNext = nullptr;
        instanceCreateInfo.pApplicationInfo = &appInfo;
        instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(instanceLayerNames.size());
        instanceCreateInfo.ppEnabledLayerNames = instanceLayerNames.data();
        instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensionNames.size());
        instanceCreateInfo.ppEnabledExtensionNames = instanceExtensionNames.data();

        result = vkCreateInstance(&instanceCreateInfo, nullptr, &_instance);
        vkThrowIfFailed(result);

        // Now load vk symbols.
        volkLoadInstance(_instance);

        // Setup debug callback
        if (settings.validation && hasValidationLayer)
        {
            VkDebugReportCallbackCreateInfoEXT debugCreateInfo = { VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT };
            debugCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
            debugCreateInfo.pfnCallback = VkDebugCallback;
            debugCreateInfo.pUserData = nullptr;
            if (vkCreateDebugReportCallbackEXT(
                _instance, &debugCreateInfo, nullptr, &_debugCallback) != VK_SUCCESS)
            {
                ALIMER_LOGWARN("vkCreateDebugReportCallbackEXT failed: %s.", vkGetVulkanResultString(result));
            }
        }

        // Enumerate physical devices.
        uint32_t gpuCount = 0;
        vkThrowIfFailed(vkEnumeratePhysicalDevices(_instance, &gpuCount, nullptr));
        ALIMER_ASSERT(gpuCount > 0);

        vector<VkPhysicalDevice> physicalDevices(gpuCount);
        vkEnumeratePhysicalDevices(_instance, &gpuCount, physicalDevices.data());

        ALIMER_LOGTRACE("Enumerating physical devices");
        std::map<int, VkPhysicalDevice> physicalDevicesRated;
        for (uint32_t i = 0; i < gpuCount; i++)
        {
            VkPhysicalDeviceProperties properties;
            VkPhysicalDeviceFeatures features;
            vkGetPhysicalDeviceProperties(physicalDevices[i], &properties);
            vkGetPhysicalDeviceFeatures(physicalDevices[i], &features);

            ALIMER_LOGTRACE("Physical device %d:", i);
            ALIMER_LOGTRACE("\t          Name: %s", properties.deviceName);
            ALIMER_LOGTRACE("\t   API version: %x", properties.apiVersion);
            ALIMER_LOGTRACE("\tDriver version: %x", properties.driverVersion);
            ALIMER_LOGTRACE("\t      VendorId: %x", properties.vendorID);
            ALIMER_LOGTRACE("\t      DeviceId: %x", properties.deviceID);
            ALIMER_LOGTRACE("\t          Type: %d", properties.deviceType);

            int score = 0;
            if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                score += 1000;
            }
            score += properties.limits.maxImageDimension2D;
            if (!features.geometryShader)
            {
                score = 0;
            }
            physicalDevicesRated[score] = physicalDevices[i];
        }

        // Take the first device from rated devices that support our queue requirements
        for (const auto& physicalDeviceRated : physicalDevicesRated)
        {
            VkPhysicalDevice physicalDevice = physicalDeviceRated.second;
            _physicalDevice = physicalDevice;
            break;
        }
    }

    VulkanGraphics::~VulkanGraphics()
    {
        waitIdle();

        for (auto& it : _renderPassCache)
        {
            vkDestroyRenderPass(_device, it.second, nullptr);
        }
        _renderPassCache.clear();

        //_descriptorSetAllocators.clear();
        //_pipelineLayouts.clear();

        vkDestroyPipelineCache(_device, _pipelineCache, nullptr);

        // Destroy default command buffer.
        SafeDelete(_defaultCommandBuffer);

        // Destroy default command pool.
        if (_commandPool != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(_device, _commandPool, nullptr);
            _commandPool = VK_NULL_HANDLE;
        }

        // Destroy all created fences.
        for (auto fence : _allFences)
        {
            vkDestroyFence(_device, fence, nullptr);
        }
        _allFences.clear();

        // Destroy all created semaphores.
        for (auto semaphore : _allSemaphores)
        {
            vkDestroySemaphore(_device, semaphore, nullptr);
        }
        _allSemaphores.clear();

        // Destroy memory allocator.
        vmaDestroyAllocator(_allocator);

        // Destroy logical device.
        if (_device != VK_NULL_HANDLE)
        {
            vkDestroyDevice(_device, nullptr);
            _device = VK_NULL_HANDLE;
        }

        if (_debugCallback != VK_NULL_HANDLE)
        {
            vkDestroyDebugReportCallbackEXT(_instance, _debugCallback, nullptr);
            _debugCallback = VK_NULL_HANDLE;
        }

        vkDestroyInstance(_instance, nullptr);
        _instance = VK_NULL_HANDLE;
    }

    bool VulkanGraphics::waitIdle()
    {
        VkResult result = vkDeviceWaitIdle(_device);
        if (result < VK_SUCCESS)
        {
            ALIMER_LOGTRACE("[Vulkan] - vkDeviceWaitIdle failed");
            return false;
        }

        return true;
    }

    bool VulkanGraphics::Initialize()
    {
        vkGetPhysicalDeviceProperties(_physicalDevice, &_deviceProperties);
        vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &_deviceMemoryProperties);
        vkGetPhysicalDeviceFeatures(_physicalDevice, &_deviceFeatures);

        // Enumerate device extensions.
        uint32_t extCount = 0;
        if (vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &extCount, nullptr) != VK_SUCCESS)
        {
            ALIMER_LOGCRITICAL("Vulkan: Failed to query device extensions");
        }

        vector<VkExtensionProperties> extensions(extCount);
        vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &extCount, extensions.data());

        vector<VkExtension> queryDeviceExtensions = {
            { VK_KHR_SWAPCHAIN_EXTENSION_NAME,VkExtensionType::Required, false},
            { VK_KHR_MAINTENANCE1_EXTENSION_NAME, VkExtensionType::Required, false },
            { VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME, VkExtensionType::Required, false }
        };

        for (uint32_t i = 0; i < extCount; i++)
        {
            for (auto& queryDeviceExtension : queryDeviceExtensions)
            {
                if (strcmp(extensions[i].extensionName, queryDeviceExtension.name) == 0)
                {
                    queryDeviceExtension.supported = true;
                    continue;
                }
            }
        }

        bool requiredExtensionsEnabled = true;
        vector<const char*> deviceExtensions;
        for (auto& queryDeviceExtension : queryDeviceExtensions)
        {
            if (!queryDeviceExtension.supported)
            {
                switch (queryDeviceExtension.type)
                {
                case VkExtensionType::Optional:
                    ALIMER_LOGDEBUG("Optional Vulkan extension %s not supported", queryDeviceExtension.name);
                    break;

                case VkExtensionType::Desired:
                    ALIMER_LOGWARN("Vulkan extension %s not supported", queryDeviceExtension.name);
                    break;

                case VkExtensionType::Required:
                    requiredExtensionsEnabled = false;
                    ALIMER_LOGERROR("Required Vulkan extension %s not supported", queryDeviceExtension.name);
                    break;
                default:
                    break;
                }
            }
            else
            {
                deviceExtensions.push_back(queryDeviceExtension.name);
            }
        }

        if (!requiredExtensionsEnabled)
            return false;

        // Find vendor.
        _vendorID = _deviceProperties.vendorID;
        switch (_vendorID)
        {
        case 0x13B5:
            _vendor = GpuVendor::Arm;
            break;
        case 0x10DE:
            _vendor = GpuVendor::Nvidia;
            break;
        case 0x1002:
        case 0x1022:
            _vendor = GpuVendor::Amd;
            break;
        case 0x8086:
            _vendor = GpuVendor::Intel;
            break;
        default:
            _vendor = GpuVendor::Unknown;
            break;
        }

        // Queue props.
        uint32_t queueFamilyProps;
        vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyProps, nullptr);
        _queueFamilyProperties.resize(queueFamilyProps);
        vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyProps, _queueFamilyProperties.data());

        // TODO: Add main surface
        VkSurfaceKHR _surface = VK_NULL_HANDLE;
        for (uint32_t i = 0; i < queueFamilyProps; i++)
        {
            VkBool32 supported = _surface == VK_NULL_HANDLE;
            if (_surface != VK_NULL_HANDLE)
            {
                vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, i, _surface, &supported);
            }

            static const VkQueueFlags required = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT;
            if (supported
                && ((_queueFamilyProperties[i].queueFlags & required) == required))
            {
                _graphicsQueueFamily = i;
                break;
            }
        }

        for (uint32_t i = 0; i < queueFamilyProps; i++)
        {
            static const VkQueueFlags required = VK_QUEUE_COMPUTE_BIT;
            if (i != _graphicsQueueFamily
                && (_queueFamilyProperties[i].queueFlags & required) == required)
            {
                _computeQueueFamily = i;
                break;
            }
        }

        for (uint32_t i = 0; i < queueFamilyProps; i++)
        {
            static const VkQueueFlags required = VK_QUEUE_TRANSFER_BIT;
            if (i != _graphicsQueueFamily
                && i != _computeQueueFamily
                && (_queueFamilyProperties[i].queueFlags & required) == required)
            {
                _transferQueueFamily = i;
                break;
            }
        }

        if (_transferQueueFamily == VK_QUEUE_FAMILY_IGNORED)
        {
            for (uint32_t i = 0; i < queueFamilyProps; i++)
            {
                static const VkQueueFlags required = VK_QUEUE_TRANSFER_BIT;
                if (i != _graphicsQueueFamily
                    && (_queueFamilyProperties[i].queueFlags & required) == required)
                {
                    _transferQueueFamily = i;
                    break;
                }
            }
        }

        if (_graphicsQueueFamily == VK_QUEUE_FAMILY_IGNORED)
            return false;

        uint32_t universalQueueIndex = 1;
        uint32_t graphicsQueueIndex = 0;
        uint32_t computeQueueIndex = 0;
        uint32_t transferQueueIndex = 0;

        if (_computeQueueFamily == VK_QUEUE_FAMILY_IGNORED)
        {
            _computeQueueFamily = _graphicsQueueFamily;
            computeQueueIndex = min(_queueFamilyProperties[_graphicsQueueFamily].queueCount - 1, universalQueueIndex);
            universalQueueIndex++;
        }

        if (_transferQueueFamily == VK_QUEUE_FAMILY_IGNORED)
        {
            _transferQueueFamily = _graphicsQueueFamily;
            transferQueueIndex = min(_queueFamilyProperties[_graphicsQueueFamily].queueCount - 1, universalQueueIndex);
            universalQueueIndex++;
        }
        else if (_transferQueueFamily == _computeQueueFamily)
        {
            transferQueueIndex = min(_queueFamilyProperties[_computeQueueFamily].queueCount - 1, 1u);
        }

        static const float graphicsQueuePrio = 0.5f;
        static const float computeQueuePrio = 1.0f;
        static const float transferQueuePrio = 1.0f;
        float priorities[3] = { graphicsQueuePrio, computeQueuePrio, transferQueuePrio };

        // Now create logical device.
        uint32_t queueFamilyCount = 0;
        VkDeviceQueueCreateInfo queueCreateInfos[3] = {};
        queueCreateInfos[queueFamilyCount].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfos[queueFamilyCount].queueFamilyIndex = _graphicsQueueFamily;
        queueCreateInfos[queueFamilyCount].queueCount = min(universalQueueIndex, _queueFamilyProperties[_graphicsQueueFamily].queueCount);
        queueCreateInfos[queueFamilyCount].pQueuePriorities = priorities;
        queueFamilyCount++;

        if (_computeQueueFamily != _graphicsQueueFamily)
        {
            queueCreateInfos[queueFamilyCount].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfos[queueFamilyCount].queueFamilyIndex = _computeQueueFamily;
            queueCreateInfos[queueFamilyCount].queueCount = min(_transferQueueFamily == _computeQueueFamily ? 2u : 1u,
                _queueFamilyProperties[_computeQueueFamily].queueCount);
            queueCreateInfos[queueFamilyCount].pQueuePriorities = priorities + 1;
            queueFamilyCount++;
        }

        if (_transferQueueFamily != _graphicsQueueFamily
            && _transferQueueFamily != _computeQueueFamily)
        {
            queueCreateInfos[queueFamilyCount].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfos[queueFamilyCount].queueFamilyIndex = _transferQueueFamily;
            queueCreateInfos[queueFamilyCount].queueCount = 1;
            queueCreateInfos[queueFamilyCount].pQueuePriorities = priorities + 2;
            queueFamilyCount++;
        }

        VkDeviceCreateInfo deviceCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
        deviceCreateInfo.pNext = nullptr;
        deviceCreateInfo.flags = 0;
        deviceCreateInfo.queueCreateInfoCount = queueFamilyCount;
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos;

        if (deviceExtensions.size() > 0)
        {
            deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
            deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
        }

        // Enabled features
        VkPhysicalDeviceFeatures enabledFeatures = {};
        if (_deviceFeatures.textureCompressionETC2)
            enabledFeatures.textureCompressionETC2 = VK_TRUE;
        if (_deviceFeatures.textureCompressionBC)
            enabledFeatures.textureCompressionBC = VK_TRUE;
        if (_deviceFeatures.textureCompressionASTC_LDR)
            enabledFeatures.textureCompressionASTC_LDR = VK_TRUE;
        if (_deviceFeatures.fullDrawIndexUint32)
            enabledFeatures.fullDrawIndexUint32 = VK_TRUE;
        if (_deviceFeatures.imageCubeArray)
            enabledFeatures.imageCubeArray = VK_TRUE;
        if (_deviceFeatures.fillModeNonSolid)
            enabledFeatures.fillModeNonSolid = VK_TRUE;
        if (_deviceFeatures.independentBlend)
            enabledFeatures.independentBlend = VK_TRUE;
        if (_deviceFeatures.sampleRateShading)
            enabledFeatures.sampleRateShading = VK_TRUE;
        if (_deviceFeatures.fragmentStoresAndAtomics)
            enabledFeatures.fragmentStoresAndAtomics = VK_TRUE;
        if (_deviceFeatures.shaderStorageImageExtendedFormats)
            enabledFeatures.shaderStorageImageExtendedFormats = VK_TRUE;
        if (_deviceFeatures.samplerAnisotropy)
            enabledFeatures.samplerAnisotropy = VK_TRUE;

        deviceCreateInfo.pEnabledFeatures = &enabledFeatures;

        VkResult result = vkCreateDevice(_physicalDevice, &deviceCreateInfo, nullptr, &_device);
        if (result != VK_SUCCESS)
        {
            ALIMER_LOGERROR("Failed to create device: ", vkGetVulkanResultString(result));
            return false;
        }
        volkLoadDevice(_device);

        // Get queue's.
        vkGetDeviceQueue(_device, _graphicsQueueFamily, graphicsQueueIndex, &_graphicsQueue);
        vkGetDeviceQueue(_device, _computeQueueFamily, computeQueueIndex, &_computeQueue);
        vkGetDeviceQueue(_device, _transferQueueFamily, transferQueueIndex, &_transferQueue);

        // Create memory allocator.
        CreateAllocator();

        VkPipelineCacheCreateInfo pipelineCacheCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO };
        pipelineCacheCreateInfo.pNext = nullptr;
        pipelineCacheCreateInfo.flags = 0;
        vkThrowIfFailed(vkCreatePipelineCache(_device, &pipelineCacheCreateInfo, nullptr, &_pipelineCache));

        // Create default command pool.
        _commandPool = CreateCommandPool(
            _graphicsQueueFamily,
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
        );

        // Create default primary command buffer;
        _defaultCommandBuffer = new VulkanCommandBuffer(this, _commandPool, false);

        // Create the main swap chain.
        //_swapChain = new VulkanSwapchain(this, _window);

        return true;
    }

    void VulkanGraphics::CreateAllocator()
    {
        VmaAllocatorCreateInfo createInfo = {};
        createInfo.physicalDevice = _physicalDevice;
        createInfo.device = _device;

        VmaVulkanFunctions vulkanFunctions = {};
        vulkanFunctions.vkAllocateMemory = vkAllocateMemory;
        vulkanFunctions.vkBindBufferMemory = vkBindBufferMemory;
        vulkanFunctions.vkBindImageMemory = vkBindImageMemory;
        vulkanFunctions.vkCreateBuffer = vkCreateBuffer;
        vulkanFunctions.vkCreateImage = vkCreateImage;
        vulkanFunctions.vkDestroyBuffer = vkDestroyBuffer;
        vulkanFunctions.vkDestroyImage = vkDestroyImage;
        vulkanFunctions.vkFreeMemory = vkFreeMemory;
        vulkanFunctions.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
        vulkanFunctions.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR;
        vulkanFunctions.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
        vulkanFunctions.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2KHR;
        vulkanFunctions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
        vulkanFunctions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
        vulkanFunctions.vkMapMemory = vkMapMemory;
        vulkanFunctions.vkUnmapMemory = vkUnmapMemory;

        createInfo.pVulkanFunctions = &vulkanFunctions;

        VkResult result = vmaCreateAllocator(&createInfo, &_allocator);
        if (result != VK_SUCCESS)
        {
            ALIMER_LOGERROR("Vulkan - Failed to create allocator.");
        }
    }

    bool VulkanGraphics::beginFrame(SwapchainImpl* swapchain)
    {
        // Acquire the next image from the swap chain.
        static_cast<VulkanSwapchain*>(swapchain)->acquireNextImage(&_swapchainImageIndex, &_swapchainImageAcquiredSemaphore);

        // Begin command buffer rendering.
        _defaultCommandBuffer->Begin(nullptr);

        return true;
    }

    void VulkanGraphics::endFrame(SwapchainImpl* swapchain)
    {
        _defaultCommandBuffer->EndCore();

        VkCommandBuffer commandBuffer = _defaultCommandBuffer->GetHandle();

        // Get signal semaphore.
        VkSemaphore signalSemaphore = AcquireSemaphore();
        if (signalSemaphore == VK_NULL_HANDLE)
            return;

        // Submit command buffers.
        VkPipelineStageFlags submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.pNext = nullptr;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &_swapchainImageAcquiredSemaphore;
        submitInfo.pWaitDstStageMask = &submitPipelineStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &signalSemaphore;

        vkThrowIfFailed(vkQueueSubmit(_graphicsQueue, 1, &submitInfo, (VkFence)nullptr));

        // Submit Swapchain.
        VkResult result = static_cast<VulkanSwapchain*>(swapchain)->queuePresent(
            _graphicsQueue,
            _swapchainImageIndex,
            signalSemaphore
        );

        if (result != VK_SUCCESS)
        {
            if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
            {
                static_cast<VulkanSwapchain*>(swapchain)->resize(0, 0, true);
            }
        }

        // Wait frame.
        waitIdle();

        // Release semaphores
        ReleaseSemaphore(_swapchainImageAcquiredSemaphore);
        ReleaseSemaphore(signalSemaphore);

        // DestroyPendingResources();
    }

    void VulkanGraphics::BeginRenderPass()
    {
        //_defaultCommandBuffer->BeginRenderPass();
    }

    void VulkanGraphics::EndRenderPass()
    {
        //_defaultCommandBuffer->EndRenderPassCore();
    }


    SwapchainImpl* VulkanGraphics::CreateSwapchain(void* windowHandle, const uvec2& size)
    {
        return new VulkanSwapchain(this, windowHandle, size);
    }

    /*CommandBuffer* VulkanGraphics::GetDefaultCommandBuffer() const
    {
        return _defaultCommandBuffer;
    }

    CommandBuffer* VulkanGraphics::CreateCommandBuffer()
    {
        return nullptr;
    }


    RenderPass* VulkanGraphics::CreateRenderPassImpl(const RenderPassDescription* descriptor)
    {
        return new VulkanRenderPass(this, descriptor);
    }

    GpuBuffer* VulkanGraphics::CreateBufferImpl(const BufferDescriptor* descriptor, const void* initialData)
    {
        return new VulkanBuffer(this, descriptor, initialData);
    }

    VertexInputFormat* VulkanGraphics::CreateVertexInputFormatImpl(const VertexInputFormatDescriptor* descriptor)
    {
        return nullptr;
    }

    ShaderModule* VulkanGraphics::CreateShaderModuleImpl(const std::vector<uint32_t>& spirv)
    {
        return new VulkanShaderModule(this, spirv);
    }

    ShaderProgram* VulkanGraphics::CreateShaderProgramImpl(const ShaderProgramDescriptor* descriptor)
    {
        return new VulkanShader(this, descriptor);
    }

    Texture* VulkanGraphics::CreateTextureImpl(const TextureDescriptor* descriptor, const ImageLevel* initialData)
    {
        return nullptr;
        //return new VulkanTexture(this, descriptor, initialData);
    }*/

    VkCommandPool VulkanGraphics::CreateCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags)
    {
        VkCommandPoolCreateInfo createInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
        createInfo.pNext = nullptr;
        createInfo.queueFamilyIndex = queueFamilyIndex;
        createInfo.flags = createFlags;

        VkCommandPool commandPool;
        vkThrowIfFailed(vkCreateCommandPool(_device, &createInfo, nullptr, &commandPool));
        return commandPool;
    }

    VkCommandBuffer VulkanGraphics::CreateCommandBuffer(VkCommandBufferLevel level, bool begin)
    {
        VkCommandBufferAllocateInfo info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
        info.commandPool = _commandPool;
        info.level = level;
        info.commandBufferCount = 1;

        VkCommandBuffer vkCommandBuffer;
        vkThrowIfFailed(vkAllocateCommandBuffers(_device, &info, &vkCommandBuffer));

        // If requested, also start recording for the new command buffer
        if (begin)
        {
            VkCommandBufferBeginInfo cmdBufferBeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
            vkThrowIfFailed(vkBeginCommandBuffer(vkCommandBuffer, &cmdBufferBeginInfo));
        }

        return vkCommandBuffer;
    }

    void VulkanGraphics::FlushCommandBuffer(VkCommandBuffer commandBuffer, bool free)
    {
        FlushCommandBuffer(commandBuffer, _graphicsQueue, free);
    }

    void VulkanGraphics::FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free)
    {
        if (commandBuffer == VK_NULL_HANDLE)
            return;

        vkThrowIfFailed(vkEndCommandBuffer(commandBuffer));

        VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        // Create fence to ensure that the command buffer has finished executing
        VkFenceCreateInfo fenceCreateInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, 0 };

        VkFence fence = AcquireFence();

        // Submit to the queue
        vkThrowIfFailed(vkQueueSubmit(queue, 1, &submitInfo, fence));

        // Wait for the fence to signal that command buffer has finished executing.
        vkThrowIfFailed(vkWaitForFences(_device, 1, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max()));
        ReleaseFence(fence);

        if (free)
        {
            vkFreeCommandBuffers(_device, _commandPool, 1, &commandBuffer);
        }
    }

    void VulkanGraphics::ClearImageWithColor(VkCommandBuffer commandBuffer, VkImage image, VkImageSubresourceRange range, VkImageAspectFlags aspect, VkImageLayout sourceLayout, VkImageLayout destLayout, VkAccessFlagBits srcAccessMask, VkClearColorValue *clearValue)
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

    VkRenderPass VulkanGraphics::GetVkRenderPass(const RenderPassDescription* descriptor)
    {
        Hasher renderPassHasher;

        for (uint32_t i = 0; i < MaxColorAttachments; i++)
        {
            const RenderPassAttachment& colorAttachment = descriptor->colorAttachments[i];
            Texture* texture = colorAttachment.texture;
            if (!texture)
                continue;

            renderPassHasher.u32(static_cast<uint32_t>(texture->GetFormat()));
            renderPassHasher.u32(static_cast<uint32_t>(colorAttachment.loadAction));
            renderPassHasher.u32(static_cast<uint32_t>(colorAttachment.storeAction));
        }

        uint64_t hash = renderPassHasher.get();
        auto it = _renderPassCache.find(hash);
        if (it != end(_renderPassCache))
            return it->second;

        uint32_t attachmentCount = 0;
        std::array<VkAttachmentDescription, MaxColorAttachments + 1> attachments = {};
        std::vector<VkAttachmentReference> colorReferences;
        VkAttachmentReference depthReference = {};
        bool hasDepth = false;

        for (uint32_t i = 0; i < MaxColorAttachments; i++)
        {
            const RenderPassAttachment& colorAttachment = descriptor->colorAttachments[i];
            Texture* texture = colorAttachment.texture;
            if (!texture)
                continue;

            attachments[attachmentCount].format = vk::Convert(texture->GetFormat());
            attachments[attachmentCount].samples = VK_SAMPLE_COUNT_1_BIT;
            attachments[attachmentCount].loadOp = vk::Convert(colorAttachment.loadAction);
            attachments[attachmentCount].storeOp = vk::Convert(colorAttachment.storeAction);
            attachments[attachmentCount].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[attachmentCount].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[attachmentCount].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[attachmentCount].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            colorReferences.push_back({ attachmentCount, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

            attachmentCount++;
        }

        if (descriptor->depthStencilAttachment.texture)
        {
            attachments[attachmentCount].format = vk::Convert(descriptor->depthStencilAttachment.texture->GetFormat());
            attachments[attachmentCount].samples = VK_SAMPLE_COUNT_1_BIT;
            attachments[attachmentCount].loadOp = vk::Convert(descriptor->depthStencilAttachment.loadAction);
            attachments[attachmentCount].storeOp = vk::Convert(descriptor->depthStencilAttachment.storeAction);
            attachments[attachmentCount].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // vk::Convert(descriptor.stencilAttachment.loadAction);
            attachments[attachmentCount].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;// vk::Convert(descriptor.stencilAttachment.storeAction);
            attachments[attachmentCount].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[attachmentCount].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            depthReference.attachment = attachmentCount;
            depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            attachmentCount++;

            hasDepth = true;
        }

        VkSubpassDescription subpassDescription = {};
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
        subpassDescription.pColorAttachments = colorReferences.data();
        if (hasDepth)
        {
            subpassDescription.pDepthStencilAttachment = &depthReference;
        }
        else
        {
            subpassDescription.pDepthStencilAttachment = nullptr;
        }

        subpassDescription.inputAttachmentCount = 0;
        subpassDescription.pInputAttachments = nullptr;
        subpassDescription.preserveAttachmentCount = 0;
        subpassDescription.pPreserveAttachments = nullptr;
        subpassDescription.pResolveAttachments = nullptr;

        // Subpass dependencies for layout transitions
        std::array<VkSubpassDependency, 2> dependencies;

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo createInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.attachmentCount = attachmentCount;
        createInfo.pAttachments = attachments.data();
        createInfo.subpassCount = 1;
        createInfo.pSubpasses = &subpassDescription;
        createInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
        createInfo.pDependencies = dependencies.data();

        VkRenderPass renderPass;
        VkResult result = vkCreateRenderPass(_device, &createInfo, nullptr, &renderPass);
        if (result != VK_SUCCESS)
        {
            ALIMER_LOGERROR("Vulkan - Failed to create render pass.");
            return VK_NULL_HANDLE;
        }

        _renderPassCache[hash] = renderPass;
        return renderPass;
    }

    VkFence VulkanGraphics::AcquireFence()
    {
        VkFence fence = VK_NULL_HANDLE;

        // See if there's a free fence available.
        std::lock_guard<std::mutex> lock(_fenceLock);
        if (_availableFences.size() > 0)
        {
            fence = _availableFences.front();
            _availableFences.pop();
        }
        else
        {
            // Else create a new one.
            VkFenceCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            VkResult result = vkCreateFence(_device, &createInfo, nullptr, &fence);
            if (result != VK_SUCCESS)
            {
                ALIMER_LOGERROR("Failed to create fence: %s", vkGetVulkanResultString(result));
            }

            ALIMER_LOGDEBUG("New fence created");
            _allFences.emplace(fence);
        }

        return fence;
    }

    void VulkanGraphics::ReleaseFence(VkFence fence)
    {
        std::lock_guard<std::mutex> lock(_fenceLock);
        if (_allFences.find(fence) != _allFences.end())
        {
            vkResetFences(_device, 1, &fence);
            _availableFences.push(fence);
        }
    }

    VkSemaphore VulkanGraphics::AcquireSemaphore()
    {
        VkSemaphore semaphore = VK_NULL_HANDLE;

        // See if there are free semaphores available.
        std::lock_guard<std::mutex> lock(_semaphoreLock);
        if (_availableSemaphores.size() > 0)
        {
            semaphore = _availableSemaphores.front();
            _availableSemaphores.pop();
        }
        else
        {
            // Create any remaining required semaphores.
            VkSemaphoreCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            VkResult result = vkCreateSemaphore(_device, &createInfo, nullptr, &semaphore);
            if (result != VK_SUCCESS)
            {
                ALIMER_LOGERROR("Failed to create semaphore: %s", vkGetVulkanResultString(result));
            }

            ALIMER_LOGDEBUG("New semaphore created");
            _allSemaphores.emplace(semaphore);
        }

        return semaphore;
    }

    void VulkanGraphics::ReleaseSemaphore(VkSemaphore semaphore)
    {
        std::lock_guard<std::mutex> lock(_semaphoreLock);
        if (_allSemaphores.find(semaphore) != _allSemaphores.end())
        {
            _availableSemaphores.push(semaphore);
        }
    }

#if TODO
    VulkanDescriptorSetAllocator* VulkanGraphics::RequestDescriptorSetAllocator(const DescriptorSetLayout &layout)
    {
        Hasher h;
        h.data(reinterpret_cast<const uint32_t *>(&layout), sizeof(layout));
        auto hash = h.get();
        auto itr = _descriptorSetAllocators.find(hash);
        if (itr != _descriptorSetAllocators.end())
            return itr->second.get();

        auto *allocator = new VulkanDescriptorSetAllocator(this, layout);
        _descriptorSetAllocators.insert(make_pair(hash, unique_ptr<VulkanDescriptorSetAllocator>(allocator)));
        return allocator;
    }

    VulkanPipelineLayout* VulkanGraphics::RequestPipelineLayout(const ResourceLayout &layout)
    {
        Hasher h;
        h.data(reinterpret_cast<const uint32_t*>(layout.sets), sizeof(layout.sets));
        //h.data(reinterpret_cast<const uint32_t *>(layout.ranges), sizeof(layout.ranges));
        h.u32(layout.attributeMask);

        auto hash = h.get();

        auto it = _pipelineLayouts.find(hash);
        if (it != _pipelineLayouts.end())
            return it->second.get();

        VulkanPipelineLayout *newPipelineLayout = new VulkanPipelineLayout(this, layout);
        _pipelineLayouts.insert(make_pair(hash, unique_ptr<VulkanPipelineLayout>(newPipelineLayout)));
        return newPipelineLayout;
}
#endif // TODO
}

#include "../../../ThirdParty/volk/volk.c"
