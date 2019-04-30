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

#if defined(VGPU_VK)
#include "vgpu.h"
#include <assert.h>
#include <string.h>
#if defined(WIN32)
#   include <malloc.h>
#   undef    alloca
#   define   alloca _malloca
#   define   freea  _freea
#else
#   include <alloca.h>
#endif

#include "volk.h"
#include "vk_mem_alloc.h"

#define _VGPU_ALLOC(type)           ((type*) malloc(sizeof(type)))
#define _VGPU_ALLOCN(type, n)       ((type*) malloc(sizeof(type) * n))
#define _VGPU_FREE(ptr)             (free((void*)(ptr)))
#define _VGPU_ALLOC_HANDLE(type)    ((type) calloc(1, sizeof(type##_T)))

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

/* Vulkan only handles */
VGPU_DEFINE_HANDLE(VGpuSwapchain);

/* Handle declaration */
typedef struct VGpuSwapchain_T {
    VkSwapchainKHR              vk_handle;
    uint32_t                    width;
    uint32_t                    height;
    VkFormat                    format;
    uint32_t                    image_count;
    VkImage*                    images;
    VkImageView*                image_views;
    VkSemaphore*                image_semaphores;
    uint32_t                    image_index;
    VGpuTexture                 depthStencilTexture;
    VGpuClearValue              depthStencilClearValue;
} VGpuSwapchain_T;

typedef struct VGpuTexture_T {
    bool                        externalHandle;
    VkImage                     handle;
    VmaAllocation               memory;
} VGpuTexture_T;

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

    VkPhysicalDevice            physicalDevice = VK_NULL_HANDLE;
    uint32_t                    graphics_queue_family = VK_QUEUE_FAMILY_IGNORED;
    uint32_t                    compute_queue_family = VK_QUEUE_FAMILY_IGNORED;
    uint32_t                    transfer_queue_family = VK_QUEUE_FAMILY_IGNORED;
    VkDevice                    device = VK_NULL_HANDLE;
    VkQueue                     graphics_queue = VK_NULL_HANDLE;
    VkQueue                     compute_queue = VK_NULL_HANDLE;
    VkQueue                     transfer_queue = VK_NULL_HANDLE;
    VmaAllocator                memoryAllocator = VK_NULL_HANDLE;
    VkCommandPool               commandPool = VK_NULL_HANDLE;
    VGpuSwapchain               swapchain = nullptr;
    uint32_t                    max_inflight_frames;
    uint32_t                    frame_index;
} _vk;

#if VULKAN_DEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_messenger_cb(
    VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT                  messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
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
        auto* name = pCallbackData->pObjects[i].pObjectName;
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
            auto* name = pCallbackData->pObjects[i].pObjectName;
            VGPU_LOGI("  Object #%u: %s\n", i, name ? name : "N/A");
        }
    }

    return VK_FALSE;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_cb(VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT, uint64_t,
    size_t, int32_t messageCode, const char* pLayerPrefix,
    const char* pMessage, void* pUserData)
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

/* Conversion functions */
static const char* _vgpu_vk_get_result_string(VkResult result)
{
    switch (result)
    {
    case VK_SUCCESS:
        return "Success";
    case VK_NOT_READY:
        return "A fence or query has not yet completed";
    case VK_TIMEOUT:
        return "A wait operation has not completed in the specified time";
    case VK_EVENT_SET:
        return "An event is signaled";
    case VK_EVENT_RESET:
        return "An event is unsignaled";
    case VK_INCOMPLETE:
        return "A return array was too small for the result";
    case VK_ERROR_OUT_OF_HOST_MEMORY:
        return "A host memory allocation has failed";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        return "A device memory allocation has failed";
    case VK_ERROR_INITIALIZATION_FAILED:
        return "Initialization of an object could not be completed for implementation-specific reasons";
    case VK_ERROR_DEVICE_LOST:
        return "The logical or physical device has been lost";
    case VK_ERROR_MEMORY_MAP_FAILED:
        return "Mapping of a memory object has failed";
    case VK_ERROR_LAYER_NOT_PRESENT:
        return "A requested layer is not present or could not be loaded";
    case VK_ERROR_EXTENSION_NOT_PRESENT:
        return "A requested extension is not supported";
    case VK_ERROR_FEATURE_NOT_PRESENT:
        return "A requested feature is not supported";
    case VK_ERROR_INCOMPATIBLE_DRIVER:
        return "The requested version of Vulkan is not supported by the driver or is otherwise incompatible";
    case VK_ERROR_TOO_MANY_OBJECTS:
        return "Too many objects of the type have already been created";
    case VK_ERROR_FORMAT_NOT_SUPPORTED:
        return "A requested format is not supported on this device";
    case VK_ERROR_SURFACE_LOST_KHR:
        return "A surface is no longer available";
    case VK_SUBOPTIMAL_KHR:
        return "A swapchain no longer matches the surface properties exactly, but can still be used";
    case VK_ERROR_OUT_OF_DATE_KHR:
        return "A surface has changed in such a way that it is no longer compatible with the swapchain";
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
        return "The display used by a swapchain does not use the same presentable image layout";
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
        return "The requested window is already connected to a VkSurfaceKHR, or to some other non-Vulkan API";
    case VK_ERROR_VALIDATION_FAILED_EXT:
        return "A validation layer found an error";
    default:
        return "ERROR: UNKNOWN VULKAN ERROR";
    }
}


