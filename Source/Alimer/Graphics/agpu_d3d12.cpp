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

#include "agpu.h"

#if AGPU_D3D12
#define AGPU_IMPLEMENTATION
#include "agpu_backend.h"
#include "Core/Log.h"

using Microsoft::WRL::ComPtr;

#if defined(_DEBUG) || defined(PROFILE)
#   if !defined(_XBOX_ONE) || !defined(_TITLE) || !defined(_DURANGO)
#       pragma comment(lib,"dxguid.lib")
#   endif
#endif

void agpuD3D12GetDXGIAdapter(AGpuRenderer* renderer, IDXGIAdapter1** ppAdapter)
{
    *ppAdapter = nullptr;

    HRESULT hr = S_OK;
    bool releaseFactory = false;
    if (!renderer->d3d12.factory)
    {
        hr = renderer->dynLib.CreateDXGIFactory1(IID_PPV_ARGS(&renderer->d3d12.factory));
        if (FAILED(hr))
        {
            ALIMER_LOGERROR("Unable to create a DXGI factory. Make sure that your OS and driver support DirectX 12");
            return;
        }
        releaseFactory = true;
    }

    ComPtr<IDXGIAdapter1> adapter;
#if defined(__dxgi1_6_h__) && defined(NTDDI_WIN10_RS4)
    ComPtr<IDXGIFactory6> factory6;
    hr = renderer->d3d12.factory->QueryInterface(factory6.ReleaseAndGetAddressOf());
    if (SUCCEEDED(hr))
    {
        for (UINT adapterIndex = 0;
            DXGI_ERROR_NOT_FOUND != factory6->EnumAdapterByGpuPreference(
                adapterIndex,
                DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
                IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf()));
            adapterIndex++)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Don't select the Basic Render Driver adapter.
                continue;
            }

            // Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
            if (SUCCEEDED(renderer->dynLib.D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
            {
#ifdef _DEBUG
                wchar_t buff[256] = {};
                swprintf_s(buff, L"Direct3D Adapter (%u): VID:%04X, PID:%04X - %ls\n", adapterIndex, desc.VendorId, desc.DeviceId, desc.Description);
                OutputDebugStringW(buff);
#endif
                break;
            }
        }
    }
    else
#endif
        for (UINT adapterIndex = 0;
            DXGI_ERROR_NOT_FOUND != renderer->d3d12.factory->EnumAdapters1(
                adapterIndex,
                adapter.ReleaseAndGetAddressOf());
            ++adapterIndex)
    {
        DXGI_ADAPTER_DESC1 desc;
        if (FAILED(adapter->GetDesc1(&desc)))
        {
            ALIMER_LOGERROR("DXGI - Failed to get desc");
        }

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            // Don't select the Basic Render Driver adapter.
            continue;
        }

        // Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
        if (SUCCEEDED(renderer->dynLib.D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
        {
#ifdef _DEBUG
            wchar_t buff[256] = {};
            swprintf_s(buff, L"Direct3D Adapter (%u): VID:%04X, PID:%04X - %ls\n", adapterIndex, desc.VendorId, desc.DeviceId, desc.Description);
            OutputDebugStringW(buff);
#endif
            break;
        }
    }

#if !defined(NDEBUG)
    if (!adapter)
    {
        // Try WARP12 instead
        if (FAILED(renderer->d3d12.factory->EnumWarpAdapter(IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf()))))
        {
            throw std::exception("WARP12 not available. Enable the 'Graphics Tools' optional feature");
        }

        OutputDebugStringA("Direct3D Adapter - WARP12\n");
    }
#endif

    if (!adapter)
    {
        throw std::exception("No Direct3D 12 device found");
    }

    *ppAdapter = adapter.Detach();

    if (releaseFactory)
    {
        renderer->d3d12.factory.Reset();
    }
}

