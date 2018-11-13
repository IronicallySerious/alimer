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
#include <wrl/client.h>
#include <wrl/event.h>
#include "../Core/Platform.h"
#include "../Core/Log.h"

#if defined(_DEBUG) || defined(PROFILE)
#   if !defined(_XBOX_ONE) || !defined(_TITLE) || !defined(_DURANGO)
#       pragma comment(lib,"dxguid.lib")
#   endif
#endif

#if defined(_DURANGO) || defined(_XBOX_ONE)
#   define AGPU_D3D12_DYNAMIC_LIB 0
#elif defined(WINAPI_FAMILY) && (WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP)
#   define AGPU_D3D12_DYNAMIC_LIB 0
#elif defined(_WIN64) || defined(_WIN32)
#   define AGPU_D3D12_DYNAMIC_LIB 1
#endif

#if AGPU_D3D12_DYNAMIC_LIB
typedef HRESULT(WINAPI* PFN_CREATE_DXGI_FACTORY2)(UINT flags, REFIID _riid, void** _factory);
typedef HRESULT(WINAPI* PFN_GET_DXGI_DEBUG_INTERFACE1)(UINT Flags, REFIID riid, _COM_Outptr_ void** pDebug);
#endif

namespace d3d12
{
    using Microsoft::WRL::ComPtr;

    template<typename T> void Release(T*& resource)
    {
        if (resource != nullptr) {
            resource->Release();
            resource = nullptr;
        }
    }

#ifdef _DEBUG
#define DXCall(hr) \
    do \
    { \
        ALIMER_ASSERT_MSG(SUCCEEDED(hr), Alimer::GetDXErrorString(hr).CString()); \
    } \
    while(0)
#else
    // Throws a DXException on failing HRESULT
    inline void DXCall(HRESULT hr)
    {
        if (FAILED(hr))
        {
            ALIMER_LOGCRITICALF("DirectX Error: %s", Alimer::GetDXErrorString(hr).CString());
        }
    }

#endif

#ifdef AGPU_D3D12_DYNAMIC_LIB
    HMODULE s_dxgiLib = nullptr;
    HMODULE s_d3d12Lib = nullptr;

    PFN_CREATE_DXGI_FACTORY2            CreateDXGIFactory2 = nullptr;
    PFN_GET_DXGI_DEBUG_INTERFACE1       DXGIGetDebugInterface1 = nullptr;

    PFN_D3D12_GET_DEBUG_INTERFACE       D3D12GetDebugInterface = nullptr;
    PFN_D3D12_CREATE_DEVICE             D3D12CreateDevice = nullptr;
    PFN_D3D12_SERIALIZE_ROOT_SIGNATURE  D3D12SerializeRootSignature = nullptr;
#endif

    static const uint64_t RenderLatency = 2;
    static const uint64_t NumCmdAllocators = RenderLatency;

    DWORD           factoryFlags = 0;
    IDXGIFactory4*  factory = nullptr;

    void agpuD3D12GetDXGIAdapter(IDXGIAdapter1** ppAdapter);

    struct AGpuRendererD3D12 : public AGpuRendererI
    {
        AGpuRendererD3D12()
        {
        }

        ~AGpuRendererD3D12()
        {
        }

        AgpuResult initialize(const AgpuDescriptor* descriptor);
        void shutdown() override;
        void waitIdle();
        AgpuResult beginFrame() override;
        uint64_t endFrame() override;

        /* Fence */
        AgpuFence CreateFence() override;
        void DestroyFence(AgpuFence fence) override;
        void FenceSignal(AgpuFence fence, ID3D12CommandQueue* queue, uint64_t fenceValue);
        void FenceWait(AgpuFence fence, uint64_t fenceValue);
        bool FenceSignaled(AgpuFence fence, uint64_t fenceValue);
        void FenceClear(AgpuFence fence, uint64_t fenceValue);

        template<typename T> void DeferredRelease(T*& resource, bool forceDeferred = false)
        {
            IUnknown* base = resource;
            DeferredRelease_(base, forceDeferred);
            resource = nullptr;
        }

    private:
        void DeferredRelease_(IUnknown* resource, bool forceDeferred = false);
        void ProcessDeferredReleases(uint64_t frameIndex);

