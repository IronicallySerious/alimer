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
#include "core/Platform.h"

#ifdef _DEBUG
#   if !defined(_XBOX_ONE) || !defined(_TITLE) || !defined(_DURANGO)
#       pragma comment(lib,"dxguid.lib")
#   endif
#endif

namespace alimer
{
#ifdef ALIMER_D3D_DYNAMIC_LIB
    PFN_CREATE_DXGI_FACTORY1        CreateDXGIFactory1;
    PFN_GET_DXGI_DEBUG_INTERFACE    DXGIGetDebugInterface;

    PFN_CREATE_DXGI_FACTORY2        CreateDXGIFactory2;
    PFN_GET_DXGI_DEBUG_INTERFACE1   DXGIGetDebugInterface1;


    PFN_D3D11_CREATE_DEVICE         D3D11CreateDevice;

    static inline HRESULT D3D11LoadLibraries()
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

        HMODULE d3d11Module = LoadLibraryW(L"d3d11.dll");
        if (!d3d11Module)
        {
            OutputDebugStringW(L"Failed to load d3d11.dll");
            return S_FALSE;
        }

        /* DXGI entry points */
        CreateDXGIFactory1 = (PFN_CREATE_DXGI_FACTORY1)GetProcAddress(dxgiModule, "CreateDXGIFactory1");
        DXGIGetDebugInterface = (PFN_GET_DXGI_DEBUG_INTERFACE)GetProcAddress(dxgiModule, "DXGIGetDebugInterface");
        CreateDXGIFactory2 = (PFN_CREATE_DXGI_FACTORY2)GetProcAddress(dxgiModule, "CreateDXGIFactory2");
        DXGIGetDebugInterface1 = (PFN_GET_DXGI_DEBUG_INTERFACE1)GetProcAddress(dxgiModule, "DXGIGetDebugInterface1");

        if (CreateDXGIFactory1 == nullptr)
        {
            OutputDebugStringW(L"Cannot find CreateDXGIFactory1 entry point.");
            return S_FALSE;
        }

        /* D3D12 entry points */
        D3D11CreateDevice = (PFN_D3D11_CREATE_DEVICE)GetProcAddress(d3d11Module, "D3D11CreateDevice");

        if (!D3D11CreateDevice)
        {
            OutputDebugStringW(L"Cannot find D3D11CreateDevice entry point.");
            return S_FALSE;
        }

        return S_OK;
    }
#endif

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

        availableCheck = true;

#if ALIMER_D3D_DYNAMIC_LIB
        if (FAILED(D3D11LoadLibraries()))
        {
            return false;
        }
#endif

        isAvailable = true;
        return isAvailable;
    }

    GraphicsImpl::GraphicsImpl()
    {
        ALIMER_ASSERT_MSG(IsSupported(), "D3D12 backend is not supported");

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

        ComPtr<IDXGIFactory5> factory5;
        if (SUCCEEDED(_dxgiFactory.As(&factory5)))
        {
            BOOL allowTearing = FALSE;
            if (SUCCEEDED(factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing)))
                && allowTearing)
            {
                _allowTearing = true;
            }
        }

        ComPtr<IDXGIAdapter1> adapter;
        GetHardwareAdapter(adapter.GetAddressOf());

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

    GraphicsImpl::~GraphicsImpl()
    {
    }

    void GraphicsImpl::GetHardwareAdapter(IDXGIAdapter1** ppAdapter)
    {
        *ppAdapter = nullptr;

        ComPtr<IDXGIAdapter1> adapter;

#if defined(__dxgi1_6_h__) && defined(NTDDI_WIN10_RS4)
        ComPtr<IDXGIFactory6> factory6;
        HRESULT hr = _dxgiFactory.As(&factory6);
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
                DXGI_ERROR_NOT_FOUND != _dxgiFactory->EnumAdapters1(adapterIndex, adapter.ReleaseAndGetAddressOf());
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
        return true;
    }

    void GraphicsImpl::EndFrame()
    {
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
}
