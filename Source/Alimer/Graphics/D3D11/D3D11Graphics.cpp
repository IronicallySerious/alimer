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

#include "D3D11Graphics.h"
#include "D3D11SwapChain.h"
#include "D3D11RenderPass.h"
#include "D3D11Texture.h"
#include "D3D11CommandBuffer.h"
#include "D3D11GpuBuffer.h"
#include "D3D11Shader.h"
#include "D3D11PipelineState.h"
#include "D3D11GpuAdapter.h"
#include "../ShaderCompiler.h"
#include "../../Core/Log.h"
#include "../../Application/Windows/WindowWindows.h"

using namespace Microsoft::WRL;

namespace Alimer
{
#if defined(_DEBUG)
    // Check for SDK Layer support.
    inline bool SdkLayersAvailable()
    {
        HRESULT hr = D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_NULL,       // There is no need to create a real hardware device.
            0,
            D3D11_CREATE_DEVICE_DEBUG,  // Check for the SDK layers.
            nullptr,                    // Any feature level will do.
            0,
            D3D11_SDK_VERSION,
            nullptr,                    // No need to keep the D3D device reference.
            nullptr,                    // No need to know the feature level.
            nullptr                     // No need to keep the D3D device context reference.
        );

        return SUCCEEDED(hr);
    }
#endif

    bool D3D11Graphics::IsSupported()
    {
        return true;
    }

    D3D11Graphics::D3D11Graphics(bool validation)
        : Graphics(GraphicsDeviceType::Direct3D11, validation)
    {
        HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));
        if (FAILED(hr))
        {
            return;
        }

        // Enumerate adapters.
        ComPtr<IDXGIAdapter1> adapter;
        for (UINT adapterIndex = 0;
            _dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND; ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Don't select the Basic Render Driver adapter.
                // If you want a software adapter, pass in "/warp" on the command line.
                continue;
            }

            _adapters.push_back(new D3D11GpuAdapter(adapter));
        }
    }

    D3D11Graphics::~D3D11Graphics()
    {
        Finalize();
    }

    void D3D11Graphics::Finalize()
    {
        WaitIdle();

        SafeDelete(_swapChain);

        Graphics::Finalize();
    }

    bool D3D11Graphics::BackendInitialize()
    {
        UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
        if (_validation)
        {
            if (SdkLayersAvailable())
            {
                // If the project is in a debug build, enable debugging via SDK Layers with this flag.
                creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
            }
            else
            {
                OutputDebugStringA("WARNING: Direct3D Debug Device is not available\n");
            }
        }
#endif

        static const D3D_FEATURE_LEVEL s_featureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };

        // Create the Direct3D 11 API device object and a corresponding context.
        ComPtr<ID3D11Device> device;
        ComPtr<ID3D11DeviceContext> context;

        HRESULT hr = E_FAIL;
        hr = D3D11CreateDevice(
            static_cast<D3D11GpuAdapter*>(_adapter)->GetDXGIAdapter(),
            D3D_DRIVER_TYPE_UNKNOWN,
            0,
            creationFlags,
            s_featureLevels,
            _countof(s_featureLevels),
            D3D11_SDK_VERSION,
            device.GetAddressOf(),  // Returns the Direct3D device created.
            &_d3dFeatureLevel,     // Returns feature level of device created.
            context.GetAddressOf()  // Returns the device immediate context.
        );

        if (FAILED(hr))
        {
            // If the initialization fails, fall back to the WARP device.
            // For more information on WARP, see: 
            // http://go.microsoft.com/fwlink/?LinkId=286690
            hr = D3D11CreateDevice(
                nullptr,
                D3D_DRIVER_TYPE_WARP, // Create a WARP device instead of a hardware device.
                0,
                creationFlags,
                s_featureLevels,
                _countof(s_featureLevels),
                D3D11_SDK_VERSION,
                device.GetAddressOf(),
                &_d3dFeatureLevel,
                context.GetAddressOf()
            );

            if (SUCCEEDED(hr))
            {
                ALIMER_LOGINFO("D3D11 backend - using WARP");
            }
        }

#if defined(_DEBUG)
        // https://blogs.msdn.microsoft.com/chuckw/2015/07/27/dxgi-debug-device/
#if ALIMER_PLATFORM_UWP
        Microsoft::WRL::ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf()))))
        {
            dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
            dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);
        }
