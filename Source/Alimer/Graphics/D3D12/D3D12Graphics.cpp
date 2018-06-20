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
#include "D3D12Texture.h"
#include "D3D12CommandBuffer.h"
#include "D3D12GpuBuffer.h"
#include "D3D12Shader.h"
#include "D3D12PipelineState.h"
#include "../ShaderCompiler.h"
#include "../../Core/Log.h"
#include "../../Application/Windows/WindowWindows.h"

#if ALIMER_DEV && ( defined(_DEBUG) || defined(PROFILE) )
#	if !defined(_XBOX_ONE) || !defined(_TITLE)
#		pragma comment(lib,"dxguid.lib")
#	endif
#endif

#if !ALIMER_PLATFORM_UWP
static bool s_d3d12Initialized = false;
static HMODULE DXGIDebugHandle = nullptr;
static HMODULE DXGIHandle = nullptr;
static HMODULE D3D12Handle = nullptr;

decltype(CreateDXGIFactory2)* CreateDXGIFactory2Fn = nullptr;
PFN_D3D12_GET_DEBUG_INTERFACE D3D12GetDebugInterfaceFn = nullptr;
PFN_D3D12_CREATE_DEVICE D3D12CreateDeviceFn = nullptr;
PFN_D3D12_SERIALIZE_ROOT_SIGNATURE D3D12SerializeRootSignatureFn = nullptr;
PFN_D3D12_SERIALIZE_VERSIONED_ROOT_SIGNATURE D3D12SerializeVersionedRootSignatureFn = nullptr;
HRESULT D3D12LoadLibraries()
{
    if (s_d3d12Initialized)
        return S_OK;

    DXGIDebugHandle = LoadLibraryW(L"dxgidebug.dll");
    DXGIHandle = LoadLibraryW(L"dxgi.dll");
    D3D12Handle = LoadLibraryW(L"d3d12.dll");
    if (!DXGIHandle)
        return S_FALSE;
    if (!D3D12Handle)
        return S_FALSE;

    // Load symbols.
    //DXGIGetDebugInterface1Fn = (PFN_GET_DXGI_DEBUG_INTERFACE)::GetProcAddress(DXGIHandle, "DXGIGetDebugInterface1");
    CreateDXGIFactory2Fn = (decltype(CreateDXGIFactory2Fn))::GetProcAddress(DXGIHandle, "CreateDXGIFactory2");
    D3D12CreateDeviceFn = (PFN_D3D12_CREATE_DEVICE)::GetProcAddress(D3D12Handle, "D3D12CreateDevice");
    D3D12GetDebugInterfaceFn = (PFN_D3D12_GET_DEBUG_INTERFACE)::GetProcAddress(D3D12Handle, "D3D12GetDebugInterface");
    D3D12SerializeRootSignatureFn = (PFN_D3D12_SERIALIZE_ROOT_SIGNATURE)::GetProcAddress(D3D12Handle, "D3D12SerializeRootSignature");
    D3D12SerializeVersionedRootSignatureFn = (PFN_D3D12_SERIALIZE_VERSIONED_ROOT_SIGNATURE)::GetProcAddress(D3D12Handle, "D3D12SerializeVersionedRootSignature");

    if (!CreateDXGIFactory2Fn)
        return S_FALSE;
    if (!D3D12CreateDeviceFn)
        return S_FALSE;
    if (!D3D12GetDebugInterfaceFn)
        return S_FALSE;
    if (!D3D12SerializeRootSignatureFn)
        return S_FALSE;

    // Done.
    s_d3d12Initialized = true;
    return S_OK;
}
#endif

HRESULT privateD3D12GetDebugInterface(
    _In_ REFIID riid,
    _COM_Outptr_opt_ void** ppvDebug)
{
#ifdef VK_D3D_UWP
    return D3D12GetDebugInterface(riid, ppvDebug);
#else
    return D3D12GetDebugInterfaceFn(riid, ppvDebug);
#endif
}

HRESULT privateCreateDXGIFactory2(UINT Flags, REFIID riid, _COM_Outptr_ void **ppFactory)
{
#ifdef VK_D3D_UWP
    return CreateDXGIFactory2(Flags, riid, ppFactory);
#else
    return CreateDXGIFactory2Fn(Flags, riid, ppFactory);
#endif
}

HRESULT privateD3D12CreateDevice(
    _In_opt_ IUnknown* pAdapter,
    D3D_FEATURE_LEVEL MinimumFeatureLevel,
    _In_ REFIID riid, // Expected: ID3D12Device
    _COM_Outptr_opt_ void** ppDevice)
{
#ifdef VK_D3D_UWP
    return D3D12CreateDevice(pAdapter, MinimumFeatureLevel, riid, ppDevice);
#else
    return D3D12CreateDeviceFn(pAdapter, MinimumFeatureLevel, riid, ppDevice);
#endif
}

