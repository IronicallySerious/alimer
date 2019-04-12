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

#include "GraphicsImplD3D11.h"
#include "engine/Window.h"

#include <DirectXMath.h>
#include <DirectXColors.h>

namespace
{
    inline DXGI_FORMAT NoSRGB(DXGI_FORMAT format)
    {
        switch (format)
        {
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:   return DXGI_FORMAT_R8G8B8A8_UNORM;
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:   return DXGI_FORMAT_B8G8R8A8_UNORM;
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:   return DXGI_FORMAT_B8G8R8X8_UNORM;
        default:                                return format;
        }
    }
}

namespace alimer
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

    bool GraphicsImpl::IsSupported()
    {
        static bool availableCheck = false;
        static bool isAvailable = false;

        if (availableCheck) {
            return isAvailable;
        }

        ComPtr<IDXGIFactory1> tempDXGIFactory;
        ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&tempDXGIFactory)));

        ComPtr<IDXGIAdapter1> adapter;
        GetHardwareAdapter(tempDXGIFactory, adapter.GetAddressOf());

        availableCheck = true;
        isAvailable = adapter != nullptr;
        return isAvailable;
    }

    GraphicsImpl::GraphicsImpl()
    {
        ALIMER_ASSERT_MSG(IsSupported(), "D3D12 backend is not supported");
    }

    GraphicsImpl::~GraphicsImpl()
    {
        Shutdown();
    }

    void GraphicsImpl::Shutdown()
    {
        _d3dDepthStencilView.Reset();
        _d3dRenderTargetView.Reset();
        _renderTarget.Reset();
        _depthStencil.Reset();
        _swapChain.Reset();
        _d3dContext.Reset();
        _d3dAnnotation.Reset();

#ifdef _DEBUG
        {
            ComPtr<ID3D11Debug> d3dDebug;
            if (SUCCEEDED(_d3dDevice.As(&d3dDebug)))
            {
                d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY);
            }
        }
#endif

        _d3dDevice.Reset();
        _dxgiFactory.Reset();
    }

    void GraphicsImpl::GetHardwareAdapter(ComPtr<IDXGIFactory1> factory, IDXGIAdapter1** ppAdapter)
    {
        *ppAdapter = nullptr;

        ComPtr<IDXGIAdapter1> adapter;

#if defined(__dxgi1_6_h__) && defined(NTDDI_WIN10_RS4)
        ComPtr<IDXGIFactory6> factory6;
        HRESULT hr = factory.As(&factory6);
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

#ifdef _DEBUG
                wchar_t buff[256] = {};
                swprintf_s(buff, L"Direct3D Adapter (%u): VID:%04X, PID:%04X - %ls\n", adapterIndex, desc.VendorId, desc.DeviceId, desc.Description);
                OutputDebugStringW(buff);
#endif

                break;
            }
        }
        else
#endif
            for (UINT adapterIndex = 0;
                DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(adapterIndex, adapter.ReleaseAndGetAddressOf());
                adapterIndex++)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Don't select the Basic Render Driver adapter.
                continue;
            }

#ifdef _DEBUG
            wchar_t buff[256] = {};
            swprintf_s(buff, L"Direct3D Adapter (%u): VID:%04X, PID:%04X - %ls\n", adapterIndex, desc.VendorId, desc.DeviceId, desc.Description);
            OutputDebugStringW(buff);