#else
        typedef HRESULT(WINAPI * LPDXGIGETDEBUGINTERFACE)(REFIID, void **);

        HMODULE dxgidebug = LoadLibraryEx(L"dxgidebug.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
        if (dxgidebug)
        {
            Microsoft::WRL::ComPtr<IDXGIInfoQueue> dxgiInfoQueue;

            auto dxgiGetDebugInterface = reinterpret_cast<LPDXGIGETDEBUGINTERFACE>(
                reinterpret_cast<void*>(GetProcAddress(dxgidebug, "DXGIGetDebugInterface")));

            if (SUCCEEDED(dxgiGetDebugInterface(IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf()))))
            {
                const GUID DXGI_DEBUG_ALL = { 0xe48ae283, 0xda80, 0x490b, { 0x87, 0xe6, 0x43, 0xe9, 0xa9, 0xcf, 0xda, 0x8 } };

                dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
                dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);
            }
        }
#endif

        ComPtr<ID3D11Debug> d3dDebug;
        if (SUCCEEDED(device.As(&d3dDebug)))
        {
            ComPtr<ID3D11InfoQueue> d3dInfoQueue;
            if (SUCCEEDED(d3dDebug.As(&d3dInfoQueue)))
            {
                D3D11_MESSAGE_ID hide[] =
                {
                    D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
                };
                D3D11_INFO_QUEUE_FILTER filter = {};
                filter.DenyList.NumIDs = _countof(hide);
                filter.DenyList.pIDList = hide;
                d3dInfoQueue->AddStorageFilterEntries(&filter);
            }
        }
#endif

        ThrowIfFailed(device.As(&_d3dDevice));
        ThrowIfFailed(context.As(&_d3dContext));
        ThrowIfFailed(context.As(&_d3dAnnotation));

        if (!InitializeCaps())
            return false;

        // Create Swapchain.
        const WindowHandle& handle = _window->GetHandle();
        _swapChain = new D3D11SwapChain(this);

#if ALIMER_PLATFORM_UWP
        _swapChain->SetCoreWindow(handle.info.uwp.window, _window->GetWidth(), _window->GetHeight());
#else
        _swapChain->SetWindow(handle.info.win.window, _window->GetWidth(), _window->GetHeight());
#endif

        // Immediate/default command queue.
        _defaultCommandBuffer = new D3D11CommandBuffer(this, _d3dContext.Get());

        return true;
    }

    void D3D11Graphics::WaitIdle()
    {
        _d3dContext->Flush();
    }

    SharedPtr<RenderPass> D3D11Graphics::BeginFrameCore()
    {
        // TODO: Add check for swap chain resize.
        return _swapChain->GetRenderPass();
    }

    void D3D11Graphics::EndFrameCore()
    {
        // Present the frame.
        _swapChain->Present();

        // Flush immediate context.
        _d3dContext->Flush();
    }

    bool D3D11Graphics::InitializeCaps()
    {
        D3D11_FEATURE_DATA_THREADING threadingSupport = { 0 };
        ThrowIfFailed(_d3dDevice->CheckFeatureSupport(D3D11_FEATURE_THREADING, &threadingSupport, sizeof(threadingSupport)));
        return true;
    }

    void D3D11Graphics::HandleDeviceLost()
    {
        // TODO
    }

    SharedPtr<RenderPass> D3D11Graphics::CreateRenderPass(const RenderPassDescription& description)
    {
        return MakeShared<D3D11RenderPass>(this, description);
    }

    SharedPtr<GpuBuffer> D3D11Graphics::CreateBuffer(const GpuBufferDescription& description, const void* initialData)
    {
        return MakeShared<D3D11GpuBuffer>(this, description, initialData);
    }

    SharedPtr<Shader> D3D11Graphics::CreateComputeShader(const ShaderStageDescription& desc)
    {
        return MakeShared<D3D11Shader>(this, desc);
    }

    SharedPtr<Shader> D3D11Graphics::CreateShader(const ShaderStageDescription& vertex, const ShaderStageDescription& fragment)
    {
        return MakeShared<D3D11Shader>(this, vertex, fragment);
    }

    SharedPtr<PipelineState> D3D11Graphics::CreateRenderPipelineState(const RenderPipelineDescriptor& descriptor)
    {
        return MakeShared<D3D11PipelineState>(this, descriptor);
    }
}