static void _vgpuVkLogIfFailed(VkResult result)
{
    if (result < VK_SUCCESS)
    {
        /*VGPU_LOGE(
            "Fatal vulkan result is \"%s\" in %u at line %u",
            _vgpu_vk_get_result_string(result),
            __FILE__,
            __LINE__);*/
    }
}

static VkFormat _vgpuVkGetFormat(VGpuPixelFormat format)
{
    static VkFormat formats[VGPU_PIXEL_FORMAT_COUNT] = {
        VK_FORMAT_UNDEFINED,
        // 8-bit pixel formats
        VK_FORMAT_R8_UNORM,                     // VGPU_PIXEL_FORMAT_A8_UNORM
        VK_FORMAT_R8_UNORM,                     // VGPU_PIXEL_FORMAT_R8_UNORM
        VK_FORMAT_R8_SNORM,                     // VGPU_PIXEL_FORMAT_R8_SNORM
        VK_FORMAT_R8_UINT,                      // VGPU_PIXEL_FORMAT_R8_UINT
        VK_FORMAT_R8_SINT,                      // VGPU_PIXEL_FORMAT_R8_SINT

        // 16-bit pixel formats
        VK_FORMAT_R16_UNORM,                    // VGPU_PIXEL_FORMAT_R16_UNORM
        VK_FORMAT_R16_SNORM,                    // VGPU_PIXEL_FORMAT_R16_SNORM
        VK_FORMAT_R16_UINT,                     // VGPU_PIXEL_FORMAT_R16_UINT
        VK_FORMAT_R16_SINT,                     // VGPU_PIXEL_FORMAT_R16_SINT
        VK_FORMAT_R16_SFLOAT,                   // VGPU_PIXEL_FORMAT_R16_FLOAT
        VK_FORMAT_R8G8_UNORM,                   // VGPU_PIXEL_FORMAT_RG8_UNORM
        VK_FORMAT_R8G8_SNORM,                   // VGPU_PIXEL_FORMAT_RG8_SNORM
        VK_FORMAT_R8G8_UINT,                    // VGPU_PIXEL_FORMAT_RG8_UINT
        VK_FORMAT_R8G8_SINT,                    // VGPU_PIXEL_FORMAT_RG8_SINT

        // Packed 16-bit pixel formats
        VK_FORMAT_R5G6B5_UNORM_PACK16,          // VGPU_PIXEL_FORMAT_R5G6B5_UNORM
        VK_FORMAT_R4G4B4A4_UNORM_PACK16,        // VGPU_PIXEL_FORMAT_RGBA4_UNORM

        // 32-bit pixel formats
        VK_FORMAT_R32_UINT,                     // VGPU_PIXEL_FORMAT_R32_UINT,
        VK_FORMAT_R32_SINT,                     // VGPU_PIXEL_FORMAT_R32_SINT,
        VK_FORMAT_R32_SFLOAT,                   // VGPU_PIXEL_FORMAT_R32_FLOAT
        VK_FORMAT_R16G16_UNORM,                 // VGPU_PIXEL_FORMAT_RG16_UNORM
        VK_FORMAT_R16G16_SNORM,                 // VGPU_PIXEL_FORMAT_RG16_SNORM
        VK_FORMAT_R16G16_UINT,                  // VGPU_PIXEL_FORMAT_RG16_UINT
        VK_FORMAT_R16G16_SINT,                  // VGPU_PIXEL_FORMAT_RG16_SINT
        VK_FORMAT_R16G16_SFLOAT,                // VGPU_PIXEL_FORMAT_RG16_FLOAT
        VK_FORMAT_R8G8B8A8_UNORM,               // VGPU_PIXEL_FORMAT_RGBA8_UNORM
        VK_FORMAT_R8G8B8A8_SRGB,                // VGPU_PIXEL_FORMAT_RGBA8_UNORM_SRGB
        VK_FORMAT_R8G8B8A8_SNORM,               // VGPU_PIXEL_FORMAT_RGBA8_SNORM
        VK_FORMAT_R8G8B8A8_UINT,                // VGPU_PIXEL_FORMAT_RGBA8_UINT
        VK_FORMAT_R8G8B8A8_SINT,                // VGPU_PIXEL_FORMAT_RGBA8_SINT
        VK_FORMAT_B8G8R8A8_UNORM,               // VGPU_PIXEL_FORMAT_BGRA8_UNORM
        VK_FORMAT_B8G8R8A8_SRGB,                // VGPU_PIXEL_FORMAT_BGRA8_UNORM_SRGB

        // Packed 32-Bit Pixel formats
        VK_FORMAT_A2R10G10B10_UNORM_PACK32,     // VGPU_PIXEL_FORMAT_RGB10A2_UNORM
        VK_FORMAT_A2R10G10B10_UINT_PACK32,      // VGPU_PIXEL_FORMAT_RGB10A2_UINT
        VK_FORMAT_B10G11R11_UFLOAT_PACK32,      // VGPU_PIXEL_FORMAT_RG11B10_FLOAT
        VK_FORMAT_E5B9G9R9_UFLOAT_PACK32,       // VGPU_PIXEL_FORMAT_RGB9E5_FLOAT

        // 64-Bit Pixel Formats
        VK_FORMAT_R32G32_UINT,                  // VGPU_PIXEL_FORMAT_RG32_UINT
        VK_FORMAT_R32G32_SINT,                  // VGPU_PIXEL_FORMAT_RG32_SINT
        VK_FORMAT_R32G32_SFLOAT,                // VGPU_PIXEL_FORMAT_RG32_FLOAT
        VK_FORMAT_R16G16B16_UNORM,              // VGPU_PIXEL_FORMAT_RGBA16_UNORM
        VK_FORMAT_R16G16B16_SNORM,              // VGPU_PIXEL_FORMAT_RGBA16_SNORM
        VK_FORMAT_R16G16B16_UINT,               // VGPU_PIXEL_FORMAT_RGBA16_UINT
        VK_FORMAT_R16G16B16_SINT,               // VGPU_PIXEL_FORMAT_RGBA16_SINT
        VK_FORMAT_R16G16B16_SFLOAT,             // VGPU_PIXEL_FORMAT_RGBA16_FLOAT

        // 128-Bit Pixel Formats
        VK_FORMAT_R32G32B32A32_UINT,            // VGPU_PIXEL_FORMAT_RGBA32_UINT
        VK_FORMAT_R32G32B32A32_SINT,            // VGPU_PIXEL_FORMAT_RGBA32_SINT
        VK_FORMAT_R32G32B32A32_SFLOAT,          // VGPU_PIXEL_FORMAT_RGBA32_FLOAT

        // Depth-stencil
        VK_FORMAT_D16_UNORM,                    // VGPU_PIXEL_FORMAT_D16_UNORM
        VK_FORMAT_D32_SFLOAT,                   // VGPU_PIXEL_FORMAT_D32_FLOAT
        VK_FORMAT_D24_UNORM_S8_UINT,            // VGPU_PIXEL_FORMAT_D24_UNORM_S8_UINT
        VK_FORMAT_D32_SFLOAT_S8_UINT,           // VGPU_PIXEL_FORMAT_D32_FLOAT_S8_UINT
        VK_FORMAT_S8_UINT,                      // VGPU_PIXEL_FORMAT_S8

        // Compressed formats
        VK_FORMAT_BC1_RGB_UNORM_BLOCK,          // VGPU_PIXEL_FORMAT_BC1_UNORM,        
        VK_FORMAT_BC1_RGB_SRGB_BLOCK,           // VGPU_PIXEL_FORMAT_BC1_UNORM_SRGB
        VK_FORMAT_BC2_UNORM_BLOCK,              // VGPU_PIXEL_FORMAT_BC2_UNORM
        VK_FORMAT_BC2_SRGB_BLOCK,               // VGPU_PIXEL_FORMAT_BC2_UNORM_SRGB
        VK_FORMAT_BC3_UNORM_BLOCK,              // VGPU_PIXEL_FORMAT_BC3_UNORM
        VK_FORMAT_BC3_SRGB_BLOCK,               // VGPU_PIXEL_FORMAT_BC3_UNORM_SRGB
        VK_FORMAT_BC4_UNORM_BLOCK,              // VGPU_PIXEL_FORMAT_BC4_UNORM     
        VK_FORMAT_BC4_SNORM_BLOCK,              // VGPU_PIXEL_FORMAT_BC4_SNORM    
        VK_FORMAT_BC5_UNORM_BLOCK,              // VGPU_PIXEL_FORMAT_BC5_UNORM    
        VK_FORMAT_BC5_SNORM_BLOCK,              // VGPU_PIXEL_FORMAT_BC5_SNORM    
        VK_FORMAT_BC6H_SFLOAT_BLOCK,            // VGPU_PIXEL_FORMAT_BC6HS16
        VK_FORMAT_BC6H_UFLOAT_BLOCK,            // VGPU_PIXEL_FORMAT_BC6HU16
        VK_FORMAT_BC7_UNORM_BLOCK,              // VGPU_PIXEL_FORMAT_BC7_UNORM
        VK_FORMAT_BC7_SRGB_BLOCK,               // VGPU_PIXEL_FORMAT_BC7_UNORM_SRGB

        // Compressed PVRTC Pixel Formats
        VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG,  // VGPU_PIXEL_FORMAT_PVRTC_RGB2
        VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG,  // VGPU_PIXEL_FORMAT_PVRTC_RGBA2
        VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG,  // VGPU_PIXEL_FORMAT_PVRTC_RGB4
        VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG,  // VGPU_PIXEL_FORMAT_PVRTC_RGBA4

        // Compressed ETC Pixel Formats
        VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK,      // VGPU_PIXEL_FORMAT_ETC2_RGB8
        VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK,    // VGPU_PIXEL_FORMAT_ETC2_RGB8A1

        // Compressed ASTC Pixel Formats
        VK_FORMAT_ASTC_4x4_UNORM_BLOCK,         // VGPU_PIXEL_FORMAT_ASTC4x4,
        VK_FORMAT_ASTC_5x5_UNORM_BLOCK,         // VGPU_PIXEL_FORMAT_ASTC5x5
        VK_FORMAT_ASTC_6x6_UNORM_BLOCK,         // VGPU_PIXEL_FORMAT_ASTC6x6
        VK_FORMAT_ASTC_8x5_UNORM_BLOCK,         // VGPU_PIXEL_FORMAT_ASTC8x5
        VK_FORMAT_ASTC_8x6_UNORM_BLOCK,         // VGPU_PIXEL_FORMAT_ASTC8x6
        VK_FORMAT_ASTC_8x8_UNORM_BLOCK,         // VGPU_PIXEL_FORMAT_ASTC8x8
        VK_FORMAT_ASTC_10x10_UNORM_BLOCK,       // VGPU_PIXEL_FORMAT_ASTC10x10
        VK_FORMAT_ASTC_12x12_UNORM_BLOCK,       // VGPU_PIXEL_FORMAT_ASTC12x12
    };

    return formats[format];
}