#endif

            break;
        }

        *ppAdapter = adapter.Detach();
    }

    bool GraphicsImpl::BeginFrame()
    {
        _d3dAnnotation->BeginEvent(L"Main");

        // Clear main view.
        ID3D11RenderTargetView* renderTargetView = _d3dRenderTargetView.Get();
        ID3D11DepthStencilView* depthStencilView = _d3dDepthStencilView.Get();

        _d3dContext->ClearRenderTargetView(renderTargetView, DirectX::Colors::CornflowerBlue);
        if (depthStencilView)
        {
            _d3dContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        }

        _d3dContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

        return true;
    }

    void GraphicsImpl::EndFrame()
    {
        _d3dAnnotation->EndEvent();

        HRESULT hr;
        if (_tearing)
        {
            // Recommended to always use tearing if supported when using a sync interval of 0.
            hr = _swapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
        }
        else
        {
            // The first argument instructs DXGI to block until VSync, putting the application
            // to sleep until the next VSync. This ensures we don't waste any cycles rendering
            // frames that will never be displayed to the screen.
            hr = _swapChain->Present(1, 0);
        }

        // Discard the contents of the render target.
        // This is a valid operation only when the existing contents will be entirely
        // overwritten. If dirty or scroll rects are used, this call should be removed.
        // Only do this if we Clear RenderTarget and depth stencil using CommandContext otherwise it will flicker.
        _d3dContext->DiscardView(_d3dRenderTargetView.Get());

        if (_d3dDepthStencilView)
        {
            // Discard the contents of the depth stencil.
            _d3dContext->DiscardView(_d3dDepthStencilView.Get());
        }

        // If the device was removed either by a disconnection or a driver upgrade, we 
        // must recreate all device resources.
        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
        {
#ifdef _DEBUG
            char buff[64] = {};
            sprintf_s(buff, "Device Lost on Present: Reason code 0x%08X\n", (hr == DXGI_ERROR_DEVICE_REMOVED) ? _d3dDevice->GetDeviceRemovedReason() : hr);
            OutputDebugStringA(buff);
#endif
            HandleDeviceLost();
        }
        else
        {
            ThrowIfFailed(hr);

            if (!_dxgiFactory->IsCurrent())
            {
                // Output information is cached on the DXGI Factory. If it is stale we need to create a new factory.
                CreateFactory();
            }
        }
    }

    bool GraphicsImpl::Initialize(Window* renderWindow, uint32_t sampleCount)
    {
        _sampleCount = sampleCount;
        if (_sampleCount > 1)
        {
            _tearing = false;
        }

        CreateDeviceResources();

#if ALIMER_PLATFORM_UWP
        _window = renderWindow->GetNativeHandle();
#else
        _window = renderWindow->GetNativeHandle();
        if (!IsWindow(_window))
        {
            ALIMER_ASSERT_MSG(false, "Invalid hWnd handle");
        }

        _backbufferSize = renderWindow->GetSize();

        if (_backbufferSize.x == 0
            || _backbufferSize.y == 0)
        {
            RECT rect;
            GetClientRect(_window, &rect);
            _backbufferSize.x = static_cast<uint32_t>(rect.right - rect.left);
            _backbufferSize.y = static_cast<uint32_t>(rect.bottom - rect.top);
        }

#endif

        return Resize(_backbufferSize);
    }

    bool GraphicsImpl::Resize(const math::uint2& newSize)
    {
        _backbufferSize = newSize;
        return CreateWindowSizeDependentResources();
    }

    void GraphicsImpl::InitializeCaps(IDXGIAdapter1* adapter)
    {
        DXGI_ADAPTER_DESC1 desc = { };
        adapter->GetDesc1(&desc);

        std::wstring deviceName(desc.Description);

        _info.backend = GraphicsBackend::Direct3D12;
        _info.backendName = "Direct3D12 - Level " + std::string(D3DFeatureLevelToVersion(_d3dFeatureLevel));
        _info.deviceName = std::string(deviceName.begin(), deviceName.end());
        _info.vendorName = D3DGetVendorByID(desc.VendorId);
        _info.vendorId = desc.VendorId;

        // Under D3D12 minimum supported feature level is 11.0
        _caps.features.instancing = true;
        _caps.features.alphaToCoverage = true;
        _caps.features.independentBlend = (_d3dFeatureLevel >= D3D_FEATURE_LEVEL_10_0);
        _caps.features.computeShader = (_d3dFeatureLevel >= D3D_FEATURE_LEVEL_10_0);
        _caps.features.geometryShader = (_d3dFeatureLevel >= D3D_FEATURE_LEVEL_11_0);
        _caps.features.tessellationShader = (_d3dFeatureLevel >= D3D_FEATURE_LEVEL_11_0);
        _caps.features.sampleRateShading = true;
        _caps.features.dualSrcBlend = true;
        _caps.features.logicOp = (_d3dFeatureLevel >= D3D_FEATURE_LEVEL_11_1);
        _caps.features.multiViewport = true;
        _caps.features.indexUInt32 = true;
        _caps.features.drawIndirect = (_d3dFeatureLevel >= D3D_FEATURE_LEVEL_11_0);
        _caps.features.alphaToOne = true;
        _caps.features.fillModeNonSolid = true;
        _caps.features.samplerAnisotropy = true;
        _caps.features.textureCompressionBC = true;
        _caps.features.textureCompressionPVRTC = false;
        _caps.features.textureCompressionETC2 = false;
        _caps.features.textureCompressionATC = false;
        _caps.features.textureCompressionASTC = false;
        _caps.features.pipelineStatisticsQuery = true;
        _caps.features.texture1D = true;
        _caps.features.texture3D = true;
        _caps.features.texture2DArray = (_d3dFeatureLevel >= D3D_FEATURE_LEVEL_10_0);
        _caps.features.textureCubeArray = (_d3dFeatureLevel >= D3D_FEATURE_LEVEL_10_1);;
        _caps.features.raytracing = false;

        // Limits
        _caps.limits.maxTextureDimension1D = _d3dFeatureLevel >= D3D_FEATURE_LEVEL_11_0 ? D3D11_REQ_TEXTURE1D_U_DIMENSION : 8192u;
        _caps.limits.maxTextureDimension2D = _d3dFeatureLevel >= D3D_FEATURE_LEVEL_11_0 ? D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION : 8192u;
        _caps.limits.maxTextureDimension3D = _d3dFeatureLevel >= D3D_FEATURE_LEVEL_10_0 ? D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION : 256u;
        _caps.limits.maxTextureDimensionCube = _d3dFeatureLevel >= D3D_FEATURE_LEVEL_10_0 ? D3D11_REQ_TEXTURECUBE_DIMENSION : 8192u;
        _caps.limits.maxTextureArrayLayers = _d3dFeatureLevel >= D3D_FEATURE_LEVEL_10_0 ? D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION : 256u;
        _caps.limits.maxColorAttachments = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;
        _caps.limits.maxUniformBufferSize = D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16;
        _caps.limits.minUniformBufferOffsetAlignment = 256u;
        _caps.limits.maxStorageBufferSize = std::numeric_limits<uint32_t>::max();
        _caps.limits.minStorageBufferOffsetAlignment = 16;
        _caps.limits.maxSamplerAnisotropy = D3D11_MAX_MAXANISOTROPY;
        _caps.limits.maxViewports = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
        _caps.limits.maxViewportDimensions[0] = D3D11_VIEWPORT_BOUNDS_MAX;
        _caps.limits.maxViewportDimensions[1] = D3D11_VIEWPORT_BOUNDS_MAX;
        _caps.limits.maxPatchVertices = D3D11_IA_PATCH_MAX_CONTROL_POINT_COUNT;
        _caps.limits.pointSizeRange[0] = 1.0f;
        _caps.limits.pointSizeRange[1] = 1.0f;
        _caps.limits.lineWidthRange[0] = 1.0f;
        _caps.limits.lineWidthRange[1] = 1.0f;
        _caps.limits.maxComputeSharedMemorySize = D3D11_CS_THREAD_LOCAL_TEMP_REGISTER_POOL;
        _caps.limits.maxComputeWorkGroupCount[0] = D3D11_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION;
        _caps.limits.maxComputeWorkGroupCount[1] = D3D11_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION;
        _caps.limits.maxComputeWorkGroupCount[2] = D3D11_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION;
        _caps.limits.maxComputeWorkGroupInvocations = D3D11_CS_THREAD_GROUP_MAX_THREADS_PER_GROUP;
        _caps.limits.maxComputeWorkGroupSize[0] = D3D11_CS_THREAD_GROUP_MAX_X;
        _caps.limits.maxComputeWorkGroupSize[1] = D3D11_CS_THREAD_GROUP_MAX_Y;
        _caps.limits.maxComputeWorkGroupSize[2] = D3D11_CS_THREAD_GROUP_MAX_Z;
    }

    void GraphicsImpl::CreateFactory()
    {

#if defined(_DEBUG)
        bool debugDXGI = false;
        if (DXGIGetDebugInterface1 != nullptr)
        {
            ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
            if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf()))))
            {
                debugDXGI = true;

                ThrowIfFailed(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(_dxgiFactory.ReleaseAndGetAddressOf())));

                dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
                dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);

                DXGI_INFO_QUEUE_MESSAGE_ID hide[] =
                {
                    80 /* IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides. */,
                };
                DXGI_INFO_QUEUE_FILTER filter = {};
                filter.DenyList.NumIDs = _countof(hide);
                filter.DenyList.pIDList = hide;
                dxgiInfoQueue->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);
            }
        }

        if (!debugDXGI)
