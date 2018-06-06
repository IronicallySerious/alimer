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
#pragma once

#define VULKAN_EXPORT
#ifndef NOMINMAX
#	define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#endif

#include <vulkan/vulkan.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#pragma warning(push)
#pragma warning(disable : 4467)
#include <wrl.h>
#pragma warning(pop)
using namespace Microsoft::WRL;

#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <utility>

#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP)
#	define VK_D3D_UWP 1 // Universal Windows platform
#elif defined(_WIN64) || defined(_WIN32)
#	define VK_D3D_WINDOWS 1
#endif

#if VK_D3D_WINDOWS
extern decltype(CreateDXGIFactory2)* CreateDXGIFactory2Fn;
extern PFN_D3D12_GET_DEBUG_INTERFACE D3D12GetDebugInterfaceFn;
extern PFN_D3D12_CREATE_DEVICE D3D12CreateDeviceFn;
HRESULT D3D12LoadLibraries();
#endif

#define ICD_LOADER_MAGIC 0x01CDC0DE

typedef union {
    uintptr_t loaderMagic;
    void *loaderData;
} VK_LOADER_DATA;

template <class T>
inline void VKD3D_UNUSED(T const&)
{
}

static inline bool IsPowerOfTwo(uintptr_t value) {
    // Test POT:  (x != 0) && ((x & (x - 1)) == 0)
    return value && ((value & (value - 1)) == 0);
}

static inline uint32_t GetMipmapLevels(uint32_t dim)
{
    if (!IsPowerOfTwo(dim)) { return 0; }

    uint32_t exp = 0;
    while (dim) {
        exp++;
        dim >>= 1;
    }
    return exp;
}

static inline uint32_t GetMipmapLevels3D(VkExtent3D extent)
{
    uint32_t maxDim = std::max({ extent.width, extent.height, extent.depth });
    return std::max(GetMipmapLevels(maxDim), 1U);
}

static inline uint32_t GetMipmapLevels2D(VkExtent2D extent)
{
    return GetMipmapLevels3D({ extent.width, extent.height, 1 });
}

inline VkResult VkResultFromHRESULT(HRESULT hr)
{
    switch (hr)
    {
    case ERROR_DEVICE_REMOVED:
        return VK_ERROR_DEVICE_LOST;
    case E_OUTOFMEMORY:
        return VK_ERROR_OUT_OF_DEVICE_MEMORY;
    default:
        return VK_ERROR_INCOMPATIBLE_DRIVER;
    }
}

static inline int GetRefCount(IUnknown* _interface)
{
    _interface->AddRef();
    return _interface->Release();
}

struct VkInstance_T
{
    VkApplicationInfo        applicationInfo;
    std::vector<std::string> enabledLayerNames;
    std::vector<std::string> enabledExtensionNames;
    ComPtr<IDXGIFactory4>    dxgiFactory;
    std::vector<VkPhysicalDevice> physicalDevices;
    std::unordered_map<std::string, PFN_vkVoidFunction> procAddrMap;
};

struct VkPhysicalDevice_T
{
    VkInstance            instance;
    ComPtr<IDXGIAdapter1> adapter;
    DXGI_ADAPTER_DESC1    adapterDesc;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceMemoryProperties memoryProperties;
};

struct VkDevice_T
{
    ComPtr<ID3D12Device>              d3dDevice;
    std::vector<VkQueue>              queues;
    VkPhysicalDevice                  physicalDevice;
    D3D12_FEATURE_DATA_D3D12_OPTIONS  dataOptions;
    D3D12_FEATURE_DATA_ROOT_SIGNATURE dataRootSignature;
};

struct VkQueue_T
{
    ComPtr<ID3D12CommandQueue> d3dCommandQueue;
};

HRESULT privateD3D12GetDebugInterface(_In_ REFIID riid, _COM_Outptr_opt_ void** ppvDebug);
HRESULT privateCreateDXGIFactory2(UINT Flags, REFIID riid, _COM_Outptr_ void **ppFactory);

HRESULT privateD3D12CreateDevice(
    _In_opt_ IUnknown* pAdapter,
    D3D_FEATURE_LEVEL MinimumFeatureLevel,
    _In_ REFIID riid, // Expected: ID3D12Device
    _COM_Outptr_opt_ void** ppDevice);
