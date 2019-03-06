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

#include "GraphicsDeviceD3D11.h"
#include "SwapChainD3D11.h"
#include "CommandContextD3D11.h"
#include "TextureD3D11.h"
#include "SamplerD3D11.h"
#include "BufferD3D11.h"
#include "ShaderD3D11.h"
//#include "D3D11Pipeline.h"
#include "foundation/StringUtils.h"
#include "Core/Platform.h"
#include "Core/Log.h"

using namespace Microsoft::WRL;

#if ALIMER_D3D_DYNAMIC_LIB
typedef HRESULT(WINAPI* PFN_CREATE_DXGI_FACTORY1)(REFIID riid, _COM_Outptr_ void **ppFactory);
typedef HRESULT(WINAPI* PFN_GET_DXGI_DEBUG_INTERFACE)(REFIID riid, _COM_Outptr_ void** pDebug);
#endif

namespace alimer
{
#ifdef ALIMER_D3D_DYNAMIC_LIB
    static HMODULE s_dxgiLib = nullptr;
    static HMODULE s_d3d11Lib = nullptr;

    static PFN_CREATE_DXGI_FACTORY1 CreateDXGIFactory1 = nullptr;
    static PFN_GET_DXGI_DEBUG_INTERFACE DXGIGetDebugInterface = nullptr;
    static PFN_D3D11_CREATE_DEVICE D3D11CreateDevice = nullptr;

    HRESULT D3D11LoadLibraries()
    {
        static bool loaded = false;
        if (loaded)
            return S_OK;
        loaded = true;

        // Load libraries first.
        s_dxgiLib = LoadLibraryW(L"dxgi.dll");
        if (!s_dxgiLib)
        {
            OutputDebugStringW(L"Failed to load dxgi.dll");
            return S_FALSE;
        }

        s_d3d11Lib = LoadLibraryW(L"d3d11.dll");
        if (!s_d3d11Lib)
        {
            OutputDebugStringW(L"Failed to load d3d11.dll");
            return S_FALSE;
        }

        /* DXGI entry points */
        CreateDXGIFactory1 = (PFN_CREATE_DXGI_FACTORY1)GetProcAddress(s_dxgiLib, "CreateDXGIFactory1");
        DXGIGetDebugInterface = (PFN_GET_DXGI_DEBUG_INTERFACE)GetProcAddress(s_dxgiLib, "DXGIGetDebugInterface");
        if (CreateDXGIFactory1 == nullptr)
        {
            OutputDebugStringW(L"Cannot find CreateDXGIFactory1 entry point.");
            return S_FALSE;
        }

        /* D3D11 entry points */
        D3D11CreateDevice = (PFN_D3D11_CREATE_DEVICE)GetProcAddress(s_d3d11Lib, "D3D11CreateDevice");

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

    static void GetD3D11HardwareAdapter(_In_ IDXGIFactory1* factory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter)
    {
        ComPtr<IDXGIAdapter1> adapter;
        *ppAdapter = nullptr;

        for (UINT adapterIndex = 0;
            factory->EnumAdapters1(adapterIndex, adapter.ReleaseAndGetAddressOf()) != DXGI_ERROR_NOT_FOUND;
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

            break;
        }

        *ppAdapter = adapter.Detach();
    }


    bool GraphicsDeviceD3D11::IsSupported()
    {
        static bool availableCheck = false;
        static bool isAvailable = false;

        if (availableCheck)
            return isAvailable;

        availableCheck = true;
#if ALIMER_D3D_DYNAMIC_LIB
        if (FAILED(D3D11LoadLibraries()))
        {
            isAvailable = false;
            return false;
        }
#endif
        // Create temp dxgi factory for check support.
        ComPtr<IDXGIFactory1> factory;
        HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
        if (FAILED(hr))
        {
            isAvailable = false;
            return false;
        }

        ComPtr<IDXGIAdapter1> hardwareAdapter;
        GetD3D11HardwareAdapter(factory.Get(), &hardwareAdapter);
        isAvailable = hardwareAdapter != nullptr;
        return isAvailable;
    }

    GraphicsDeviceD3D11::GraphicsDeviceD3D11(const GraphicsDeviceDescriptor* descriptor)
        : GraphicsDevice(GraphicsBackend::Direct3D11, descriptor)
        , _cache(this)
    {
        if (FAILED(D3D11LoadLibraries()))
        {
            ALIMER_LOGCRITICAL("D3D11 - Failed to load functions");
            return;
        }

        HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&_factory));
        if (FAILED(hr))
        {
            ALIMER_LOGCRITICAL("D3D11 - Failed to create DXGIFactory1");
            return;
        }

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
                ALIMER_LOGDEBUG("Direct3D Debug Device is not available");
            }
        }