void agpuShutdownD3D12Backend(AGpuRenderer* renderer);
AgpuResult agpuD3D12BeginFrame(AGpuRenderer* renderer);
uint64_t agpuD3D12EndFrame(AGpuRenderer* renderer);
void agpuD3D12WaitForGpu(AGpuRenderer* renderer);

AgpuBool32 agpuIsD3D12Supported(AGpuRenderer* renderer)
{
    static AgpuBool32 availableCheck = AGPU_FALSE;
    static AgpuBool32 isAvailable = AGPU_FALSE;

    if (availableCheck)
        return isAvailable;

    availableCheck = AGPU_TRUE;
#if AGPU_D3D_DYNAMIC_LIB
    // DXGI
    renderer->dynLib.dxgiLib = LoadLibraryW(L"dxgi.dll");
    if (!renderer->dynLib.dxgiLib)
    {
        OutputDebugStringW(L"Failed to load dxgi.dll");
        availableCheck = AGPU_FALSE;
        return availableCheck;
    }

    renderer->dynLib.CreateDXGIFactory1 = (PFN_CREATE_DXGI_FACTORY1)GetProcAddress(renderer->dynLib.dxgiLib, "CreateDXGIFactory1");
    renderer->dynLib.CreateDXGIFactory2 = (PFN_CREATE_DXGI_FACTORY2)GetProcAddress(renderer->dynLib.dxgiLib, "CreateDXGIFactory2");
    renderer->dynLib.DXGIGetDebugInterface1 = (PFN_GET_DXGI_DEBUG_INTERFACE1)GetProcAddress(renderer->dynLib.dxgiLib, "DXGIGetDebugInterface1");
    if (!renderer->dynLib.CreateDXGIFactory1)
    {
        OutputDebugStringW(L"Cannot find CreateDXGIFactory1 entry point.");
        availableCheck = AGPU_FALSE;
        return availableCheck;
    }

    renderer->dynLib.d3d12Lib = LoadLibraryW(L"d3d12.dll");
    if (!renderer->dynLib.d3d12Lib)
    {
        OutputDebugStringW(L"Failed to load d3d12.dll");
        availableCheck = AGPU_FALSE;
        return availableCheck;
    }

    renderer->dynLib.D3D12GetDebugInterface = (PFN_D3D12_GET_DEBUG_INTERFACE)GetProcAddress(renderer->dynLib.d3d12Lib, "D3D12GetDebugInterface");
    renderer->dynLib.D3D12CreateDevice = (PFN_D3D12_CREATE_DEVICE)GetProcAddress(renderer->dynLib.d3d12Lib, "D3D12CreateDevice");
    renderer->dynLib.D3D12SerializeRootSignature = (PFN_D3D12_SERIALIZE_ROOT_SIGNATURE)GetProcAddress(renderer->dynLib.d3d12Lib, "D3D12SerializeRootSignature");
    if (!renderer->dynLib.D3D12CreateDevice)
    {
        OutputDebugStringW(L"Cannot find D3D12CreateDevice entry point.");
        availableCheck = AGPU_FALSE;
        return availableCheck;
    }

    if (!renderer->dynLib.D3D12SerializeRootSignature)
    {
        OutputDebugStringW(L"Cannot find D3D12SerializeRootSignature entry point.");
        availableCheck = AGPU_FALSE;
        return availableCheck;
    }
#else
    s_d3d11data.dynLib.CreateDXGIFactory1 = CreateDXGIFactory1;
    s_d3d11data.dynLib.CreateDXGIFactory2 = CreateDXGIFactory2;
    s_d3d11data.dynLib.DXGIGetDebugInterface = DXGIGetDebugInterface;
    s_d3d11data.dynLib.DXGIGetDebugInterface1 = DXGIGetDebugInterface1;

    // D3D12
    s_d3d12data.dynLib.D3D12GetDebugInterface = D3D12GetDebugInterface;
    s_d3d12data.dynLib.D3D12CreateDevice = D3D12CreateDevice;
    s_d3d12data.dynLib.D3D12SerializeRootSignature = D3D12SerializeRootSignature;
#endif

    ComPtr<IDXGIAdapter1> adapter;
    agpuD3D12GetDXGIAdapter(renderer, adapter.GetAddressOf());
    isAvailable = adapter != nullptr;
    return isAvailable;
}

