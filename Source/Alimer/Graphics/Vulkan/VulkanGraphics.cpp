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
#include "../../Core/String.h"
#include "../../Application/Window.h"
#include "VulkanGraphics.h"
#include "VulkanGpuAdapter.h"
#include "VulkanCommandQueue.h"
#include "VulkanCommandBuffer.h"
#include "VulkanRenderPass.h"
#include "VulkanTexture.h"
#include "VulkanBuffer.h"
#include "VulkanShader.h"
#include "VulkanPipelineLayout.h"
#include "VulkanPipelineState.h"
#include "VulkanConvert.h"

#if defined(VK_USE_PLATFORM_WIN32_KHR)
#	include "../../Application/Windows/WindowWindows.h"
#endif

#include "AlimerVersion.h"
#include <map>
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
            ALIMER_LOGERROR("[Vulkan] - [{}] Code {} : {}", pLayerPrefix, messageCode, pMessage);
        }
        else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
        {
            ALIMER_LOGWARN("[Vulkan] - [{}] Code {} : {}", pLayerPrefix, messageCode, pMessage);
        }
        else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
        {
            ALIMER_LOGERROR("[Vulkan] - PERFORMANCE WARNING: [{}] Code {} : {}", pLayerPrefix, messageCode, pMessage);
        }
        else if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
        {
            ALIMER_LOGINFO("[Vulkan] - [{}] Code {} : {}", pLayerPrefix, messageCode, pMessage);
        }
        else if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
        {
            ALIMER_LOGDEBUG("[{}] Code {} : {}", pLayerPrefix, messageCode, pMessage);
        }
        else
        {
            ALIMER_LOGINFO("{}: {}", pLayerPrefix, pMessage);
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

    VulkanGraphics::VulkanGraphics(bool validation, const string& applicationName)
        : Graphics(GraphicsDeviceType::Vulkan, validation)
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
                ALIMER_LOGDEBUG("Loader/Runtime support detected for Vulkan %d.%d.%d",
                    VK_VERSION_MAJOR(checkApiVersion),
                    VK_VERSION_MINOR(checkApiVersion),
                    VK_VERSION_PATCH(checkApiVersion));

                apiVersion = checkApiVersion;
            }
        }

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = applicationName.c_str();
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
        if (validation)
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

        // Enumerate physical devices.
        uint32_t gpuCount = 0;
        vkThrowIfFailed(vkEnumeratePhysicalDevices(_instance, &gpuCount, nullptr));
        ALIMER_ASSERT(gpuCount > 0);

        vector<VkPhysicalDevice> physicalDevices(gpuCount);
        vkEnumeratePhysicalDevices(_instance, &gpuCount, physicalDevices.data());

        std::map<int, VkPhysicalDevice> physicalDevicesRated;
        for (const VkPhysicalDevice& physicalDevice : physicalDevices)
        {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(physicalDevice, &properties);
            VkPhysicalDeviceFeatures features;
            vkGetPhysicalDeviceFeatures(physicalDevice, &features);
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
            physicalDevicesRated[score] = physicalDevice;
        }

        // Take the first device from rated devices that support our queue requirements
        for (const auto& physicalDeviceRated : physicalDevicesRated)
        {
            VkPhysicalDevice physicalDevice = physicalDeviceRated.second;
            _adapters.push_back(new VulkanGpuAdapter(physicalDevice));
        }

        // Setup debug callback
        if (validation && hasValidationLayer)
        {
            VkDebugReportCallbackCreateInfoEXT debugCreateInfo = { VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT };
            debugCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
            debugCreateInfo.pfnCallback = VkDebugCallback;
            debugCreateInfo.pUserData = nullptr;
            result = vkCreateDebugReportCallbackEXT(_instance, &debugCreateInfo, nullptr, &_debugCallback);
        }
    }

    VulkanGraphics::~VulkanGraphics()
    {
        Finalize();
    }

    void VulkanGraphics::Finalize()
    {
        WaitIdle();
        Graphics::Finalize();

        // Destroy main swap chain.
        SafeDelete(_swapChain);

        for (auto& it : _renderPassCache)
        {
            vkDestroyRenderPass(_logicalDevice, it.second, nullptr);
        }
        _renderPassCache.clear();

        _descriptorSetAllocators.clear();
        _pipelineLayouts.clear();

        vkDestroyPipelineCache(_logicalDevice, _pipelineCache, nullptr);

        // Destroy default command buffer.
        SafeDelete(_defaultCommandBuffer);

        // Destroy default command pool.
        if (_commandPool != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(_logicalDevice, _commandPool, nullptr);
            _commandPool = VK_NULL_HANDLE;
        }

        vkDestroySemaphore(_logicalDevice, _semaphores.presentComplete, nullptr);
        vkDestroySemaphore(_logicalDevice, _semaphores.renderComplete, nullptr);
        vkDestroyFence(_logicalDevice, _frameFence, nullptr);

        // Destroy memory allocator.
        vmaDestroyAllocator(_allocator);

        // Destroy logical device.
        if (_logicalDevice != VK_NULL_HANDLE)
        {
            vkDestroyDevice(_logicalDevice, nullptr);
            _logicalDevice = VK_NULL_HANDLE;
        }

        if (_debugCallback != VK_NULL_HANDLE)
        {
            vkDestroyDebugReportCallbackEXT(_instance, _debugCallback, nullptr);
            _debugCallback = VK_NULL_HANDLE;
        }

        vkDestroyInstance(_instance, nullptr);
        _instance = VK_NULL_HANDLE;
    }

    void VulkanGraphics::WaitIdle()
    {
        VkResult result = vkDeviceWaitIdle(_logicalDevice);
        if (result < VK_SUCCESS)
        {
            ALIMER_LOGERROR("vkDeviceWaitIdle failed");
        }
    }

    bool VulkanGraphics::BackendInitialize()
    {
        auto vkGpuAdapter = static_cast<VulkanGpuAdapter*>(_adapter);
        _vkPhysicalDevice = vkGpuAdapter->GetVkHandle();

        vkGetPhysicalDeviceMemoryProperties(_vkPhysicalDevice, &_deviceMemoryProperties);

        // Queue props.
        uint32_t queueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(_vkPhysicalDevice, &queueFamilyCount, nullptr);
        _queueFamilyProperties.resize(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(_vkPhysicalDevice, &queueFamilyCount, _queueFamilyProperties.data());

        // Enumerate device extensions.
        uint32_t extCount = 0;
        if (vkEnumerateDeviceExtensionProperties(_vkPhysicalDevice, nullptr, &extCount, nullptr) != VK_SUCCESS)
            ALIMER_LOGCRITICAL("Vulkan: Failed to query device extensions");

        vector<VkExtensionProperties> extensions(extCount);
        vkEnumerateDeviceExtensionProperties(_vkPhysicalDevice, nullptr, &extCount, extensions.data());

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
                    ALIMER_LOGDEBUG("Optional Vulkan extension {} not supported", queryDeviceExtension.name);
                    break;

                case VkExtensionType::Desired:
                    ALIMER_LOGWARN("Vulkan extension {} not supported", queryDeviceExtension.name);
                    break;

                case VkExtensionType::Required:
                    requiredExtensionsEnabled = false;
                    ALIMER_LOGERROR("Required Vulkan extension {} not supported", queryDeviceExtension.name);
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

        // Now create logical device.
        vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

        const float defaultQueuePriority(0.0f);

        // Graphics queue.
        const VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;
        if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT) {
            _queueFamilyIndices.graphics = GetQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
            VkDeviceQueueCreateInfo queueInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
            queueInfo.queueFamilyIndex = _queueFamilyIndices.graphics;
            queueInfo.queueCount = 1;
            queueInfo.pQueuePriorities = &defaultQueuePriority;
            queueCreateInfos.push_back(queueInfo);
        }
        else {
            _queueFamilyIndices.graphics = static_cast<uint32_t>(-1);
        }

        // Dedicated compute queue
        if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT) {
            _queueFamilyIndices.compute = GetQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);
            if (_queueFamilyIndices.compute != _queueFamilyIndices.graphics) {
                // If compute family index differs, we need an additional queue create info for the compute queue
                VkDeviceQueueCreateInfo queueInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
                queueInfo.queueFamilyIndex = _queueFamilyIndices.compute;
                queueInfo.queueCount = 1;
                queueInfo.pQueuePriorities = &defaultQueuePriority;
                queueCreateInfos.push_back(queueInfo);
            }
        }
        else {
            // Else we use the same queue
            _queueFamilyIndices.compute = _queueFamilyIndices.graphics;
        }

        // Create the logical device representation
        VkPhysicalDeviceFeatures gpuFeatures = vkGpuAdapter->GetFeatures();
        VkPhysicalDeviceFeatures enabledFeatures = {};
        if (gpuFeatures.samplerAnisotropy)
        {
            enabledFeatures.samplerAnisotropy = VK_TRUE;
        }

        VkDeviceCreateInfo deviceCreateInfo = {};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());;
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
        deviceCreateInfo.pEnabledFeatures = &enabledFeatures;

        if (deviceExtensions.size() > 0) {
            deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
            deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
        }

        VkResult result = vkCreateDevice(_vkPhysicalDevice, &deviceCreateInfo, nullptr, &_logicalDevice);
        vkThrowIfFailed(result);

        CreateAllocator();

        VkPipelineCacheCreateInfo pipelineCacheCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO };
        pipelineCacheCreateInfo.pNext = nullptr;
        pipelineCacheCreateInfo.flags = 0;
        vkThrowIfFailed(vkCreatePipelineCache(_logicalDevice, &pipelineCacheCreateInfo, nullptr, &_pipelineCache));

        // Get queue's.
        vkGetDeviceQueue(_logicalDevice, _queueFamilyIndices.graphics, 0, &_graphicsQueue);
        vkGetDeviceQueue(_logicalDevice, _queueFamilyIndices.compute, 0, &_computeQueue);

        // Create default command pool.
        _commandPool = CreateCommandPool(
            _queueFamilyIndices.graphics,
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
        );

        // Create default primary command buffer;
        _defaultCommandBuffer = new VulkanCommandBuffer(this, _commandPool, false);

        // Create the main swap chain.
        _swapChain = new VulkanSwapchain(this, _window.Get());

        // Create sync primitives
        VkSemaphoreCreateInfo semaphoreCreateInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

        // Create a semaphore used to synchronize image presentation
        // Ensures that the image is displayed before we start submitting new commands to the queu
        vkThrowIfFailed(vkCreateSemaphore(_logicalDevice, &semaphoreCreateInfo, nullptr, &_semaphores.presentComplete));

        // Create a semaphore used to synchronize command submission
        // Ensures that the image is not presented until all commands have been sumbitted and executed
        vkThrowIfFailed(vkCreateSemaphore(_logicalDevice, &semaphoreCreateInfo, nullptr, &_semaphores.renderComplete));

        // Create frame fence.
        VkFenceCreateInfo fenceCreateInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, 0 };
        vkCreateFence(_logicalDevice, &fenceCreateInfo, NULL, &_frameFence);

        return true;
    }

    void VulkanGraphics::CreateAllocator()
    {
        VmaAllocatorCreateInfo createInfo = {};
        createInfo.physicalDevice = _vkPhysicalDevice;
        createInfo.device = _logicalDevice;

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

    void VulkanGraphics::AddWaitSemaphore(VkSemaphore semaphore)
    {
    }

    bool VulkanGraphics::BeginFrame()
    {
        // Acquire the next image from the swap chain
        VkResult result = _swapChain->AcquireNextImage(_semaphores.presentComplete, &_swapchainImageIndex);
        // Recreate the swapchain if it's no longer compatible with the surface (OUT_OF_DATE) or no longer optimal for presentation (SUBOPTIMAL)
        if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR))
        {
            //WindowResize();
        }
        else {
            vkThrowIfFailed(result);
        }

        for (auto &allocator : _descriptorSetAllocators)
        {
            allocator.second->BeginFrame();
        }

        // Begin command buffer.
        _defaultCommandBuffer->Begin(nullptr);

        return true;
    }

    void VulkanGraphics::EndFrame()
    {
        _defaultCommandBuffer->End();

        VkCommandBuffer commandBuffer = _defaultCommandBuffer->GetVkCommandBuffer();

        // Submit command buffers.
        VkPipelineStageFlags submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.pNext = nullptr;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &_semaphores.presentComplete;
        submitInfo.pWaitDstStageMask = &submitPipelineStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &_semaphores.renderComplete;

        vkThrowIfFailed(vkQueueSubmit(_graphicsQueue, 1, &submitInfo, _frameFence));

        // Wait for fence to signal that all command buffers are ready,
        VkResult fenceRes;
        do {
            fenceRes = vkWaitForFences(_logicalDevice, 1, &_frameFence, VK_TRUE, 100000000);
        } while (fenceRes == VK_TIMEOUT);
        vkThrowIfFailed(fenceRes);
        vkResetFences(_logicalDevice, 1, &_frameFence);

        // Submit Swapchain.
        VkResult result = _swapChain->QueuePresent(_graphicsQueue, _swapchainImageIndex, _semaphores.renderComplete);
        if (!((result == VK_SUCCESS) || (result == VK_SUBOPTIMAL_KHR)))
        {
            if (result == VK_ERROR_OUT_OF_DATE_KHR)
            {
                // Swap chain is no longer compatible with the surface and needs to be recreated
                //WindowResize();
                return;
            }
            else {
                vkThrowIfFailed(result);
            }
        }

        // DestroyPendingResources();
    }

    CommandBuffer* VulkanGraphics::GetDefaultCommandBuffer() const
    {
        return _defaultCommandBuffer;
    }

    RenderPass* VulkanGraphics::GetBackbufferRenderPass() const
    {
        return _swapChain->GetRenderPass(_swapchainImageIndex);
    }

    SharedPtr<RenderPass> VulkanGraphics::CreateRenderPass(const RenderPassDescription& description)
    {
        return MakeShared<VulkanRenderPass>(this, description);
    }

    BufferHandle* VulkanGraphics::CreateBuffer(BufferUsageFlags usage, uint64_t size, uint32_t stride, ResourceUsage resourceUsage, const void* initialData)
    {
        return nullptr;
        //return new VulkanBuffer(this, description, initialData);
    }

    Shader* VulkanGraphics::CreateComputeShader(const void *pCode, size_t codeSize)
    {
        return new VulkanShader(this, pCode, codeSize);
    }

    Shader* VulkanGraphics::CreateShader(
        const void *pVertexCode, size_t vertexCodeSize,
        const void *pFragmentCode, size_t fragmentCodeSize)
    {
        return new VulkanShader(this,
            pVertexCode, vertexCodeSize,
            pFragmentCode, fragmentCodeSize
        );
    }

    PipelineState* VulkanGraphics::CreateRenderPipelineState(const RenderPipelineDescription& description)
    {
        return new VulkanPipelineState(this, description);
    }

    uint32_t VulkanGraphics::GetQueueFamilyIndex(VkQueueFlagBits queueFlags)
    {
        if (queueFlags & VK_QUEUE_COMPUTE_BIT)
        {
            for (uint32_t i = 0; i < static_cast<uint32_t>(_queueFamilyProperties.size()); i++)
            {
                if ((_queueFamilyProperties[i].queueFlags & queueFlags) && ((_queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
                {
                    return i;
                    break;
                }
            }
        }
        for (uint32_t i = 0; i < static_cast<uint32_t>(_queueFamilyProperties.size()); i++)
        {
            if (_queueFamilyProperties[i].queueFlags & queueFlags)
            {
                return i;
                break;
            }
        }

        ALIMER_LOGCRITICAL("Vulkan - Could not find a matching queue family index");
    }

    VkCommandPool VulkanGraphics::CreateCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags)
    {
        VkCommandPoolCreateInfo createInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
        createInfo.pNext = nullptr;
        createInfo.queueFamilyIndex = queueFamilyIndex;
        createInfo.flags = createFlags;

        VkCommandPool commandPool;
        vkThrowIfFailed(vkCreateCommandPool(_logicalDevice, &createInfo, nullptr, &commandPool));
        return commandPool;
    }


    VkCommandBuffer VulkanGraphics::CreateCommandBuffer(VkCommandBufferLevel level, bool begin)
    {
        VkCommandBufferAllocateInfo info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
        info.commandPool = _commandPool;
        info.level = level;
        info.commandBufferCount = 1;

        VkCommandBuffer vkCommandBuffer;
        vkThrowIfFailed(vkAllocateCommandBuffers(_logicalDevice, &info, &vkCommandBuffer));

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

        VkFence fence;
        vkThrowIfFailed(vkCreateFence(_logicalDevice, &fenceCreateInfo, nullptr, &fence));

        // Submit to the queue
        vkThrowIfFailed(vkQueueSubmit(queue, 1, &submitInfo, fence));

        // Wait for the fence to signal that command buffer has finished executing.
        vkThrowIfFailed(vkWaitForFences(_logicalDevice, 1, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max()));
        vkDestroyFence(_logicalDevice, fence, nullptr);

        if (free)
        {
            vkFreeCommandBuffers(_logicalDevice, _commandPool, 1, &commandBuffer);
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

    VkRenderPass VulkanGraphics::GetVkRenderPass(const RenderPassDescription& description)
    {
        Hasher renderPassHasher;

        for (uint32_t i = 0; i < MaxColorAttachments; i++)
        {
            const RenderPassAttachment& colorAttachment = description.colorAttachments[i];
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
            const RenderPassAttachment& colorAttachment = description.colorAttachments[i];
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

        if (description.depthStencilAttachment.texture)
        {
            attachments[attachmentCount].format = vk::Convert(description.depthStencilAttachment.texture->GetFormat());
            attachments[attachmentCount].samples = VK_SAMPLE_COUNT_1_BIT;
            attachments[attachmentCount].loadOp = vk::Convert(description.depthStencilAttachment.loadAction);
            attachments[attachmentCount].storeOp = vk::Convert(description.depthStencilAttachment.storeAction);
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
        VkResult result = vkCreateRenderPass(_logicalDevice, &createInfo, nullptr, &renderPass);
        if (result != VK_SUCCESS)
        {
            ALIMER_LOGERROR("Vulkan - Failed to create render pass.");
            return VK_NULL_HANDLE;
        }

        _renderPassCache[hash] = renderPass;
        return renderPass;
    }

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
}