#endif
        // Determine DirectX hardware feature levels this app will support.
        static const D3D_FEATURE_LEVEL s_featureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };

        GetD3D11HardwareAdapter(_factory.Get(), _adapter.ReleaseAndGetAddressOf());
        UINT featLevelCount = _countof(s_featureLevels);
        hr = D3D11CreateDevice(
            _adapter.Get(),
            D3D_DRIVER_TYPE_UNKNOWN,
            0,
            creationFlags,
            s_featureLevels,
            featLevelCount,
            D3D11_SDK_VERSION,
            _d3dDevice.ReleaseAndGetAddressOf(),
            &_d3dFeatureLevel,
            &_d3dContext
        );

        if (hr == E_INVALIDARG && featLevelCount > 1)
        {
            assert(s_featureLevels[0] == D3D_FEATURE_LEVEL_11_1);

            // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
            hr = D3D11CreateDevice(
                _adapter.Get(),
                D3D_DRIVER_TYPE_UNKNOWN,
                nullptr,
                creationFlags,
                &s_featureLevels[1],
                featLevelCount - 1,
                D3D11_SDK_VERSION,
                _d3dDevice.ReleaseAndGetAddressOf(),
                &_d3dFeatureLevel,
                &_d3dContext
            );
        }
        ThrowIfFailed(hr);

#ifndef NDEBUG
        ComPtr<ID3D11Debug> d3dDebug;
        if (SUCCEEDED(_d3dDevice.As(&d3dDebug)))
        {
            ComPtr<ID3D11InfoQueue> d3dInfoQueue;
            if (SUCCEEDED(d3dDebug.As(&d3dInfoQueue)))
            {
#ifdef _DEBUG
                d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
                d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif
                D3D11_INFO_QUEUE_FILTER filter;
                memset(&filter, 0, sizeof(filter));

                D3D11_MESSAGE_SEVERITY denySeverity = D3D11_MESSAGE_SEVERITY_INFO;
                filter.DenyList.NumSeverities = 1;
                filter.DenyList.pSeverityList = &denySeverity;

                D3D11_MESSAGE_ID denyIds[] =
                {
                    D3D11_MESSAGE_ID_OMSETRENDERTARGETS_INVALIDVIEW,
                    D3D11_MESSAGE_ID_DEVICE_DRAW_INDEX_BUFFER_TOO_SMALL,
                    D3D11_MESSAGE_ID_DEVICE_DRAW_RENDERTARGETVIEW_NOT_SET,
                    D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
                };

                filter.DenyList.NumIDs = sizeof(denyIds) / sizeof(D3D11_MESSAGE_ID);
                filter.DenyList.pIDList = (D3D11_MESSAGE_ID*)&denyIds;
                d3dInfoQueue->PushStorageFilter(&filter);
            }
        }
#endif /* NDEBUG */

        // Obtain Direct3D 11.1 interfaces (if available)
        if (SUCCEEDED(_d3dDevice.As(&_d3dDevice1)))
        {
        }

        InitializeCaps();

        // Create main render context.
        _renderContext = new CommandContextD3D11(this);
    }

    GraphicsDeviceD3D11::~GraphicsDeviceD3D11()
    {
        Finalize();
        _cache.Clear();

        SafeRelease(_d3dContext);
        _d3dDevice1.Reset();

#ifdef _DEBUG
        ComPtr<ID3D11Debug> d3dDebug;
        if (SUCCEEDED(_d3dDevice.As(&d3dDebug)))
        {
            d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY);
        }
