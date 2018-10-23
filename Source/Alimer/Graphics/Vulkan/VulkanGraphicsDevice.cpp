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

#include "VulkanGraphicsDevice.h"
#include "VulkanCommandBuffer.h"
#include "VulkanSwapchain.h"
#include "VulkanRenderPass.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "VulkanShader.h"
#include "VulkanPipelineLayout.h"
#include "VulkanConvert.h"
#include "../../Math/Math.h"
#include "../../Core/Log.h"
#include "AlimerVersion.h"
#include <SPIRV/GlslangToSpv.h>

namespace Alimer
{
    static VKAPI_ATTR VkBool32 VKAPI_CALL VkMessengerCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT                  messageType,
        const VkDebugUtilsMessengerCallbackDataEXT*      pCallbackData,
        void *pUserData)
    {
        auto *context = static_cast<VulkanGraphicsDevice*>(pUserData);

        switch (messageSeverity)
        {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                || messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
            {
                ALIMER_LOGERRORF("[Vulkan]: Validation Error: %s", pCallbackData->pMessage);
                context->NotifyValidationError(pCallbackData->pMessage);
            }
            else
            {
                ALIMER_LOGERRORF("[Vulkan]: %s", pCallbackData->pMessage);
            }

            break;

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                || messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
            {
                ALIMER_LOGWARNF("[Vulkan]: Validation Warning: %s", pCallbackData->pMessage);
            }
            else
            {
                ALIMER_LOGWARNF("[Vulkan]: %s", pCallbackData->pMessage);
            }
            break;

            // Log to verbose only
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                || messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
            {
                ALIMER_LOGTRACEF("[Vulkan]: Validation Info: %s", pCallbackData->pMessage);
            }
            else
            {
                // Verbose message
                ALIMER_LOGTRACEF("[Vulkan]: %s", pCallbackData->pMessage);
            }
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
                ALIMER_LOGINFOF("  Object #%u: %s\n", i, name ? name : "N/A");
            }
        }

        return VK_FALSE;
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

        //auto *graphics = static_cast<VulkanGraphicsDevice*>(pUserData);

        if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
        {
            ALIMER_LOGERRORF("[Vulkan] - [%s] Code %d : %s", pLayerPrefix, messageCode, pMessage);
        }
        else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
        {
            ALIMER_LOGWARNF("[Vulkan] - [%s] Code %d : %s", pLayerPrefix, messageCode, pMessage);
        }
        else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
        {
            ALIMER_LOGERRORF("[Vulkan] - PERFORMANCE WARNING: [%s] Code %d : %s", pLayerPrefix, messageCode, pMessage);
        }
        else if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
        {
            ALIMER_LOGINFOF("[Vulkan] - [%s] Code %d : %s", pLayerPrefix, messageCode, pMessage);
        }
        else if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
        {
            ALIMER_LOGDEBUGF("[%s] Code %d : %s", pLayerPrefix, messageCode, pMessage);
        }
        else
        {
            ALIMER_LOGINFOF("%s: %s", pLayerPrefix, pMessage);
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

    static std::vector<VkExtension> s_vkQueryDeviceExtensions = {
           { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VkExtensionType::Required, false},
           { VK_KHR_MAINTENANCE1_EXTENSION_NAME, VkExtensionType::Required, false },
           { VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME, VkExtensionType::Required, false }
    };

    bool VulkanGraphicsDevice::IsSupported()
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
            return false;
        }

        // TODO: Try to create instance and get device with minimum extensions
        isAvailable = true;
        return isAvailable;
    }

    VulkanGraphicsDevice::VulkanGraphicsDevice(bool validation)
        : GraphicsDevice(GraphicsBackend::Vulkan, validation)
    {
#ifdef ALIMER_THREADING
        _cookie.store(0);
#endif
        uint32_t apiVersion = VK_MAKE_VERSION(1, 0, 57);

        if (!IsSupported())
        {
            ALIMER_LOGERROR("[Vulkan] - Not supported.");
            return;
        }

        // Determine if the new instance version command is available
        if (vkEnumerateInstanceVersion != nullptr)
        {
            uint32_t checkApiVersion = 0;
            if (vkEnumerateInstanceVersion(&checkApiVersion) == VK_SUCCESS)
            {
                // Translate the version into major/minor for easier comparison
                ALIMER_LOGTRACEF("[Vulkan] - Loader/Runtime support detected %u.%u.%u",
                    VK_VERSION_MAJOR(checkApiVersion),
                    VK_VERSION_MINOR(checkApiVersion),
                    VK_VERSION_PATCH(checkApiVersion));

                apiVersion = checkApiVersion;
            }
        }

        uint32_t ext_count = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, nullptr);
        std::vector<VkExtensionProperties> queried_extensions(ext_count);
        if (ext_count)
        {
            vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, queried_extensions.data());
        }

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

        const auto has_layer = [&](const char *name) -> bool {
            auto itr = find_if(begin(queried_layers), end(queried_layers), [name](const VkLayerProperties &e) -> bool {
                return strcmp(e.layerName, name) == 0;
            });
            return itr != end(queried_layers);
        };

        std::vector<const char*> instance_exts = { VK_KHR_SURFACE_EXTENSION_NAME };
        std::vector<const char*> instance_layers;

        if (has_extension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) &&
            has_extension(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME) &&
            has_extension(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME))
        {
            instance_exts.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
            instance_exts.push_back(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME);
            instance_exts.push_back(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME);
            _supportsExternal = true;
        }

        if (has_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
        {
            instance_exts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            _supportsDebugUtils = true;
        }

        // Enable surface extensions depending on os.
#if ALIMER_PLATFORM_WINDOWS
        instance_exts.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif ALIMER_PLATFORM_LINUX
        instance_exts.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#elif ALIMER_PLATFORM_APPLE_OSX
        instance_exts.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#elif ALIMER_PLATFORM_APPLE_IOS || ALIMER_PLATFORM_APPLE_TV
        instance_exts.push_back(VK_MVK_IOS_SURFACE_EXTENSION_NAME);
#elif ALIMER_PLATFORM_ANDROID
        instance_exts.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#else
        instance_exts.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#endif

        if (!_supportsDebugUtils && has_extension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
        {
            instance_exts.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        }

        if (validation)
        {
            if (has_layer("VK_LAYER_LUNARG_standard_validation"))
            {
                instance_layers.push_back("VK_LAYER_LUNARG_standard_validation");
            }
            else
            {
                if (has_layer("VK_LAYER_GOOGLE_threading")
                    && has_layer("VK_LAYER_LUNARG_parameter_validation")
                    && has_layer("VK_LAYER_LUNARG_object_tracker")
                    && has_layer("VK_LAYER_LUNARG_core_validation")
                    && has_layer("VK_LAYER_LUNARG_swapchain")
                    && has_layer("VK_LAYER_GOOGLE_unique_objects"))
                {
                    instance_layers.push_back("VK_LAYER_GOOGLE_threading");
                    instance_layers.push_back("VK_LAYER_LUNARG_parameter_validation");
                    instance_layers.push_back("VK_LAYER_LUNARG_object_tracker");
                    instance_layers.push_back("VK_LAYER_LUNARG_core_validation");
                    instance_layers.push_back("VK_LAYER_LUNARG_swapchain");
                    instance_layers.push_back("VK_LAYER_GOOGLE_unique_objects");
                }
            }
        }

        // ApplicationInfo
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Alimer";
        appInfo.applicationVersion = 0;
        appInfo.pEngineName = "Alimer";
        appInfo.engineVersion = VK_MAKE_VERSION(ALIMER_VERSION_MAJOR, ALIMER_VERSION_MINOR, ALIMER_VERSION_PATCH);
        appInfo.apiVersion = apiVersion;

        // Instance create info.
        VkInstanceCreateInfo instanceCreateInfo = {};
        instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pNext = nullptr;
        instanceCreateInfo.flags = 0;
        instanceCreateInfo.pApplicationInfo = &appInfo;
        instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(instance_layers.size());
        instanceCreateInfo.ppEnabledLayerNames = instance_layers.data();
        instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(instance_exts.size());
        instanceCreateInfo.ppEnabledExtensionNames = instance_exts.data();

        VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &_instance);
        if (result != VK_SUCCESS)
        {
            ALIMER_LOGERRORF("Failed to create vulkan instance: %s", vkGetVulkanResultString(result));
            return;
        }

        // Now load vk symbols.
        volkLoadInstance(_instance);

        // Setup debug callback
        if (validation)
        {
            if (_supportsDebugUtils)
            {
                VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo = {};
                debugUtilsMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
                debugUtilsMessengerCreateInfo.pNext = nullptr;
                debugUtilsMessengerCreateInfo.messageSeverity =
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
                    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
                    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
                debugUtilsMessengerCreateInfo.pfnUserCallback = VkMessengerCallback;
                debugUtilsMessengerCreateInfo.messageType =
                    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                    | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
                    | VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
                debugUtilsMessengerCreateInfo.pUserData = this;

                result = vkCreateDebugUtilsMessengerEXT(_instance, &debugUtilsMessengerCreateInfo, nullptr, &_debugMessenger);
                if (result != VK_SUCCESS)
                {
                    ALIMER_LOGWARNF("vkCreateDebugUtilsMessengerEXT failed: %s.", vkGetVulkanResultString(result));
                }
            }
            else if (has_extension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
            {
                VkDebugReportCallbackCreateInfoEXT debugCreateInfo = {};
                debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
                debugCreateInfo.pNext = nullptr;
                debugCreateInfo.flags =
                    VK_DEBUG_REPORT_ERROR_BIT_EXT
                    | VK_DEBUG_REPORT_WARNING_BIT_EXT
                    | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
                debugCreateInfo.pfnCallback = VkDebugCallback;
                debugCreateInfo.pUserData = this;

                result = vkCreateDebugReportCallbackEXT(_instance, &debugCreateInfo, nullptr, &_debugCallback);
                if (result != VK_SUCCESS)
                {
                    ALIMER_LOGWARNF("vkCreateDebugReportCallbackEXT failed: %s.", vkGetVulkanResultString(result));
                }
            }
        }

        // Enumerate physical devices.
        uint32_t gpuCount = 0;
        vkThrowIfFailed(vkEnumeratePhysicalDevices(_instance, &gpuCount, nullptr));
        ALIMER_ASSERT(gpuCount > 0);

        std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
        vkEnumeratePhysicalDevices(_instance, &gpuCount, physicalDevices.data());

        ALIMER_LOGTRACE("[Vulkan] - Enumerating physical devices");
        std::multimap<int32_t, VkPhysicalDevice> rankedDevices;
        for (uint32_t i = 0; i < gpuCount; i++)
        {
            int32_t score = ScorePhysicalDevice(physicalDevices[i]);
            rankedDevices.emplace(score, physicalDevices[i]);
        }

        // Take the first device from rated devices that support our queue requirements
        if (rankedDevices.rbegin()->first > 0)
        {
            _physicalDevice = rankedDevices.rbegin()->second;
        }

        vkGetPhysicalDeviceProperties(_physicalDevice, &_deviceProperties);
        vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &_deviceMemoryProperties);
        vkGetPhysicalDeviceFeatures(_physicalDevice, &_deviceFeatures);

        // Queue props.
        uint32_t queueFamilyProps;
        vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyProps, nullptr);
        _queueFamilyProperties.resize(queueFamilyProps);
        vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyProps, _queueFamilyProperties.data());

        // Initialize glslang library.
        glslang::InitializeProcess();
    }

    VulkanGraphicsDevice::~VulkanGraphicsDevice()
    {
        WaitIdle();
        Shutdown();
    }

    void VulkanGraphicsDevice::Shutdown()
    {
        glslang::FinalizeProcess();

        // Destroy main swap chain.
        _mainSwapchain.Reset();

        // Destroy pinned memory buffer.
        if (_pinnedMemoryBuffer)
        {
            UnmapBuffer(_pinnedMemoryBuffer);
            SafeDelete(_pinnedMemoryBuffer);
        }

        // Clear base resources.
        GraphicsDevice::Shutdown();

        _framebuffers.Clear();
        _renderPasses.Clear();
        //_descriptorSetAllocators.clear();
        _pipelineLayouts.Clear();

        vkDestroyPipelineCache(_device, _pipelineCache, nullptr);

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
        vmaDestroyAllocator(_memoryAllocator);

        // Destroy logical device.
        if (_device != VK_NULL_HANDLE)
        {
            vkDestroyDevice(_device, nullptr);
            _device = VK_NULL_HANDLE;
        }

        if (_debugMessenger != VK_NULL_HANDLE)
        {
            vkDestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);
            _debugMessenger = VK_NULL_HANDLE;
        }

        if (_debugCallback != VK_NULL_HANDLE)
        {
            vkDestroyDebugReportCallbackEXT(_instance, _debugCallback, nullptr);
            _debugCallback = VK_NULL_HANDLE;
        }

        vkDestroyInstance(_instance, nullptr);
        _instance = VK_NULL_HANDLE;
    }

    uint64_t VulkanGraphicsDevice::AllocateCookie()
    {
#ifdef ALIMER_THREADING
        return _cookie.fetch_add(16, std::memory_order_relaxed) + 16;
#else
        _cookie += 16;
        return coo_cookiekie;
#endif
    }

    int32_t VulkanGraphicsDevice::ScorePhysicalDevice(VkPhysicalDevice physicalDevice)
    {
        int32_t score = 0;

        // Checks if the requested extensions are supported.
        uint32_t extensionPropertyCount;
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionPropertyCount, nullptr);
        std::vector<VkExtensionProperties> extensionProperties(extensionPropertyCount);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionPropertyCount, extensionProperties.data());

        // Iterates through all extensions requested.
        for (auto& queryDeviceExtension : s_vkQueryDeviceExtensions)
        {
            bool extensionFound = false;

            // Checks if the extension is in the available extensions.
            for (auto &extension : extensionProperties)
            {
                if (strcmp(queryDeviceExtension.name, extension.extensionName) == 0)
                {
                    extensionFound = true;
                    break;
                }
            }

            // Returns a score of 0 if this device is missing a required extension.
            if (!extensionFound
                && queryDeviceExtension.type == VkExtensionType::Required)
            {
                return 0;
            }
        }

        // Obtain the device features and properties of the current device being rateds.
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceProperties(physicalDevice, &properties);
        vkGetPhysicalDeviceFeatures(physicalDevice, &features);