AgpuResult agpuSetupD3D12Backend(AGpuRenderer* renderer, const AgpuDescriptor* descriptor)
{
    renderer->d3d12.factoryFlags = 0;

#if defined(_DEBUG)
    // Enable the debug layer (requires the Graphics Tools "optional feature").
    if (descriptor->validation)
    {
        ComPtr<ID3D12Debug> d3d12debug;
        if (SUCCEEDED(renderer->dynLib.D3D12GetDebugInterface(IID_PPV_ARGS(d3d12debug.GetAddressOf()))))
        {
            d3d12debug->EnableDebugLayer();

            ComPtr<ID3D12Debug1> d3d12debug1;
            if (d3d12debug.As(&d3d12debug1))
            {
                d3d12debug1->SetEnableGPUBasedValidation(true);
            }
        }
        else
        {
            ALIMER_LOGWARN("Direct3D Debug Device is not available");
        }

        ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
        if (SUCCEEDED(renderer->dynLib.DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf()))))
        {
            renderer->d3d12.factoryFlags = DXGI_CREATE_FACTORY_DEBUG;

            dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
            dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);
        }
    }
#endif

    HRESULT hr = renderer->dynLib.CreateDXGIFactory2(renderer->d3d12.factoryFlags, IID_PPV_ARGS(&renderer->d3d12.factory));
    if (FAILED(hr))
    {
        ALIMER_LOGERROR("Unable to create a DXGI 1.4 device. Make sure that your OS and driver support DirectX 12");
        return AGPU_ERROR;
    }

    agpuD3D12GetDXGIAdapter(renderer, &renderer->d3d12.adapter);
    hr = renderer->dynLib.D3D12CreateDevice(
        renderer->d3d12.adapter.Get(),
        D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(&renderer->d3d12.device)
    );
    if (FAILED(hr))
    {
        ALIMER_LOGERROR("Unable to create a DXGI 1.4 device. Make sure that your OS and driver support DirectX 12");
        return AGPU_ERROR;
    }

    renderer->d3d12.device->SetName(L"AlimerID3D12Device");

#ifndef NDEBUG
    if (descriptor->validation)
    {
        // Configure debug device (if active).
        ComPtr<ID3D12InfoQueue> d3dInfoQueue;
        if (SUCCEEDED(renderer->d3d12.device.As(&d3dInfoQueue)))
        {
#ifdef _DEBUG
            d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
            d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
#endif
            D3D12_MESSAGE_ID hide[] =
            {
                D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
                D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
                D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE
            };
            D3D12_INFO_QUEUE_FILTER filter = {};
            filter.DenyList.NumIDs = _countof(hide);
            filter.DenyList.pIDList = hide;
            d3dInfoQueue->AddStorageFilterEntries(&filter);
        }
    }
