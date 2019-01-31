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

#if defined(_WIN32)
#    ifndef VK_USE_PLATFORM_WIN32_KHR
#        define VK_USE_PLATFORM_WIN32_KHR 1
#    endif
#   ifndef NOMINMAX
#       define NOMINMAX
#   endif
#   define WIN32_LEAN_AND_MEAN
#elif defined(__ANDROID__)
#    define VK_USE_PLATFORM_ANDROID_KHR 1
#elif defined(__linux__)
#    ifdef VGPU_LINUX_WAYLAND
#        define VK_USE_PLATFORM_WAYLAND_KHR 1
#    else
#        define VK_USE_PLATFORM_XLIB_KHR 1
#    endif
#endif

#include "volk/volk.h"
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc/vk_mem_alloc.h>
//#include <spirv-cross/spirv_hlsl.hpp>

#if !defined(VGPU_DEBUG) && !defined(NDEBUG)
#   define VGPU_DEBUG 1
#endif

#ifndef VGPU_ASSERT
#   include <assert.h>
#   define VGPU_ASSERT(c) assert(c)
#endif

#ifndef VGPU_ALLOC
#   include <stdlib.h>
#   include <malloc.h>
#   define VGPU_ALLOC(type) ((type*) malloc(sizeof(type)))
#   define VGPU_ALLOCN(type, n) ((type*) malloc(sizeof(type) * n))
#   define VGPU_FREE(ptr) free(ptr)
#   define VGPU_ALLOC_HANDLE(type) ((type) malloc(sizeof(type##_T)))
#endif 

#define _vgpu_min(a,b) ((a<b)?a:b)
#define _vgpu_max(a,b) ((a>b)?a:b)
#define _vgpu_clamp(v,v0,v1) ((v<v0)?(v0):((v>v1)?(v1):(v)))

/* Vulkan only handles (ATM) */
VGPU_DEFINE_HANDLE(VgpuPhysicalDevice);
VGPU_DEFINE_HANDLE(VgpuCommandQueue);

/* Handle declaration */
typedef struct VgpuPhysicalDevice_T {
    VkPhysicalDevice                        device;
    VkPhysicalDeviceProperties              properties;
    VkPhysicalDeviceMemoryProperties        memoryProperties;
    VkPhysicalDeviceFeatures                features;
    uint32_t                                queueFamilyCount;
    VkQueueFamilyProperties*                queueFamilyProperties;
    uint32_t                                extensionsCount;
    VkExtensionProperties*                  extensions;
} VgpuPhysicalDevice_T;

typedef struct VgpuCommandQueue_T {
    VkQueue                                 vk_queue;
    uint32_t                                vk_queueFamilyIndex;
    VkCommandPool                           vk_handle;
} VgpuCommandQueue_T;

typedef struct VgpuCommandBuffer_T {
    VgpuCommandQueue                        queue;
    VkCommandBuffer                         vk_handle;
} VgpuCommandBuffer_T;

typedef struct VgpuSwapchain_T {
    VkSurfaceKHR                            surface;
    VkSwapchainKHR                          swapchain;
    uint32_t                                imageIndex;  // index of currently acquired image.
    uint32_t                                imageCount;
    VkImage*                                images;
    VkSemaphore*                            imageSemaphores;
} VgpuSwapchain_T;

typedef struct VgpuTexture_T {
    VkImage                                 image;
    VkFormat                                format;
} VgpuTexture_T;

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

struct VgpuDeviceFeaturesVk
{
    bool supportsPhysicalDeviceProperties2 = false;
    bool supportsExternal = false;
    bool supportsDedicated = false;
    bool supportsImageFormatList = false;
    bool supportsDebugMarker = false;
    bool supportsDebugUtils = false;
    bool supportsMirrorClampToEdge = false;
    bool supportsGoogleDisplayTiming = false;
    VkPhysicalDeviceSubgroupProperties subgroupProperties = {};
    VkPhysicalDevice8BitStorageFeaturesKHR storage8bitFeatures = {};
    VkPhysicalDevice16BitStorageFeaturesKHR storage_16bitFeatures = {};
    VkPhysicalDeviceFloat16Int8FeaturesKHR float16Int8Features = {};
    VkPhysicalDeviceFeatures enabledFeatures = {};
};

static VgpuPhysicalDevice vgpuVkAllocateDevice(VkPhysicalDevice physicalDevice)
{
    VgpuPhysicalDevice handle = VGPU_ALLOC_HANDLE(VgpuPhysicalDevice);
    handle->device = physicalDevice;
    vkGetPhysicalDeviceProperties(physicalDevice, &handle->properties);
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &handle->memoryProperties);
    vkGetPhysicalDeviceFeatures(physicalDevice, &handle->features);

    // Queue family properties, used for setting up requested queues upon device creation
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &handle->queueFamilyCount, nullptr);
    VGPU_ASSERT(handle->queueFamilyCount > 0);
    handle->queueFamilyProperties = (VkQueueFamilyProperties*)calloc(handle->queueFamilyCount, sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &handle->queueFamilyCount, handle->queueFamilyProperties);

    // Get list of supported extensions
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &handle->extensionsCount, nullptr);
    if (handle->extensionsCount > 0)
    {
        handle->extensions = (VkExtensionProperties*)calloc(handle->extensionsCount, sizeof(VkExtensionProperties));
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &handle->extensionsCount, handle->extensions);
    }

    return handle;
}

static void vgpuVkFreeDevice(VgpuPhysicalDevice handle)
{
    VGPU_ASSERT(handle);
    free(handle->queueFamilyProperties);
    free(handle->extensions);
}

static bool vgpuVkIsExtensionSupported(VgpuPhysicalDevice handle, const char* extension)
{
    VGPU_ASSERT(handle);

    for (uint32_t i = 0; i < handle->extensionsCount; ++i)
    {
        if (strcmp(handle->extensions[i].extensionName, extension) == 0)
        {
            return true;
        }
    }

    return false;
}

