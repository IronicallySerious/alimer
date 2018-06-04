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

#include "../../Core/Log.h"
#include "../../Core/Window.h"
#include "VulkanGraphics.h"
#include "VulkanCommandBuffer.h"
#include "VulkanTexture.h"
#include "VulkanConvert.h"

#if defined(VK_USE_PLATFORM_WIN32_KHR)
#	include "../../Core/Windows/WindowWindows.h"
#endif

using namespace std;

namespace Alimer
{
    /*
    * A layer can expose extensions, keep track of those
    * extensions here.
    */
    struct LayerProperties {
        VkLayerProperties properties;
        std::vector<VkExtensionProperties> instance_extensions;
        std::vector<VkExtensionProperties> device_extensions;
    };

    static inline VkResult InitGlobalExtensionProperties(LayerProperties &layer_props)
    {
        VkExtensionProperties *instanceExtensions;
        uint32_t instanceExtensionCount;
        VkResult res;
        char *layer_name = nullptr;

        layer_name = layer_props.properties.layerName;

        do {
            res = vkEnumerateInstanceExtensionProperties(layer_name, &instanceExtensionCount, nullptr);
            if (res) return res;

            if (instanceExtensionCount == 0)
                return VK_SUCCESS;

            layer_props.instance_extensions.resize(instanceExtensionCount);
            instanceExtensions = layer_props.instance_extensions.data();
            res = vkEnumerateInstanceExtensionProperties(layer_name, &instanceExtensionCount, instanceExtensions);
        } while (res == VK_INCOMPLETE);

        return res;
    }