static VkImageType _vgpuVkGetImageType(VGpuTextureType type)
{
    static const VkImageType types[VGPU_TEXTURE_TYPE_COUNT] = {
        VK_IMAGE_TYPE_1D,
        VK_IMAGE_TYPE_2D,
        VK_IMAGE_TYPE_3D,
        VK_IMAGE_TYPE_2D
    };

    return types[type];
}

static VkSampleCountFlagBits _vgpuVkGetSampleCount(uint32_t sample_count)
{
    switch (sample_count)
    {
    case 0u:
    case 1u:  return VK_SAMPLE_COUNT_1_BIT;
    case 2u:  return VK_SAMPLE_COUNT_2_BIT;
    case 4u:  return VK_SAMPLE_COUNT_4_BIT;
    case 8u:  return VK_SAMPLE_COUNT_8_BIT;
    case 16u: return VK_SAMPLE_COUNT_16_BIT;
    case 32u: return VK_SAMPLE_COUNT_32_BIT;
    case 64u: return VK_SAMPLE_COUNT_64_BIT;
    default:  return VK_SAMPLE_COUNT_1_BIT;
    }

    return VK_SAMPLE_COUNT_1_BIT;
}

static VkImageUsageFlags  _vgpuVkGetTextureUsage(
    VGpuTextureUsageFlags usage,
    VGpuPixelFormat format,
    uint32_t sampleCount)
{
    VkImageUsageFlags vkUsage = 0u;
    if (usage & VGPU_TEXTURE_USAGE_SHADER_READ) {
        vkUsage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }

    if (usage & VGPU_TEXTURE_USAGE_SHADER_READ) {
        vkUsage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }

    if (usage & VGPU_TEXTURE_USAGE_SHADER_WRITE) {
        vkUsage |= VK_IMAGE_USAGE_STORAGE_BIT;
    }

    if (usage & VGPU_TEXTURE_USAGE_RENDER_TARGET)
    {
        if (vgpuIsDepthStencilFormat(format)) {
            if (sampleCount > 1)
            {
                vkUsage |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            }
            else
            {
                vkUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            }
        }
        else {
            if (sampleCount > 1)
            {
                vkUsage |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            }
            else
            {
                vkUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            }
        }
    }

    return vkUsage;
}