#if defined(ALIMER_DEV)
        //ALIMER_LOGTRACEF("Physical device %d:", i);
        ALIMER_LOGTRACEF("\t          Name: %s", properties.deviceName);
        ALIMER_LOGTRACEF("\t   API version: %x", properties.apiVersion);
        ALIMER_LOGTRACEF("\tDriver version: %x", properties.driverVersion);
        ALIMER_LOGTRACEF("\t      VendorId: %x", properties.vendorID);
        ALIMER_LOGTRACEF("\t      DeviceId: %x", properties.deviceID);
        ALIMER_LOGTRACEF("\t          Type: %d", properties.deviceType);
        //LogVulkanDevice(physicalDeviceProperties);
#endif

        // Adds a large score boost for discrete GPUs (dedicated graphics cards).
        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            score += 1000;
        }

        // Gives a higher score to devices with a higher maximum texture size.
        score += properties.limits.maxImageDimension2D;

        return score;
    }

    bool VulkanGraphicsDevice::Initialize(const RenderingSettings& settings)
    {
        // Log info.
        ALIMER_LOGINFOF("Selected Vulkan GPU: %s", _deviceProperties.deviceName);

        // Enumerate device extensions and layers.
        uint32_t ext_count = 0;
        vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &ext_count, nullptr);
        std::vector<VkExtensionProperties> queried_extensions(ext_count);
        vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &ext_count, queried_extensions.data());

        uint32_t layer_count = 0;
        vkEnumerateDeviceLayerProperties(_physicalDevice, &layer_count, nullptr);
        std::vector<VkLayerProperties> queried_layers(layer_count);
        if (layer_count)
            vkEnumerateDeviceLayerProperties(_physicalDevice, &layer_count, queried_layers.data());

        const auto has_extension = [&](const char *name) -> bool {
            auto itr = std::find_if(std::begin(queried_extensions), std::end(queried_extensions), [name](const VkExtensionProperties &e) -> bool {
                return strcmp(e.extensionName, name) == 0;
            });
            return itr != std::end(queried_extensions);
        };


        for (uint32_t i = 0; i < ext_count; i++)
        {
            for (auto& queryDeviceExtension : s_vkQueryDeviceExtensions)
            {
                if (has_extension(queryDeviceExtension.name))
                {
                    queryDeviceExtension.supported = true;
                    continue;
                }
            }
        }

        bool requiredExtensionsEnabled = true;
        std::vector<const char*> enabled_extensions;
        for (auto& queryDeviceExtension : s_vkQueryDeviceExtensions)
        {
            if (!queryDeviceExtension.supported)
            {
                switch (queryDeviceExtension.type)
                {
                case VkExtensionType::Optional:
                    ALIMER_LOGDEBUGF("Optional Vulkan extension %s not supported", queryDeviceExtension.name);
                    break;

                case VkExtensionType::Desired:
                    ALIMER_LOGWARNF("Vulkan extension %s not supported", queryDeviceExtension.name);
                    break;

                case VkExtensionType::Required:
                    requiredExtensionsEnabled = false;
                    ALIMER_LOGERRORF("Required Vulkan extension %s not supported", queryDeviceExtension.name);
                    break;
                default:
                    break;
                }
            }
            else
            {
                enabled_extensions.push_back(queryDeviceExtension.name);
            }
        }

        if (!requiredExtensionsEnabled)
            return nullptr;

        // Find vendor.
        _features.SetVendorId(_deviceProperties.vendorID);

        // Create main surface.
        VkSurfaceKHR surface = CreateSurface(settings);
        const uint32_t queueFamilyProps = static_cast<uint32_t>(_queueFamilyProperties.size());
        for (uint32_t i = 0; i < queueFamilyProps; i++)
        {
            VkBool32 supported = surface == VK_NULL_HANDLE;
            if (surface != VK_NULL_HANDLE)
            {
                vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, i, surface, &supported);
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

        if (has_extension(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME) &&
            has_extension(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME))
        {
            _supportsDedicated = true;
            enabled_extensions.push_back(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
            enabled_extensions.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        }

        if (has_extension(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME))
        {
            _supportsImageFormatList = true;
            enabled_extensions.push_back(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
        }

        if (has_extension(VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
        {
            _supportsDebugMarker = true;
            enabled_extensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
        }

#ifdef _WIN32
        _supportsExternal = false;
#else
        if (_supportsExternal
            && _supportsDedicated
            && has_extension(VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME)
            && has_extension(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME)
            && has_extension(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME)
            && has_extension(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME))
        {
            _supportsExternal = true;
            enabled_extensions.push_back(VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME);
            enabled_extensions.push_back(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME);
            enabled_extensions.push_back(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
            enabled_extensions.push_back(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
        }
        else
        {
            _supportsExternal = false;
        }
#endif

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

        VkDeviceCreateInfo deviceCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
        deviceCreateInfo.pNext = nullptr;
        deviceCreateInfo.flags = 0;
        deviceCreateInfo.queueCreateInfoCount = queueFamilyCount;
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos;
        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(enabled_extensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = enabled_extensions.data();
        deviceCreateInfo.pEnabledFeatures = &enabledFeatures;

        VkResult result = vkCreateDevice(_physicalDevice, &deviceCreateInfo, nullptr, &_device);
        if (result != VK_SUCCESS)
        {
            ALIMER_LOGERRORF("Failed to create device: ", vkGetVulkanResultString(result));
            return false;
        }
        volkLoadDevice(_device);

        // Get queue's.
        vkGetDeviceQueue(_device, _graphicsQueueFamily, graphicsQueueIndex, &_graphicsQueue);
        vkGetDeviceQueue(_device, _computeQueueFamily, computeQueueIndex, &_computeQueue);
        vkGetDeviceQueue(_device, _transferQueueFamily, transferQueueIndex, &_transferQueue);

        // Create memory allocator.
        CreateMemoryAllocator();

        // Create default command pool.
        _commandPool = CreateCommandPool(_graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

        // Create the main swap chain.
        _mainSwapchain.Reset(new VulkanSwapchain(
            this,
            surface,
            settings.backBufferWidth,
            settings.backBufferHeight)
        );

        // Create default command context;
        _context = new VulkanCommandBuffer(this, _graphicsQueue, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

        // Pipeline cache.
        VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
        pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        pipelineCacheCreateInfo.pNext = nullptr;
        pipelineCacheCreateInfo.flags = 0;
        pipelineCacheCreateInfo.initialDataSize = 0;
        pipelineCacheCreateInfo.pInitialData = nullptr;
        vkThrowIfFailed(vkCreatePipelineCache(_device, &pipelineCacheCreateInfo, nullptr, &_pipelineCache));

        return GraphicsDevice::Initialize(settings);
    }

    bool VulkanGraphicsDevice::WaitIdle()
    {
        VkResult result = vkDeviceWaitIdle(_device);
        if (result < VK_SUCCESS)
        {
            ALIMER_LOGTRACEF("[Vulkan] - vkDeviceWaitIdle failed : %s", vkGetVulkanResultString(result));
            return false;
        }

        return true;
    }

    void VulkanGraphicsDevice::CreateMemoryAllocator()
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

        VkResult result = vmaCreateAllocator(&createInfo, &_memoryAllocator);
        if (result != VK_SUCCESS)
        {
            ALIMER_LOGERROR("[Vulkan] - Failed to create vma allocator.");
            return;
        }

        // Allocate an 128MB pinned memory buffer to use for efficient data transfers.
        BufferDescriptor descriptor = {};
        descriptor.resourceUsage = ResourceUsage::Staging;
        descriptor.usage = BufferUsage::None;
        descriptor.size = (static_cast<uint64_t>(128) << 20ULL);
        _pinnedMemoryBuffer = new VulkanBuffer(this, &descriptor, nullptr);

        // Keep the pinned memory mapped for the lifetime of the device instance.
        result = MapBuffer(_pinnedMemoryBuffer, 0, descriptor.size, &_pinnedMemoryPtr);
        if (result != VK_SUCCESS)
        {
            ALIMER_LOGERROR("[Vulkan] - Failed to map buffer.");
            return;
        }
    }

    void VulkanGraphicsDevice::PresentImpl()
    {
        VkSubmitInfo submitInfo;
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pNext = nullptr;
        submitInfo.waitSemaphoreCount = static_cast<uint32_t>(_waitSemaphores.size());
        submitInfo.pWaitSemaphores = _waitSemaphores.data();
        submitInfo.pWaitDstStageMask = _waitStageFlags.data();
        submitInfo.commandBufferCount = static_cast<uint32_t>(_submittedCommandBuffers.size());
        submitInfo.pCommandBuffers = _submittedCommandBuffers.data();
        submitInfo.signalSemaphoreCount = 0;
        submitInfo.pSignalSemaphores = nullptr;

        // Get fence and wait on submit.
        VkFence fence = AcquireFence();
        VkResult result = vkQueueSubmit(_graphicsQueue, 1, &submitInfo, fence);
        result = vkWaitForFences(_device, 1, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
        ReleaseFence(fence);

        // Release wait semaphores.
        for (VkSemaphore semaphore : _waitSemaphores)
        {
            ReleaseSemaphore(semaphore);
        }
        _waitSemaphores.clear();
        _waitStageFlags.clear();

        // Reset recording.
        for (VkCommandBuffer commandBuffer : _submittedCommandBuffers)
        {
            VkCommandBufferBeginInfo beginInfo;
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.pNext = nullptr;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            beginInfo.pInheritanceInfo = nullptr;
            vkBeginCommandBuffer(commandBuffer, &beginInfo);
        }
        _submittedCommandBuffers.clear();

        // Submit Swapchain.
        _mainSwapchain->QueuePresent(_graphicsQueue);
    }

    void VulkanGraphicsDevice::AddWaitSemaphore(VkSemaphore semaphore, VkPipelineStageFlags flags)
    {
        _waitSemaphores.push_back(semaphore);
        _waitStageFlags.push_back(flags);
    }

    void VulkanGraphicsDevice::SubmitCommandBuffer(VkCommandBuffer commandBuffer)
    {
        _submittedCommandBuffers.push_back(commandBuffer);
    }

    void VulkanGraphicsDevice::SubmitCommandBuffer(VkCommandBuffer commandBuffer, VkFence fence)
    {
        VkSubmitInfo submitInfo;
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pNext = nullptr;
        submitInfo.waitSemaphoreCount = static_cast<uint32_t>(_waitSemaphores.size());
        submitInfo.pWaitSemaphores = _waitSemaphores.data();
        submitInfo.pWaitDstStageMask = _waitStageFlags.data();
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        submitInfo.signalSemaphoreCount = 0;
        submitInfo.pSignalSemaphores = nullptr;

        // Get fence and wait on submit.
        vkThrowIfFailed(vkQueueSubmit(_graphicsQueue, 1, &submitInfo, fence));
        vkThrowIfFailed(vkWaitForFences(_device, 1, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max()));
        vkResetFences(_device, 1, &fence);
    }

    VkSurfaceKHR VulkanGraphicsDevice::CreateSurface(const RenderingSettings& settings)
    {
        VkResult result = VK_SUCCESS;
        VkSurfaceKHR surface = VK_NULL_HANDLE;

        // Create the os-specific surface.
#if defined(VK_USE_PLATFORM_WIN32_KHR)
        VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
        surfaceCreateInfo.hinstance = GetModuleHandleW(nullptr);
        surfaceCreateInfo.hwnd = settings.windowHandle;
        result = vkCreateWin32SurfaceKHR(_instance, &surfaceCreateInfo, nullptr, &surface);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
        VkWaylandSurfaceCreateInfoKHR surfaceCreateInfo = {};
        surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
        surfaceCreateInfo.display = display;
        surfaceCreateInfo.surface = settings.windowHandle;
        err = vkCreateWaylandSurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
        VkXcbSurfaceCreateInfoKHR surfaceCreateInfo = {};
        surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
        surfaceCreateInfo.connection = connection;
        surfaceCreateInfo.window = settings.windowHandle;
        err = vkCreateXcbSurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
        VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo = { VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR };
        surfaceCreateInfo.window = static_cast<ANativeWindow*>(settings.windowHandle);
        result = vkCreateAndroidSurfaceKHR(_instance, &surfaceCreateInfo, nullptr, &surface);
#endif

        if (result != VK_SUCCESS)
        {
            ALIMER_LOGERROR("[Vulkan] - Failed to create surface.");
        }

        return surface;
    }

    Framebuffer* VulkanGraphicsDevice::GetSwapchainFramebuffer() const
    {
        return _mainSwapchain->GetCurrentFramebuffer();
    }

    GpuBuffer* VulkanGraphicsDevice::CreateBufferImpl(const BufferDescriptor* descriptor, const void* initialData)
    {
        return new VulkanBuffer(this, descriptor, initialData);
    }

    Texture* VulkanGraphicsDevice::CreateTextureImpl(const TextureDescriptor* descriptor, const ImageLevel* initialData)
    {
        return new VulkanTexture(this, descriptor, initialData);
    }

    /*std::unique_ptr<ShaderModule> VulkanGraphicsDevice::CreateShaderModuleImpl(Util::Hash hash, const uint32_t* pCode, size_t size)
    {
        return std::make_unique<VulkanShader>(this, hash, pCode, size);
    }

    std::unique_ptr<Program> VulkanGraphicsDevice::CreateProgramImpl(Util::Hash hash, const std::vector<ShaderModule*>& stages)
    {
        return std::make_unique<VulkanProgram>(this, hash, stages);
    }*/

    VkCommandPool VulkanGraphicsDevice::CreateCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags)
    {
        VkCommandPoolCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = createFlags;
        createInfo.queueFamilyIndex = queueFamilyIndex;

        VkCommandPool commandPool;
        VkResult result = vkCreateCommandPool(_device, &createInfo, nullptr, &commandPool);
        if (result != VK_SUCCESS)
        {
            ALIMER_LOGERROR("[Vulkan] - Failed to create command pool.");
        }

        return commandPool;
    }

    VkCommandBuffer VulkanGraphicsDevice::CreateCommandBuffer(VkCommandBufferLevel level, bool begin)
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
            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            vkThrowIfFailed(vkBeginCommandBuffer(vkCommandBuffer, &beginInfo));
        }

        return vkCommandBuffer;
    }

    void VulkanGraphicsDevice::FlushCommandBuffer(VkCommandBuffer commandBuffer, bool free)
    {
        FlushCommandBuffer(commandBuffer, _graphicsQueue, free);
    }

    void VulkanGraphicsDevice::FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free)
    {
        if (commandBuffer == VK_NULL_HANDLE)
            return;

        vkThrowIfFailed(vkEndCommandBuffer(commandBuffer));

        VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

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

    void VulkanGraphicsDevice::ClearImageWithColor(
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

    VulkanFramebuffer* VulkanGraphicsDevice::RequestFramebuffer(const RenderPassDescriptor* descriptor)
    {
        auto renderPass = RequestRenderPass(descriptor);
        Util::Hasher h;
        h.u64(renderPass->GetHash());

        for (uint32_t i = 0; i < MaxColorAttachments; i++)
        {
            if (descriptor->colorAttachments[i].attachment)
            {
                h.u64(static_cast<VulkanTextureView*>(descriptor->colorAttachments[i].attachment)->GetId());
            }
        }

        if (descriptor->depthStencil)
        {
            h.u64(static_cast<VulkanTextureView*>(descriptor->depthStencil)->GetId());
        }

        uint64_t hash = h.get();
        auto framebuffer = _framebuffers.Find(hash);
        if (!framebuffer)
        {
            framebuffer = _framebuffers.Insert(hash, std::make_unique<VulkanFramebuffer>(this, renderPass, descriptor));
        }
        return framebuffer;
    }

    VkResult VulkanGraphicsDevice::BufferSubData(VulkanBuffer* buffer, VkDeviceSize offset, VkDeviceSize size, const void* pData)
    {
        // Get required alignment flush size for selected physical device.
        VkDeviceSize alignedFlushSize = _deviceProperties.limits.nonCoherentAtomSize;

        // Copy the buffer data to the device in blocks based on size of pinned memory buffer.
        auto bytesRemaining = size;
        auto dstOffset = offset;
        auto srcOffset = 0ULL;
        while (bytesRemaining)
        {
            // Determine total byte size to copy this iteration.
            auto bytesToCopy = std::min(_pinnedMemoryBuffer->GetSize(), bytesRemaining);

            // Copy the host memory to the pinned memory buffer.
            memcpy(_pinnedMemoryPtr, &reinterpret_cast<const uint8_t*>(pData)[srcOffset], bytesToCopy);

            // Flush must be aligned according to physical device's limits.
            auto alignedBytesToCopy = static_cast<VkDeviceSize>(std::ceil(bytesToCopy / static_cast<float>(alignedFlushSize))) * alignedFlushSize;

            // Flush the memory write.
            VmaAllocationInfo allocInfo = {};
            vmaGetAllocationInfo(_memoryAllocator, _pinnedMemoryBuffer->GetAllocation(), &allocInfo);

            VkMappedMemoryRange memoryRange = {};
            memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            memoryRange.memory = allocInfo.deviceMemory;
            memoryRange.offset = 0;
            memoryRange.size = alignedBytesToCopy;
            vkFlushMappedMemoryRanges(_device, 1, &memoryRange);

            // Copy the pinned memory buffer to the destination buffer.
            VkBufferCopy copyRegion = {};
            copyRegion.srcOffset = 0;
            copyRegion.dstOffset = dstOffset;
            copyRegion.size = bytesToCopy;

            // Get a one-time submit command buffer.
            auto cmdBuffer = CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
            vkCmdCopyBuffer(cmdBuffer,
                _pinnedMemoryBuffer->GetHandle(),
                buffer->GetHandle(),
                1, &copyRegion);
            // Submit and wait for all operations to complete.
            FlushCommandBuffer(cmdBuffer, _graphicsQueue, true);

            // Update running counters.
            bytesRemaining -= bytesToCopy;
            dstOffset += bytesToCopy;
            srcOffset += bytesToCopy;
        }

        // Return success.
        return VK_SUCCESS;
    }

    VkResult VulkanGraphicsDevice::MapBuffer(VulkanBuffer* buffer, VkDeviceSize offset, VkDeviceSize size, void** ppData)
    {
        return vmaMapMemory(_memoryAllocator, buffer->GetAllocation(), ppData);
    }

    void VulkanGraphicsDevice::UnmapBuffer(VulkanBuffer* buffer)
    {
        vmaUnmapMemory(_memoryAllocator, buffer->GetAllocation());
    }

    VulkanRenderPass* VulkanGraphicsDevice::RequestRenderPass(const RenderPassDescriptor* descriptor)
    {
        Util::Hasher renderPassHasher;

        for (uint32_t i = 0; i < MaxColorAttachments; i++)
        {
            const RenderPassColorAttachmentDescriptor& colorAttachment = descriptor->colorAttachments[i];
            auto attachment = colorAttachment.attachment;
            if (attachment == nullptr)
                continue;

            renderPassHasher.u32(static_cast<uint32_t>(attachment->GetFormat()));
            renderPassHasher.u32(static_cast<uint32_t>(colorAttachment.loadAction));
            renderPassHasher.u32(static_cast<uint32_t>(colorAttachment.storeAction));
        }

        uint64_t hash = renderPassHasher.get();
        auto renderPass = _renderPasses.Find(hash);
        if (!renderPass)
        {
            renderPass = _renderPasses.Insert(hash, std::make_unique<VulkanRenderPass>(hash, this, descriptor));
        }
        return renderPass;
    }

    VulkanPipelineLayout* VulkanGraphicsDevice::RequestPipelineLayout(const VulkanResourceLayout* layout)
    {
        Util::Hasher h;
        //h.data(reinterpret_cast<const uint32_t*>(layout.sets), sizeof(layout.sets));
        //h.data(reinterpret_cast<const uint32_t *>(layout.ranges), sizeof(layout.ranges));
        h.u32(layout->vertexAttributeMask);
        h.u32(layout->renderTargetMask);

        auto hash = h.get();

        auto *ret = _pipelineLayouts.Find(hash);
        if (!ret)
        {
            ret = _pipelineLayouts.Insert(hash, std::make_unique<VulkanPipelineLayout>(this, hash, layout));
        }

        return ret;
    }

    VkFence VulkanGraphicsDevice::AcquireFence()
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
                ALIMER_LOGERRORF("[Vulkan] - Failed to create fence: %s", vkGetVulkanResultString(result));
            }

            ALIMER_LOGTRACE("[Vulkan] - New fence created");
            _allFences.emplace(fence);
        }

        return fence;
    }

    void VulkanGraphicsDevice::ReleaseFence(VkFence fence)
    {
        std::lock_guard<std::mutex> lock(_fenceLock);
        if (_allFences.find(fence) != _allFences.end())
        {
            vkResetFences(_device, 1, &fence);
            _availableFences.push(fence);
        }
    }

    VkSemaphore VulkanGraphicsDevice::AcquireSemaphore()
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
                ALIMER_LOGERRORF("[Vulkan] - Failed to create semaphore: %s", vkGetVulkanResultString(result));
            }

            ALIMER_LOGTRACE("[Vulkan] - New semaphore created");
            _allSemaphores.emplace(semaphore);
        }

        return semaphore;
    }

    void VulkanGraphicsDevice::ReleaseSemaphore(VkSemaphore semaphore)
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

   
#endif // TODO
}
