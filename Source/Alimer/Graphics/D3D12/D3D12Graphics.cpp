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

#include "D3D12Graphics.h"
#include "D3D12CommandListManager.h"
#include "D3D12Swapchain.h"
/*#include "D3D12Texture.h"
//#include "D3D12CommandBuffer.h"
#include "D3D12GpuBuffer.h"
#include "D3D12Shader.h"
#include "D3D12PipelineState.h"
#include "D3D12GpuAdapter.h"
#include "../ShaderCompiler.h"*/
#include "../../Debug/Log.h"

#if defined(_DEBUG)
#include <dxgidebug.h>
#endif

#if defined(_DEBUG) || defined(PROFILE)
#if !defined(_XBOX_ONE) || !defined(_TITLE) || !defined(_DURANGO)
#   pragma comment(lib,"dxguid.lib")
#endif
#endif

#include <d3dcompiler.h>
#if !(defined(WINAPI_FAMILY_PARTITION) && !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP))
#   pragma comment (lib, "d3dcompiler.lib")
#endif

#if ALIMER_D3D_DYNAMIC_LIB
typedef HRESULT(WINAPI* PFN_CREATE_DXGI_FACTORY2)(UINT flags, REFIID _riid, void** _factory);
typedef HRESULT(WINAPI* PFN_GET_DXGI_DEBUG_INTERFACE1)(UINT Flags, REFIID riid, _COM_Outptr_ void** pDebug);
#endif

namespace Alimer
{
#ifdef ALIMER_D3D_DYNAMIC_LIB
    static
        HMODULE s_dxgiLib = nullptr;
    HMODULE s_d3d12Lib = nullptr;

    PFN_CREATE_DXGI_FACTORY2                        CreateDXGIFactory2 = nullptr;
    PFN_GET_DXGI_DEBUG_INTERFACE1                   DXGIGetDebugInterface1 = nullptr;

    PFN_D3D12_GET_DEBUG_INTERFACE                   D3D12GetDebugInterface = nullptr;
    PFN_D3D12_CREATE_DEVICE                         D3D12CreateDevice = nullptr;
    PFN_D3D12_SERIALIZE_ROOT_SIGNATURE              D3D12SerializeRootSignature = nullptr;
    PFN_D3D12_SERIALIZE_VERSIONED_ROOT_SIGNATURE    D3D12SerializeVersionedRootSignature = nullptr;

    HRESULT D3D12LoadLibraries()
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

    _Use_decl_annotations_
        static void GetD3D12HardwareAdapter(_In_ IDXGIFactory2* factory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter)
    {
        ComPtr<IDXGIAdapter1> adapter;
        *ppAdapter = nullptr;

#if defined(__dxgi1_6_h__) && defined(NTDDI_WIN10_RS4)
        ComPtr<IDXGIFactory6> factory6;
        if (SUCCEEDED(factory->QueryInterface(factory6.ReleaseAndGetAddressOf())))
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
                if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
                {
                    break;
                }
            }
        }
        else
#endif
            for (UINT adapterIndex = 0;
                DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(
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
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
            {
                break;
            }
        }

        *ppAdapter = adapter.Detach();
    }

    bool D3D12Graphics::IsSupported()
    {
        static bool availableCheck = false;
        static bool isAvailable = false;

        if (availableCheck)
            return isAvailable;

        availableCheck = true;
#if ALIMER_D3D_DYNAMIC_LIB
        if (FAILED(D3D12LoadLibraries()))
        {
            isAvailable = false;
            return false;
        }
#endif
        // Create temp dxgi factory for check support.
        ComPtr<IDXGIFactory2> factory;
        HRESULT hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));
        if (FAILED(hr))
        {
            isAvailable = false;
            return false;
        }

        ComPtr<IDXGIAdapter1> hardwareAdapter;
        GetD3D12HardwareAdapter(factory.Get(), &hardwareAdapter);
        isAvailable = hardwareAdapter != nullptr;
        return isAvailable;
    }

    D3D12Graphics::D3D12Graphics(bool validation)
        : Graphics(GraphicsBackend::D3D12, validation)
        , _descriptorAllocator{
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
        D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
        D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
        D3D12_DESCRIPTOR_HEAP_TYPE_DSV }
    {
        if (!IsSupported())
        {
            ALIMER_LOGERROR("D3D12 backend is not supported.");
            return;
        }

        UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
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
                ALIMER_LOGWARN("Direct3D Debug Device is not available");
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
        }

        GetD3D12HardwareAdapter(_factory.Get(), _adapter.ReleaseAndGetAddressOf());

