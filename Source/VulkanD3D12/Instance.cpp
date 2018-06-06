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

#if VK_D3D_WINDOWS
static bool s_d3d12Initialized = false;
static HMODULE DXGIDebugHandle = nullptr;
static HMODULE DXGIHandle = nullptr;
static HMODULE D3D12Handle = nullptr;

HRESULT D3D12LoadLibraries()
{
    if (s_d3d12Initialized)
        return S_OK;

    DXGIDebugHandle = LoadLibraryW(L"dxgidebug.dll");
    DXGIHandle = LoadLibraryW(L"dxgi.dll");
    D3D12Handle = LoadLibraryW(L"d3d12.dll");

    if (!DXGIHandle)
        return S_FALSE;
    if (!D3D12Handle)
        return S_FALSE;

    // Load symbols.
    //DXGIGetDebugInterface1Fn = (PFN_GET_DXGI_DEBUG_INTERFACE)::GetProcAddress(DXGIHandle, "DXGIGetDebugInterface1");
    CreateDXGIFactory2Fn = (decltype(CreateDXGIFactory2Fn))::GetProcAddress(DXGIHandle, "CreateDXGIFactory2");
    D3D12CreateDeviceFn = (PFN_D3D12_CREATE_DEVICE)::GetProcAddress(D3D12Handle, "D3D12CreateDevice");
    D3D12GetDebugInterfaceFn =(PFN_D3D12_GET_DEBUG_INTERFACE)::GetProcAddress(D3D12Handle, "D3D12GetDebugInterface");
    //D3D12SerializeRootSignatureFn =  (PFN_D3D12_SERIALIZE_ROOT_SIGNATURE)::GetProcAddress(D3D12Handle, "D3D12SerializeRootSignature");

    if (!CreateDXGIFactory2Fn)
        return S_FALSE;
    if (!D3D12CreateDeviceFn)
        return S_FALSE;
    if (!D3D12GetDebugInterfaceFn)
        return S_FALSE;
    //if (!D3D12SerializeRootSignatureFn)
    //   return S_FALSE;

    // Done.
    s_d3d12Initialized = true;
    return S_OK;
}
#endif

void InitProcAddrs(VkInstance instance);

VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceVersion(uint32_t* pApiVersion)
{
    if (pApiVersion)
        *pApiVersion = VK_MAKE_VERSION(1, 1, 0);

    return VK_SUCCESS;
}