using namespace std;

VGpuSwapchain _vgpuVkCreateSwapchain(uint32_t width, uint32_t height, VkSurfaceKHR surface, VkSwapchainKHR oldSwapchain, const VGpuSwapchainDescriptor* descriptor);
void _vgpuVkDestroySwapchain(VGpuSwapchain swapchain);

vgpu_result vgpuInitialize(const char* app_name, const VGpuRendererSettings * settings)
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

    const auto has_extension = [&](const char* name) -> bool {
        auto itr = find_if(begin(queried_extensions), end(queried_extensions), [name](const VkExtensionProperties & e) -> bool {
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

        const auto has_layer = [&](const char* name) -> bool {
            auto itr = find_if(begin(queried_layers), end(queried_layers), [name](const VkLayerProperties & e) -> bool {
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
            if (settings->devicePreference == VGPU_DEVICE_PREFERENCE_HIGH_PERFORMANCE) {
                score += 1000u;
            }
            break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            score += 90U;
            if (settings->devicePreference == VGPU_DEVICE_PREFERENCE_LOW_POWER) {
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

    _vk.physicalDevice = gpus[best_device_index];
    VkPhysicalDeviceProperties device_props;
    vkGetPhysicalDeviceProperties(_vk.physicalDevice, &device_props);

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
    vkGetPhysicalDeviceQueueFamilyProperties(_vk.physicalDevice, &queue_count, nullptr);
    vector<VkQueueFamilyProperties> queue_props(queue_count);
    vkGetPhysicalDeviceQueueFamilyProperties(_vk.physicalDevice, &queue_count, queue_props.data());

    for (uint32_t i = 0; i < queue_count; i++)
    {
        VkBool32 supported = _vk.surface == VK_NULL_HANDLE;
        if (_vk.surface != VK_NULL_HANDLE) {
            vkGetPhysicalDeviceSurfaceSupportKHR(_vk.physicalDevice, i, _vk.surface, &supported);
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
        vkGetPhysicalDeviceFeatures2KHR(_vk.physicalDevice, &features);
    else
        vkGetPhysicalDeviceFeatures(_vk.physicalDevice, &features.features);

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

    result = vkCreateDevice(_vk.physicalDevice, &deviceCreateInfo, nullptr, &_vk.device);
    if (result != VK_SUCCESS)
    {
        return VGPU_ERROR_INITIALIZATION_FAILED;
    }

    volkLoadDevice(_vk.device);
    vkGetDeviceQueue(_vk.device, _vk.graphics_queue_family, graphics_queue_index, &_vk.graphics_queue);
    vkGetDeviceQueue(_vk.device, _vk.compute_queue_family, compute_queue_index, &_vk.compute_queue);
    vkGetDeviceQueue(_vk.device, _vk.transfer_queue_family, transfer_queue_index, &_vk.transfer_queue);

    // Initialize vma memory allocator
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
    allocatorCreateInfo.physicalDevice = _vk.physicalDevice;
    allocatorCreateInfo.device = _vk.device;
    allocatorCreateInfo.pVulkanFunctions = &vma_vulkan_func;
    result = vmaCreateAllocator(&allocatorCreateInfo, &_vk.memoryAllocator);
    if (result != VK_SUCCESS)
    {
        //ALIMER_LOGERROR("Vulkan: Failed to create vma allocator");
        return VGPU_ERROR_INITIALIZATION_FAILED;
    }

    // Setup graphics command Pool
    VkCommandPoolCreateInfo commandPoolCreateInfo = {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        nullptr,
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        _vk.graphics_queue_family };

    if (vkCreateCommandPool(_vk.device, &commandPoolCreateInfo, nullptr, &_vk.commandPool) != VK_SUCCESS)
    {
        return VGPU_ERROR_INITIALIZATION_FAILED;
    }

    // Setup per frame resources.
    _vk.max_inflight_frames = 3u;
    if (!_vk.headless)
    {
        _vk.swapchain = _vgpuVkCreateSwapchain(settings->width, settings->height, _vk.surface, VK_NULL_HANDLE, &settings->swapchain);
        _vk.max_inflight_frames = _vk.swapchain->image_count;
    }
    return VGPU_SUCCESS;
}

VkCommandBuffer _vgpu_vk_create_command_buffer()
{
    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = _vk.commandPool;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = 1u;

    VkCommandBuffer handle;
    if (vkAllocateCommandBuffers(_vk.device, &allocateInfo, &handle) != VK_SUCCESS)
    {
        return VK_NULL_HANDLE;
    }

    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    if (vkBeginCommandBuffer(handle, &commandBufferBeginInfo) != VK_SUCCESS)
    {
        return VK_NULL_HANDLE;
    }

    return handle;
}

bool _vgpu_vk_flush_command_buffer(VkCommandBuffer commandBuffer)
{
    VkResult result = VK_SUCCESS;

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        return false;
    }

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &commandBuffer;

    // Create fence to ensure that the command buffer has finished executing
    VkFence fence = VK_NULL_HANDLE;
    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    _vgpuVkLogIfFailed(vkCreateFence(_vk.device, &fenceCreateInfo, NULL, &fence));

    // Submit to the queue
    result = vkQueueSubmit(_vk.graphics_queue, 1, &submit_info, fence);
    if (result != VK_SUCCESS)
    {
        return false;
    }

    // Wait for the fence to signal that command buffer has finished executing.
    result = vkWaitForFences(_vk.device, 1, &fence, VK_TRUE, UINT64_MAX);
    if (result != VK_SUCCESS)
    {
        return false;
    }

    vkDestroyFence(_vk.device, fence, nullptr);
    vkFreeCommandBuffers(_vk.device, _vk.commandPool, 1u, &commandBuffer);

    return true;
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

static void vgpuVkClearImageWithColor(
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

VGpuSwapchain _vgpuVkCreateSwapchain(uint32_t width, uint32_t height, VkSurfaceKHR surface, VkSwapchainKHR oldSwapchain, const VGpuSwapchainDescriptor* descriptor)
{
    VkSurfaceCapabilitiesKHR surface_properties;
    if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_vk.physicalDevice, surface, &surface_properties) != VK_SUCCESS)
    {
        return nullptr;
    }

    // Happens on nVidia Windows when you minimize a window.
    if (surface_properties.maxImageExtent.width == 0
        && surface_properties.maxImageExtent.height == 0)
    {
        return nullptr;
    }

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(_vk.physicalDevice, surface, &format_count, nullptr);
    vector<VkSurfaceFormatKHR> formats(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(_vk.physicalDevice, surface, &format_count, formats.data());

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
            return nullptr;
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
    vkGetPhysicalDeviceSurfacePresentModesKHR(_vk.physicalDevice, surface, &num_present_modes, nullptr);
    vector<VkPresentModeKHR> present_modes(num_present_modes);
    vkGetPhysicalDeviceSurfacePresentModesKHR(_vk.physicalDevice, surface, &num_present_modes, present_modes.data());

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

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.surface = surface;
    createInfo.minImageCount = desired_swapchain_images;
    createInfo.imageFormat = format.format;
    createInfo.imageColorSpace = format.colorSpace;
    createInfo.imageExtent.width = swapchain_size.width;
    createInfo.imageExtent.height = swapchain_size.height;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.preTransform = pre_transform;
    createInfo.compositeAlpha = composite_mode;
    createInfo.presentMode = swapchain_present_mode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = oldSwapchain;

    // Enable transfer source on swap chain images if supported
    if (surface_properties.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
        createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }

    // Enable transfer destination on swap chain images if supported
    if (surface_properties.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
        createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    VkSwapchainKHR vk_handle = VK_NULL_HANDLE;
    VkResult result = vkCreateSwapchainKHR(_vk.device, &createInfo, nullptr, &vk_handle);
    if (result != VK_SUCCESS) {
        return nullptr;
    }

    if (oldSwapchain != VK_NULL_HANDLE)
    {
        /*for (uint32_t i = 0u; i < handle->image_count; i++)
        {
            vkDestroyImageView(_vk.device, handle->image_views[i], nullptr);
        }*/

        vkDestroySwapchainKHR(_vk.device, oldSwapchain, nullptr);
    }

    VGpuSwapchain handle = _VGPU_ALLOC_HANDLE(VGpuSwapchain);
    handle->vk_handle = vk_handle;
    handle->width = swapchain_size.width;
    handle->height = swapchain_size.height;
    handle->format = format.format;
    handle->image_index = 0;

    if (vkGetSwapchainImagesKHR(_vk.device, vk_handle, &handle->image_count, nullptr) != VK_SUCCESS)
    {
        return nullptr;
    }

    handle->images = _VGPU_ALLOCN(VkImage, handle->image_count);

    if (vkGetSwapchainImagesKHR(_vk.device, vk_handle, &handle->image_count, handle->images) != VK_SUCCESS)
    {
        return nullptr;
    }

    // Create image views for swapchain images.
    handle->image_views = _VGPU_ALLOCN(VkImageView, handle->image_count);
    handle->image_semaphores = _VGPU_ALLOCN(VkSemaphore, handle->image_count);

    /* Clear or transition to default image layout */
    const bool canClear = surface_properties.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        nullptr,
        _vk.commandPool,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        1u
    };

    VkCommandBuffer clearImageCmdBuffer = _vgpu_vk_create_command_buffer();

    for (uint32_t i = 0u; i < handle->image_count; ++i)
    {
        // Clear with default value if supported.
        if (canClear)
        {
            // Clear images with default color.
            VkClearColorValue clearColor = {};
            clearColor.float32[0] = descriptor->colorClearValue.r;
            clearColor.float32[1] = descriptor->colorClearValue.g;
            clearColor.float32[2] = descriptor->colorClearValue.b;
            clearColor.float32[3] = descriptor->colorClearValue.a;

            VkImageSubresourceRange clearRange = {};
            clearRange.layerCount = 1;
            clearRange.levelCount = 1;

            // Clear with default color.
            vgpuVkClearImageWithColor(
                clearImageCmdBuffer,
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
                clearImageCmdBuffer,
                handle->images[i],
                VK_IMAGE_ASPECT_COLOR_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
            );
        }

        VkImageViewCreateInfo imageViewCreateInfo = {};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.pNext = nullptr;
        imageViewCreateInfo.flags = 0;
        imageViewCreateInfo.image = handle->images[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = format.format;
        imageViewCreateInfo.components = {
            VK_COMPONENT_SWIZZLE_R,
            VK_COMPONENT_SWIZZLE_G,
            VK_COMPONENT_SWIZZLE_B,
            VK_COMPONENT_SWIZZLE_A
        };
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;

        result = vkCreateImageView(_vk.device, &imageViewCreateInfo, nullptr, &handle->image_views[i]);
        if (result != VK_SUCCESS) {
            return nullptr;
        }

        VkSemaphoreCreateInfo semaphoreCreateInfo = {};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        result = vkCreateSemaphore(_vk.device, &semaphoreCreateInfo, nullptr, &handle->image_semaphores[i]);
        if (result != VK_SUCCESS) {
            return nullptr;
        }
    }

    // Make sure depth/stencil format is supported - fall back to VK_FORMAT_D16_UNORM if not
    if (descriptor->depthStencilFormat != VGPU_PIXEL_FORMAT_UNDEFINED)
    {
        VGpuPixelFormat depthStencilFormat = descriptor->depthStencilFormat;
        VkFormat vkDepthStencilFormat = _vgpuVkGetFormat(depthStencilFormat);

        VkImageFormatProperties properties = {};
        result = vkGetPhysicalDeviceImageFormatProperties(
            _vk.physicalDevice,
            vkDepthStencilFormat,
            VK_IMAGE_TYPE_2D,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            0,
            &properties);

        // Fall back to something that's guaranteed to work
        if (result != VK_SUCCESS)
        {
            depthStencilFormat = VGPU_PIXEL_FORMAT_D16_UNORM;
        }

        handle->depthStencilTexture = vgpuCreateTexture2D(width, height, false, 1u, depthStencilFormat, descriptor->sampleCount, VGPU_TEXTURE_USAGE_RENDER_TARGET);
        handle->depthStencilClearValue.depth = descriptor->depthStencilClearValue.depth;
        handle->depthStencilClearValue.stencil = descriptor->depthStencilClearValue.stencil;
    }

    _vgpu_vk_flush_command_buffer(clearImageCmdBuffer);
    return handle;
}

void _vgpuVkDestroySwapchain(VGpuSwapchain swapchain)
{
    assert(swapchain);
    if (swapchain->vk_handle == VK_NULL_HANDLE) {
        return;
    }

    vkDeviceWaitIdle(_vk.device);

    for (uint32_t i = 0; i < swapchain->image_count; ++i)
    {
        vkDestroyImageView(_vk.device, swapchain->image_views[i], nullptr);
        vkDestroySemaphore(_vk.device, swapchain->image_semaphores[i], nullptr);
    }

    _VGPU_FREE(swapchain->image_views);
    _VGPU_FREE(swapchain->image_semaphores);

    // Destroy depth-stencil texture.
    if (swapchain->depthStencilTexture != nullptr)
    {
        vgpuDestroyTexture(swapchain->depthStencilTexture);
    }

    /*if (swapchain->renderpass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(_vk.device, swapchain->renderpass, NULL);
    }*/

    vkDestroySwapchainKHR(_vk.device, swapchain->vk_handle, nullptr);
}

void vgpuShutdown()
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

    if (!_vk.headless)
    {
        _vgpuVkDestroySwapchain(_vk.swapchain);
    }

    if (_vk.memoryAllocator != VK_NULL_HANDLE)
    {
        VmaStats stats;
        vmaCalculateStats(_vk.memoryAllocator, &stats);

        VGPU_LOGI("Total device memory leaked: %llu bytes.", stats.total.usedBytes);

        vmaDestroyAllocator(_vk.memoryAllocator);
        _vk.memoryAllocator = VK_NULL_HANDLE;
    }

    if (_vk.commandPool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(_vk.device, _vk.commandPool, nullptr);
        _vk.commandPool = VK_NULL_HANDLE;
    }

    if (_vk.device != VK_NULL_HANDLE)
    {
        vkDestroyDevice(_vk.device, nullptr);
        _vk.device = VK_NULL_HANDLE;
    }

    if (_vk.surface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(_vk.instance, _vk.surface, nullptr);
    }

    if (_vk.instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(_vk.instance, nullptr);
    }

    _vk.instance = VK_NULL_HANDLE;
}

vgpu_result vgpuBeginFrame()
{
    uint32_t frame_index = _vk.frame_index;
    if (!_vk.headless)
    {
        VkResult result = vkAcquireNextImageKHR(_vk.device,
            _vk.swapchain->vk_handle,
            UINT64_MAX,
            _vk.swapchain->image_semaphores[frame_index],
            VK_NULL_HANDLE,
            &_vk.swapchain->image_index);
        if (result != VK_SUCCESS) {
            return VGPU_ERROR_BEGIN_FRAME_FAILED;
        }
    }

    return VGPU_SUCCESS;
}

vgpu_result vgpuEndFrame()
{
    vgpu_result result = VGPU_SUCCESS;

    const uint32_t frame_index = _vk.frame_index;
    if (!_vk.headless)
    {
        VkResult present_result = VK_SUCCESS;

        VkPresentInfoKHR present_info = {};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.waitSemaphoreCount = 0;
        present_info.pWaitSemaphores = nullptr;
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &_vk.swapchain->vk_handle;
        present_info.pImageIndices = &_vk.swapchain->image_index;
        present_info.pResults = &present_result;
        vkQueuePresentKHR(_vk.graphics_queue, &present_info);
        if (present_result != VK_SUCCESS)
        {
            return VGPU_ERROR_END_FRAME_FAILED;
        }
    }

    // Advance to next frame.
    _vk.frame_index = (_vk.frame_index + 1u) % _vk.max_inflight_frames;

    return result;
}

VGpuTexture vgpuCreateTexture(const VGpuTextureDescriptor* descriptor)
{
    VkImageCreateInfo createInfo;
    memset(&createInfo, 0, sizeof(createInfo));
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.imageType = _vgpuVkGetImageType(descriptor->type);
    createInfo.format = _vgpuVkGetFormat(descriptor->format);
    createInfo.extent.width = descriptor->width;
    createInfo.extent.height = descriptor->height;
    if (descriptor->type == VGPU_TEXTURE_TYPE_3D)
    {
        createInfo.extent.depth = descriptor->depthOrArraySize;
    }
    else
    {
        createInfo.extent.depth = 1u;
        createInfo.arrayLayers = descriptor->depthOrArraySize;
    }

    createInfo.mipLevels = descriptor->mipLevels;
    createInfo.samples = _vgpuVkGetSampleCount(descriptor->sampleCount);
    createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    createInfo.usage = _vgpuVkGetTextureUsage(descriptor->usage, descriptor->format, descriptor->sampleCount);
    createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (descriptor->type == VGPU_TEXTURE_TYPE_CUBE)
    {
        createInfo.arrayLayers *= 6;
        createInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }

    VmaAllocationCreateInfo memory_info{};
    memory_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    if (createInfo.usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT)
    {
        memory_info.preferredFlags = VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
    }

    VkImage handle;
    VmaAllocation memory;
    VkResult result = vmaCreateImage(_vk.memoryAllocator, &createInfo, &memory_info,
        &handle, &memory, nullptr);

    if (result != VK_SUCCESS) {
        return nullptr;
    }

    VGpuTexture texture = _VGPU_ALLOC_HANDLE(VGpuTexture);
    texture->externalHandle = false;
    texture->handle = handle;
    texture->memory = memory;
    return texture;
}

VGpuTexture vgpuCreateExternalTexture(const VGpuTextureDescriptor* descriptor, void* handle)
{
    VGpuTexture texture = _VGPU_ALLOC_HANDLE(VGpuTexture);
    texture->handle = (VkImage)handle;
    texture->memory = VK_NULL_HANDLE;
    texture->externalHandle = true;
    return texture;
}

void vgpuDestroyTexture(VGpuTexture texture)
{
    assert(texture);
    if (!texture->externalHandle)
    {
        if (texture->memory != VK_NULL_HANDLE)
        {
            vmaDestroyImage(_vk.memoryAllocator, texture->handle, texture->memory);
        }
        else
        {
            vkDestroyImage(_vk.device, texture->handle, nullptr);
        }
    }

    _VGPU_FREE(texture);
}

#endif