// Put an image memory barrier for setting an image layout on the sub resource into the given command buffer
static void vgpuVkTransitionImageLayout(
    VkCommandBuffer commandBuffer,
    VkImage image,
    VkImageLayout oldImageLayout,
    VkImageLayout newImageLayout,
    VkImageSubresourceRange subresourceRange,
    VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
    VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT)
{
    // Create an image barrier object
    VkImageMemoryBarrier imageMemoryBarrier{};
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.oldLayout = oldImageLayout;
    imageMemoryBarrier.newLayout = newImageLayout;
    imageMemoryBarrier.image = image;
    imageMemoryBarrier.subresourceRange = subresourceRange;

    // Source layouts (old)
    // Source access mask controls actions that have to be finished on the old layout
    // before it will be transitioned to the new layout
    switch (oldImageLayout)
    {
    case VK_IMAGE_LAYOUT_UNDEFINED:
        // Image layout is undefined (or does not matter)
        // Only valid as initial layout
        // No flags required, listed only for completeness
        imageMemoryBarrier.srcAccessMask = 0;
        break;

    case VK_IMAGE_LAYOUT_PREINITIALIZED:
        // Image is preinitialized
        // Only valid as initial layout for linear images, preserves memory contents
        // Make sure host writes have been finished
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        // Image is a color attachment
        // Make sure any writes to the color buffer have been finished
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        // Image is a depth/stencil attachment
        // Make sure any writes to the depth/stencil buffer have been finished
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        // Image is a transfer source 
        // Make sure any reads from the image have been finished
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        // Image is a transfer destination
        // Make sure any writes to the image have been finished
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        // Image is read by a shader
        // Make sure any shader reads from the image have been finished
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        break;
    default:
        // Other source layouts aren't handled (yet)
        break;
    }

    // Target layouts (new)
    // Destination access mask controls the dependency for the new image layout
    switch (newImageLayout)
    {
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        // Image will be used as a transfer destination
        // Make sure any writes to the image have been finished
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        // Image will be used as a transfer source
        // Make sure any reads from the image have been finished
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        // Image will be used as a color attachment
        // Make sure any writes to the color buffer have been finished
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        // Image layout will be used as a depth/stencil attachment
        // Make sure any writes to depth/stencil buffer have been finished
        imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        // Image will be read in a shader (sampler, input attachment)
        // Make sure any writes to the image have been finished
        if (imageMemoryBarrier.srcAccessMask == 0)
        {
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
        }
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        break;
    default:
        // Other source layouts aren't handled (yet)
        break;
    }

    // Put barrier inside setup command buffer
    vkCmdPipelineBarrier(
        commandBuffer,
        srcStageMask,
        dstStageMask,
        0,
        0, nullptr,
        0, nullptr,
        1, &imageMemoryBarrier);
}

// Uses a fixed sub resource layout with first mip level and layer
static void vgpuVkTransitionImageLayout(
    VkCommandBuffer commandBuffer,
    VkImage image,
    VkImageAspectFlags aspectMask,
    VkImageLayout oldImageLayout,
    VkImageLayout newImageLayout,
    VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
    VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT)
{
    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = aspectMask;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.layerCount = 1;
    vgpuVkTransitionImageLayout(commandBuffer, image, oldImageLayout, newImageLayout, subresourceRange, srcStageMask, dstStageMask);
}