#endif
            ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(_dxgiFactory.ReleaseAndGetAddressOf())));

        if (_tearing)
        {
            ComPtr<IDXGIFactory5> factory5;
            if (SUCCEEDED(_dxgiFactory.As(&factory5)))
            {
                _tearing = false;
                BOOL allowTearing = FALSE;
                if (SUCCEEDED(factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing)))
                    && allowTearing)
                {
                    _tearing = true;
                }
            }
        }
    }

    void GraphicsImpl::CreateDeviceResources()
    {
        CreateFactory();

        ComPtr<IDXGIAdapter1> adapter;
        GetHardwareAdapter(_dxgiFactory, adapter.GetAddressOf());

        UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
        if (SdkLayersAvailable())
        {
            // If the project is in a debug build, enable debugging via SDK Layers with this flag.
            creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
        }
        else
        {
            OutputDebugStringA("WARNING: Direct3D Debug Device is not available\n");
        }
#endif

        // Create the Direct3D 11 API device object and a corresponding context.
        ComPtr<ID3D11Device> device;
        ComPtr<ID3D11DeviceContext> context;

        static const D3D_FEATURE_LEVEL s_featureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };


        HRESULT hr = E_FAIL;
        if (adapter)
        {
            hr = D3D11CreateDevice(
                adapter.Get(),
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
        }
#if defined(NDEBUG)
        else
        {
            throw std::exception("No Direct3D hardware device found");
        }
#else
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
                OutputDebugStringA("Direct3D Adapter - WARP\n");
            }
        }
