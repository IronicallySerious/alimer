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

#include "GraphicsDeviceD3D12.h"
#include "SwapChainD3D12.h"
#include "foundation/Utils.h"

#ifdef _DEBUG
#   if !defined(_XBOX_ONE) || !defined(_TITLE) || !defined(_DURANGO)
#       pragma comment(lib,"dxguid.lib")
#   endif
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
    static DXGI_GPU_PREFERENCE GetDXGIGpuPreference(GpuPowerPreference preference)
    {
        switch (preference)
        {
        case GpuPowerPreference::LowPower:
            return DXGI_GPU_PREFERENCE_MINIMUM_POWER;
        case GpuPowerPreference::HighPerformance:
            return DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE;
        default:
            return DXGI_GPU_PREFERENCE_UNSPECIFIED;
        }
    }
#endif

    bool GraphicsDeviceD3D12::IsSupported()
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
        GetAdapter(factory, GpuPowerPreference::Default, &hardwareAdapter);
        isAvailable = hardwareAdapter != nullptr;
        SafeRelease(hardwareAdapter);
        ALIMER_VERIFY(factory->Release() == 0);
        return isAvailable;
    }

    GraphicsDeviceD3D12::GraphicsDeviceD3D12(GpuPowerPreference powerPreference)
    {
        ALIMER_ASSERT_MSG(IsSupported(), "D3D12 backend is not supported");

        UINT dxgiFactoryFlags = 0;
#if defined(_DEBUG)
        bool validation = false;
        // Enable the debug layer (requires the Graphics Tools "optional feature").
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
            debugController->Release();
            validation = true;
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

        // Get adapter based on preference (if supported).
        GetAdapter(_factory, powerPreference, &_adapter);

        // Create device with adapter and min feature level.
        hr = D3D12CreateDevice(_adapter, _d3dMinFeatureLevel, IID_PPV_ARGS(&_d3dDevice));
        if (FAILED(hr))
        {

        }

#if defined(_DEBUG)
        if (validation)
        {
            // Configure debug device (if active).
            ID3D12InfoQueue* infoQueue;
            if (SUCCEEDED(_d3dDevice->QueryInterface(&infoQueue)))
            {
                D3D12_MESSAGE_ID hide[] =
                {
                    D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,

                    // These happen when capturing with VS diagnostics
                    D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
                    D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE
                };
                D3D12_INFO_QUEUE_FILTER filter = {};
                filter.DenyList.NumIDs = _countof(hide);
                filter.DenyList.pIDList = hide;
                infoQueue->AddStorageFilterEntries(&filter);

                infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
                infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
                //infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

                ALIMER_VERIFY(infoQueue->Release() == 0);
            }
        }
#endif

        InitializeCaps();

        // Create command queue's
        D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
        commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        commandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        commandQueueDesc.NodeMask = 0;
        ThrowIfFailed(_d3dDevice->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&_d3d12DirectCommandQueue)));
    }

    GraphicsDeviceD3D12::~GraphicsDeviceD3D12()
    {
        SafeRelease(_d3d12DirectCommandQueue);
        SafeRelease(_d3dDevice);
        SafeRelease(_adapter);
        ALIMER_VERIFY(_factory->Release() == 0);
    }

    void GraphicsDeviceD3D12::InitializeCaps()
    {
        // Check the maximum feature level, and make sure it's above our minimum
        static const D3D_FEATURE_LEVEL s_featureLevels[] =
        {
            D3D_FEATURE_LEVEL_12_1,
            D3D_FEATURE_LEVEL_12_0,
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
        };

        D3D12_FEATURE_DATA_FEATURE_LEVELS featureLevels =
        {
            _countof(s_featureLevels), s_featureLevels, D3D_FEATURE_LEVEL_11_0
        };

        HRESULT hr = _d3dDevice->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &featureLevels, sizeof(featureLevels));
        if (SUCCEEDED(hr))
        {
            _d3dFeatureLevel = featureLevels.MaxSupportedFeatureLevel;
        }
        else
        {
            _d3dFeatureLevel = _d3dMinFeatureLevel;
        }

        DXGI_ADAPTER_DESC1 desc = { };
        _adapter->GetDesc1(&desc);

        std::wstring deviceName(desc.Description);

        _info.backend = GraphicsBackend::Direct3D12;
        _info.backendName = "Direct3D12 - Level " + D3DFeatureLevelToVersion(_d3dFeatureLevel);
        _info.deviceName = std::string(deviceName.begin(), deviceName.end());
        _info.vendorName = D3DGetVendorByID(desc.VendorId);
        _info.vendorId = desc.VendorId;

        // Under D3D12 minimum supported feature level is 11.0
        _caps.features.instancing = true;
        _caps.features.alphaToCoverage = true;
        _caps.features.independentBlend = true;
        _caps.features.computeShader = true;
        _caps.features.geometryShader = true;
        _caps.features.tessellationShader = true;
        _caps.features.sampleRateShading = true;
        _caps.features.dualSrcBlend = true;
        _caps.features.logicOp = (_d3dFeatureLevel >= D3D_FEATURE_LEVEL_11_1);
        _caps.features.multiViewport = true;
        _caps.features.indexUInt32 = true;
        _caps.features.drawIndirect = true;
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
        _caps.features.texture2DArray = true;
        _caps.features.textureCubeArray = true;

        D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = { };
        ThrowIfFailed(_d3dDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(options5)));
        if (options5.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED)
        {
            _caps.features.raytracing = true;
        }
        else
        {
            _caps.features.raytracing = false;
        }

        // Limits
        _caps.limits.maxTextureDimension1D = D3D12_REQ_TEXTURE1D_U_DIMENSION;
        _caps.limits.maxTextureDimension2D = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
        _caps.limits.maxTextureDimension3D = D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;
        _caps.limits.maxTextureDimensionCube = D3D12_REQ_TEXTURECUBE_DIMENSION;
        _caps.limits.maxTextureArrayLayers = D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;
        _caps.limits.maxColorAttachments = D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT;
        _caps.limits.maxUniformBufferSize = D3D12_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16;
        _caps.limits.minUniformBufferOffsetAlignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
        _caps.limits.maxStorageBufferSize = std::numeric_limits<uint32_t>::max();
        _caps.limits.minStorageBufferOffsetAlignment = 16;
        _caps.limits.maxSamplerAnisotropy = D3D12_MAX_MAXANISOTROPY;
        _caps.limits.maxViewports = D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
        _caps.limits.maxViewportDimensions[0] = D3D12_VIEWPORT_BOUNDS_MAX;
        _caps.limits.maxViewportDimensions[1] = D3D12_VIEWPORT_BOUNDS_MAX;
        _caps.limits.maxPatchVertices = D3D12_IA_PATCH_MAX_CONTROL_POINT_COUNT;
        _caps.limits.pointSizeRange[0] = 1.0f;
        _caps.limits.pointSizeRange[1] = 1.0f;
        _caps.limits.lineWidthRange[0] = 1.0f;
        _caps.limits.lineWidthRange[1] = 1.0f;
        _caps.limits.maxComputeSharedMemorySize = D3D12_CS_THREAD_LOCAL_TEMP_REGISTER_POOL;
        _caps.limits.maxComputeWorkGroupCount[0] = D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION;
        _caps.limits.maxComputeWorkGroupCount[1] = D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION;
        _caps.limits.maxComputeWorkGroupCount[2] = D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION;
        _caps.limits.maxComputeWorkGroupInvocations = D3D12_CS_THREAD_GROUP_MAX_THREADS_PER_GROUP;
        _caps.limits.maxComputeWorkGroupSize[0] = D3D12_CS_THREAD_GROUP_MAX_X;
        _caps.limits.maxComputeWorkGroupSize[1] = D3D12_CS_THREAD_GROUP_MAX_Y;
        _caps.limits.maxComputeWorkGroupSize[2] = D3D12_CS_THREAD_GROUP_MAX_Z;
    }

    void GraphicsDeviceD3D12::GetAdapter(_In_ IDXGIFactory2* factory, GpuPowerPreference preference, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter)
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

    PixelFormat GraphicsDeviceD3D12::GetDefaultDepthStencilFormat() const
    {
        D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport =
        {
            DXGI_FORMAT_D24_UNORM_S8_UINT, D3D12_FORMAT_SUPPORT1_NONE, D3D12_FORMAT_SUPPORT2_NONE
        };

        if (SUCCEEDED(_d3dDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport)))
            && formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_RENDER_TARGET | D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL)
        {
            return PixelFormat::Depth24UNormStencil8;
        }

        formatSupport.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
        if (SUCCEEDED(_d3dDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport)))
            && formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_RENDER_TARGET | D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL)
        {
            return PixelFormat::Depth32FloatStencil8;
        }

        return PixelFormat::Undefined;
    }

    PixelFormat GraphicsDeviceD3D12::GetDefaultDepthFormat() const
    {
        D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport =
        {
            DXGI_FORMAT_D32_FLOAT, D3D12_FORMAT_SUPPORT1_NONE, D3D12_FORMAT_SUPPORT2_NONE
        };

        if (SUCCEEDED(_d3dDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport)))
            && formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_RENDER_TARGET | D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL)
        {
            return PixelFormat::Depth32Float;
        }

        formatSupport.Format = DXGI_FORMAT_D16_UNORM;
        if (SUCCEEDED(_d3dDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport)))
            && formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_RENDER_TARGET | D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL)
        {
            return PixelFormat::Depth16UNorm;
        }

        return PixelFormat::Undefined;
    }

    ID3D12CommandQueue* GraphicsDeviceD3D12::GetCommandQueue(D3D12_COMMAND_LIST_TYPE type) const
    {
        ID3D12CommandQueue* commandQueue = nullptr;
        switch (type)
        {
        case D3D12_COMMAND_LIST_TYPE_DIRECT:
            commandQueue = _d3d12DirectCommandQueue;
            break;
        case D3D12_COMMAND_LIST_TYPE_COMPUTE:
            //commandQueue = _d3d12ComputeCommandQueue;
            break;
        case D3D12_COMMAND_LIST_TYPE_COPY:
            //commandQueue = _d3d12CopyCommandQueue;
            break;
        default:
            ALIMER_ASSERT_MSG(false, "Invalid command queue type.");
        }

        return commandQueue;
    }

    SwapChain* GraphicsDeviceD3D12::CreateSwapChainImpl(const SwapChainSurface* surface, const SwapChainDescriptor* descriptor)
    {
        return new SwapChainD3D12(this, surface, descriptor);
    }
}