HRESULT privateD3D12SerializeRootSignature(
    _In_ const D3D12_ROOT_SIGNATURE_DESC* pRootSignature,
    _In_ D3D_ROOT_SIGNATURE_VERSION Version,
    _Out_ ID3DBlob** ppBlob,
    _Always_(_Outptr_opt_result_maybenull_) ID3DBlob** ppErrorBlob)
{
#ifdef VK_D3D_UWP
    return D3D12SerializeRootSignature(pRootSignature, Version, ppBlob, ppErrorBlob);
#else
    return D3D12SerializeRootSignatureFn(pRootSignature, Version, ppBlob, ppErrorBlob);
#endif
}

HRESULT privateD3D12SerializeVersionedRootSignature(
    _In_ const D3D12_VERSIONED_ROOT_SIGNATURE_DESC* pRootSignature,
    _Out_ ID3DBlob** ppBlob,
    _Always_(_Outptr_opt_result_maybenull_) ID3DBlob** ppErrorBlob)
{
#ifdef VK_D3D_UWP
    return D3D12SerializeVersionedRootSignature(pRootSignature, ppBlob, ppErrorBlob);
#else
    return D3D12SerializeVersionedRootSignatureFn(pRootSignature, ppBlob, ppErrorBlob);
#endif
}

namespace Alimer
{
    _Use_decl_annotations_ static void GetHardwareAdapter(_In_ IDXGIFactory2* pFactory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter)
    {
        ComPtr<IDXGIAdapter1> adapter;
        *ppAdapter = nullptr;

        for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Don't select the Basic Render Driver adapter.
                // If you want a software adapter, pass in "/warp" on the command line.
                continue;
            }