    private:
        IDXGIAdapter1*              _dxgiAdapter = nullptr;
        ID3D12Device*               _d3dDevice = nullptr;
        D3D_FEATURE_LEVEL           _d3dFeatureLevel = D3D_FEATURE_LEVEL_11_0;
        AgpuFence                   _frameFence = nullptr;
        ID3D12CommandQueue*         _graphicsQueue = nullptr;
        ID3D12GraphicsCommandList*  _commandList = nullptr;
        ID3D12CommandAllocator*     _commandAllocators[AGPU_MAX_BACK_BUFFER_COUNT];
        uint64_t                    _currentCPUFrame = 0;
        uint64_t                    _currentGPUFrame = 0;
        uint64_t                    _currentFrameIndex = 0;
        bool                        _shuttingDown = false;
        std::vector<IUnknown*>      _deferredReleases[RenderLatency];
    };

    AgpuResult AGpuRendererD3D12::initialize(const AgpuDescriptor* descriptor)
    {
        _shuttingDown = false;
        agpuD3D12GetDXGIAdapter(&_dxgiAdapter);

        DXCall(D3D12CreateDevice(_dxgiAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&_d3dDevice)));

#ifndef NDEBUG
        if (descriptor->validation)
        {
            // Configure debug device (if active).
            ID3D12InfoQueue* d3dInfoQueue;
            if (SUCCEEDED(_d3dDevice->QueryInterface(&d3dInfoQueue)))
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

                Release(d3dInfoQueue);
            }
        }
