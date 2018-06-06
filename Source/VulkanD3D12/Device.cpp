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

static constexpr uint32_t QueueFamilyGeneralPresent = 0;
static constexpr uint32_t QueueFamilyGeneral = 1;
static constexpr uint32_t QueueFamilyCompute = 2;
static constexpr uint32_t QueueFamilyCopy = 3;

#if VK_D3D_WINDOWS
decltype(CreateDXGIFactory2)* CreateDXGIFactory2Fn = nullptr;
PFN_D3D12_GET_DEBUG_INTERFACE D3D12GetDebugInterfaceFn = nullptr;
PFN_D3D12_CREATE_DEVICE D3D12CreateDeviceFn = nullptr;
#endif

HRESULT privateD3D12GetDebugInterface(
    _In_ REFIID riid,
    _COM_Outptr_opt_ void** ppvDebug)
{
#ifdef VK_D3D_UWP
    return D3D12GetDebugInterface(riid, ppvDebug);
#else
    return D3D12GetDebugInterfaceFn(riid, ppvDebug);
#endif
}

HRESULT privateCreateDXGIFactory2(UINT Flags, REFIID riid, _COM_Outptr_ void **ppFactory)
{
#ifdef VK_D3D_UWP
    return CreateDXGIFactory2(Flags, riid, ppFactory);
#else
    return CreateDXGIFactory2Fn(Flags, riid, ppFactory);
#endif
}

HRESULT privateD3D12CreateDevice(
    _In_opt_ IUnknown* pAdapter,
    D3D_FEATURE_LEVEL MinimumFeatureLevel,
    _In_ REFIID riid, // Expected: ID3D12Device
    _COM_Outptr_opt_ void** ppDevice)
{
#ifdef VK_D3D_UWP
    return D3D12CreateDevice(pAdapter, MinimumFeatureLevel, riid, ppDevice);
#else
    return D3D12CreateDeviceFn(pAdapter, MinimumFeatureLevel, riid, ppDevice);
#endif
}

VKAPI_ATTR VkResult vkCreateDevice(
    VkPhysicalDevice             physicalDevice,
    const VkDeviceCreateInfo*    pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDevice*                    pDevice)
{
    VkDevice device = nullptr;
    if (pAllocator)
    {
        device = static_cast<VkDevice>(pAllocator->pfnAllocation(nullptr, sizeof(VkDevice_T), 8, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE));
    }
    else
    {
        device = new VkDevice_T();
    }

    HRESULT hr = privateD3D12CreateDevice(
        physicalDevice->adapter.Get(),
        D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(&device->d3dDevice)
    );

    if (FAILED(hr))
    {
        return VkResultFromHRESULT(hr);
    }

    hr = device->d3dDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &device->dataOptions, sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS));
    if (FAILED(hr))
    {
        return VkResultFromHRESULT(hr);
    }

    hr = device->d3dDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &device->dataRootSignature, sizeof(D3D12_FEATURE_DATA_ROOT_SIGNATURE));
    if (FAILED(hr))
    {
        device->dataRootSignature.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    for (uint32_t i = 0; i < pCreateInfo->queueCreateInfoCount; ++i)
    {
        VkQueue queue = nullptr;
        if (pAllocator)
        {
            queue = static_cast<VkQueue>(pAllocator->pfnAllocation(nullptr, sizeof(VkQueue_T), 8, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE));
        }
        else
        {
            queue = new VkQueue_T();
        }

        const uint32_t queueFamilyIndex = pCreateInfo->pQueueCreateInfos[i].queueFamilyIndex;
        const float priority = pCreateInfo->pQueueCreateInfos->pQueuePriorities[0];

        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        switch (queueFamilyIndex)
        {
        case QueueFamilyGeneralPresent:
        case QueueFamilyGeneral:
            queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            break;
        case QueueFamilyCompute:
            queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
            break;

        case QueueFamilyCopy:
            queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY; break;
        }

        queueDesc.Priority = priority < 0.5 ? D3D12_COMMAND_QUEUE_PRIORITY_NORMAL : D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.NodeMask = 0;

        hr = device->d3dDevice->CreateCommandQueue(
            &queueDesc,
            IID_PPV_ARGS(queue->d3dCommandQueue.GetAddressOf())
        );
        if (FAILED(hr))
        {
            return VkResultFromHRESULT(hr);
        }
        device->queues.push_back(queue);
    }

    *pDevice = device;
    return VK_SUCCESS;
}

VKAPI_ATTR void vkDestroyDevice(
    VkDevice                     device,
    const VkAllocationCallbacks* pAllocator)
{
    if (pAllocator)
    {
        pAllocator->pfnFree(nullptr, device);
    }
    else
    {
        delete device;
    }

    device = nullptr;
}

VKAPI_ATTR void vkGetDeviceQueue(
    VkDevice device,
    uint32_t queueFamilyIndex,
    uint32_t queueIndex,
    VkQueue* pQueue)
{
    *pQueue = device->queues[queueIndex];
}

VKAPI_ATTR PFN_vkVoidFunction vkGetDeviceProcAddr(VkDevice device, const char* pName)
{
    // TODO
    return nullptr;
}