#endif
        _d3dDevice.Reset();
    }

    bool GraphicsDeviceD3D11::InitializeImpl(const SwapChainDescriptor* descriptor)
    {
        _renderWindow.reset(new SwapChainD3D11(this, descriptor));
        return _renderWindow != nullptr;
    }

    void GraphicsDeviceD3D11::Tick()
    {
        _d3dContext->Flush();
    }

    PixelFormat GraphicsDeviceD3D11::GetDefaultDepthStencilFormat() const
    {
        D3D11_FEATURE_DATA_FORMAT_SUPPORT data = {};
        data.InFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
        if (SUCCEEDED(_d3dDevice->CheckFeatureSupport(D3D11_FEATURE_FORMAT_SUPPORT, &data, sizeof(data)))
            && data.OutFormatSupport & D3D11_FORMAT_SUPPORT_RENDER_TARGET | D3D11_FORMAT_SUPPORT_DEPTH_STENCIL)
        {
            return PixelFormat::Depth24UNormStencil8;
        }

        data.InFormat = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
        if (SUCCEEDED(_d3dDevice->CheckFeatureSupport(D3D11_FEATURE_FORMAT_SUPPORT, &data, sizeof(data)))
            && data.OutFormatSupport & D3D11_FORMAT_SUPPORT_RENDER_TARGET | D3D11_FORMAT_SUPPORT_DEPTH_STENCIL)
        {
            return PixelFormat::Depth32FloatStencil8;
        }

        return PixelFormat::Undefined;
    }

    PixelFormat GraphicsDeviceD3D11::GetDefaultDepthFormat() const
    {
        D3D11_FEATURE_DATA_FORMAT_SUPPORT data = {};
        data.InFormat = DXGI_FORMAT_D32_FLOAT;
        if (SUCCEEDED(_d3dDevice->CheckFeatureSupport(D3D11_FEATURE_FORMAT_SUPPORT, &data, sizeof(data)))
            && data.OutFormatSupport & D3D11_FORMAT_SUPPORT_RENDER_TARGET | D3D11_FORMAT_SUPPORT_DEPTH_STENCIL)
        {
            return PixelFormat::Depth32Float;
        }

        data.InFormat = DXGI_FORMAT_D16_UNORM;
        if (SUCCEEDED(_d3dDevice->CheckFeatureSupport(D3D11_FEATURE_FORMAT_SUPPORT, &data, sizeof(data)))
            && data.OutFormatSupport & D3D11_FORMAT_SUPPORT_RENDER_TARGET | D3D11_FORMAT_SUPPORT_DEPTH_STENCIL)
        {
            return PixelFormat::Depth16UNorm;
        }

        return PixelFormat::Undefined;
    }

    Texture* GraphicsDeviceD3D11::GetCurrentTexture() const
    {
        return static_cast<SwapChainD3D11*>(_renderWindow.get())->GetCurrentTexture();
    }

    Texture* GraphicsDeviceD3D11::GetDepthStencilTexture() const
    {
        return static_cast<SwapChainD3D11*>(_renderWindow.get())->GetDepthStencilTexture();
    }

    Texture* GraphicsDeviceD3D11::GetMultisampleColorTexture() const
    {
        return static_cast<SwapChainD3D11*>(_renderWindow.get())->GetMultisampleColorTexture();
    }

    void GraphicsDeviceD3D11::InitializeCaps()
    {
        DXGI_ADAPTER_DESC desc;
        _adapter->GetDesc(&desc);

#ifdef _DEBUG
        wchar_t buff[256] = {};
        swprintf_s(buff, L"D3D11 device created using Adapter: VID:%04X, PID:%04X - %ls\n", desc.VendorId, desc.DeviceId, desc.Description);
        OutputDebugStringW(buff);
#endif

        _info.backend = GraphicsBackend::Direct3D11;
        _info.vendorId = desc.VendorId;
        _info.deviceId = desc.DeviceId;
        _info.deviceName = ToUtf8(desc.Description);

        switch (_d3dFeatureLevel)
        {
        case D3D_FEATURE_LEVEL_10_0:
            _shaderModelMajor = 4;
            _shaderModelMinor = 0;
            break;
        case D3D_FEATURE_LEVEL_10_1:
            _shaderModelMajor = 4;
            _shaderModelMinor = 1;
            break;
        case D3D_FEATURE_LEVEL_11_0:
            _shaderModelMajor = 5;
            _shaderModelMinor = 0;
            break;
        case D3D_FEATURE_LEVEL_11_1:
            _shaderModelMajor = 5;
            _shaderModelMinor = 0;
            break;
        case D3D_FEATURE_LEVEL_12_0:
            _shaderModelMajor = 5;
            _shaderModelMinor = 1;
            break;
        case D3D_FEATURE_LEVEL_12_1:
            _shaderModelMajor = 5;
            _shaderModelMinor = 1;
            break;
        default:
            break;
        }

        D3D11_FEATURE_DATA_THREADING threadingFeature = { 0 };
        HRESULT hr = _d3dDevice->CheckFeatureSupport(D3D11_FEATURE_THREADING, &threadingFeature, sizeof(threadingFeature));
        if (SUCCEEDED(hr)
            && threadingFeature.DriverConcurrentCreates
            && threadingFeature.DriverCommandLists)
        {
            //_features.SetMultithreading(true);
        }

        // Features
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
        _caps.features.textureCubeArray = (_d3dFeatureLevel >= D3D_FEATURE_LEVEL_10_1);

        // Limits
        _caps.limits.maxTextureDimension1D = _d3dFeatureLevel >= D3D_FEATURE_LEVEL_11_0 ? 16384u : 8192u;
        _caps.limits.maxTextureDimension2D = _d3dFeatureLevel >= D3D_FEATURE_LEVEL_11_0 ? 16384u : 8192u;
        _caps.limits.maxTextureDimension3D = _d3dFeatureLevel >= D3D_FEATURE_LEVEL_10_0 ? 2048u : 256u;
        _caps.limits.maxTextureDimensionCube = _d3dFeatureLevel >= D3D_FEATURE_LEVEL_10_0 ? 16384u : 8192u;
        _caps.limits.maxTextureArrayLayers = (_d3dFeatureLevel >= D3D_FEATURE_LEVEL_10_0 ? 2048u : 256u);;
        _caps.limits.maxColorAttachments = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;
        _caps.limits.maxUniformBufferSize = D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16;
        _caps.limits.minUniformBufferOffsetAlignment = 256u;
        _caps.limits.maxStorageBufferSize;
        _caps.limits.minStorageBufferOffsetAlignment = 16u;
        _caps.limits.maxSamplerAnisotropy = 16u;
        _caps.limits.maxViewports = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
        _caps.limits.maxViewportDimensions[0] = D3D11_VIEWPORT_BOUNDS_MAX;
        _caps.limits.maxViewportDimensions[1] = D3D11_VIEWPORT_BOUNDS_MAX;
        _caps.limits.maxPatchVertices = 32u; /* VK: maxTessellationPatchSize*/
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

    void GraphicsDeviceD3D11::HandleDeviceLost()
    {
        // TODO
    }

    D3D11Cache &GraphicsDeviceD3D11::GetCache()
    {
        return _cache;
    }

    Texture* GraphicsDeviceD3D11::CreateTextureImpl(const TextureDescriptor* descriptor, void* nativeTexture, const void* pInitData)
    {
        return new TextureD3D11(this, descriptor, nativeTexture, pInitData);
    }

    BufferHandle* GraphicsDeviceD3D11::CreateBufferImpl(const BufferDescriptor* descriptor, const void* pInitData)
    {
        return new BufferD3D11(this, descriptor, pInitData);
    }

    Sampler* GraphicsDeviceD3D11::CreateSamplerImpl(const SamplerDescriptor* descriptor)
    {
        return new SamplerD3D11(this, descriptor);
    }

    ShaderHandle* GraphicsDeviceD3D11::CreateShaderImpl(ShaderStage stage, const std::string& code, const std::string& entryPoint)
    {
        return new ShaderD3D11(this, stage, code);
    }
}