#endif

        ThrowIfFailed(hr);

#ifndef NDEBUG
        ComPtr<ID3D11Debug> d3dDebug;
        if (SUCCEEDED(device.As(&d3dDebug)))
        {
            ComPtr<ID3D11InfoQueue> d3dInfoQueue;
            if (SUCCEEDED(d3dDebug.As(&d3dInfoQueue)))
            {
#ifdef _DEBUG
                d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
                d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif
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

        InitializeCaps(adapter.Get());
    }

    bool GraphicsImpl::CreateWindowSizeDependentResources()
    {
        if (!_window)
        {
            OutputDebugStringA("Call SetWindow with a valid window handle");
            return false;
        }

        // Clear the previous window size specific context.
        ID3D11RenderTargetView* nullViews[] = { nullptr };
        _d3dContext->OMSetRenderTargets(_countof(nullViews), nullViews, nullptr);
        _d3dRenderTargetView.Reset();
        _d3dDepthStencilView.Reset();
        _renderTarget.Reset();
        _depthStencil.Reset();
        _d3dContext->Flush();

        UINT backBufferWidth = std::max<UINT>(_backbufferSize.x, 1);
        UINT backBufferHeight = std::max<UINT>(_backbufferSize.y, 1);
        DXGI_FORMAT backBufferFormat = _tearing ? NoSRGB(_backBufferFormat) : _backBufferFormat;

        if (_swapChain)
        {
            // If the swap chain already exists, resize it.
            HRESULT hr = _swapChain->ResizeBuffers(
                _backBufferCount,
                backBufferWidth,
                backBufferHeight,
                backBufferFormat,
                _tearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0
            );

            if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
            {
#ifdef _DEBUG
                char buff[64] = {};
                sprintf_s(buff, "Device Lost on ResizeBuffers: Reason code 0x%08X\n", (hr == DXGI_ERROR_DEVICE_REMOVED) ? _d3dDevice->GetDeviceRemovedReason() : hr);
                OutputDebugStringA(buff);
#endif
                // If the device was removed for any reason, a new device and swap chain will need to be created.
                HandleDeviceLost();

                // Everything is set up now. Do not continue execution of this method. HandleDeviceLost will reenter this method 
                // and correctly set up the new device.
                return false;
            }
            else
            {
                ThrowIfFailed(hr);
            }
        }
        else
        {
            // Create a descriptor for the swap chain.
            DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
            swapChainDesc.Width = backBufferWidth;
            swapChainDesc.Height = backBufferHeight;
            swapChainDesc.Format = backBufferFormat;
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.BufferCount = _backBufferCount;
            swapChainDesc.SampleDesc.Count = _sampleCount;
            swapChainDesc.SampleDesc.Quality = _sampleCount > 1 ? D3D11_STANDARD_MULTISAMPLE_PATTERN : 0;
            swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
            swapChainDesc.SwapEffect = _tearing ? DXGI_SWAP_EFFECT_FLIP_DISCARD : DXGI_SWAP_EFFECT_DISCARD;
            swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
            swapChainDesc.Flags = _tearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

            DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
            fsSwapChainDesc.Windowed = TRUE;

            // Create a SwapChain from a Win32 window.
            ThrowIfFailed(_dxgiFactory->CreateSwapChainForHwnd(
                _d3dDevice.Get(),
                _window,
                &swapChainDesc,
                &fsSwapChainDesc,
                nullptr,
                _swapChain.ReleaseAndGetAddressOf()
            ));

            // This class does not support exclusive full-screen mode and prevents DXGI from responding to the ALT+ENTER shortcut
            ThrowIfFailed(_dxgiFactory->MakeWindowAssociation(_window, DXGI_MWA_NO_ALT_ENTER));
        }

        UpdateColorSpace();

        // Create a render target view of the swap chain back buffer.
        ThrowIfFailed(_swapChain->GetBuffer(0, IID_PPV_ARGS(_renderTarget.ReleaseAndGetAddressOf())));

        (D3D11_RTV_DIMENSION_TEXTURE2D, _backBufferFormat);
        ThrowIfFailed(
            _d3dDevice->CreateRenderTargetView(_renderTarget.Get(), nullptr, _d3dRenderTargetView.ReleaseAndGetAddressOf()
            ));

        if (_depthBufferFormat != DXGI_FORMAT_UNKNOWN)
        {
            // Create a depth stencil view for use with 3D rendering if needed.
            D3D11_TEXTURE2D_DESC depthStencilDesc = {};
            depthStencilDesc.Width = backBufferWidth;
            depthStencilDesc.Height = backBufferHeight;
            depthStencilDesc.MipLevels = 1;
            depthStencilDesc.ArraySize = 1;
            depthStencilDesc.Format = _depthBufferFormat;
            depthStencilDesc.SampleDesc.Count = _sampleCount;
            depthStencilDesc.SampleDesc.Quality = _sampleCount > 1 ? D3D11_STANDARD_MULTISAMPLE_PATTERN : 0;
            depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
            depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
            depthStencilDesc.CPUAccessFlags = 0;
            depthStencilDesc.MiscFlags = 0;

            ThrowIfFailed(_d3dDevice->CreateTexture2D(
                &depthStencilDesc,
                nullptr,
                _depthStencil.ReleaseAndGetAddressOf()
            ));

            D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
            depthStencilViewDesc.Format = _depthBufferFormat;
            depthStencilViewDesc.ViewDimension = _sampleCount > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;

            ThrowIfFailed(_d3dDevice->CreateDepthStencilView(
                _depthStencil.Get(),
                &depthStencilViewDesc,
                _d3dDepthStencilView.ReleaseAndGetAddressOf()
            ));
        }

        return true;
    }

    void GraphicsImpl::UpdateColorSpace()
    {
        DXGI_COLOR_SPACE_TYPE colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;

        bool isDisplayHDR10 = false;

#if defined(NTDDI_WIN10_RS2)
        if (_swapChain)
        {
            ComPtr<IDXGIOutput> output;
            if (SUCCEEDED(_swapChain->GetContainingOutput(output.GetAddressOf())))
            {
                ComPtr<IDXGIOutput6> output6;
                if (SUCCEEDED(output.As(&output6)))
                {
                    DXGI_OUTPUT_DESC1 desc;
                    ThrowIfFailed(output6->GetDesc1(&desc));

                    if (desc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020)
                    {
                        // Display output is HDR10.
                        isDisplayHDR10 = true;
                    }
                }
            }
        }
#endif

        const bool enableHDR = true;
        if (enableHDR && isDisplayHDR10)
        {
            switch (_backBufferFormat)
            {
            case DXGI_FORMAT_R10G10B10A2_UNORM:
                // The application creates the HDR10 signal.
                colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
                break;

            case DXGI_FORMAT_R16G16B16A16_FLOAT:
                // The system creates the HDR10 signal; application uses linear values.
                colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709;
                break;

            default:
                break;
            }
        }

        _colorSpace = colorSpace;

        ComPtr<IDXGISwapChain3> swapChain3;
        if (SUCCEEDED(_swapChain.As(&swapChain3)))
        {
            UINT colorSpaceSupport = 0;
            if (SUCCEEDED(swapChain3->CheckColorSpaceSupport(colorSpace, &colorSpaceSupport))
                && (colorSpaceSupport & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT))
            {
                ThrowIfFailed(swapChain3->SetColorSpace1(colorSpace));
            }
        }
    }

    void GraphicsImpl::HandleDeviceLost()
    {
        Shutdown();

        CreateDeviceResources();
        CreateWindowSizeDependentResources();
    }

    PixelFormat GraphicsImpl::GetDefaultDepthStencilFormat() const
    {
        D3D11_FEATURE_DATA_FORMAT_SUPPORT formatSupport = {};
        formatSupport.InFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

        if (SUCCEEDED(_d3dDevice->CheckFeatureSupport(D3D11_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport)))
            && formatSupport.OutFormatSupport & D3D11_FORMAT_SUPPORT_RENDER_TARGET | D3D11_FORMAT_SUPPORT_DEPTH_STENCIL)
        {
            return PixelFormat::Depth24UNormStencil8;
        }

        formatSupport.InFormat = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
        if (SUCCEEDED(_d3dDevice->CheckFeatureSupport(D3D11_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport)))
            && formatSupport.OutFormatSupport & D3D11_FORMAT_SUPPORT_RENDER_TARGET | D3D11_FORMAT_SUPPORT_DEPTH_STENCIL)
        {
            return PixelFormat::Depth32FloatStencil8;
        }

        return PixelFormat::Undefined;
    }

    PixelFormat GraphicsImpl::GetDefaultDepthFormat() const
    {
        D3D11_FEATURE_DATA_FORMAT_SUPPORT formatSupport = {};
        formatSupport.InFormat = DXGI_FORMAT_D32_FLOAT;

        if (SUCCEEDED(_d3dDevice->CheckFeatureSupport(D3D11_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport)))
            && formatSupport.OutFormatSupport & D3D11_FORMAT_SUPPORT_RENDER_TARGET | D3D11_FORMAT_SUPPORT_DEPTH_STENCIL)
        {
            return PixelFormat::Depth32Float;
        }

        formatSupport.InFormat = DXGI_FORMAT_D16_UNORM;
        if (SUCCEEDED(_d3dDevice->CheckFeatureSupport(D3D11_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport)))
            && formatSupport.OutFormatSupport & D3D11_FORMAT_SUPPORT_RENDER_TARGET | D3D11_FORMAT_SUPPORT_DEPTH_STENCIL)
        {
            return PixelFormat::Depth16UNorm;
        }

        return PixelFormat::Undefined;
    }

    bool GraphicsImpl::IsInitialized() const
    {
        return _d3dDevice != nullptr;
    }
}