#ifdef _DEBUG
        DXGI_ADAPTER_DESC1 desc = { };
        _adapter->GetDesc1(&desc);
        wchar_t buff[256] = {};
        swprintf_s(buff, L"Creating DX12 device using Adapter: VID:%04X, PID:%04X - %ls\n", desc.VendorId, desc.DeviceId, desc.Description);
        OutputDebugStringW(buff);
#endif

        ThrowIfFailed(D3D12CreateDevice(_adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&_d3dDevice)));

#ifndef NDEBUG
        if (validation)
        {
            // Configure debug device (if active).
            ComPtr<ID3D12InfoQueue> d3dInfoQueue;
            if (SUCCEEDED(_d3dDevice.As(&d3dInfoQueue)))
            {
#ifdef _DEBUG
                d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
                d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
#endif
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
                d3dInfoQueue->AddStorageFilterEntries(&filter);
            }
        }
#endif

        InitializeFeatures();

        // Create the command list manager class.
        _commandListManager = new D3D12CommandListManager(_d3dDevice.Get());

        // Init heaps.
        for (uint32_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
        {
            _descriptorAllocator[i].SetGraphics(this);
        }
    }

    D3D12Graphics::~D3D12Graphics()
    {
        // Ensure that the GPU is no longer referencing resources that are about to be
        // cleaned up by the destructor.
        WaitIdle();

        // Delete command list manager.
        SafeDelete(_commandListManager);

        // Delete main swap chain if created.
        _mainSwapChain.Reset();

        // Clear DescriptorHeap Pools.
        {
            std::lock_guard<std::mutex> guard(_heapAllocationMutex);
            _descriptorHeapPool.clear();
        }
    }

    void D3D12Graphics::InitializeFeatures()
    {
        // https://github.com/Microsoft/DirectX-Graphics-Samples/blob/master/TechniqueDemos/D3D12MemoryManagement/src/Framework.cpp
        D3D12_FEATURE_DATA_D3D12_OPTIONS options;
        HRESULT hr = _d3dDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options));
        if (FAILED(hr))
        {
            ALIMER_LOGERRORF("Failed to acquire D3D12 options for ID3D12Device 0x%p, hr=0x%.8x", _d3dDevice.Get(), hr);
        }

        D3D12_FEATURE_DATA_GPU_VIRTUAL_ADDRESS_SUPPORT gpuVaSupport;
        hr = _d3dDevice->CheckFeatureSupport(D3D12_FEATURE_GPU_VIRTUAL_ADDRESS_SUPPORT, &gpuVaSupport, sizeof(gpuVaSupport));
        if (FAILED(hr))
        {
            ALIMER_LOGERRORF("Failed to acquire GPU virtual address support for ID3D12Device 0x%p, hr=0x%.8x", _d3dDevice.Get(), hr);
        }

        // Determine maximum supported feature level for this device
        static const D3D_FEATURE_LEVEL s_featureLevels[] =
        {
            D3D_FEATURE_LEVEL_12_1,
            D3D_FEATURE_LEVEL_12_0,
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
        };

        D3D12_FEATURE_DATA_FEATURE_LEVELS featLevels =
        {
            _countof(s_featureLevels), s_featureLevels, D3D_FEATURE_LEVEL_11_0
        };

        hr = _d3dDevice->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &featLevels, sizeof(featLevels));
        if (SUCCEEDED(hr))
        {
            _d3dFeatureLevel = featLevels.MaxSupportedFeatureLevel;
        }
        else
        {
            _d3dFeatureLevel = D3D_FEATURE_LEVEL_11_0;
        }

        // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
        _featureDataRootSignature.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

        if (FAILED(_d3dDevice->CheckFeatureSupport(
            D3D12_FEATURE_ROOT_SIGNATURE,
            &_featureDataRootSignature,
            sizeof(_featureDataRootSignature))))
        {
            _featureDataRootSignature.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
        }

#if ALIMER_DXR
        D3D12_FEATURE_DATA_D3D12_OPTIONS5 options;
        if (SUCCEEDED(_d3dDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options, sizeof(options)))
            && options.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED)
        {
            _raytracingSupported = true;
        }
#endif
    }

    bool D3D12Graphics::Initialize(const GraphicsSettings& settings)
    {
        if (!settings.headless)
        {
            // Create Swapchain.
            _mainSwapChain = new D3D12Swapchain(this, &settings.swapchain);
        }

        return true;
    }

    bool D3D12Graphics::WaitIdle()
    {
        _commandListManager->WaitIdle();
        return true;
    }

    void D3D12Graphics::Frame()
    {
        if (_mainSwapChain != nullptr)
        {
            //CmdEndRenderPass();
        }

        // Send the command list off to the GPU for processing.
        //_primaryCommandBuffer->End();
        //s_pActiveCommandBuffer = nullptr;

        /* End frame for upload */
        //EndFrameUpload();

        //ID3D12CommandList* ppCommandLists[] = { _primaryCommandBuffer->GetCommandList() };
        //_graphicsQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

        // Present the frame.
        if (_mainSwapChain)
        {
            _mainSwapChain->Present();
        }
    }

