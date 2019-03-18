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

#include "GraphicsDeviceFactoryD3D12.h"
#include "GraphicsDeviceD3D12.h"
#include "SwapChainD3D12.h"
#include "foundation/Utils.h"

#ifdef _DEBUG
#if !defined(_XBOX_ONE) || !defined(_TITLE) || !defined(_DURANGO)
#   pragma comment(lib,"dxguid.lib")
#endif
#endif

namespace alimer
{
#ifdef ALIMER_D3D12_DYNAMIC_LIB
    PFN_CREATE_DXGI_FACTORY2        CreateDXGIFactory2;
    PFN_GET_DXGI_DEBUG_INTERFACE1   DXGIGetDebugInterface1;

    PFN_D3D12_GET_DEBUG_INTERFACE                   D3D12GetDebugInterface;
    PFN_D3D12_CREATE_DEVICE                         D3D12CreateDevice;
    PFN_D3D12_SERIALIZE_ROOT_SIGNATURE              D3D12SerializeRootSignature;
    PFN_D3D12_SERIALIZE_VERSIONED_ROOT_SIGNATURE    D3D12SerializeVersionedRootSignature;

    static inline HRESULT D3D12LoadLibraries()
    {
        static bool loaded = false;
        if (loaded) {
            return S_OK;
        }

        loaded = true;

        // Load libraries first.
        HMODULE dxgiModule = LoadLibraryW(L"dxgi.dll");
        if (!dxgiModule)
        {
            OutputDebugStringW(L"Failed to load dxgi.dll");
            return S_FALSE;
        }

        HMODULE d3d12Module = LoadLibraryW(L"d3d12.dll");
        if (!d3d12Module)
        {
            OutputDebugStringW(L"Failed to load d3d12.dll");
            return S_FALSE;
        }

        /* DXGI entry points */
        CreateDXGIFactory2 = (PFN_CREATE_DXGI_FACTORY2)GetProcAddress(dxgiModule, "CreateDXGIFactory2");
        DXGIGetDebugInterface1 = (PFN_GET_DXGI_DEBUG_INTERFACE1)GetProcAddress(dxgiModule, "DXGIGetDebugInterface1");
        if (CreateDXGIFactory2 == nullptr)
        {
            OutputDebugStringW(L"Cannot find CreateDXGIFactory2 entry point.");
            return S_FALSE;
        }

        /* D3D12 entry points */
        D3D12GetDebugInterface = (PFN_D3D12_GET_DEBUG_INTERFACE)GetProcAddress(d3d12Module, "D3D12GetDebugInterface");
        D3D12CreateDevice = (PFN_D3D12_CREATE_DEVICE)GetProcAddress(d3d12Module, "D3D12CreateDevice");
        D3D12SerializeRootSignature = (PFN_D3D12_SERIALIZE_ROOT_SIGNATURE)GetProcAddress(d3d12Module, "D3D12SerializeRootSignature");
        D3D12SerializeVersionedRootSignature = (PFN_D3D12_SERIALIZE_VERSIONED_ROOT_SIGNATURE)GetProcAddress(d3d12Module, "D3D12SerializeVersionedRootSignature");

        if (!D3D12CreateDevice)
        {
            OutputDebugStringW(L"Cannot find D3D12CreateDevice entry point.");
            return S_FALSE;
        }

        if (!D3D12SerializeRootSignature)
        {
            OutputDebugStringW(L"Cannot find D3D12SerializeRootSignature entry point.");
            return S_FALSE;
        }

        return S_OK;
    }
#endif

#if defined(__dxgi1_6_h__) && defined(NTDDI_WIN10_RS4)
    static DXGI_GPU_PREFERENCE GetDXGIGpuPreference(PowerPreference preference)
    {
        switch (preference)
        {
        case PowerPreference::LowPower:
            return DXGI_GPU_PREFERENCE_MINIMUM_POWER;
        case PowerPreference::HighPerformance:
            return DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE;
        default:
            return DXGI_GPU_PREFERENCE_UNSPECIFIED;
        }
    }
#endif