static void  vgpuVkClearImageWithColor(
    VkCommandBuffer commandBuffer, 
    VkImage image, 
    VkImageSubresourceRange range,
    VkImageAspectFlags aspect, 
    VkImageLayout sourceLayout,
    VkImageLayout destLayout,
    VkClearColorValue *clearValue)
{
    // Transition to destination layout.
    vgpuVkTransitionImageLayout(commandBuffer, image, aspect, sourceLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

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
    vgpuVkTransitionImageLayout(commandBuffer, image, aspect, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, destLayout);
}

struct VgpuRendererVk
{
public:
    VgpuRendererVk();
    ~VgpuRendererVk();

    void shutdown();
    VgpuResult initialize(const char* applicationName, const VgpuDescriptor* descriptor);
    VgpuResult waitIdle();
    VgpuResult beginFrame();
    VgpuResult endFrame();

    VkResult initializeExtensionsAndLayers();
    VkResult createInstance(const char* applicationName, bool validation, bool headless);
    VkResult detectPhysicalDevice(VgpuDevicePreference preference);
    VkResult createDevice();
    void initializeFeatures();

    /* Swapchain */
    VkResult createSurface(const VgpuSwapchainDescriptor* descriptor, VkSurfaceKHR* pSurface);
    VgpuResult createSwapchain(const VgpuSwapchainDescriptor* descriptor, VkSwapchainKHR oldSwapchain, VkSurfaceKHR surface, VgpuSwapchain* pSwapchain);
    void destroySwapchain(VgpuSwapchain swapchain);
    VgpuResult present(VgpuSwapchain swapchain, VkQueue queue);

    /* CommandQueue */
    VgpuResult createCommandQueue(VkQueue queue, uint32_t queueFamilyIndex, VgpuCommandQueue* pCommandQueue);
    void destoryCommandQueue(VgpuCommandQueue queue);

    /* CommandBuffer */
    VgpuResult createCommandBuffer(VgpuCommandQueue queue, VkCommandBufferLevel level, VgpuCommandBuffer* pCommandBuffer);
    VgpuResult beginCommandBuffer(VgpuCommandBuffer commandBuffer);
    VgpuResult endCommandBuffer(VgpuCommandBuffer commandBuffer);
    VgpuResult commitCommandBuffer(VgpuCommandBuffer commandBuffer, bool waitUntilCompleted);

private:
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
    bool                            KHR_wayland_surface;
#endif

    /* Layers */
    bool                            VK_LAYER_LUNARG_standard_validation;
    bool                            VK_LAYER_RENDERDOC_Capture;

    VkInstance                              _instance = VK_NULL_HANDLE;
    VkDebugReportCallbackEXT                _debugCallback = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT                _debugMessenger = VK_NULL_HANDLE;
    VgpuPhysicalDevice                      _physicalDevice = VK_NULL_HANDLE;
    VgpuDeviceFeaturesVk                    _featuresVk;
    uint32_t                                _graphicsQueueFamily = VK_QUEUE_FAMILY_IGNORED;
    uint32_t                                _computeQueueFamily = VK_QUEUE_FAMILY_IGNORED;
    uint32_t                                _transferQueueFamily = VK_QUEUE_FAMILY_IGNORED;
    VkDevice                                _device = VK_NULL_HANDLE;
    VkQueue                                 _graphicsQueue = VK_NULL_HANDLE;
    VkQueue                                 _computeQueue = VK_NULL_HANDLE;
    VkQueue                                 _transferQueue = VK_NULL_HANDLE;
    VmaAllocator                            _memoryAllocator = VK_NULL_HANDLE;
    VgpuCommandQueue                        _graphicsCommandQueue = VK_NULL_HANDLE;
    VgpuCommandQueue                        _computeCommandQueue = VK_NULL_HANDLE;
    VgpuCommandQueue                        _copyCommandQueue = VK_NULL_HANDLE;
    VkSurfaceKHR                            _mainWindowSurface = VK_NULL_HANDLE;
    VgpuSwapchain                           _mainSwapchain = VK_NULL_HANDLE;
    uint32_t                                _maxInflightFrames;
    uint32_t                                _frameIndex;
    uint32_t                                _frameNumber;
    VkFence*                                _waitFences;
};

VgpuRendererVk::VgpuRendererVk()
{

}
VgpuRendererVk::~VgpuRendererVk()
{

}

void VgpuRendererVk::shutdown()
{
    vkDeviceWaitIdle(_device);

    // Delete swap chain if created.
    if (_mainSwapchain != nullptr)
    {
        destroySwapchain(_mainSwapchain);
        _mainSwapchain = nullptr;
    }

    /*for (uint32_t i = 0; i < RenderLatency; i++)
    {
        vkDestroyFence(_device, _waitFences[i], nullptr);
        vkDestroySemaphore(_device, _renderCompleteSemaphores[i], nullptr);
        vkDestroySemaphore(_device, _presentCompleteSemaphores[i], nullptr);
    }

    _commandBuffers.clear();*/
    destoryCommandQueue(_graphicsCommandQueue);
    destoryCommandQueue(_computeCommandQueue);
    destoryCommandQueue(_copyCommandQueue);

    if (_device != VK_NULL_HANDLE)
    {
        vkDestroyDevice(_device, nullptr);
        _device = VK_NULL_HANDLE;
    }

    if (_memoryAllocator != VK_NULL_HANDLE)
    {
        vmaDestroyAllocator(_memoryAllocator);
        _memoryAllocator = VK_NULL_HANDLE;
    }

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

    vkDestroyInstance(_instance, nullptr);
    vgpuVkFreeDevice(_physicalDevice);
}

VgpuResult VgpuRendererVk::initialize(const char* applicationName, const VgpuDescriptor* descriptor)
{
    if (volkInitialize() != VK_SUCCESS)
    {
        return VGPU_ERROR_INITIALIZATION_FAILED;
    }

    VkResult result = initializeExtensionsAndLayers();
    if (result != VK_SUCCESS)
    {
        return vgpuVkConvertResult(result);
    }

    result = createInstance(applicationName, descriptor->validation, descriptor->swapchain == nullptr);
    if (result != VK_SUCCESS)
    {
        return vgpuVkConvertResult(result);
    }

    result = detectPhysicalDevice(descriptor->devicePreference);
    if (result != VK_SUCCESS)
    {
        return vgpuVkConvertResult(result);
    }

    result = createSurface(descriptor->swapchain, &_mainWindowSurface);
    if (result != VK_SUCCESS)
    {
        return vgpuVkConvertResult(result);
    }

    result = createDevice();
    if (result != VK_SUCCESS)
    {
        return vgpuVkConvertResult(result);
    }

    // Create command queue's.
    createCommandQueue(_graphicsQueue, _graphicsQueueFamily, &_graphicsCommandQueue);
    createCommandQueue(_computeQueue, _computeQueueFamily, &_computeCommandQueue);
    createCommandQueue(_transferQueue, _transferQueueFamily, &_copyCommandQueue);

    initializeFeatures();

    if (descriptor->swapchain != nullptr)
    {
        createSwapchain(
            descriptor->swapchain,
            _mainSwapchain != nullptr ? _mainSwapchain->swapchain : VK_NULL_HANDLE,
            _mainWindowSurface,
            &_mainSwapchain);
    }

    // Create per frame resources
    _frameIndex = 0;
    _frameNumber = 0;
    _maxInflightFrames = _mainSwapchain != nullptr ? _mainSwapchain->imageCount : 3u;
    _waitFences = VGPU_ALLOCN(VkFence, _maxInflightFrames);
    for (uint32_t i = 0u; i < _maxInflightFrames; i++)
    {
        VkFenceCreateInfo fenceCreateInfo;
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.pNext = nullptr;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        result = vkCreateFence(_device, &fenceCreateInfo, nullptr, &_waitFences[i]);
        if (result != VK_SUCCESS)
        {
            return vgpuVkConvertResult(result);
        }
    }

    return VGPU_SUCCESS;
}

VkResult VgpuRendererVk::initializeExtensionsAndLayers()
{
    uint32_t i, count, layerCount;
    VkExtensionProperties* queriedExtensions;
    VkLayerProperties* queriedLayers;

    VkResult result = vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    if (result < VK_SUCCESS)
    {
        return result;
    }

    queriedExtensions = (VkExtensionProperties*)calloc(count, sizeof(VkExtensionProperties));
    result = vkEnumerateInstanceExtensionProperties(nullptr, &count, queriedExtensions);
    if (result < VK_SUCCESS)
    {
        free(queriedExtensions);
        return result;
    }

    result = vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    if (result < VK_SUCCESS)
    {
        return result;
    }

    queriedLayers = (VkLayerProperties*)calloc(layerCount, sizeof(VkLayerProperties));
    result = vkEnumerateInstanceLayerProperties(&layerCount, queriedLayers);
    if (result < VK_SUCCESS)
    {
        free(queriedLayers);
        return result;
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

    // Free allocated stuff
    free(queriedExtensions);
    free(queriedLayers);

    return VK_SUCCESS;
}

VkResult VgpuRendererVk::createInstance(const char* applicationName, bool validation, bool headless)
{
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

    uint32_t instanceLayersCount = 0;
    uint32_t instanceExtensionsCount = 0;
    const char* instanceLayers[10];
    const char* instanceExtensions[20];

    if (KHR_get_physical_device_properties2)
    {
        instanceExtensions[instanceExtensionsCount++] = VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME;
    }

    if (KHR_get_physical_device_properties2
        && KHR_external_memory_capabilities
        && KHR_external_semaphore_capabilities)
    {
        instanceExtensions[instanceExtensionsCount++] = VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME;
        instanceExtensions[instanceExtensionsCount++] = VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME;
        instanceExtensions[instanceExtensionsCount++] = VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME;
    }

    if (EXT_debug_utils)
    {
        instanceExtensions[instanceExtensionsCount++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    }

#ifdef VGPU_DEBUG
    if (validation)
    {
        if (!EXT_debug_utils && EXT_debug_report)
        {
            instanceExtensions[instanceExtensionsCount++] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
        }

        bool force_no_validation = false;
        /*if (getenv("VGPU_VULKAN_NO_VALIDATION"))
        {
            force_no_validation = true;
        }*/

        if (!force_no_validation && VK_LAYER_LUNARG_standard_validation)
        {
            instanceLayers[instanceLayersCount++] = "VK_LAYER_LUNARG_standard_validation";
        }
    }
#endif

    if (!headless
        && KHR_surface)
    {
        instanceExtensions[instanceExtensionsCount++] = VK_KHR_SURFACE_EXTENSION_NAME;

#if defined(_WIN32) || defined(_WIN64)
        if (KHR_win32_surface)
        {
            instanceExtensions[instanceExtensionsCount++] = "VK_KHR_win32_surface";
        }
#elif defined(__ANDROID__)
        if (KHR_android_surface)
        {
            instanceExtensions[instanceExtensionsCount++] = "VK_KHR_android_surface"
        }
#elif defined(__APPLE__)
        if (MVK_macos_surface)
        {
            instanceExtensions[instanceExtensionsCount++] = "VK_MVK_macos_surface";
        }
#elif defined(__linux__)
        if (strcmp(KHR_xlib_surface)
        {
            instanceExtensions[instanceExtensionsCount++] = "VK_KHR_xlib_surface";
        }
        else if (KHR_wayland_surface)
        {
            instanceExtensions[instanceExtensionsCount++] = "VK_KHR_wayland_surface";
        }
#endif
    }

    VkInstanceCreateInfo instanceCreateInfo;
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = nullptr;
    instanceCreateInfo.flags = 0;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledLayerCount = instanceLayersCount;
    instanceCreateInfo.ppEnabledLayerNames = instanceLayers;
    instanceCreateInfo.enabledExtensionCount = instanceExtensionsCount;
    instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions;
    VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &_instance);
    if (result != VK_SUCCESS)
    {
        return result;
    }

    volkLoadInstance(_instance);

    if (validation)
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
            vkCreateDebugUtilsMessengerEXT(_instance, &info, nullptr, &_debugMessenger);
        }
        else if (EXT_debug_report)
        {
            VkDebugReportCallbackCreateInfoEXT info = { VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT };
            info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
                VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
            info.pfnCallback = vgpuVkDebugCallback;
            info.pUserData = this;
            vkCreateDebugReportCallbackEXT(_instance, &info, nullptr, &_debugCallback);
        }
    }

    return result;
}

VkResult VgpuRendererVk::detectPhysicalDevice(VgpuDevicePreference preference)
{
    uint32_t gpuCount = 0;
    vkEnumeratePhysicalDevices(_instance, &gpuCount, nullptr);
    if (gpuCount == 0)
    {
        //ALIMER_LOGCRITICAL("Vulkan: No physical device detected");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    VkPhysicalDevice* physicalDevices = (VkPhysicalDevice*)alloca(gpuCount * sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(_instance, &gpuCount, physicalDevices);
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
            if (preference == VGPU_DEVICE_PREFERENCE_DISCRETE) {
                score += 1000U;
            }
            break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            score += 90U;
            if (preference == VGPU_DEVICE_PREFERENCE_INTEGRATED) {
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
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    _physicalDevice = vgpuVkAllocateDevice(physicalDevices[bestDeviceIndex]);
    return VK_SUCCESS;
}

VkResult VgpuRendererVk::createDevice()
{
    VkResult result = VK_SUCCESS;

    for (uint32_t i = 0; i < _physicalDevice->queueFamilyCount; i++)
    {
        VkBool32 supported = _mainWindowSurface == VK_NULL_HANDLE;
        if (_mainWindowSurface != VK_NULL_HANDLE)
        {
            vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice->device, i, _mainWindowSurface, &supported);
        }

        static const VkQueueFlags required = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT;
        if (supported
            && ((_physicalDevice->queueFamilyProperties[i].queueFlags & required) == required))
        {
            _graphicsQueueFamily = i;
            break;
        }
    }

    for (uint32_t i = 0; i < _physicalDevice->queueFamilyCount; i++)
    {
        static const VkQueueFlags required = VK_QUEUE_COMPUTE_BIT;
        if (i != _graphicsQueueFamily
            && (_physicalDevice->queueFamilyProperties[i].queueFlags & required) == required)
        {
            _computeQueueFamily = i;
            break;
        }
    }

    for (uint32_t i = 0; i < _physicalDevice->queueFamilyCount; i++)
    {
        static const VkQueueFlags required = VK_QUEUE_TRANSFER_BIT;
        if (i != _graphicsQueueFamily
            && i != _computeQueueFamily
            && (_physicalDevice->queueFamilyProperties[i].queueFlags & required) == required)
        {
            _transferQueueFamily = i;
            break;
        }
    }

    if (_transferQueueFamily == VK_QUEUE_FAMILY_IGNORED)
    {
        for (uint32_t i = 0; i < _physicalDevice->queueFamilyCount; i++)
        {
            static const VkQueueFlags required = VK_QUEUE_TRANSFER_BIT;
            if (i != _graphicsQueueFamily
                && (_physicalDevice->queueFamilyProperties[i].queueFlags & required) == required)
            {
                _transferQueueFamily = i;
                break;
            }
        }
    }

    if (_graphicsQueueFamily == VK_QUEUE_FAMILY_IGNORED)
    {
        //ALIMER_LOGERROR("Vulkan: Invalid graphics queue family");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    uint32_t universalQueueIndex = 1;
    uint32_t graphicsQueueIndex = 0;
    uint32_t computeQueueIndex = 0;
    uint32_t transferQueueIndex = 0;

    if (_computeQueueFamily == VK_QUEUE_FAMILY_IGNORED)
    {
        _computeQueueFamily = _graphicsQueueFamily;
        computeQueueIndex = _vgpu_min(_physicalDevice->queueFamilyProperties[_graphicsQueueFamily].queueCount - 1, universalQueueIndex);
        universalQueueIndex++;
    }

    if (_transferQueueFamily == VK_QUEUE_FAMILY_IGNORED)
    {
        _transferQueueFamily = _graphicsQueueFamily;
        transferQueueIndex = _vgpu_min(_physicalDevice->queueFamilyProperties[_graphicsQueueFamily].queueCount - 1, universalQueueIndex);
        universalQueueIndex++;
    }
    else if (_transferQueueFamily == _computeQueueFamily)
    {
        transferQueueIndex = _vgpu_min(_physicalDevice->queueFamilyProperties[_computeQueueFamily].queueCount - 1, 1u);
    }

    static const float graphicsQueuePriority = 0.5f;
    static const float computeQueuePriority = 1.0f;
    static const float transferQueuePriority = 1.0f;
    float queuePriorities[3] = { graphicsQueuePriority, computeQueuePriority, transferQueuePriority };

    uint32_t queueFamilyCount = 0;
    VkDeviceQueueCreateInfo queueCreateInfo[3] = {};

    VkDeviceCreateInfo deviceCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfo;

    queueCreateInfo[queueFamilyCount].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo[queueFamilyCount].queueFamilyIndex = _graphicsQueueFamily;
    queueCreateInfo[queueFamilyCount].queueCount = _vgpu_min(universalQueueIndex, _physicalDevice->queueFamilyProperties[_graphicsQueueFamily].queueCount);
    queueCreateInfo[queueFamilyCount].pQueuePriorities = queuePriorities;
    queueFamilyCount++;

    if (_computeQueueFamily != _graphicsQueueFamily)
    {
        queueCreateInfo[queueFamilyCount].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo[queueFamilyCount].queueFamilyIndex = _computeQueueFamily;
        queueCreateInfo[queueFamilyCount].queueCount = _vgpu_min(_transferQueueFamily == _computeQueueFamily ? 2u : 1u,
            _physicalDevice->queueFamilyProperties[_computeQueueFamily].queueCount);
        queueCreateInfo[queueFamilyCount].pQueuePriorities = queuePriorities + 1;
        queueFamilyCount++;
    }

    if (_transferQueueFamily != _graphicsQueueFamily
        && _transferQueueFamily != _computeQueueFamily)
    {
        queueCreateInfo[queueFamilyCount].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo[queueFamilyCount].queueFamilyIndex = _transferQueueFamily;
        queueCreateInfo[queueFamilyCount].queueCount = 1;
        queueCreateInfo[queueFamilyCount].pQueuePriorities = queuePriorities + 2;
        queueFamilyCount++;
    }

    uint32_t enabledExtensionsCount = 0;
    const char* enabledExtensions[20];
    if (_mainWindowSurface != VK_NULL_HANDLE)
    {
        enabledExtensions[enabledExtensionsCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    }

    // Enable VK_KHR_maintenance1 to support nevagive viewport and match DirectX coordinate system.
    if (vgpuVkIsExtensionSupported(_physicalDevice, VK_KHR_MAINTENANCE1_EXTENSION_NAME))
    {
        enabledExtensions[enabledExtensionsCount++] = VK_KHR_MAINTENANCE1_EXTENSION_NAME;
    }

    if (vgpuVkIsExtensionSupported(_physicalDevice, VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME)
        && vgpuVkIsExtensionSupported(_physicalDevice, VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME))
    {
        _featuresVk.supportsDedicated = true;
        enabledExtensions[enabledExtensionsCount++] = VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME;
        enabledExtensions[enabledExtensionsCount++] = VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME;
    }

    if (vgpuVkIsExtensionSupported(_physicalDevice, VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME))
    {
        _featuresVk.supportsImageFormatList = true;
        enabledExtensions[enabledExtensionsCount++] = VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME;
    }

    if (vgpuVkIsExtensionSupported(_physicalDevice, VK_GOOGLE_DISPLAY_TIMING_EXTENSION_NAME))
    {
        _featuresVk.supportsGoogleDisplayTiming = true;
        enabledExtensions[enabledExtensionsCount++] = VK_GOOGLE_DISPLAY_TIMING_EXTENSION_NAME;
    }

    if (EXT_debug_utils)
    {
        _featuresVk.supportsDebugUtils = true;
    }

    if (vgpuVkIsExtensionSupported(_physicalDevice, VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
    {
        _featuresVk.supportsDebugMarker = true;
        enabledExtensions[enabledExtensionsCount++] = VK_EXT_DEBUG_MARKER_EXTENSION_NAME;
    }

    if (vgpuVkIsExtensionSupported(_physicalDevice, VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME))
    {
        _featuresVk.supportsMirrorClampToEdge = true;
        enabledExtensions[enabledExtensionsCount++] = VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME;
    }

    VkPhysicalDeviceFeatures2KHR features = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR };
    if (KHR_get_physical_device_properties2)
    {
        vkGetPhysicalDeviceFeatures2KHR(_physicalDevice->device, &features);
    }
    else
    {
        vkGetPhysicalDeviceFeatures(_physicalDevice->device, &features.features);
    }

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

        features.features = enabled_features;
    }

    if (KHR_get_physical_device_properties2)
    {
        deviceCreateInfo.pNext = &features;
    }
    else
    {
        deviceCreateInfo.pEnabledFeatures = &features.features;
    }


    deviceCreateInfo.queueCreateInfoCount = queueFamilyCount;
    deviceCreateInfo.enabledLayerCount = 0;
    deviceCreateInfo.ppEnabledLayerNames = nullptr;
    deviceCreateInfo.enabledExtensionCount = enabledExtensionsCount;
    deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions;

    result = vkCreateDevice(_physicalDevice->device, &deviceCreateInfo, nullptr, &_device);
    if (result != VK_SUCCESS)
    {
        //ALIMER_LOGERROR("Vulkan: Failed to create device");
        return result;
    }

    volkLoadDevice(_device);
    vkGetDeviceQueue(_device, _graphicsQueueFamily, graphicsQueueIndex, &_graphicsQueue);
    vkGetDeviceQueue(_device, _computeQueueFamily, computeQueueIndex, &_computeQueue);
    vkGetDeviceQueue(_device, _transferQueueFamily, transferQueueIndex, &_transferQueue);

    // Set up VMA.
    VmaVulkanFunctions vulkanFunctions = {};
    vulkanFunctions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
    vulkanFunctions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
    vulkanFunctions.vkAllocateMemory = vkAllocateMemory;
    vulkanFunctions.vkFreeMemory = vkFreeMemory;
    vulkanFunctions.vkMapMemory = vkMapMemory;
    vulkanFunctions.vkUnmapMemory = vkUnmapMemory;
    vulkanFunctions.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
    vulkanFunctions.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
    vulkanFunctions.vkBindBufferMemory = vkBindBufferMemory;
    vulkanFunctions.vkBindImageMemory = vkBindImageMemory;
    vulkanFunctions.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
    vulkanFunctions.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
    vulkanFunctions.vkCreateBuffer = vkCreateBuffer;
    vulkanFunctions.vkDestroyBuffer = vkDestroyBuffer;
    vulkanFunctions.vkCreateImage = vkCreateImage;
    vulkanFunctions.vkDestroyImage = vkDestroyImage;
    vulkanFunctions.vkCmdCopyBuffer = vkCmdCopyBuffer;
#if VMA_DEDICATED_ALLOCATION
    vulkanFunctions.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR;
    vulkanFunctions.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2KHR;
#endif

    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.physicalDevice = _physicalDevice->device;
    allocatorCreateInfo.device = _device;
    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;
    result = vmaCreateAllocator(&allocatorCreateInfo, &_memoryAllocator);
    if (result != VK_SUCCESS)
    {
        return result;
    }

    return result;
}

VkResult VgpuRendererVk::createSurface(const VgpuSwapchainDescriptor*  descriptor, VkSurfaceKHR* pSurface)
{
    VkResult result = VK_SUCCESS;
    if (descriptor == nullptr)
    {
        return result;
    }

#if defined(VK_USE_PLATFORM_WIN32_KHR)
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo;
    memset(&surfaceCreateInfo, 0, sizeof(surfaceCreateInfo));
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    if (descriptor->nativeDisplay != 0)
    {
        surfaceCreateInfo.hinstance = (HINSTANCE)descriptor->nativeDisplay;
    }
    else
    {
        surfaceCreateInfo.hinstance = GetModuleHandleW(nullptr);
    }

    surfaceCreateInfo.hwnd = (HWND)descriptor->nativeHandle;
    PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHRProc = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(_instance, "vkCreateWin32SurfaceKHR");
    result = vkCreateWin32SurfaceKHRProc(_instance, &surfaceCreateInfo, nullptr, pSurface);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo;
    memset(&surfaceCreateInfo, 0, sizeof(surfaceCreateInfo));
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.window = (ANativeWindow)descriptor->nativeHandle;
    PFN_vkCreateAndroidSurfaceKHR vkCreateAndroidSurfaceKHRProc = (PFN_vkCreateAndroidSurfaceKHR)vkGetInstanceProcAddr(_instance, "vkCreateAndroidSurfaceKHR");
    result = vkCreateAndroidSurfaceKHRProc(_instance, &surfaceCreateInfo, NULL, pSurface);
#elif defined(VK_USE_PLATFORM_IOS_MVK)
    VkIOSSurfaceCreateInfoMVK surfaceCreateInfo;
    memset(&surfaceCreateInfo, 0, sizeof(surfaceCreateInfo));
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_IOS_SURFACE_CREATE_INFO_MVK;
    surfaceCreateInfo.pView = descriptor->nativeHandle;
    PFN_vkCreateIOSSurfaceMVK vkCreateIOSSurfaceMVKProc = (PFN_vkCreateIOSSurfaceMVK)vkGetInstanceProcAddr(_instance, "vkCreateIOSSurfaceMVK");
    result = vkCreateIOSSurfaceMVKProc(_instance, &surfaceCreateInfo, nullptr, pSurface);
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
    VkMacOSSurfaceCreateInfoMVK surfaceCreateInfo;
    memset(&surfaceCreateInfo, 0, sizeof(surfaceCreateInfo));
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
    surfaceCreateInfo.pView = descriptor->nativeHandle;
    PFN_vkCreateMacOSSurfaceMVK vkCreateMacOSSurfaceMVKProc = (PFN_vkCreateMacOSSurfaceMVK)vkGetInstanceProcAddr(_instance, "vkCreateMacOSSurfaceMVK");
    result = vkCreateMacOSSurfaceMVKProc(_instance, &surfaceCreateInfo, NULL, pSurface);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    VkWaylandSurfaceCreateInfoKHR surfaceCreateInfo;
    memset(&surfaceCreateInfo, 0, sizeof(surfaceCreateInfo));
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.display = (wl_display*)descriptor->display;
    surfaceCreateInfo.surface = (wl_surface*)descriptor->windowHandle;
    PFN_vkCreateWaylandSurfaceKHR vkCreateWaylandSurfaceKHRProc = (PFN_vkCreateWaylandSurfaceKHR)vkGetInstanceProcAddr(_instance, "vkCreateWaylandSurfaceKHR");
    result = vkCreateWaylandSurfaceKHRProc(_instance, &surfaceCreateInfo, nullptr, pSurface);
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
    VkXlibSurfaceCreateInfoKHR surfaceCreateInfo;
    memset(&surfaceCreateInfo, 0, sizeof(surfaceCreateInfo));
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.dpy = (Display*)descriptor->nativeDisplay;
    surfaceCreateInfo.window = (Window)descriptor->nativeHandle;
    PFN_vkCreateXlibSurfaceKHR vkCreateXlibSurfaceKHRProc = (PFN_vkCreateXlibSurfaceKHR)vkGetInstanceProcAddr(_instance, "vkCreateXlibSurfaceKHR");
    result = vkCreateXlibSurfaceKHRProc(_instance, &surfaceCreateInfo, nullptr, pSurface);
#endif

    return result;
}

VgpuResult VgpuRendererVk::createSwapchain(const VgpuSwapchainDescriptor* descriptor, VkSwapchainKHR oldSwapchain, VkSurfaceKHR surface, VgpuSwapchain* pSwapchain)
{
    VkSurfaceCapabilitiesKHR surfaceCaps;
    if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice->device, surface, &surfaceCaps) != VK_SUCCESS)
    {
        return VGPU_ERROR_INITIALIZATION_FAILED;
    }

    // No surface, should sleep and retry maybe?.
    if (surfaceCaps.maxImageExtent.width == 0 &&
        surfaceCaps.maxImageExtent.height == 0)
    {
        return VGPU_ERROR_INITIALIZATION_FAILED;
    }

    VkBool32 supported = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice->device, _graphicsQueueFamily, surface, &supported);
    if (!supported)
    {
        return VGPU_ERROR_INITIALIZATION_FAILED;
    }

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice->device, surface, &formatCount, nullptr);
    VkSurfaceFormatKHR *formats = (VkSurfaceFormatKHR*)alloca(sizeof(VkSurfaceFormatKHR) * formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice->device, surface, &formatCount, formats);

    VkSurfaceFormatKHR format = { VK_FORMAT_UNDEFINED, VK_COLORSPACE_SRGB_NONLINEAR_KHR };
    if (formatCount == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
    {
        format = formats[0];
        format.format = VK_FORMAT_B8G8R8A8_UNORM;
    }
    else
    {
        if (formatCount == 0)
        {
            //ALIMER_LOGERROR("Surface has no formats.");
            return VGPU_ERROR_INITIALIZATION_FAILED;
        }

        bool found = false;
        for (uint32_t i = 0; i < formatCount; i++)
        {
            if (formats[i].format == VK_FORMAT_R8G8B8A8_SRGB ||
                formats[i].format == VK_FORMAT_B8G8R8A8_SRGB ||
                formats[i].format == VK_FORMAT_A8B8G8R8_SRGB_PACK32)
            {
                format = formats[i];
                found = true;
            }
        }

        if (!found)
        {
            format = formats[0];
        }
    }

    VkExtent2D swapchainSize;
    if (surfaceCaps.currentExtent.width == ~0u)
    {
        swapchainSize.width = descriptor->width;
        swapchainSize.height = descriptor->height;
    }
    else
    {
        swapchainSize.width = _vgpu_max(_vgpu_min(descriptor->width, surfaceCaps.maxImageExtent.width), surfaceCaps.minImageExtent.width);
        swapchainSize.height = _vgpu_max(_vgpu_min(descriptor->height, surfaceCaps.maxImageExtent.height), surfaceCaps.minImageExtent.height);
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice->device, surface, &presentModeCount, nullptr);
    VkPresentModeKHR *presentModes = (VkPresentModeKHR*)alloca(sizeof(VkPresentModeKHR) * presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice->device, surface, &presentModeCount, presentModes);

    VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
    if (!descriptor->vsync)
    {
        for (uint32_t i = 0; i < presentModeCount; i++)
        {
            if (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR
                || presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                swapchainPresentMode = presentModes[i];
                break;
            }
        }
    }

    uint32_t desiredNumberOfSwapchainImages = descriptor->tripleBuffer ? 3 : surfaceCaps.minImageCount + 1;
    if (desiredNumberOfSwapchainImages < surfaceCaps.minImageCount) {
        desiredNumberOfSwapchainImages = surfaceCaps.minImageCount;
    }

    if ((surfaceCaps.maxImageCount > 0) && (desiredNumberOfSwapchainImages > surfaceCaps.maxImageCount))
    {
        desiredNumberOfSwapchainImages = surfaceCaps.maxImageCount;
    }

    //ALIMER_LOGINFO("Targeting {} swapchain images.", desiredNumberOfSwapchainImages);

    VkSurfaceTransformFlagBitsKHR preTransform;
    if (surfaceCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
        preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    else
        preTransform = surfaceCaps.currentTransform;

    VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    if (surfaceCaps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR)
        compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    if (surfaceCaps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
        compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    if (surfaceCaps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR)
        compositeAlpha = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
    if (surfaceCaps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR)
        compositeAlpha = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.surface = surface;
    createInfo.minImageCount = desiredNumberOfSwapchainImages;
    createInfo.imageFormat = format.format;
    createInfo.imageColorSpace = format.colorSpace;
    createInfo.imageExtent.width = swapchainSize.width;
    createInfo.imageExtent.height = swapchainSize.height;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = nullptr;
    createInfo.preTransform = preTransform;
    createInfo.compositeAlpha = compositeAlpha;
    createInfo.presentMode = swapchainPresentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = oldSwapchain;

    VgpuTextureUsage textureUsage = VGPU_TEXTURE_USAGE_RENDER_TARGET;

    // Enable transfer source on swap chain images if supported.
    if (surfaceCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
        createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        textureUsage = (VgpuTextureUsage)(textureUsage | VGPU_TEXTURE_USAGE_TRANSFER_SRC);
    }

    // Enable transfer destination on swap chain images if supported
    if (surfaceCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
        createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        textureUsage = (VgpuTextureUsage)(textureUsage | VGPU_TEXTURE_USAGE_TRANSFER_DEST);
    }

    VkSwapchainKHR swapchain;
    VkResult result = vkCreateSwapchainKHR(_device, &createInfo, nullptr, &swapchain);
    if (result != VK_SUCCESS)
    {
        //ALIMER_LOGCRITICAL("Vulkan: Failed to create swapchain, error: {}", vkGetVulkanResultString(result));
        return vgpuVkConvertResult(result);
    }

    if (oldSwapchain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(_device, oldSwapchain, nullptr);
    }


    VgpuSwapchain handle = VGPU_ALLOC_HANDLE(VgpuSwapchain);
    handle->surface = surface;
    handle->swapchain = swapchain;

    vkGetSwapchainImagesKHR(_device, swapchain, &handle->imageCount, nullptr);
    handle->images = VGPU_ALLOCN(VkImage, handle->imageCount);
    vkGetSwapchainImagesKHR(_device, swapchain, &handle->imageCount, handle->images);

    // Create semaphores to be signaled when a swapchain image becomes available.
    handle->imageSemaphores = VGPU_ALLOCN(VkSemaphore, handle->imageCount);
    for (uint32_t i = 0u; i < handle->imageCount; ++i)
    {
        //VkFenceCreateInfo fenceCreateInfo = {};
        //fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        //fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        //vkThrowIfFailed(vkCreateFence(_device, &fenceCreateInfo, nullptr, &_waitFences[i]));

        VkSemaphoreCreateInfo semaphoreCreateInfo;
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreCreateInfo.pNext = nullptr;
        semaphoreCreateInfo.flags = 0;
        result = vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &handle->imageSemaphores[i]);
        if (result != VK_SUCCESS)
        {
            return vgpuVkConvertResult(result);
        }
    }

    // Clear or transition to default image layout
    const bool canClear = createInfo.imageUsage & VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VgpuCommandBuffer clearImageCmdBuffer;
    createCommandBuffer(_graphicsCommandQueue, VK_COMMAND_BUFFER_LEVEL_PRIMARY, &clearImageCmdBuffer);
    beginCommandBuffer(clearImageCmdBuffer);
    for (uint32_t i = 0u; i < handle->imageCount; ++i)
    {
        // Create backend texture.
        //_swapchainTextures[i] = std::make_unique<TextureVk>(_device, &textureDescriptor, swapchainImages[i], nullptr);

        // Clear with default value if supported.
        if (canClear)
        {
            // Clear images with default color.
            VkClearColorValue clearColor = {};
            clearColor.float32[3] = 1.0f;
            clearColor.float32[0] = 1.0f;

            VkImageSubresourceRange clearRange = {};
            clearRange.layerCount = 1;
            clearRange.levelCount = 1;

            // Clear with default color.
            vgpuVkClearImageWithColor(
                clearImageCmdBuffer->vk_handle,
                handle->images[i],
                clearRange,
                VK_IMAGE_ASPECT_COLOR_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                &clearColor);
        }
        else
        {
            // Transition image to present layout.
            vgpuVkTransitionImageLayout(
                clearImageCmdBuffer->vk_handle,
                handle->images[i],
                VK_IMAGE_ASPECT_COLOR_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
            );
        }
    }
    endCommandBuffer(clearImageCmdBuffer);
    commitCommandBuffer(clearImageCmdBuffer, true);

    handle->imageIndex = 0;
    *pSwapchain = handle;
    return VGPU_SUCCESS;
}

void VgpuRendererVk::destroySwapchain(VgpuSwapchain swapchain)
{
    VGPU_ASSERT(swapchain);
    for (uint32_t i = 0u; swapchain->imageSemaphores != NULL && i < swapchain->imageCount; ++i)
    {
        if (swapchain->imageSemaphores[i] != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(_device, swapchain->imageSemaphores[i], NULL);
        }
    }

    if (swapchain->imageSemaphores != NULL)
    {
        VGPU_FREE(swapchain->imageSemaphores);
    }

    if (swapchain->swapchain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(_device, swapchain->swapchain, NULL);
    }

    VGPU_FREE(swapchain->images);
    VGPU_FREE(swapchain);
}

VgpuResult VgpuRendererVk::present(VgpuSwapchain swapchain, VkQueue queue)
{
    VGPU_ASSERT(swapchain);
    VGPU_ASSERT(queue);

    VkResult presentResult = VK_SUCCESS;
    VkPresentInfoKHR presentInfo;

    memset(&presentInfo, 0, sizeof(presentInfo));
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = NULL;
    // Check if a wait semaphore has been specified to wait for before presenting the image
    //if (waitSemaphore != VK_NULL_HANDLE)
    //{
    //    presentInfo.pWaitSemaphores = &waitSemaphore;
    //    presentInfo.waitSemaphoreCount = 1;
    //}
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain->swapchain;
    presentInfo.pImageIndices = &swapchain->imageIndex;
    presentInfo.pResults = &presentResult;
    vkQueuePresentKHR(queue, &presentInfo);
    if (presentResult != VK_SUCCESS)
    {
        return vgpuVkConvertResult(presentResult);
    }

    return VGPU_SUCCESS;
}

VgpuResult VgpuRendererVk::createCommandQueue(VkQueue queue, uint32_t queueFamilyIndex, VgpuCommandQueue* pCommandQueue)
{
    VkCommandPoolCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    createInfo.queueFamilyIndex = queueFamilyIndex;

    VkCommandPool vk_handle;
    VkResult result = vkCreateCommandPool(_device, &createInfo, nullptr, &vk_handle);
    if (result != VK_SUCCESS)
    {
        return vgpuVkConvertResult(result);
    }

    VgpuCommandQueue handle = VGPU_ALLOC_HANDLE(VgpuCommandQueue);
    handle->vk_queue = queue;
    handle->vk_queueFamilyIndex = queueFamilyIndex;
    handle->vk_handle = vk_handle;
    *pCommandQueue = handle;
    return VGPU_SUCCESS;
}

void VgpuRendererVk::destoryCommandQueue(VgpuCommandQueue queue)
{
    VGPU_ASSERT(queue);

    if (queue->vk_handle != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(_device, queue->vk_handle, nullptr);
    }

    queue->vk_queue = VK_NULL_HANDLE;
    queue->vk_queueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    VGPU_FREE(queue);
}

VgpuResult VgpuRendererVk::createCommandBuffer(VgpuCommandQueue queue, VkCommandBufferLevel level, VgpuCommandBuffer* pCommandBuffer)
{
    VkCommandBufferAllocateInfo info;
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.pNext = nullptr;
    info.commandPool = queue->vk_handle;
    info.level = level;
    info.commandBufferCount = 1;

    VkCommandBuffer vk_handle;
    VkResult result = vkAllocateCommandBuffers(_device, &info, &vk_handle);
    if (result != VK_SUCCESS)
    {
        return vgpuVkConvertResult(result);
    }

    VgpuCommandBuffer handle = VGPU_ALLOC_HANDLE(VgpuCommandBuffer);
    handle->queue = queue;
    handle->vk_handle = vk_handle;
    *pCommandBuffer = handle;
    return VGPU_SUCCESS;
}

VgpuResult VgpuRendererVk::beginCommandBuffer(VgpuCommandBuffer commandBuffer)
{
    VGPU_ASSERT(commandBuffer);

    VkCommandBufferBeginInfo beginInfo;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext = nullptr;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr;
    VkResult result = vkBeginCommandBuffer(commandBuffer->vk_handle, &beginInfo);
    if (result != VK_SUCCESS)
    {
        return vgpuVkConvertResult(result);
    }

    return VGPU_SUCCESS;
}

VgpuResult VgpuRendererVk::endCommandBuffer(VgpuCommandBuffer commandBuffer)
{
    VGPU_ASSERT(commandBuffer);

    VkResult result = vkEndCommandBuffer(commandBuffer->vk_handle);
    if (result != VK_SUCCESS)
    {
        return vgpuVkConvertResult(result);
    }

    return VGPU_SUCCESS;
}

VgpuResult VgpuRendererVk::commitCommandBuffer(VgpuCommandBuffer commandBuffer, bool waitUntilCompleted)
{
    VGPU_ASSERT(commandBuffer);
    VkResult result = VK_SUCCESS;

    VkSubmitInfo submitInfo;
    memset(&submitInfo, 0, sizeof(submitInfo));
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer->vk_handle;

    VkFence fence = VK_NULL_HANDLE;
    if (waitUntilCompleted)
    {
        // Create fence to ensure that the command buffer has finished executing
        VkFenceCreateInfo fenceCreateInfo;
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.pNext = nullptr;
        fenceCreateInfo.flags = 0;
        result = vkCreateFence(_device, &fenceCreateInfo, nullptr, &fence);
        if (result != VK_SUCCESS)
        {
            return vgpuVkConvertResult(result);
        }
    }

    // Submit to the queue
    result = vkQueueSubmit(commandBuffer->queue->vk_queue, 1, &submitInfo, fence);
    if (result != VK_SUCCESS)
    {
        return vgpuVkConvertResult(result);
    }

    if (waitUntilCompleted)
    {
        // Wait for the fence to signal that command buffer has finished executing.
        result = vkWaitForFences(_device, 1, &fence, VK_TRUE, UINT64_MAX);
        if (result != VK_SUCCESS)
        {
            return vgpuVkConvertResult(result);
        }

        vkDestroyFence(_device, fence, nullptr);

        // Free immediatelly the command buffer
        vkFreeCommandBuffers(_device, commandBuffer->queue->vk_handle, 1, &commandBuffer->vk_handle);
    }

    return VGPU_SUCCESS;
}


void VgpuRendererVk::initializeFeatures()
{
    /*_features.SetBackend(GraphicsBackend::Vulkan);
    _features.SetVendorId(_physicalDeviceProperties.vendorID);
    // Find vendor
    switch (_physicalDeviceProperties.vendorID)
    {
    case 0x13B5:
        _features.SetVendor(GpuVendor::ARM);
        break;
    case 0x10DE:
        _features.SetVendor(GpuVendor::NVIDIA);
        break;
    case 0x1002:
    case 0x1022:
        _features.SetVendor(GpuVendor::AMD);
        break;
    case 0x163C:
    case 0x8086:
        _features.SetVendor(GpuVendor::INTEL);
        break;
    default:
        _features.SetVendor(GpuVendor::Unknown);
        break;
    }

    _features.SetDeviceId(_physicalDeviceProperties.deviceID);
    _features.SetDeviceName(_physicalDeviceProperties.deviceName);
    _features.SetMultithreading(true);
    _features.SetMaxColorAttachments(_physicalDeviceProperties.limits.maxColorAttachments);
    _features.SetMaxBindGroups(_physicalDeviceProperties.limits.maxBoundDescriptorSets);
    _features.SetMinUniformBufferOffsetAlignment(static_cast<uint32_t>(_physicalDeviceProperties.limits.minUniformBufferOffsetAlignment));*/
}

VgpuResult VgpuRendererVk::waitIdle()
{
    VkResult result = vkDeviceWaitIdle(_device);
    if (result < VK_SUCCESS)
    {
        return vgpuVkConvertResult(result);
    }

    return VGPU_SUCCESS;
}

VgpuResult VgpuRendererVk::beginFrame()
{
    VkResult result = VK_SUCCESS;
    
    if (_mainSwapchain != nullptr)
    {
        vkAcquireNextImageKHR(
            _device,
            _mainSwapchain->swapchain,
            UINT64_MAX,
            _mainSwapchain->imageSemaphores[_frameIndex],
            VK_NULL_HANDLE,
            &_mainSwapchain->imageIndex);

        if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR))
        {
            //_mainSwapChain->Resize();
        }
        else
        {
            if (result < VK_SUCCESS)
            {
                return vgpuVkConvertResult(result);
            }
        }
    }

    return VGPU_SUCCESS;
}