#if TODO
    CommandBuffer* D3D12Graphics::GetDefaultCommandBuffer() const
    {
        return nullptr;
    }

    SharedPtr<RenderPass> D3D12Graphics::BeginFrameCore()
    {
        uint32_t index = _swapChain->GetCurrentBackBufferIndex();
        //return _textures[index];
        return nullptr;
    }

    void D3D12Graphics::EndFrameCore()
    {
        // Submit frame command buffer.
        if (_frameCommandBuffer)
        {
            _frameCommandBuffer->Commit(true);
            RecycleCommandBuffer(_frameCommandBuffer);
        }

        // Present the frame.
        HRESULT hr = _swapChain->Present(1, 0);
        if (hr == DXGI_ERROR_DEVICE_REMOVED
            || hr == DXGI_ERROR_DEVICE_RESET
            || hr == DXGI_ERROR_DRIVER_INTERNAL_ERROR)
        {
#ifdef _DEBUG
            char buff[64] = {};
            sprintf_s(buff, "Device Lost on Present: Reason code 0x%08X\n", (hr == DXGI_ERROR_DEVICE_REMOVED) ? _d3dDevice->GetDeviceRemovedReason() : hr);
            OutputDebugStringA(buff);
#endif

            HandleDeviceLost();
        }
    }

    void D3D12Graphics::HandleDeviceLost()
    {
        // TODO
    }



    /*SharedPtr<CommandBuffer> D3D12Graphics::GetCommandBuffer()
    {
        _frameCommandBuffer = RetrieveCommandBuffer();
        if (_frameCommandBuffer == nullptr)
        {
            _frameCommandBuffer = MakeShared<D3D12CommandBuffer>(this);
        }
        else
        {
            _frameCommandBuffer->Reset();
        }

        return _frameCommandBuffer;
    }*/

    SharedPtr<RenderPass> D3D12Graphics::CreateRenderPass(const RenderPassDescription& description)
    {
        return nullptr;
    }

    SharedPtr<GpuBuffer> D3D12Graphics::CreateBuffer(const GpuBufferDescription& description, const void* initialData)
    {
        return MakeShared<D3D12GpuBuffer>(this, description, initialData);
    }

    SharedPtr<Shader> D3D12Graphics::CreateComputeShader(const ShaderStageDescription& desc)
    {
        return MakeShared<D3D12Shader>(this, desc);
    }

    SharedPtr<Shader> D3D12Graphics::CreateShader(const ShaderStageDescription& vertex, const ShaderStageDescription& fragment)
    {
        return MakeShared<D3D12Shader>(this, vertex, fragment);
    }

    SharedPtr<PipelineState> D3D12Graphics::CreateRenderPipelineState(const RenderPipelineDescriptor& descriptor)
    {
        return MakeShared<D3D12PipelineState>(this, descriptor);
    }

    SharedPtr<D3D12CommandBuffer> D3D12Graphics::RetrieveCommandBuffer()
    {
        std::lock_guard<std::mutex> lock(_commandBufferMutex);

        if (_commandBufferObjectId == 0)
            return nullptr;

        return _recycledCommandBuffers.at(--_commandBufferObjectId);
    }

    void D3D12Graphics::RecycleCommandBuffer(D3D12CommandBuffer* commandBuffer)
    {
        std::lock_guard<std::mutex> lock(_commandBufferMutex);

        if (_commandBufferObjectId < CommandBufferRecycleCount)
        {
            _recycledCommandBuffers.at(_commandBufferObjectId++) = commandBuffer;
        }
    }
#endif // TODO

    ID3D12DescriptorHeap* D3D12Graphics::RequestNewHeap(
        D3D12_DESCRIPTOR_HEAP_TYPE type,
        uint32_t numDescriptors)
    {
        std::lock_guard<std::mutex> guard(_heapAllocationMutex);

        D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
        heapDesc.Type = type;
        heapDesc.NumDescriptors = numDescriptors;
        if (type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
            || type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
        {
            heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        }
        else
        {
            heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        }
        heapDesc.NodeMask = 0;

        ComPtr<ID3D12DescriptorHeap> heap;
        ThrowIfFailed(_d3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heap)));
        _descriptorHeapPool.emplace_back(heap);
        return heap.Get();
    }
}
