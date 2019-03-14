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

#ifdef _DEBUG
#include <dxgidebug.h>
#if !defined(_XBOX_ONE) || !defined(_TITLE) || !defined(_DURANGO)
#   pragma comment(lib,"dxguid.lib")
#endif
#endif

#ifdef ALIMER_D3D12_DYNAMIC_LIB
typedef HRESULT(WINAPI* PFN_CREATE_DXGI_FACTORY2)(UINT flags, REFIID _riid, void** _factory);
typedef HRESULT(WINAPI* PFN_GET_DXGI_DEBUG_INTERFACE1)(UINT Flags, REFIID riid, _COM_Outptr_ void** pDebug);
#endif

using namespace Microsoft::WRL;

namespace alimer
{
#ifdef ALIMER_D3D12_DYNAMIC_LIB
    static HMODULE s_dxgiLib = nullptr;
    static HMODULE s_d3d12Lib = nullptr;

    PFN_CREATE_DXGI_FACTORY2                        CreateDXGIFactory2 = nullptr;
    PFN_GET_DXGI_DEBUG_INTERFACE1                   DXGIGetDebugInterface1 = nullptr;

    PFN_D3D12_GET_DEBUG_INTERFACE                   D3D12GetDebugInterface = nullptr;
    PFN_D3D12_CREATE_DEVICE                         D3D12CreateDevice = nullptr;
    PFN_D3D12_SERIALIZE_ROOT_SIGNATURE              D3D12SerializeRootSignature = nullptr;
    PFN_D3D12_SERIALIZE_VERSIONED_ROOT_SIGNATURE    D3D12SerializeVersionedRootSignature = nullptr;

    static inline HRESULT D3D12LoadLibraries()
    {
        static bool loaded = false;
        if (loaded) {
            return S_OK;
        }

        loaded = true;

        // Load libraries first.
        s_dxgiLib = LoadLibraryW(L"dxgi.dll");
        if (!s_dxgiLib)
        {
            OutputDebugStringW(L"Failed to load dxgi.dll");
            return S_FALSE;
        }

        s_d3d12Lib = LoadLibraryW(L"d3d12.dll");
        if (!s_d3d12Lib)
        {
            OutputDebugStringW(L"Failed to load d3d12.dll");
            return S_FALSE;
        }

        /* DXGI entry points */
        CreateDXGIFactory2 = (PFN_CREATE_DXGI_FACTORY2)GetProcAddress(s_dxgiLib, "CreateDXGIFactory2");
        DXGIGetDebugInterface1 = (PFN_GET_DXGI_DEBUG_INTERFACE1)GetProcAddress(s_dxgiLib, "DXGIGetDebugInterface1");
        if (CreateDXGIFactory2 == nullptr)
        {
            OutputDebugStringW(L"Cannot find CreateDXGIFactory2 entry point.");
            return S_FALSE;
        }

        /* D3D12 entry points */
        D3D12GetDebugInterface = (PFN_D3D12_GET_DEBUG_INTERFACE)GetProcAddress(s_d3d12Lib, "D3D12GetDebugInterface");
        D3D12CreateDevice = (PFN_D3D12_CREATE_DEVICE)GetProcAddress(s_d3d12Lib, "D3D12CreateDevice");
        D3D12SerializeRootSignature = (PFN_D3D12_SERIALIZE_ROOT_SIGNATURE)GetProcAddress(s_d3d12Lib, "D3D12SerializeRootSignature");
        D3D12SerializeVersionedRootSignature = (PFN_D3D12_SERIALIZE_VERSIONED_ROOT_SIGNATURE)GetProcAddress(s_d3d12Lib, "D3D12SerializeVersionedRootSignature");

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

    GraphicsDeviceFactoryD3D12::GraphicsDeviceFactoryD3D12(bool validation)
        : GraphicsDeviceFactory(GraphicsBackend::Direct3D12)
    {
#if ALIMER_D3D12_DYNAMIC_LIB
        if (FAILED(D3D12LoadLibraries()))
        {
            return;
        }
#endif

        UINT dxgiFactoryFlags = 0;

#ifdef _DEBUG
        // Enable the debug layer (requires the Graphics Tools "optional feature").
        if (validation)
        {
            ComPtr<ID3D12Debug> debugController;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
            {
                debugController->EnableDebugLayer();

                ComPtr<ID3D12Debug1> d3d12debug1;
                if (SUCCEEDED(debugController.As(&d3d12debug1)))
                {
                    d3d12debug1->SetEnableGPUBasedValidation(true);
                }

                // Enable additional debug layers.
                dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
            }
            else
            {
                validation = false;
                //ALIMER_LOGWARN("Direct3D Debug Device is not available");
            }
        }
#endif
    }

    GraphicsDeviceFactoryD3D12::~GraphicsDeviceFactoryD3D12()
    {
    }
}