VKAPI_ATTR VkResult vkCreateInstance(
    const VkInstanceCreateInfo*  pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkInstance*                  pInstance)
{
#if VK_D3D_WINDOWS
    D3D12LoadLibraries();
#endif

    VkInstance instance = nullptr;
    if (pAllocator)
    {
        instance = static_cast<VkInstance>(pAllocator->pfnAllocation(nullptr, sizeof(VkInstance_T), 8, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE));
    }
    else
    {
        instance = new VkInstance_T();
    }

    instance->applicationInfo = *pCreateInfo->pApplicationInfo;
    for (uint32_t i = 0; i < pCreateInfo->enabledLayerCount; ++i)
    {
        instance->enabledLayerNames.push_back(pCreateInfo->ppEnabledLayerNames[i]);
    }
    for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; ++i)
    {
        instance->enabledExtensionNames.push_back(pCreateInfo->ppEnabledExtensionNames[i]);
    }

    UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
    // Enable the debug layer (requires the Graphics Tools "optional feature").
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(privateD3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();

            // Enable additional debug layers.
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }
#endif

    HRESULT hr = privateCreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&instance->dxgiFactory));
    if (FAILED(hr))
    {
        return VK_ERROR_INCOMPATIBLE_DRIVER;
    }

    // Enum devices.
    ComPtr<IDXGIAdapter1> adapter;
    for (UINT adapterIndex = 0;
        instance->dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND; ++adapterIndex)
    {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            // Don't select the Basic Render Driver adapter.
            // If you want a software adapter, pass in "/warp" on the command line.
            continue;
        }

        // Check to see if the adapter supports Direct3D 12.
        ComPtr<ID3D12Device> device;
        if (SUCCEEDED(privateD3D12CreateDevice(
            adapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&device))))
        {
            VkPhysicalDevice physicalDevice = new VkPhysicalDevice_T();
            physicalDevice->instance = instance;
            physicalDevice->adapter = adapter;
            physicalDevice->adapterDesc = desc;

            // Init features.
            physicalDevice->features.robustBufferAccess;
            physicalDevice->features.fullDrawIndexUint32 = VK_TRUE;
            physicalDevice->features.imageCubeArray = VK_TRUE;
            physicalDevice->features.independentBlend = VK_TRUE;
            physicalDevice->features.geometryShader = VK_TRUE;
            physicalDevice->features.tessellationShader = VK_TRUE;
            physicalDevice->features.sampleRateShading;
            physicalDevice->features.dualSrcBlend = VK_TRUE;
            physicalDevice->features.logicOp = VK_TRUE;
            physicalDevice->features.multiDrawIndirect;
            physicalDevice->features.drawIndirectFirstInstance;
            physicalDevice->features.depthClamp = VK_TRUE;
            physicalDevice->features.depthBiasClamp = VK_TRUE;
            physicalDevice->features.fillModeNonSolid = VK_TRUE;
            physicalDevice->features.depthBounds = VK_TRUE;
            physicalDevice->features.wideLines;
            physicalDevice->features.largePoints;
            physicalDevice->features.alphaToOne = VK_TRUE;
            physicalDevice->features.multiViewport = VK_TRUE;
            physicalDevice->features.samplerAnisotropy = VK_TRUE;
            physicalDevice->features.textureCompressionETC2 = VK_FALSE;
            physicalDevice->features.textureCompressionASTC_LDR = VK_FALSE;
            physicalDevice->features.textureCompressionBC = VK_TRUE;
            physicalDevice->features.occlusionQueryPrecise;
            physicalDevice->features.pipelineStatisticsQuery;
            physicalDevice->features.vertexPipelineStoresAndAtomics;
            physicalDevice->features.fragmentStoresAndAtomics;
            physicalDevice->features.shaderTessellationAndGeometryPointSize;
            physicalDevice->features.shaderImageGatherExtended;
            physicalDevice->features.shaderStorageImageExtendedFormats;
            physicalDevice->features.shaderStorageImageMultisample;
            physicalDevice->features.shaderStorageImageReadWithoutFormat;
            physicalDevice->features.shaderStorageImageWriteWithoutFormat;
            physicalDevice->features.shaderUniformBufferArrayDynamicIndexing;
            physicalDevice->features.shaderSampledImageArrayDynamicIndexing;
            physicalDevice->features.shaderStorageBufferArrayDynamicIndexing;
            physicalDevice->features.shaderStorageImageArrayDynamicIndexing;
            physicalDevice->features.shaderClipDistance;
            physicalDevice->features.shaderCullDistance;
            physicalDevice->features.shaderFloat64 = VK_TRUE;
            physicalDevice->features.shaderInt64 = VK_FALSE;
            physicalDevice->features.shaderInt16 = VK_FALSE;
            physicalDevice->features.shaderResourceResidency;
            physicalDevice->features.shaderResourceMinLod;
            physicalDevice->features.sparseBinding;
            physicalDevice->features.sparseResidencyBuffer;
            physicalDevice->features.sparseResidencyImage2D;
            physicalDevice->features.sparseResidencyImage3D;
            physicalDevice->features.sparseResidency2Samples;
            physicalDevice->features.sparseResidency4Samples;
            physicalDevice->features.sparseResidency8Samples;
            physicalDevice->features.sparseResidency16Samples;
            physicalDevice->features.sparseResidencyAliased;
            physicalDevice->features.variableMultisampleRate;
            physicalDevice->features.inheritedQueries;

            VkPhysicalDeviceLimits limits = {};
            limits.maxImageDimension1D = D3D12_REQ_TEXTURE1D_U_DIMENSION;
            limits.maxImageDimension2D = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
            limits.maxImageDimension3D = D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;
            limits.maxImageDimensionCube = D3D12_REQ_TEXTURECUBE_DIMENSION;
            limits.maxImageArrayLayers = D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;
            limits.maxTexelBufferElements;
            limits.maxUniformBufferRange;
            limits.maxStorageBufferRange;
            limits.maxPushConstantsSize;
            limits.maxMemoryAllocationCount;
            limits.maxSamplerAllocationCount;
            limits.bufferImageGranularity;
            limits.sparseAddressSpaceSize;
            limits.maxBoundDescriptorSets;
            limits.maxPerStageDescriptorSamplers;
            limits.maxPerStageDescriptorUniformBuffers;
            limits.maxPerStageDescriptorStorageBuffers;
            limits.maxPerStageDescriptorSampledImages;
            limits.maxPerStageDescriptorStorageImages;
            limits.maxPerStageDescriptorInputAttachments;
            limits.maxPerStageResources;
            limits.maxDescriptorSetSamplers;
            limits.maxDescriptorSetUniformBuffers;
            limits.maxDescriptorSetUniformBuffersDynamic;
            limits.maxDescriptorSetStorageBuffers;
            limits.maxDescriptorSetStorageBuffersDynamic;
            limits.maxDescriptorSetSampledImages;
            limits.maxDescriptorSetStorageImages;
            limits.maxDescriptorSetInputAttachments;
            limits.maxVertexInputAttributes;
            limits.maxVertexInputBindings;
            limits.maxVertexInputAttributeOffset;
            limits.maxVertexInputBindingStride;
            limits.maxVertexOutputComponents;
            limits.maxTessellationGenerationLevel;
            limits.maxTessellationPatchSize;
            limits.maxTessellationControlPerVertexInputComponents;
            limits.maxTessellationControlPerVertexOutputComponents;
            limits.maxTessellationControlPerPatchOutputComponents;
            limits.maxTessellationControlTotalOutputComponents;
            limits.maxTessellationEvaluationInputComponents;
            limits.maxTessellationEvaluationOutputComponents;
            limits.maxGeometryShaderInvocations;
            limits.maxGeometryInputComponents;
            limits.maxGeometryOutputComponents;
            limits.maxGeometryOutputVertices;
            limits.maxGeometryTotalOutputComponents;
            limits.maxFragmentInputComponents;
            limits.maxFragmentOutputAttachments;
            limits.maxFragmentDualSrcAttachments;
            limits.maxFragmentCombinedOutputResources;
            limits.maxComputeSharedMemorySize;
            limits.maxComputeWorkGroupCount[0];
            limits.maxComputeWorkGroupCount[1];
            limits.maxComputeWorkGroupCount[2];
            limits.maxComputeWorkGroupInvocations = D3D12_CS_THREAD_GROUP_MAX_THREADS_PER_GROUP;
            limits.maxComputeWorkGroupSize[0] = D3D12_CS_THREAD_GROUP_MAX_X;
            limits.maxComputeWorkGroupSize[1] = D3D12_CS_THREAD_GROUP_MAX_Y;
            limits.maxComputeWorkGroupSize[2] = D3D12_CS_THREAD_GROUP_MAX_Z;
            limits.subPixelPrecisionBits = D3D12_SUBPIXEL_FRACTIONAL_BIT_COUNT;
            limits.subTexelPrecisionBits = D3D12_SUBTEXEL_FRACTIONAL_BIT_COUNT;
            limits.mipmapPrecisionBits;
            limits.maxDrawIndexedIndexValue;
            limits.maxDrawIndirectCount;
            limits.maxSamplerLodBias;
            limits.maxSamplerAnisotropy;
            limits.maxViewports;
            limits.maxViewportDimensions[0];
            limits.maxViewportDimensions[1];
            limits.viewportBoundsRange[0];
            limits.viewportBoundsRange[1];
            limits.viewportSubPixelBits;
            limits.minMemoryMapAlignment;
            limits.minTexelBufferOffsetAlignment;
            limits.minUniformBufferOffsetAlignment;
            limits.minStorageBufferOffsetAlignment;
            limits.minTexelOffset = D3D12_COMMONSHADER_TEXEL_OFFSET_MAX_NEGATIVE;
            limits.maxTexelOffset = D3D12_COMMONSHADER_TEXEL_OFFSET_MAX_POSITIVE;
            limits.minTexelGatherOffset;
            limits.maxTexelGatherOffset;
            limits.minInterpolationOffset;
            limits.maxInterpolationOffset;
            limits.subPixelInterpolationOffsetBits;
            limits.maxFramebufferWidth;
            limits.maxFramebufferHeight;
            limits.maxFramebufferLayers;
            limits.framebufferColorSampleCounts;
            limits.framebufferDepthSampleCounts;
            limits.framebufferStencilSampleCounts;
            limits.framebufferNoAttachmentsSampleCounts;
            limits.maxColorAttachments;
            limits.sampledImageColorSampleCounts;
            limits.sampledImageIntegerSampleCounts;
            limits.sampledImageDepthSampleCounts;
            limits.sampledImageStencilSampleCounts;
            limits.storageImageSampleCounts;
            limits.maxSampleMaskWords;
            limits.timestampComputeAndGraphics;
            limits.timestampPeriod;
            limits.maxClipDistances = D3D12_CLIP_OR_CULL_DISTANCE_COUNT;
            limits.maxCullDistances = D3D12_CLIP_OR_CULL_DISTANCE_COUNT;
            limits.maxCombinedClipAndCullDistances = D3D12_CLIP_OR_CULL_DISTANCE_COUNT;
            limits.discreteQueuePriorities;
            limits.pointSizeRange[0];
            limits.pointSizeRange[1];
            limits.lineWidthRange[0];
            limits.lineWidthRange[1];
            limits.pointSizeGranularity;
            limits.lineWidthGranularity;
            limits.strictLines;
            limits.standardSampleLocations;
            limits.optimalBufferCopyOffsetAlignment;
            limits.optimalBufferCopyRowPitchAlignment;
            limits.nonCoherentAtomSize;

            VkPhysicalDeviceSparseProperties sparseProperties = {};
            sparseProperties.residencyStandard2DBlockShape;
            sparseProperties.residencyStandard2DMultisampleBlockShape;
            sparseProperties.residencyStandard3DBlockShape;
            sparseProperties.residencyAlignedMipSize;
            sparseProperties.residencyNonResidentStrict;

            physicalDevice->properties.apiVersion = VK_MAKE_VERSION(1, 1, 0);
            physicalDevice->properties.driverVersion = desc.Revision;
            physicalDevice->properties.vendorID = desc.VendorId;
            physicalDevice->properties.deviceID = desc.DeviceId;
            physicalDevice->properties.deviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
            physicalDevice->properties.limits = limits;
            physicalDevice->properties.sparseProperties = sparseProperties;

            // Set the device name
            wcstombs_s(nullptr, physicalDevice->properties.deviceName, desc.Description, VK_MAX_PHYSICAL_DEVICE_NAME_SIZE);

            // Set the pipeline cache UUID
            snprintf(reinterpret_cast<char*>(&physicalDevice->properties.pipelineCacheUUID), VK_UUID_SIZE, "D3D12-%i", desc.AdapterLuid.LowPart);

            // Memory properties.
            physicalDevice->memoryProperties.memoryTypeCount = 3;
            physicalDevice->memoryProperties.memoryTypes[0].propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            physicalDevice->memoryProperties.memoryTypes[0].heapIndex = 0;
            physicalDevice->memoryProperties.memoryTypes[1].propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            physicalDevice->memoryProperties.memoryTypes[1].heapIndex = 1;
            physicalDevice->memoryProperties.memoryTypes[2].propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            physicalDevice->memoryProperties.memoryTypes[2].heapIndex = 2;
            physicalDevice->memoryProperties.memoryHeapCount = 3;
            physicalDevice->memoryProperties.memoryHeaps[0].size = desc.DedicatedVideoMemory;
            physicalDevice->memoryProperties.memoryHeaps[0].flags = VK_MEMORY_HEAP_DEVICE_LOCAL_BIT;
            physicalDevice->memoryProperties.memoryHeaps[1].size = desc.DedicatedSystemMemory;
            physicalDevice->memoryProperties.memoryHeaps[1].flags = 0;
            physicalDevice->memoryProperties.memoryHeaps[2].size = desc.SharedSystemMemory;
            physicalDevice->memoryProperties.memoryHeaps[2].flags = 0;

            instance->physicalDevices.push_back(physicalDevice);
            continue;
        }
    }

    // Init function pointers
    InitProcAddrs(instance);

    *pInstance = instance;
    return VK_SUCCESS;
}