#endif

    // Determine maximum supported feature level for this device
    static const D3D_FEATURE_LEVEL s_featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_12_1,
    };

    D3D12_FEATURE_DATA_FEATURE_LEVELS dataFeatureLevels = { };
    dataFeatureLevels.NumFeatureLevels = _countof(s_featureLevels);
    dataFeatureLevels.pFeatureLevelsRequested = s_featureLevels;
    hr = renderer->d3d12.device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &dataFeatureLevels, sizeof(dataFeatureLevels));
    if (SUCCEEDED(hr))
    {
        renderer->d3d12.featureLevel = dataFeatureLevels.MaxSupportedFeatureLevel;
    }
    else
    {
        renderer->d3d12.featureLevel = D3D_FEATURE_LEVEL_11_0;
    }

    /*D3D12_FEATURE_DATA_D3D12_OPTIONS5 dataOptions5 = { };
    hr = renderer->d3d12.device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &dataOptions5, sizeof(dataOptions5));
    if (SUCCEEDED(hr))
    {
    }*/

    // Create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    hr = renderer->d3d12.device->CreateCommandQueue(
        &queueDesc,
        IID_PPV_ARGS(renderer->d3d12.commandQueue.ReleaseAndGetAddressOf())
    );
    renderer->d3d12.commandQueue->SetName(L"MainQueue");

    // Create a command allocator for each back buffer that will be rendered to.
    const UINT m_backBufferCount = 2;
    for (UINT n = 0; n < m_backBufferCount; n++)
    {
        hr = renderer->d3d12.device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(renderer->d3d12.commandAllocators[n].ReleaseAndGetAddressOf()));

        wchar_t name[25] = {};
        swprintf_s(name, L"CommandAllocator %u", n);
        renderer->d3d12.commandAllocators[n]->SetName(name);
    }

    // Create a command list for recording graphics commands.
    hr = renderer->d3d12.device->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        renderer->d3d12.commandAllocators[0].Get(),
        nullptr,
        IID_PPV_ARGS(renderer->d3d12.commandList.ReleaseAndGetAddressOf())
    );
    hr = renderer->d3d12.commandList->Close();
    renderer->d3d12.commandList->SetName(L"MainDirectCommandList");

    // Setup backend callbacks.
    renderer->shutdown = agpuShutdownD3D12Backend;
    renderer->beginFrame = agpuD3D12BeginFrame;
    renderer->endFrame = agpuD3D12EndFrame;
    return AGPU_OK;
}

void agpuShutdownD3D12Backend(AGpuRenderer* renderer)
{
    agpuD3D12WaitForGpu(renderer);

    renderer->d3d12.device.Reset();
    renderer->d3d12.adapter.Reset();
    renderer->d3d12.factory.Reset();

#ifdef _DEBUG
    {
        ComPtr<IDXGIDebug1> dxgiDebug;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
        {
            dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
        }
    }
#endif
}

void agpuD3D12WaitForGpu(AGpuRenderer* renderer)
{
}

AgpuResult agpuD3D12BeginFrame(AGpuRenderer* renderer)
{
    // Reset command list and allocator.
    ID3D12CommandAllocator* commandAllocator = renderer->d3d12.commandAllocators[renderer->d3d12.backBufferIndex].Get();
    HRESULT hr = commandAllocator->Reset();
    if (FAILED(hr))
    {
        ALIMER_LOGERROR("D3D12 - ID3D12CommandAllocator Reset failed.");
    }

    hr = renderer->d3d12.commandList->Reset(commandAllocator, nullptr);
    if (FAILED(hr))
    {
        ALIMER_LOGERROR("D3D12 - ID3D12CommandList Reset failed.");
    }
    // Transition the render target into the correct state to allow for drawing into it.
    //D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_backBufferIndex].Get(), beforeState, D3D12_RESOURCE_STATE_RENDER_TARGET);
    //renderer->d3d12.commandList->ResourceBarrier(1, &barrier);
    return AGPU_OK;
}

uint64_t agpuD3D12EndFrame(AGpuRenderer* renderer)
{
    ID3D12GraphicsCommandList* commandList = renderer->d3d12.commandList.Get();
    // Transition the render target to the state that allows it to be presented to the display.
    //D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_backBufferIndex].Get(), beforeState, D3D12_RESOURCE_STATE_PRESENT);
    //commandList->ResourceBarrier(1, &barrier);

    // Send the command list off to the GPU for processing.
    HRESULT hr = commandList->Close();
    if (FAILED(hr))
    {
        ALIMER_LOGERROR("D3D12 - ID3D12CommandList Close failed.");
    }

    ID3D12CommandList* ppCommandLists[] = { commandList };
    renderer->d3d12.commandQueue->ExecuteCommandLists(1, ppCommandLists);

    return 0;
}

#endif /* AGPU_D3D12 */