VgpuResult VgpuRendererVk::endFrame()
{
    VkResult result = VK_SUCCESS;

    if (_mainSwapchain != nullptr)
    {
        present(_mainSwapchain, _graphicsCommandQueue->vk_queue);
    }

    // Submit the pending command buffers.
    /*const VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    result = vkWaitForFences(_device, 1, &_waitFences[_frameIndex], VK_TRUE, UINT64_MAX);
    if (result != VK_SUCCESS)
    {
        return vgpuVkConvertResult(result);
    }
    result = vkResetFences(_device, 1, &_waitFences[_frameIndex]);
    if (result != VK_SUCCESS)
    {
        return vgpuVkConvertResult(result);
    }*/

    // Advance frame index.
    _frameIndex = (_frameIndex + 1u) % _maxInflightFrames;
    _frameNumber++;
    return VGPU_SUCCESS;
}

/* Implementation */
static VgpuRendererVk* s_renderer = nullptr;

VgpuResult vgpuInitialize(const char* applicationName, const VgpuDescriptor* descriptor)
{
    if (s_renderer != nullptr)
        return VGPU_ALREADY_INITIALIZED;

    s_renderer = new VgpuRendererVk();
    return s_renderer->initialize(applicationName, descriptor);
}

void vgpuShutdown()
{
    if (s_renderer != nullptr)
    {
        s_renderer->shutdown();
        delete s_renderer;
        s_renderer = nullptr;
    }
}

VgpuResult agpuBeginFrame()
{
    return s_renderer->beginFrame();
}

VgpuResult agpuEndFrame()
{
    return s_renderer->endFrame();
}

VgpuResult vgpuWaitIdle()
{
    return s_renderer->waitIdle();
}