VKAPI_ATTR void vkDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator)
{
    if (pAllocator)
    {
        pAllocator->pfnFree(nullptr, instance);
    }
    else
    {
        delete instance;
    }
    instance = nullptr;
}

VKAPI_ATTR VkResult vkEnumerateInstanceExtensionProperties(
    const char*                                 pLayerName,
    uint32_t*                                   pCount,
    VkExtensionProperties*                      pProperties) {

    if (pCount) *pCount = 0;
    if (pProperties) pProperties = nullptr;
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult vkEnumerateInstanceLayerProperties(
    uint32_t*          pPropertyCount,
    VkLayerProperties* pProperties)
{
    if (pPropertyCount) *pPropertyCount = 0;
    if (pProperties) pProperties = nullptr;

    return VK_SUCCESS;
}

VKAPI_ATTR VkResult vkEnumeratePhysicalDevices(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceCount,
    VkPhysicalDevice*                           pPhysicalDevices)
{

    if (!pPhysicalDevices)
    {
        *pPhysicalDeviceCount = static_cast<uint32_t>(instance->physicalDevices.size());
        return VK_SUCCESS;
    }

    for (size_t i = 0; i < instance->physicalDevices.size(); ++i)
    {
        pPhysicalDevices[i] = instance->physicalDevices[i];
    }

    return VK_SUCCESS;
}

void InitProcAddrs(VkInstance instance)
{
#define ADD_PROC_ADDR(entrypoint)	instance->procAddrMap[""#entrypoint] = (PFN_vkVoidFunction)&entrypoint;

    // Instance functions
    ADD_PROC_ADDR(vkDestroyInstance);
    ADD_PROC_ADDR(vkEnumeratePhysicalDevices);
    ADD_PROC_ADDR(vkGetPhysicalDeviceFeatures);
    ADD_PROC_ADDR(vkGetPhysicalDeviceFormatProperties);
    ADD_PROC_ADDR(vkGetPhysicalDeviceImageFormatProperties);
    ADD_PROC_ADDR(vkGetPhysicalDeviceProperties);
    ADD_PROC_ADDR(vkGetPhysicalDeviceQueueFamilyProperties);
    ADD_PROC_ADDR(vkGetPhysicalDeviceMemoryProperties);
    ADD_PROC_ADDR(vkGetInstanceProcAddr);
    ADD_PROC_ADDR(vkGetDeviceProcAddr);
    ADD_PROC_ADDR(vkCreateDevice);
    ADD_PROC_ADDR(vkEnumerateDeviceExtensionProperties);
    ADD_PROC_ADDR(vkEnumerateDeviceLayerProperties);
    ADD_PROC_ADDR(vkGetPhysicalDeviceSparseImageFormatProperties);

    // Device functions:
    ADD_PROC_ADDR(vkDestroyDevice);
    ADD_PROC_ADDR(vkGetDeviceQueue);
    /*ADD_PROC_ADDR(vkQueueSubmit);
    ADD_PROC_ADDR(vkQueueWaitIdle);
    ADD_PROC_ADDR(vkDeviceWaitIdle);
    ADD_PROC_ADDR(vkAllocateMemory);
    ADD_PROC_ADDR(vkFreeMemory);
    ADD_PROC_ADDR(vkMapMemory);
    ADD_PROC_ADDR(vkUnmapMemory);
    ADD_PROC_ADDR(vkFlushMappedMemoryRanges);
    ADD_PROC_ADDR(vkInvalidateMappedMemoryRanges);
    ADD_PROC_ADDR(vkGetDeviceMemoryCommitment);
    ADD_PROC_ADDR(vkBindBufferMemory);
    ADD_PROC_ADDR(vkBindImageMemory);
    ADD_PROC_ADDR(vkGetBufferMemoryRequirements);
    ADD_PROC_ADDR(vkGetImageMemoryRequirements);
    ADD_PROC_ADDR(vkGetImageSparseMemoryRequirements);
    ADD_PROC_ADDR(vkQueueBindSparse);
    ADD_PROC_ADDR(vkCreateFence);
    ADD_PROC_ADDR(vkDestroyFence);
    ADD_PROC_ADDR(vkResetFences);
    ADD_PROC_ADDR(vkGetFenceStatus);
    ADD_PROC_ADDR(vkWaitForFences);
    ADD_PROC_ADDR(vkCreateSemaphore);
    ADD_PROC_ADDR(vkDestroySemaphore);
    ADD_PROC_ADDR(vkCreateEvent);
    ADD_PROC_ADDR(vkDestroyEvent);
    ADD_PROC_ADDR(vkGetEventStatus);
    ADD_PROC_ADDR(vkSetEvent);
    ADD_PROC_ADDR(vkResetEvent);
    ADD_PROC_ADDR(vkCreateQueryPool);
    ADD_PROC_ADDR(vkDestroyQueryPool);
    ADD_PROC_ADDR(vkGetQueryPoolResults);
    ADD_PROC_ADDR(vkCreateBuffer);
    ADD_PROC_ADDR(vkDestroyBuffer);
    ADD_PROC_ADDR(vkCreateBufferView);
    ADD_PROC_ADDR(vkDestroyBufferView);
    ADD_PROC_ADDR(vkCreateImage);
    ADD_PROC_ADDR(vkDestroyImage);
    ADD_PROC_ADDR(vkGetImageSubresourceLayout);
    ADD_PROC_ADDR(vkCreateImageView);
    ADD_PROC_ADDR(vkDestroyImageView);
    ADD_PROC_ADDR(vkCreateShaderModule);
    ADD_PROC_ADDR(vkDestroyShaderModule);
    ADD_PROC_ADDR(vkCreatePipelineCache);
    ADD_PROC_ADDR(vkDestroyPipelineCache);
    ADD_PROC_ADDR(vkGetPipelineCacheData);
    ADD_PROC_ADDR(vkMergePipelineCaches);
    ADD_PROC_ADDR(vkCreateGraphicsPipelines);
    ADD_PROC_ADDR(vkCreateComputePipelines);
    ADD_PROC_ADDR(vkDestroyPipeline);
    ADD_PROC_ADDR(vkCreatePipelineLayout);
    ADD_PROC_ADDR(vkDestroyPipelineLayout);
    ADD_PROC_ADDR(vkCreateSampler);
    ADD_PROC_ADDR(vkDestroySampler);
    ADD_PROC_ADDR(vkCreateDescriptorSetLayout);
    ADD_PROC_ADDR(vkDestroyDescriptorSetLayout);
    ADD_PROC_ADDR(vkCreateDescriptorPool);
    ADD_PROC_ADDR(vkDestroyDescriptorPool);
    ADD_PROC_ADDR(vkResetDescriptorPool);
    ADD_PROC_ADDR(vkAllocateDescriptorSets);
    ADD_PROC_ADDR(vkFreeDescriptorSets);
    ADD_PROC_ADDR(vkUpdateDescriptorSets);
    ADD_PROC_ADDR(vkCreateFramebuffer);
    ADD_PROC_ADDR(vkDestroyFramebuffer);
    ADD_PROC_ADDR(vkCreateRenderPass);
    ADD_PROC_ADDR(vkDestroyRenderPass);
    ADD_PROC_ADDR(vkGetRenderAreaGranularity);
    ADD_PROC_ADDR(vkCreateCommandPool);
    ADD_PROC_ADDR(vkDestroyCommandPool);
    ADD_PROC_ADDR(vkResetCommandPool);
    ADD_PROC_ADDR(vkAllocateCommandBuffers);
    ADD_PROC_ADDR(vkFreeCommandBuffers);
    ADD_PROC_ADDR(vkBeginCommandBuffer);
    ADD_PROC_ADDR(vkEndCommandBuffer);
    ADD_PROC_ADDR(vkResetCommandBuffer);
    ADD_PROC_ADDR(vkCmdBindPipeline);
    ADD_PROC_ADDR(vkCmdSetViewport);
    ADD_PROC_ADDR(vkCmdSetScissor);
    ADD_PROC_ADDR(vkCmdSetLineWidth);
    ADD_PROC_ADDR(vkCmdSetDepthBias);
    ADD_PROC_ADDR(vkCmdSetBlendConstants);
    ADD_PROC_ADDR(vkCmdSetDepthBounds);
    ADD_PROC_ADDR(vkCmdSetStencilCompareMask);
    ADD_PROC_ADDR(vkCmdSetStencilWriteMask);
    ADD_PROC_ADDR(vkCmdSetStencilReference);
    ADD_PROC_ADDR(vkCmdBindDescriptorSets);
    ADD_PROC_ADDR(vkCmdBindIndexBuffer);
    ADD_PROC_ADDR(vkCmdBindVertexBuffers);
    ADD_PROC_ADDR(vkCmdDraw);
    ADD_PROC_ADDR(vkCmdDrawIndexed);
    ADD_PROC_ADDR(vkCmdDrawIndirect);
    ADD_PROC_ADDR(vkCmdDrawIndexedIndirect);
    ADD_PROC_ADDR(vkCmdDispatch);
    ADD_PROC_ADDR(vkCmdDispatchIndirect);
    ADD_PROC_ADDR(vkCmdCopyBuffer);
    ADD_PROC_ADDR(vkCmdCopyImage);
    ADD_PROC_ADDR(vkCmdBlitImage);
    ADD_PROC_ADDR(vkCmdCopyBufferToImage);
    ADD_PROC_ADDR(vkCmdCopyImageToBuffer);
    ADD_PROC_ADDR(vkCmdUpdateBuffer);
    ADD_PROC_ADDR(vkCmdFillBuffer);
    ADD_PROC_ADDR(vkCmdClearColorImage);
    ADD_PROC_ADDR(vkCmdClearDepthStencilImage);
    ADD_PROC_ADDR(vkCmdClearAttachments);
    ADD_PROC_ADDR(vkCmdResolveImage);
    ADD_PROC_ADDR(vkCmdSetEvent);
    ADD_PROC_ADDR(vkCmdResetEvent);
    ADD_PROC_ADDR(vkCmdWaitEvents);
    ADD_PROC_ADDR(vkCmdPipelineBarrier);
    ADD_PROC_ADDR(vkCmdBeginQuery);
    ADD_PROC_ADDR(vkCmdEndQuery);
    ADD_PROC_ADDR(vkCmdResetQueryPool);
    ADD_PROC_ADDR(vkCmdWriteTimestamp);
    ADD_PROC_ADDR(vkCmdCopyQueryPoolResults);
    ADD_PROC_ADDR(vkCmdPushConstants);
    ADD_PROC_ADDR(vkCmdBeginRenderPass);
    ADD_PROC_ADDR(vkCmdNextSubpass);
    ADD_PROC_ADDR(vkCmdEndRenderPass);
    ADD_PROC_ADDR(vkCmdExecuteCommands);

    // Supported extensions:
    ADD_PROC_ADDR(vkDestroySurfaceKHR);
    ADD_PROC_ADDR(vkGetPhysicalDeviceSurfaceSupportKHR);
    ADD_PROC_ADDR(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
    ADD_PROC_ADDR(vkGetPhysicalDeviceSurfaceFormatsKHR);
    ADD_PROC_ADDR(vkGetPhysicalDeviceSurfacePresentModesKHR);
    ADD_PROC_ADDR(vkCreateSwapchainKHR);
    ADD_PROC_ADDR(vkDestroySwapchainKHR);
    ADD_PROC_ADDR(vkGetSwapchainImagesKHR);
    ADD_PROC_ADDR(vkAcquireNextImageKHR);
    ADD_PROC_ADDR(vkQueuePresentKHR);*/

#undef ADD_PROC_ADDR
}