            // Check to see if the adapter supports Direct3D 12, but don't create the
            // actual device yet.
            if (SUCCEEDED(privateD3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
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
#if !ALIMER_PLATFORM_UWP
        if (FAILED(D3D12LoadLibraries()))
        {
            isAvailable = false;
            return false;
        }
#endif
        // Create temp dxgi factory for check support.
        ComPtr<IDXGIFactory2> factory;
        HRESULT hr = privateCreateDXGIFactory2(0, IID_PPV_ARGS(&factory));
        if (FAILED(hr))
        {
            isAvailable = false;
            return false;
        }

        ComPtr<IDXGIAdapter1> hardwareAdapter;
        GetHardwareAdapter(factory.Get(), &hardwareAdapter);
        isAvailable = hardwareAdapter != nullptr;
        return isAvailable;
    }

    D3D12Graphics::D3D12Graphics(bool validation)
        : Graphics(GraphicsDeviceType::Direct3D12, validation)
        , _commandListManager(nullptr)
        , _descriptorAllocator{
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
        D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_TYPE_DSV }
    {

    }

    D3D12Graphics::~D3D12Graphics()
    {
        // Ensure that the GPU is no longer referencing resources that are about to be
        // cleaned up by the destructor.
        WaitIdle();

        // Delete command list manager.
        SafeDelete(_commandListManager);

        // Clear DescriptorHeap Pools.
        {
            std::lock_guard<std::mutex> guard(_heapAllocationMutex);
            _descriptorHeapPool.clear();
        }
    }

    bool D3D12Graphics::Initialize(const SharedPtr<Window>& window)
    {
#if !ALIMER_PLATFORM_UWP
        if (FAILED(D3D12LoadLibraries()))
        {
            ALIMER_LOGERROR("Failed to load D3D12 libraries.");
            return false;
        }
#endif

        UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
        // Enable the debug layer (requires the Graphics Tools "optional feature").
        // NOTE: Enabling the debug layer after device creation will invalidate the active device.
        {
            ComPtr<ID3D12Debug> debugController;
            if (SUCCEEDED(privateD3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
            {
                debugController->EnableDebugLayer();

                // Enable additional debug layers.
                dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
            }
        }
#endif

        HRESULT hr = privateCreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&_factory));
        if (FAILED(hr))
        {
            return false;
        }

        if (_useWarpDevice)
        {
            ComPtr<IDXGIAdapter1> warpAdapter;
            ThrowIfFailed(_factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

            ThrowIfFailed(privateD3D12CreateDevice(
                warpAdapter.Get(),
                D3D_FEATURE_LEVEL_11_0,
                IID_PPV_ARGS(&_d3dDevice)
            ));
        }
        else
        {
            ComPtr<IDXGIAdapter1> adapter;
            for (UINT adapterIndex = 0;
                _factory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND; ++adapterIndex)
            {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    // Don't select the Basic Render Driver adapter.
                    // If you want a software adapter, pass in "/warp" on the command line.
                    continue;
                }

                // Check to see if the adapter supports Direct3D 12, but don't create the
                // actual device yet.
                if (SUCCEEDED(privateD3D12CreateDevice(
                    adapter.Get(),
                    D3D_FEATURE_LEVEL_11_0,
                    IID_PPV_ARGS(&_d3dDevice))))
                {
                    break;
                }
            }
        }

        if (!InitializeCaps())
            return false;

#ifndef NDEBUG
        // Setup break on error + corruption.
        ComPtr<ID3D12InfoQueue> d3dInfoQueue;
        if (SUCCEEDED(_d3dDevice.As(&d3dInfoQueue)))
        {
#ifdef _DEBUG
            d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
            d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
#endif

            // Disable some warnings that we don't generally care about, unless all warnings are enabled.
            const bool EnableAllWarnings = false;
            if (!EnableAllWarnings)
            {
                D3D12_MESSAGE_ID denyIDs[] = {
                    D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
                    D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE
                };

                D3D12_INFO_QUEUE_FILTER filter = {};
                filter.DenyList.NumIDs = 2;
                filter.DenyList.pIDList = denyIDs;
                d3dInfoQueue->PushStorageFilter(&filter);
            }
        }
#endif
        // Create the command list manager class.
        _commandListManager = new D3D12CommandListManager(_d3dDevice.Get());

        // Init heaps.
        for (uint32_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
        {
            _descriptorAllocator[i].Initialize(this);
        }

        // Create Swapchain.
        CreateSwapchain(window);

        return Graphics::Initialize(window);
    }

    bool D3D12Graphics::WaitIdle()
    {
        return _commandListManager->WaitIdle();
    }

    SharedPtr<Texture> D3D12Graphics::AcquireNextImage()
    {
        uint32_t index = _swapChain->GetCurrentBackBufferIndex();
        return _textures[index];
    }

    void D3D12Graphics::EndFrame()
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

    bool D3D12Graphics::InitializeCaps()
    {
        // https://github.com/Microsoft/DirectX-Graphics-Samples/blob/master/TechniqueDemos/D3D12MemoryManagement/src/Framework.cpp
        D3D12_FEATURE_DATA_D3D12_OPTIONS options;
        HRESULT hr = _d3dDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options));
        if (FAILED(hr))
        {
            ALIMER_LOGERROR("Failed to acquire D3D12 options for ID3D12Device 0x%p, hr=0x%.8x", _d3dDevice.Get(), hr);
            return false;
        }

        D3D12_FEATURE_DATA_GPU_VIRTUAL_ADDRESS_SUPPORT gpuVaSupport;
        hr = _d3dDevice->CheckFeatureSupport(D3D12_FEATURE_GPU_VIRTUAL_ADDRESS_SUPPORT, &gpuVaSupport, sizeof(gpuVaSupport));
        if (FAILED(hr))
        {
            ALIMER_LOGERROR("Failed to acquire GPU virtual address support for ID3D12Device 0x%p, hr=0x%.8x", _d3dDevice.Get(), hr);
            return false;
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
        if (FAILED(hr))
        {
            return false;
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

        return true;
    }

    void D3D12Graphics::HandleDeviceLost()
    {
        // TODO
    }

    void D3D12Graphics::CreateSwapchain(const SharedPtr<Window>& window)
    {
        // Describe and create the swap chain.
        // TODO: Add VSync support.
        // TODO: Add Tearing support.
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.BufferCount = FrameCount;
        swapChainDesc.Width = window->GetWidth();
        swapChainDesc.Height = window->GetHeight();
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.SampleDesc.Count = 1;

        ComPtr<IDXGISwapChain1> swapChain;

#if !ALIMER_PLATFORM_UWP
        SharedPtr<WindowWindows> win32Window = StaticCast<WindowWindows>(window);

        HWND handle = win32Window->GetHandle();
        ThrowIfFailed(_factory->CreateSwapChainForHwnd(
            _commandListManager->GetD3DCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT),
            handle,
            &swapChainDesc,
            nullptr,
            nullptr,
            &swapChain
        ));

        // This sample does not support fullscreen transitions.
        ThrowIfFailed(_factory->MakeWindowAssociation(handle, DXGI_MWA_NO_ALT_ENTER));
#else
        ThrowIfFailed(_factory->CreateSwapChainForCoreWindow(
            _commandListManager->GetD3DCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT),
            reinterpret_cast<IUnknown*>(Windows::UI::Core::CoreWindow::GetForCurrentThread()),
            &swapChainDesc,
            nullptr,
            &swapChain
        ));
#endif

        ThrowIfFailed(swapChain.As(&_swapChain));

        _textures.resize(swapChainDesc.BufferCount);

        for (UINT i = 0; i < swapChainDesc.BufferCount; ++i)
        {
            // Get buffer from swapchain.
            ThrowIfFailed(_swapChain->GetBuffer(i, IID_PPV_ARGS(&_renderTargets[i])));

            auto d3dTexture = new D3D12Texture(this, _renderTargets[i].Get());
            d3dTexture->SetUsageState(D3D12_RESOURCE_STATE_PRESENT);
            _textures[i] = d3dTexture;
        }
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
        heapDesc.NodeMask = 1;

        ComPtr<ID3D12DescriptorHeap> heap;
        ThrowIfFailed(_d3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heap)));
        _descriptorHeapPool.emplace_back(heap);
        return heap.Get();
    }
}
