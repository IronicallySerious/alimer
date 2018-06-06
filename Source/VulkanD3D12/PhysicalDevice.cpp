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

#include "VulkanD3D.h"

VKAPI_ATTR void vkGetPhysicalDeviceFeatures(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures*                   pFeatures)
{
    if (pFeatures) { *pFeatures = physicalDevice->features; }
}

VKAPI_ATTR void vkGetPhysicalDeviceProperties(
    VkPhysicalDevice            physicalDevice,
    VkPhysicalDeviceProperties* pProperties)
{
    if (pProperties) { *pProperties = physicalDevice->properties; }
}

VKAPI_ATTR void vkGetPhysicalDeviceFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkFormatProperties*                         pFormatProperties)
{
}

VKAPI_ATTR VkResult vkGetPhysicalDeviceImageFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkImageTiling                               tiling,
    VkImageUsageFlags                           usage,
    VkImageCreateFlags                          flags,
    VkImageFormatProperties*                    pImageFormatProperties)
{
    //if (!D3DVkFormatIsSupported(format)) { return VK_ERROR_FORMAT_NOT_SUPPORTED; }

    if (!pImageFormatProperties) { return VK_SUCCESS; }

    VkPhysicalDeviceLimits* pLimits = &physicalDevice->properties.limits;
    VkExtent3D maxExt;
    uint32_t maxLayers;
    switch (type)
    {
    case VK_IMAGE_TYPE_1D:
        maxExt.width = pLimits->maxImageDimension1D;
        maxExt.height = 1;
        maxExt.depth = 1;
        maxLayers = pLimits->maxImageArrayLayers;
        break;
    case VK_IMAGE_TYPE_2D:
        maxExt.width = pLimits->maxImageDimension2D;
        maxExt.height = pLimits->maxImageDimension2D;
        maxExt.depth = 1;
        maxLayers = pLimits->maxImageArrayLayers;
        break;
    case VK_IMAGE_TYPE_3D:
        maxExt.width = pLimits->maxImageDimension3D;
        maxExt.height = pLimits->maxImageDimension3D;
        maxExt.depth = pLimits->maxImageDimension3D;
        maxLayers = 1;
        break;
    default:
        maxExt = { 1, 1, 1 };
        maxLayers = 1;
        break;
    }


    pImageFormatProperties->maxExtent = maxExt;
    pImageFormatProperties->maxMipLevels = GetMipmapLevels3D(maxExt);
    pImageFormatProperties->maxArrayLayers = maxLayers;
    pImageFormatProperties->sampleCounts = VK_SAMPLE_COUNT_1_BIT;
    pImageFormatProperties->maxResourceSize = 0;

    return VK_SUCCESS;
}

VKAPI_ATTR void vkGetPhysicalDeviceQueueFamilyProperties(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pQueueFamilyPropertyCount,
    VkQueueFamilyProperties*                    pQueueFamilyProperties)
{
    if (pQueueFamilyPropertyCount)
    {
        *pQueueFamilyPropertyCount = 2;
    }

    if (pQueueFamilyProperties)
    {
        VkQueueFamilyProperties queueFamilyProperties = {};
        queueFamilyProperties.queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
        queueFamilyProperties.queueCount = 1;
        queueFamilyProperties.timestampValidBits = 0;
        queueFamilyProperties.minImageTransferGranularity = VkExtent3D{ 1, 1, 1 };

        VkQueueFamilyProperties copyQueueFamilyProperties = {};
        copyQueueFamilyProperties.queueFlags = VK_QUEUE_TRANSFER_BIT;
        copyQueueFamilyProperties.queueCount = 1;
        copyQueueFamilyProperties.timestampValidBits = 0;
        copyQueueFamilyProperties.minImageTransferGranularity = VkExtent3D{ 1, 1, 1 };

        pQueueFamilyProperties[0] = queueFamilyProperties;
        pQueueFamilyProperties[1] = copyQueueFamilyProperties;
    }
}

VKAPI_ATTR void vkGetPhysicalDeviceMemoryProperties(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties*           pMemoryProperties)
{
    if (pMemoryProperties)
    {
        *pMemoryProperties = physicalDevice->memoryProperties;
    }
}

VKAPI_ATTR VkResult vkEnumerateDeviceExtensionProperties(
    VkPhysicalDevice                            physicalDevice,
    const char*                                 pLayerName,
    uint32_t*                                   pPropertyCount,
    VkExtensionProperties*                      pProperties)
{
    constexpr uint32_t propertyCount = 2;
    if (pPropertyCount)
        *pPropertyCount = propertyCount;

    if (pProperties)
    {
        // TODO: Support more
        constexpr VkExtensionProperties properties[propertyCount] =
        {
            { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_SWAPCHAIN_SPEC_VERSION },
            { VK_KHR_MAINTENANCE1_EXTENSION_NAME, VK_KHR_MAINTENANCE1_SPEC_VERSION }
        };

        for (uint32_t i = 0; i < propertyCount; ++i)
        {
            pProperties[i] = properties[i];
        }
    }

    return VK_SUCCESS;
}

VKAPI_ATTR VkResult vkEnumerateDeviceLayerProperties(
    VkPhysicalDevice   physicalDevice,
    uint32_t*          pPropertyCount,
    VkLayerProperties* pProperties)
{
    // TODO:
    if (pPropertyCount) *pPropertyCount = 0;
    if (pProperties) pProperties = nullptr;

    return VK_SUCCESS;
}

VKAPI_ATTR void vkGetPhysicalDeviceSparseImageFormatProperties(
    VkPhysicalDevice               physicalDevice,
    VkFormat                       format,
    VkImageType                    type,
    VkSampleCountFlagBits          samples,
    VkImageUsageFlags              usage,
    VkImageTiling                  tiling,
    uint32_t*                      pPropertyCount,
    VkSparseImageFormatProperties* pProperties)
{
    // TODO:
    VkSparseImageFormatProperties properties = {};
    properties.aspectMask;
    properties.imageGranularity;
    properties.flags;

    *pProperties = properties;
}