    bool GraphicsDeviceFactoryD3D12::IsSupported()
    {
        static bool availableCheck = false;
        static bool isAvailable = false;

        if (availableCheck) {
            return isAvailable;
        }

        availableCheck = true;

#if ALIMER_D3D12_DYNAMIC_LIB
        if (FAILED(D3D12LoadLibraries()))
        {
            return false;
        }
#endif

        // Create temp dxgi factory for check support.
        IDXGIFactory4* factory;
        HRESULT hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));
        if (FAILED(hr))
        {
            isAvailable = false;
            return false;
        }

        IDXGIAdapter1* hardwareAdapter;
        GetAdapter(factory, PowerPreference::Default, &hardwareAdapter);
        isAvailable = hardwareAdapter != nullptr;
        SafeRelease(hardwareAdapter);
        ALIMER_VERIFY(factory->Release() == 0);
        return isAvailable;
    }

    GraphicsDeviceFactoryD3D12::GraphicsDeviceFactoryD3D12(bool validation)
        : GraphicsDeviceFactory(GraphicsBackend::Direct3D12)
    {
        ALIMER_ASSERT_MSG(IsSupported(), "D3D12 backend is not supported");

        UINT dxgiFactoryFlags = 0;

#if D3D12_DEBUG
        // Enable the debug layer (requires the Graphics Tools "optional feature").
        if (validation)
        {
            ID3D12Debug* debugController;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
            {
                debugController->EnableDebugLayer();

                ID3D12Debug1* d3d12debug1;
                if (SUCCEEDED(debugController->QueryInterface(&d3d12debug1)))
                {
                    d3d12debug1->SetEnableGPUBasedValidation(true);
                    d3d12debug1->Release();
                }

                // Enable additional debug layers.
                dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
                _validation = true;
                debugController->Release();
            }
            else
            {
                _validation = false;
                //ALIMER_LOGWARN("Direct3D Debug Device is not available");
            }
        }
#endif

        HRESULT hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&_factory));
        if (FAILED(hr))
        {
            return;
        }

        IDXGIFactory5* factory5;
        if (SUCCEEDED(_factory->QueryInterface(&factory5)))
        {
            BOOL allowTearing = FALSE;
            hr = factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
            if (SUCCEEDED(hr) && allowTearing)
            {
                _allowTearing = true;
            }

            factory5->Release();
        }
    }

    GraphicsDeviceFactoryD3D12::~GraphicsDeviceFactoryD3D12()
    {
        ALIMER_VERIFY(_factory->Release() == 0);
    }

    void GraphicsDeviceFactoryD3D12::GetAdapter(_In_ IDXGIFactory2* factory, PowerPreference preference, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter)
    {
        IDXGIAdapter1* adapter;
        *ppAdapter = nullptr;

#if defined(__dxgi1_6_h__) && defined(NTDDI_WIN10_RS4)
        IDXGIFactory6* factory6;
        if (SUCCEEDED(factory->QueryInterface(&factory6)))
        {
            auto dxgiGpuPreference = GetDXGIGpuPreference(preference);
            for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != factory6->EnumAdapterByGpuPreference(adapterIndex, dxgiGpuPreference, IID_PPV_ARGS(&adapter)); adapterIndex++)
            {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    // Don't select the Basic Render Driver adapter.
                    continue;
                }

                // Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
                {
                    break;
                }
            }
            factory6->Release();
        }
        else
        {
#endif
            ALIMER_UNUSED(preference);
            for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
            {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    // Don't select the Basic Render Driver adapter.
                    continue;
                }

                // Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
                {
                    break;
                }
            }
        }

        *ppAdapter = adapter;
    }

    GraphicsDevice* GraphicsDeviceFactoryD3D12::CreateDeviceImpl(const AdapterDescriptor* adapterDescription)
    {
        IDXGIAdapter1* adapter;
        GetAdapter(_factory, adapterDescription->powerPreference, &adapter);

        return new GraphicsDeviceD3D12(this, adapter);
    }

    SwapChainSurface* GraphicsDeviceFactoryD3D12::CreateSurfaceFromWin32Impl(void *hInstance, void *hWnd)
    {
        return new SwapChainSurfaceD3D12(hInstance, hWnd);
    }
}