    static inline VkResult InitGlobalLayerProperties(vector<LayerProperties>& instance_layer_properties)
    {
        uint32_t instance_layer_count;
        VkLayerProperties *vk_props = NULL;
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

    VkBool32 CheckLayers(const vector<LayerProperties> &layer_props, const vector<const char *> &layer_names)
    {
        size_t checkCount = layer_names.size();
        size_t layerCount = layer_props.size();
        for (size_t i = 0; i < checkCount; i++) {
            VkBool32 found = 0;
            for (size_t j = 0; j < layerCount; j++) {
                if (!strcmp(layer_names[i], layer_props[j].properties.layerName)) {
                    found = 1;
                }
            }
            if (!found) {
                ALIMER_LOGDEBUG("[Vulkan] - Cannot find layer: %s", layer_names[i]);
                return 0;
            }
        }
        return 1;
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
        : Graphics(GraphicsDeviceType::Vulkan)
    {
        vector<LayerProperties> instanceLayerProperties;
        VkResult result = InitGlobalLayerProperties(instanceLayerProperties);

        uint32_t apiVersion = VK_API_VERSION_1_0;
        // Determine if the new instance version command is available
        if (vkEnumerateInstanceVersion != nullptr)
        {
            uint32_t checkApiVersion = 0;
            if (vkEnumerateInstanceVersion(&checkApiVersion) == VK_SUCCESS)
            {
                // Translate the version into major/minor for easier comparison
                uint32_t loader_major_version = VK_VERSION_MAJOR(checkApiVersion);
                uint32_t loader_minor_version = VK_VERSION_MINOR(checkApiVersion);
                ALIMER_LOGDEBUG("Loader/Runtime support detected for Vulkan %d.%d", loader_major_version, loader_minor_version);
                apiVersion = checkApiVersion;
            }
        }

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = applicationName.c_str();
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Alimer Engine";
        //appInfo.engineVersion = VK_MAKE_VERSION(ALIMER_VERSION_MAJOR, ALIMER_VERSION_MINOR, ALIMER_VERSION_PATCH);
        appInfo.engineVersion = VK_MAKE_VERSION(0, 9, 0);
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
        if (validation)
        {
            instanceExtensionNames.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

            instanceLayerNames.push_back("VK_LAYER_LUNARG_standard_validation");
            if (!CheckLayers(instanceLayerProperties, instanceLayerNames))
            {
                /* If standard validation is not present, search instead for the
                * individual layers that make it up, in the correct order.
                */
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
        if (validation)
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

        // Release all GPU objects
        Graphics::Finalize();

        // Destroy main swap chain.
        _swapchain.Reset();

        _commandBuffers.clear();

        //vkDestroyPipelineCache(_logicalDevice, pipelineCache, nullptr);
        vkDestroyCommandPool(_logicalDevice, _graphicsCommandPool, nullptr);
        vkDestroySemaphore(_logicalDevice, _imageAcquiredSemaphore, nullptr);
        vkDestroySemaphore(_logicalDevice, _renderCompleteSemaphore, nullptr);

        for (auto& fence : _waitFences) {
            vkDestroyFence(_logicalDevice, fence, nullptr);
        }
        _waitFences.clear();

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

    bool VulkanGraphics::WaitIdle()
    {
        VkResult result = vkDeviceWaitIdle(_logicalDevice);
        if (result < VK_SUCCESS)
            return false;

        return true;
    }

    bool VulkanGraphics::Initialize(const SharedPtr<Window>& window)
    {
        VkResult result = VK_SUCCESS;

        // Enumerate physical devices.
        uint32_t gpuCount = 0;
        // Get number of available physical devices
        vkThrowIfFailed(vkEnumeratePhysicalDevices(_instance, &gpuCount, nullptr));
        if (gpuCount > 0)
        {
            // Enumerate physical devices.
            std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
            result = vkEnumeratePhysicalDevices(_instance, &gpuCount, physicalDevices.data());

            //for (uint32_t i = 0; i < gpuCount; ++i)
            //{
            //	_physicalDevice.Push(new VulkanPhysicalDevice(physicalDevices[i]));
            //}

            _vkPhysicalDevice = physicalDevices.front();
        }

        // Store Properties features, limits and properties of the physical device for later use
        // Device properties also contain limits and sparse properties
        vkGetPhysicalDeviceProperties(_vkPhysicalDevice, &_deviceProperties);
        // Features should be checked by the examples before using them
        vkGetPhysicalDeviceFeatures(_vkPhysicalDevice, &_deviceFeatures);
        // Memory properties are used regularly for creating all kinds of buffers
        vkGetPhysicalDeviceMemoryProperties(_vkPhysicalDevice, &_deviceMemoryProperties);
        // Queue family properties, used for setting up requested queues upon device creation
        uint32_t queueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(_vkPhysicalDevice, &queueFamilyCount, nullptr);
        assert(queueFamilyCount > 0);
        _queueFamilyProperties.resize(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(_vkPhysicalDevice, &queueFamilyCount, _queueFamilyProperties.data());

        // Now create logical device.
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

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
        std::vector<const char*> deviceExtensions;
        deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        VkPhysicalDeviceFeatures enabledFeatures{};
        if (_deviceFeatures.samplerAnisotropy) {
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

        result = vkCreateDevice(_vkPhysicalDevice, &deviceCreateInfo, nullptr, &_logicalDevice);
        vkThrowIfFailed(result);

        // Get queue's.
        vkGetDeviceQueue(_logicalDevice, _queueFamilyIndices.graphics, 0, &_graphicsQueue);
        vkGetDeviceQueue(_logicalDevice, _queueFamilyIndices.compute, 0, &_computeQueue);


        // Create default command pool.
        VkCommandPoolCreateInfo cmdPoolInfo = {
            VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            nullptr,
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            _queueFamilyIndices.graphics
        };
        vkThrowIfFailed(vkCreateCommandPool(_logicalDevice, &cmdPoolInfo, nullptr, &_graphicsCommandPool));

        // Create the main swap chain.
        _swapchain.Reset(new VulkanSwapchain(this, window));

        // Allocate vulkan command buffers.
        const uint32_t imageCount = _swapchain->GetImageCount();
        std::vector<VkCommandBuffer> vkCommandBuffers(imageCount);
        VkCommandBufferAllocateInfo cmdBufAllocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
        cmdBufAllocateInfo.pNext = nullptr;
        cmdBufAllocateInfo.commandPool = _graphicsCommandPool;
        cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmdBufAllocateInfo.commandBufferCount = imageCount;
        vkThrowIfFailed(vkAllocateCommandBuffers(_logicalDevice, &cmdBufAllocateInfo, vkCommandBuffers.data()));
        _commandBuffers.resize(imageCount);
        _textures.resize(imageCount);

        for (uint32_t i = 0; i < imageCount; ++i)
        {
            _textures[i] = MakeShared<VulkanTexture>(this, _swapchain->GetImage(i));
            _commandBuffers[i] = MakeShared<VulkanCommandBuffer>(this, _graphicsCommandPool, vkCommandBuffers[i]);
        }

        // Create sync primitives.
        VkSemaphoreCreateInfo semaphoreCreateInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0 };
        vkThrowIfFailed(vkCreateSemaphore(_logicalDevice, &semaphoreCreateInfo, nullptr, &_imageAcquiredSemaphore));
        vkThrowIfFailed(vkCreateSemaphore(_logicalDevice, &semaphoreCreateInfo, nullptr, &_renderCompleteSemaphore));

        VkFenceCreateInfo fenceCreateInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
        // Create in signaled state so we don't wait on first render of each command buffer
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        _waitFences.resize(_swapchain->GetImageCount());
        for (auto& fence : _waitFences)
        {
            vkThrowIfFailed(vkCreateFence(_logicalDevice, &fenceCreateInfo, nullptr, &fence));
        }

        return Graphics::Initialize(window);
    }

    SharedPtr<Texture> VulkanGraphics::AcquireNextImage()
    {
        // Acquire the next image from the swap chain
        VkResult result = _swapchain->AcquireNextImage(_imageAcquiredSemaphore, &_swapchainImageIndex);

        if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR)) {
            //WindowResize();
        }
        else {
            vkThrowIfFailed(result);
        }

        vkThrowIfFailed(vkWaitForFences(_logicalDevice, 1, &_waitFences[_swapchainImageIndex], VK_TRUE, UINT64_MAX));
        vkThrowIfFailed(vkResetFences(_logicalDevice, 1, &_waitFences[_swapchainImageIndex]));

        //DestroyPendingResources();

        return _textures[_swapchainImageIndex];
    }

    bool VulkanGraphics::Present()
    {
        // Submit frame command buffer.
        _commandBuffers[_swapchainImageIndex]->End();
        SubmitCommandBuffer(_commandBuffers[_swapchainImageIndex]);

        VkResult result = _swapchain->QueuePresent(
            _graphicsQueue,
            _swapchainImageIndex,
            _renderCompleteSemaphore);
        if (result < VK_SUCCESS)
        {
            return false;
        }

        return true;
    }

    SharedPtr<CommandBuffer> VulkanGraphics::GetCommandBuffer()
    {
        // Init current command buffer.
        _commandBuffers[_swapchainImageIndex]->Begin();

        return _commandBuffers[_swapchainImageIndex];
    }

    GpuBufferPtr VulkanGraphics::CreateBuffer(BufferUsage usage, uint32_t elementCount, uint32_t elementSize, const void* initialData)
    {
        return nullptr;
    }

    PipelineLayoutPtr VulkanGraphics::CreatePipelineLayout()
    {
        return nullptr;
    }

    std::shared_ptr<Shader> VulkanGraphics::CreateShader(const std::string& name)
    {
        return nullptr;
    }

    std::shared_ptr<Shader> VulkanGraphics::CreateShader(const ShaderBytecode& vertex, const ShaderBytecode& fragment)
    {
        return nullptr;
    }

    PipelineStatePtr VulkanGraphics::CreateRenderPipelineState(const RenderPipelineDescriptor& descriptor)
    {
        return nullptr;
    }

    bool VulkanGraphics::PrepareDraw(PrimitiveTopology topology)
    {
        return true;
    }

    VkCommandBuffer VulkanGraphics::CreateCommandBuffer(VkCommandBufferLevel level, bool begin)
    {
        VkCommandBufferAllocateInfo info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
        info.commandPool = _graphicsCommandPool;
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

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
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
            vkFreeCommandBuffers(_logicalDevice, _graphicsCommandPool, 1, &commandBuffer);
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

    void VulkanGraphics::SubmitCommandBuffer(VulkanCommandBuffer* commandBuffer)
    {
        auto vkCommandBuffer = commandBuffer->GetVkCommandBuffer();
        const VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.pWaitDstStageMask = &waitDstStageMask;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &_imageAcquiredSemaphore;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &_renderCompleteSemaphore;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &vkCommandBuffer;
        vkThrowIfFailed(vkQueueSubmit(_graphicsQueue, 1, &submitInfo, _waitFences[_swapchainImageIndex]));
    }

    VkRenderPass VulkanGraphics::GetVkRenderPass(const RenderPassDescriptor& descriptor, uint64_t hash)
    {
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
            const RenderPassColorAttachmentDescriptor& attachment = descriptor.colorAttachments[i];
            Texture* texture = attachment.texture;
            if (!texture)
                continue;

            attachments[attachmentCount].format = vk::Convert(texture->GetFormat());
            attachments[attachmentCount].samples = VK_SAMPLE_COUNT_1_BIT;
            attachments[attachmentCount].loadOp = vk::Convert(attachment.loadAction);
            attachments[attachmentCount].storeOp = vk::Convert(attachment.storeAction);
            attachments[attachmentCount].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[attachmentCount].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[attachmentCount].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[attachmentCount].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            colorReferences.push_back({ attachmentCount, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

            attachmentCount++;
        }

        if (descriptor.depthAttachment.texture
            || descriptor.stencilAttachment.texture)
        {
            attachments[attachmentCount].format = vk::Convert(descriptor.depthAttachment.texture->GetFormat());
            attachments[attachmentCount].samples = VK_SAMPLE_COUNT_1_BIT;
            attachments[attachmentCount].loadOp = vk::Convert(descriptor.depthAttachment.loadAction);
            attachments[attachmentCount].storeOp = vk::Convert(descriptor.depthAttachment.storeAction);
            attachments[attachmentCount].stencilLoadOp = vk::Convert(descriptor.stencilAttachment.loadAction);
            attachments[attachmentCount].stencilStoreOp = vk::Convert(descriptor.stencilAttachment.storeAction);
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
        vkThrowIfFailed(result);
        _renderPassCache[hash] = renderPass;
        return renderPass;
    }
}