#endif

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
        if (SUCCEEDED(
            _d3dDevice->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &featLevels, sizeof(featLevels)))
            )
        {
            _d3dFeatureLevel = featLevels.MaxSupportedFeatureLevel;
        }
        else
        {
            _d3dFeatureLevel = D3D_FEATURE_LEVEL_11_0;
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

        DXCall(_d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&_graphicsQueue)));
        _graphicsQueue->SetName(L"Main Gfx Queue");

        // Create a command allocator for each back buffer that will be rendered to.
        for (uint64_t n = 0; n < NumCmdAllocators; n++)
        {
            DXCall(_d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_commandAllocators[n])));

            wchar_t name[25] = {};
            swprintf_s(name, L"Frame CommandAllocator %u", n);
            _commandAllocators[n]->SetName(name);
        }

        // Create a command list for recording graphics commands.
        DXCall(_d3dDevice->CreateCommandList(
            0,
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            _commandAllocators[0],
            nullptr,
            IID_PPV_ARGS(&_commandList)
        ));
        DXCall(_commandList->Close());
        _commandList->SetName(L"Primary Graphics Command List");

        _currentFrameIndex = _currentCPUFrame % NumCmdAllocators;
        DXCall(_commandAllocators[_currentFrameIndex]->Reset());
        DXCall(_commandList->Reset(_commandAllocators[_currentFrameIndex], nullptr));

        // Create fence
        _frameFence = CreateFence();

        return AGPU_OK;
    }

    void AGpuRendererD3D12::shutdown()
    {
        ALIMER_ASSERT(_currentCPUFrame == _currentGPUFrame);
        _shuttingDown = true;

        for (uint64_t i = 0; i < RenderLatency; ++i)
        {
            ProcessDeferredReleases(i);
        }

        DestroyFence(_frameFence);

        for (uint64_t i = 0; i < RenderLatency; ++i)
            Release(_commandAllocators[i]);

        Release(_commandList);
        Release(_graphicsQueue);

        Release(_d3dDevice);
        Release(factory);
        Release(_dxgiAdapter);

#ifdef _DEBUG
        {
            IDXGIDebug1* dxgiDebug;
            if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
            {
                dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
                Release(dxgiDebug);
            }
        }
#endif
    }

    void AGpuRendererD3D12::waitIdle()
    {

    }

    AgpuResult AGpuRendererD3D12::beginFrame()
    {
        // Transition the render target into the correct state to allow for drawing into it.
        //D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_backBufferIndex].Get(), beforeState, D3D12_RESOURCE_STATE_RENDER_TARGET);
        //s_renderD3D12->commandList->ResourceBarrier(1, &barrier);
        return AGPU_OK;
    }

    uint64_t AGpuRendererD3D12::endFrame()
    {
        // Transition the render target to the state that allows it to be presented to the display.
        //D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_backBufferIndex].Get(), beforeState, D3D12_RESOURCE_STATE_PRESENT);
        //commandList->ResourceBarrier(1, &barrier);

        // Send the command list off to the GPU for processing.
        DXCall(_commandList->Close());

        ID3D12CommandList* ppCommandLists[] = { _commandList };
        _graphicsQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

        ++_currentCPUFrame;

        // Signal the fence with the current frame number, so that we can check back on it
        FenceSignal(_frameFence, _graphicsQueue, _currentCPUFrame);

        // Wait for the GPU to catch up before we stomp an executing command buffer
        const uint64_t gpuLag = _currentCPUFrame - _currentGPUFrame;
        ALIMER_ASSERT(gpuLag <= RenderLatency);
        if (gpuLag >= RenderLatency)
        {
            // Make sure that the previous frame is finished
            FenceWait(_frameFence, _currentGPUFrame + 1);
            ++_currentGPUFrame;
        }

        _currentFrameIndex = _currentCPUFrame % NumCmdAllocators;

        // Prepare the command buffers to be used for the next frame
        DXCall(_commandAllocators[_currentFrameIndex]->Reset());
        DXCall(_commandList->Reset(_commandAllocators[_currentFrameIndex], nullptr));

        //EndFrame_Helpers();

        // See if we have any deferred releases to process
        ProcessDeferredReleases(_currentFrameIndex);
        return _currentCPUFrame;
    }

    void AGpuRendererD3D12::DeferredRelease_(IUnknown* resource, bool forceDeferred)
    {
        if (resource == nullptr)
            return;

        if ((_currentCPUFrame == _currentGPUFrame && forceDeferred == false)
            || _shuttingDown || _d3dDevice == nullptr)
        {
            // Free-for-all!
            resource->Release();
            return;
        }

        _deferredReleases[_currentFrameIndex].push_back(resource);
    }

    void AGpuRendererD3D12::ProcessDeferredReleases(uint64_t frameIndex)
    {

    }

    AgpuFence AGpuRendererD3D12::CreateFence()
    {
        AgpuFence fence = new AgpuFence_T();
        _d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence->d3d12Fence));

        fence->d3d12FenceEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
        ALIMER_ASSERT(fence->d3d12FenceEvent != INVALID_HANDLE_VALUE);

        return fence;
    }

    void AGpuRendererD3D12::DestroyFence(AgpuFence fence)
    {
        DeferredRelease(fence->d3d12Fence);
        delete fence;
        fence = nullptr;
    }

    void AGpuRendererD3D12::FenceSignal(AgpuFence fence, ID3D12CommandQueue* queue, uint64_t fenceValue)
    {
        ALIMER_ASSERT(fence->d3d12Fence != nullptr);
        DXCall(queue->Signal(fence->d3d12Fence, fenceValue));
    }

    void AGpuRendererD3D12::FenceWait(AgpuFence fence, uint64_t fenceValue)
    {
        ALIMER_ASSERT(fence->d3d12Fence != nullptr);
        if (fence->d3d12Fence->GetCompletedValue() < fenceValue)
        {
            DXCall(fence->d3d12Fence->SetEventOnCompletion(fenceValue, fence->d3d12FenceEvent));
            WaitForSingleObjectEx(fence->d3d12FenceEvent, INFINITE, FALSE);
        }
    }

    bool AGpuRendererD3D12::FenceSignaled(AgpuFence fence, uint64_t fenceValue)
    {
        ALIMER_ASSERT(fence->d3d12Fence != nullptr);
        return fence->d3d12Fence->GetCompletedValue() >= fenceValue;
    }

    void AGpuRendererD3D12::FenceClear(AgpuFence fence, uint64_t fenceValue)
    {
        ALIMER_ASSERT(fence->d3d12Fence != nullptr);
        DXCall(fence->d3d12Fence->Signal(fenceValue));
    }

    void agpuD3D12GetDXGIAdapter(IDXGIAdapter1** ppAdapter)
    {
        *ppAdapter = nullptr;

        HRESULT hr = S_OK;
        bool releaseFactory = false;
        if (factory == nullptr)
        {
            hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));
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
        hr = factory->QueryInterface(factory6.ReleaseAndGetAddressOf());
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
                if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
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
            if (FAILED(factory->EnumWarpAdapter(IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf()))))
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
            Release(factory);
        }
    }

    AgpuBool32 isSupported()
    {
        static AgpuBool32 availableCheck = AGPU_FALSE;
        static AgpuBool32 isAvailable = AGPU_FALSE;

        if (availableCheck)
            return isAvailable;

        availableCheck = AGPU_TRUE;
#if AGPU_D3D12_DYNAMIC_LIB
        // Load libraries first.
        s_dxgiLib = LoadLibraryW(L"dxgi.dll");
        if (!s_dxgiLib)
        {
            OutputDebugStringW(L"Failed to load dxgi.dll");
            availableCheck = AGPU_FALSE;
            return availableCheck;
        }

        s_d3d12Lib = LoadLibraryW(L"d3d12.dll");
        if (!s_d3d12Lib)
        {
            OutputDebugStringW(L"Failed to load d3d12.dll");
            availableCheck = AGPU_FALSE;
            return availableCheck;
        }

        /* DXGI entry points */
        CreateDXGIFactory2 = (PFN_CREATE_DXGI_FACTORY2)GetProcAddress(s_dxgiLib, "CreateDXGIFactory2");
        DXGIGetDebugInterface1 = (PFN_GET_DXGI_DEBUG_INTERFACE1)GetProcAddress(s_dxgiLib, "DXGIGetDebugInterface1");
        if (CreateDXGIFactory2 == nullptr)
        {
            OutputDebugStringW(L"Cannot find CreateDXGIFactory2 entry point.");
            availableCheck = AGPU_FALSE;
            return availableCheck;
        }

        /* D3D12 entry points */
        D3D12GetDebugInterface = (PFN_D3D12_GET_DEBUG_INTERFACE)GetProcAddress(s_d3d12Lib, "D3D12GetDebugInterface");
        D3D12CreateDevice = (PFN_D3D12_CREATE_DEVICE)GetProcAddress(s_d3d12Lib, "D3D12CreateDevice");
        D3D12SerializeRootSignature = (PFN_D3D12_SERIALIZE_ROOT_SIGNATURE)GetProcAddress(s_d3d12Lib, "D3D12SerializeRootSignature");
        if (!D3D12CreateDevice)
        {
            OutputDebugStringW(L"Cannot find D3D12CreateDevice entry point.");
            availableCheck = AGPU_FALSE;
            return availableCheck;
        }

        if (!D3D12SerializeRootSignature)
        {
            OutputDebugStringW(L"Cannot find D3D12SerializeRootSignature entry point.");
            availableCheck = AGPU_FALSE;
            return availableCheck;
        }
#endif

        IDXGIAdapter1* adapter;
        agpuD3D12GetDXGIAdapter(&adapter);
        isAvailable = adapter != nullptr;
        Release(adapter);
        return isAvailable;
    }

    AgpuResult createBackend(const AgpuDescriptor* descriptor, AGpuRendererI** pRenderer)
    {
#if defined(_DEBUG)
        // Enable the debug layer (requires the Graphics Tools "optional feature").
        if (descriptor->validation)
        {
            ID3D12Debug* d3d12debug;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&d3d12debug))))
            {
                d3d12debug->EnableDebugLayer();

                ID3D12Debug1* d3d12debug1;
                if (SUCCEEDED(d3d12debug->QueryInterface(&d3d12debug1)))
                {
                    d3d12debug1->SetEnableGPUBasedValidation(true);
                    Release(d3d12debug1);
                }

                Release(d3d12debug);
            }
            else
            {
                ALIMER_LOGWARN("Direct3D Debug Device is not available");
            }

            IDXGIInfoQueue* dxgiInfoQueue;
            if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiInfoQueue))))
            {
                factoryFlags = DXGI_CREATE_FACTORY_DEBUG;

                dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
                dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);

                Release(dxgiInfoQueue);
            }
        }
#endif

        // Create global dxgi factory.
        HRESULT hr = CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&factory));
        if (FAILED(hr))
        {
            ALIMER_LOGERROR("Unable to create a DXGI 1.4 device. Make sure that your OS and driver support DirectX 12");
            return AGPU_ERROR;
        }

        AGpuRendererD3D12* renderD3D12 = new AGpuRendererD3D12();
        AgpuResult result = renderD3D12->initialize(descriptor);
        *pRenderer = renderD3D12;
        return result;
    }
}

AgpuBool32 agpuIsD3D12Supported()
{
    return d3d12::isSupported();
}

AgpuResult agpuCreateD3D12Backend(const AgpuDescriptor* descriptor, AGpuRendererI** pRenderer)
{
    return d3d12::createBackend(descriptor, pRenderer);
}

#endif /* AGPU_D3D12 */
