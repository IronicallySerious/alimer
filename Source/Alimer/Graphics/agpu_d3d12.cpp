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

#if (defined(AGPU_D3D11) || defined(AGPU_D3D12)) && defined(_DEBUG)
#   include <dxgidebug.h>
#endif

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

#define ArraySize_(x) ((sizeof(x) / sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

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

    DWORD           _dxgiFactoryFlags = 0;
    IDXGIFactory4*  _dxgiFactory = nullptr;

    void GetDXGIAdapter(IDXGIAdapter1** ppAdapter)
    {
        *ppAdapter = nullptr;

        HRESULT hr = S_OK;
        bool releaseFactory = false;
        if (_dxgiFactory == nullptr)
        {
            hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&_dxgiFactory));
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
        hr = _dxgiFactory->QueryInterface(factory6.ReleaseAndGetAddressOf());
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
                DXGI_ERROR_NOT_FOUND != _dxgiFactory->EnumAdapters1(
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
            if (FAILED(_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf()))))
            {
                throw std::exception("WARP12 not available. Enable the 'Graphics Tools' optional feature");
            }

            OutputDebugStringA("Direct3D Adapter - WARP12\n");
        }
#endif

        if (!adapter)
        {
            ALIMER_ASSERT_MSG(false, "No Direct3D 12 device found");
        }

        *ppAdapter = adapter.Detach();

        if (releaseFactory)
        {
            Release(_dxgiFactory);
        }
    }

    // Resource Barriers
    void TransitionResource(ID3D12GraphicsCommandList* cmdList,
        ID3D12Resource* resource,
        D3D12_RESOURCE_STATES before,
        D3D12_RESOURCE_STATES after,
        uint32_t subResource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
    {
        D3D12_RESOURCE_BARRIER barrier = { };
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = resource;
        barrier.Transition.StateBefore = before;
        barrier.Transition.StateAfter = after;
        barrier.Transition.Subresource = subResource;
        cmdList->ResourceBarrier(1, &barrier);
    }

    DXGI_FORMAT agpuD3D12ConvertPixelFormat(AgpuPixelFormat format)
    {
        switch (format)
        {
        case AGPU_PIXEL_FORMAT_UNKNOWN:
            return DXGI_FORMAT_UNKNOWN;

        case AGPU_PIXEL_FORMAT_R8_UNORM:
            return DXGI_FORMAT_R8_UNORM;

        case AGPU_PIXEL_FORMAT_R8_SNORM:
            return DXGI_FORMAT_R8_SNORM;

        case AGPU_PIXEL_FORMAT_R16_UNORM:
            return DXGI_FORMAT_R16_UNORM;

        case AGPU_PIXEL_FORMAT_R16_SNORM:
            return DXGI_FORMAT_R16_SNORM;

        case AGPU_PIXEL_FORMAT_RG8_UNORM:
            return DXGI_FORMAT_R8G8_UNORM;

        case AGPU_PIXEL_FORMAT_RG8_SNORM:
            return DXGI_FORMAT_R8G8_SNORM;

        case AGPU_PIXEL_FORMAT_RG16_UNORM:
            return DXGI_FORMAT_R16G16_UNORM;

        case AGPU_PIXEL_FORMAT_RG16_SNORM:
            return DXGI_FORMAT_R16G16_SNORM;

        case AGPU_PIXEL_FORMAT_RGB16_UNORM:
            return DXGI_FORMAT_UNKNOWN;

        case AGPU_PIXEL_FORMAT_RGB16_SNORM:
            return DXGI_FORMAT_UNKNOWN;

        case AGPU_PIXEL_FORMAT_RGBA8_UNORM:
            return DXGI_FORMAT_R8G8B8A8_UNORM;

        case AGPU_PIXEL_FORMAT_RGBA8_UNORM_SRGB:
            return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

        case AGPU_PIXEL_FORMAT_RGBA8_SNORM:
            return DXGI_FORMAT_R8G8B8A8_SNORM;

        case AGPU_PIXEL_FORMAT_BGRA8_UNORM:
            return DXGI_FORMAT_B8G8R8A8_UNORM;

        case AGPU_PIXEL_FORMAT_BGRA8_UNORM_SRGB:
            return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

        case AGPU_PIXEL_FORMAT_D32_FLOAT:
            return DXGI_FORMAT_D32_FLOAT;

        case AGPU_PIXEL_FORMAT_D16_UNORM:
            return DXGI_FORMAT_D16_UNORM;

        case AGPU_PIXEL_FORMAT_D24_UNORM_S8:
            return DXGI_FORMAT_D24_UNORM_S8_UINT;

        case AGPU_PIXEL_FORMAT_D32_FLOAT_S8:
            return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

        case AGPU_PIXEL_FORMAT_BC1_UNORM:
            return DXGI_FORMAT_BC1_UNORM;

        case AGPU_PIXEL_FORMAT_BC1_UNORM_SRGB:
            return DXGI_FORMAT_BC1_UNORM_SRGB;

        case AGPU_PIXEL_FORMAT_BC2_UNORM:
            return DXGI_FORMAT_BC2_UNORM;

        case AGPU_PIXEL_FORMAT_BC2_UNORM_SRGB:
            return DXGI_FORMAT_BC2_UNORM_SRGB;

        case AGPU_PIXEL_FORMAT_BC3_UNORM:
            return DXGI_FORMAT_BC3_UNORM;

        case AGPU_PIXEL_FORMAT_BC3_UNORM_SRGB:
            return DXGI_FORMAT_BC3_UNORM_SRGB;

        case AGPU_PIXEL_FORMAT_BC4_UNORM:
            return DXGI_FORMAT_BC4_UNORM;

        case AGPU_PIXEL_FORMAT_BC4_SNORM:
            return DXGI_FORMAT_BC4_SNORM;

        case AGPU_PIXEL_FORMAT_BC5_UNORM:
            return DXGI_FORMAT_BC5_UNORM;

        case AGPU_PIXEL_FORMAT_BC5_SNORM:
            return DXGI_FORMAT_BC5_SNORM;

        case AGPU_PIXEL_FORMAT_BC6HS16:
            return DXGI_FORMAT_BC6H_SF16;

        case AGPU_PIXEL_FORMAT_BC6HU16:
            return DXGI_FORMAT_BC6H_UF16;

        case AGPU_PIXEL_FORMAT_BC7_UNORM:
            return DXGI_FORMAT_BC7_UNORM;

        case AGPU_PIXEL_FORMAT_BC7_UNORM_SRGB:
            return DXGI_FORMAT_BC7_UNORM_SRGB;
        default:
            ALIMER_UNREACHABLE();
        }
    }

    struct AgpuFence
    {
        ID3D12Fence*        Fence;
        HANDLE              Event;
    };

    struct PersistentDescriptorAlloc
    {
        D3D12_CPU_DESCRIPTOR_HANDLE Handles[RenderLatency] = { };
        uint32_t Index = static_cast<uint32_t>(-1);
    };

    struct TempDescriptorAlloc
    {
        D3D12_CPU_DESCRIPTOR_HANDLE StartCPUHandle = { };
        D3D12_GPU_DESCRIPTOR_HANDLE StartGPUHandle = { };
        uint32_t StartIndex = static_cast<uint32_t>(-1);
    };

    class DescriptorHeap
    {
    public:
        ~DescriptorHeap();

        void Initialize(ID3D12Device* device, uint32_t numPersistent, uint32_t numTemporary, D3D12_DESCRIPTOR_HEAP_TYPE heapType, bool shaderVisible);
        void Shutdown();
        void EndFrame();

        PersistentDescriptorAlloc AllocatePersistent();
        void FreePersistent(uint32_t& index);
        void FreePersistent(D3D12_CPU_DESCRIPTOR_HANDLE& handle);
        void FreePersistent(D3D12_GPU_DESCRIPTOR_HANDLE& handle);

        D3D12_CPU_DESCRIPTOR_HANDLE CPUHandleFromIndex(uint32_t descriptorIndex) const;
        D3D12_GPU_DESCRIPTOR_HANDLE GPUHandleFromIndex(uint32_t descriptorIndex) const;

        D3D12_CPU_DESCRIPTOR_HANDLE CPUHandleFromIndex(uint32_t descriptorIndex, uint64_t heapIndex) const;
        D3D12_GPU_DESCRIPTOR_HANDLE GPUHandleFromIndex(uint32_t descriptorIndex, uint64_t heapIndex) const;

        uint32_t IndexFromHandle(D3D12_CPU_DESCRIPTOR_HANDLE handle);
        uint32_t IndexFromHandle(D3D12_GPU_DESCRIPTOR_HANDLE handle);

        ID3D12DescriptorHeap* CurrentHeap() const;
        uint32_t TotalNumDescriptors() const { return NumPersistent + NumTemporary; }

    private:
        uint32_t _heapIndex = 0;
        uint32_t _heapCount = 0;
        ID3D12DescriptorHeap* _heaps[RenderLatency] = { };
        uint32_t NumPersistent = 0;
        uint32_t PersistentAllocated = 0;
        std::vector<uint32_t> DeadList;
        uint32_t NumTemporary = 0;
        volatile int64_t TemporaryAllocated = 0;
        uint32_t DescriptorSize = 0;
        AgpuBool32 ShaderVisible = false;
        D3D12_DESCRIPTOR_HEAP_TYPE HeapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        D3D12_CPU_DESCRIPTOR_HANDLE CPUStart[RenderLatency] = { };
        D3D12_GPU_DESCRIPTOR_HANDLE GPUStart[RenderLatency] = { };
        SRWLOCK Lock = SRWLOCK_INIT;
    };

    struct AGpuRendererD3D12 : public AGpuRendererI
    {
        AGpuRendererD3D12()
        {
        }

        ~AGpuRendererD3D12()
        {
        }

        AgpuResult Initialize(const AgpuDescriptor* descriptor);
        void Shutdown() override;
        void WaitIdle();
        void BeginFrame();
        uint64_t Frame() override;

        /* Fence */
        AgpuFence* CreateFence(uint64_t initialValue);
        void DestroyFence(AgpuFence* fence);
        void SignalFence(AgpuFence* fence, ID3D12CommandQueue* queue, uint64_t fenceValue);
        void WaitFence(AgpuFence* fence, uint64_t fenceValue);
        bool IsFenceSignaled(AgpuFence* fence, uint64_t fenceValue);
        void ClearFence(AgpuFence* fence, uint64_t fenceValue);

        /* Swapchain */
        AgpuSwapchain CreateSwapchain(const AgpuSwapchainDescriptor* descriptor);
        void DestroySwapchain(AgpuSwapchain swapchain);

        /* Texture */
        AgpuTexture CreateTexture(const AgpuTextureDescriptor* descriptor, void* externalHandle) override;
        void DestroyTexture(AgpuTexture texture) override;

        /* Framebuffer */
        AgpuFramebuffer CreateFramebuffer(const AgpuFramebufferDescriptor* descriptor) override;
        void DestroyFramebuffer(AgpuFramebuffer framebuffer) override;

        /* CommandList */
        void BeginRenderPass(AgpuFramebuffer framebuffer);
        void EndRenderPass();

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
        AgpuFence*                  _frameFence = nullptr;
        uint64_t                    _fenceValues[AGPU_MAX_BACK_BUFFER_COUNT] = {};
        ID3D12CommandQueue*         _graphicsQueue = nullptr;
        ID3D12GraphicsCommandList*  _commandList = nullptr;
        ID3D12CommandAllocator*     _commandAllocators[AGPU_MAX_BACK_BUFFER_COUNT];
        uint64_t                    _currentFrameIndex = 0;
        bool                        _shuttingDown = false;
        std::vector<IUnknown*>      _deferredReleases[RenderLatency];
        bool                        _headless = false;
        AgpuSwapchain               _mainSwapchain = nullptr;

        DescriptorHeap              _RTVDescriptorHeap;

        AgpuFramebuffer             _currentFramebuffer = nullptr;
    };

    /* DescriptorHeap */
    DescriptorHeap::~DescriptorHeap()
    {
        ALIMER_ASSERT(_heaps[0] == nullptr);
    }

    void DescriptorHeap::Initialize(ID3D12Device* device, uint32_t numPersistent, uint32_t numTemporary, D3D12_DESCRIPTOR_HEAP_TYPE heapType, bool shaderVisible)
    {
        Shutdown();

        uint32_t totalNumDescriptors = numPersistent + numTemporary;
        ALIMER_ASSERT(totalNumDescriptors > 0);

        NumPersistent = numPersistent;
        NumTemporary = numTemporary;
        HeapType = heapType;
        ShaderVisible = shaderVisible;
        if (heapType == D3D12_DESCRIPTOR_HEAP_TYPE_RTV || heapType == D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
            ShaderVisible = false;

        _heapCount = ShaderVisible ? 2 : 1;

        DeadList.resize(numPersistent);
        for (uint32_t i = 0u; i < numPersistent; ++i)
        {
            DeadList[i] = i;
        }

        D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
        heapDesc.Type = heapType;
        heapDesc.NumDescriptors = totalNumDescriptors;
        heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        heapDesc.NodeMask = 0;
        if (ShaderVisible)
            heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

        for (uint32_t i = 0; i < _heapCount; ++i)
        {
            DXCall(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&_heaps[i])));
            CPUStart[i] = _heaps[i]->GetCPUDescriptorHandleForHeapStart();
            if (ShaderVisible)
            {
                GPUStart[i] = _heaps[i]->GetGPUDescriptorHandleForHeapStart();
            }
        }

        DescriptorSize = device->GetDescriptorHandleIncrementSize(heapType);
    }

    void DescriptorHeap::Shutdown()
    {
        ALIMER_ASSERT(PersistentAllocated == 0);
        for (uint64_t i = 0; i < ArraySize_(_heaps); ++i)
        {
            Release(_heaps[i]);
        }
    }

    void DescriptorHeap::EndFrame()
    {
        ALIMER_ASSERT(_heaps[0] != nullptr);
        TemporaryAllocated = 0;
        _heapIndex = (_heapIndex + 1) % _heapCount;
    }

    PersistentDescriptorAlloc DescriptorHeap::AllocatePersistent()
    {
        ALIMER_ASSERT(_heaps[0] != nullptr);

        AcquireSRWLockExclusive(&Lock);

        ALIMER_ASSERT(PersistentAllocated < NumPersistent);
        uint32_t index = DeadList[PersistentAllocated];
        ++PersistentAllocated;

        ReleaseSRWLockExclusive(&Lock);

        PersistentDescriptorAlloc alloc;
        alloc.Index = index;
        for (uint32_t i = 0; i < _heapCount; ++i)
        {
            alloc.Handles[i] = CPUStart[i];
            alloc.Handles[i].ptr += index * DescriptorSize;
        }

        return alloc;
    }

    void DescriptorHeap::FreePersistent(uint32_t& index)
    {
        if (index == static_cast<uint32_t>(-1))
            return;

        ALIMER_ASSERT(index < NumPersistent);
        ALIMER_ASSERT(_heaps[0] != nullptr);

        AcquireSRWLockExclusive(&Lock);

        ALIMER_ASSERT(PersistentAllocated > 0);
        DeadList[PersistentAllocated - 1] = index;
        --PersistentAllocated;

        ReleaseSRWLockExclusive(&Lock);

        index = static_cast<uint32_t>(-1);
    }

    void DescriptorHeap::FreePersistent(D3D12_CPU_DESCRIPTOR_HANDLE& handle)
    {
        ALIMER_ASSERT(_heapCount == 1);
        if (handle.ptr != 0)
        {
            uint32_t index = IndexFromHandle(handle);
            FreePersistent(index);
            handle = { };
        }
    }

    void DescriptorHeap::FreePersistent(D3D12_GPU_DESCRIPTOR_HANDLE& handle)
    {
        ALIMER_ASSERT(_heapCount == 1);
        if (handle.ptr != 0)
        {
            uint32_t index = IndexFromHandle(handle);
            FreePersistent(index);
            handle = { };
        }
    }

    D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::CPUHandleFromIndex(uint32_t descriptorIndex) const
    {
        return CPUHandleFromIndex(descriptorIndex, _heapIndex);
    }

    D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GPUHandleFromIndex(uint32_t descriptorIndex) const
    {
        return GPUHandleFromIndex(descriptorIndex, _heapIndex);
    }

    D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::CPUHandleFromIndex(uint32_t descriptorIndex, uint64_t heapIndex) const
    {
        ALIMER_ASSERT(_heaps[0] != nullptr);
        ALIMER_ASSERT(heapIndex < _heapCount);
        ALIMER_ASSERT(descriptorIndex < TotalNumDescriptors());
        D3D12_CPU_DESCRIPTOR_HANDLE handle = CPUStart[heapIndex];
        handle.ptr += descriptorIndex * DescriptorSize;
        return handle;
    }

    D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GPUHandleFromIndex(uint32_t descriptorIndex, uint64_t heapIndex) const
    {
        ALIMER_ASSERT(_heaps[0] != nullptr);
        ALIMER_ASSERT(heapIndex < _heapCount);
        ALIMER_ASSERT(descriptorIndex < TotalNumDescriptors());
        ALIMER_ASSERT(ShaderVisible);
        D3D12_GPU_DESCRIPTOR_HANDLE handle = GPUStart[heapIndex];
        handle.ptr += descriptorIndex * DescriptorSize;
        return handle;
    }

    uint32_t DescriptorHeap::IndexFromHandle(D3D12_CPU_DESCRIPTOR_HANDLE handle)
    {
        ALIMER_ASSERT(_heaps[0] != nullptr);
        ALIMER_ASSERT(handle.ptr >= CPUStart[_heapIndex].ptr);
        ALIMER_ASSERT(handle.ptr < CPUStart[_heapIndex].ptr + DescriptorSize * TotalNumDescriptors());
        ALIMER_ASSERT((handle.ptr - CPUStart[_heapIndex].ptr) % DescriptorSize == 0);
        return uint32_t(handle.ptr - CPUStart[_heapIndex].ptr) / DescriptorSize;
    }

    uint32_t DescriptorHeap::IndexFromHandle(D3D12_GPU_DESCRIPTOR_HANDLE handle)
    {
        ALIMER_ASSERT(_heaps[0] != nullptr);
        ALIMER_ASSERT(handle.ptr >= GPUStart[_heapIndex].ptr);
        ALIMER_ASSERT(handle.ptr < GPUStart[_heapIndex].ptr + DescriptorSize * TotalNumDescriptors());
        ALIMER_ASSERT((handle.ptr - GPUStart[_heapIndex].ptr) % DescriptorSize == 0);
        return uint32_t(handle.ptr - GPUStart[_heapIndex].ptr) / DescriptorSize;
    }

    ID3D12DescriptorHeap* DescriptorHeap::CurrentHeap() const
    {
        ALIMER_ASSERT(_heaps[0] != nullptr);
        return _heaps[_heapIndex];
    }

    /* AGpuRendererD3D12 */
    AgpuResult AGpuRendererD3D12::Initialize(const AgpuDescriptor* descriptor)
    {
        _shuttingDown = false;
        GetDXGIAdapter(&_dxgiAdapter);

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

        // Create fence
        _frameFence = CreateFence(0);
        _fenceValues[_currentFrameIndex]++;
        _currentFrameIndex = 0;

        // Initialize helpers
        _RTVDescriptorHeap.Initialize(_d3dDevice, 256, 0, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, false);

        WaitIdle();
        for (UINT n = 0; n < NumCmdAllocators; n++)
        {
            _fenceValues[n] = _fenceValues[_currentFrameIndex];
        }

        // Create main swap chain if not headless and valid width and height.
        _headless = descriptor->headless;
        if (!_headless
            && descriptor->swapchain.width > 0
            && descriptor->swapchain.height > 0)
        {
            _mainSwapchain = CreateSwapchain(&descriptor->swapchain);
        }

        BeginFrame();

        return AGPU_OK;
    }

    void AGpuRendererD3D12::Shutdown()
    {
        WaitIdle();
        _shuttingDown = true;

        if (_mainSwapchain)
        {
            DestroySwapchain(_mainSwapchain);
        }

        for (uint64_t i = 0; i < RenderLatency; ++i)
        {
            ProcessDeferredReleases(i);
        }

        DestroyFence(_frameFence);

        for (uint64_t i = 0; i < RenderLatency; ++i)
            Release(_commandAllocators[i]);

        Release(_commandList);
        Release(_graphicsQueue);


        _RTVDescriptorHeap.Shutdown();
        //_SRVDescriptorHeap.Shutdown();
        //_DSVDescriptorHeap.Shutdown();
        //_UAVDescriptorHeap.Shutdown();

        Release(_dxgiFactory);
        Release(_dxgiAdapter);
        Release(_d3dDevice);

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

    void AGpuRendererD3D12::WaitIdle()
    {
        // Schedule a Signal command in the queue.
        const uint64_t fenceValue = _fenceValues[_currentFrameIndex];
        SignalFence(_frameFence, _graphicsQueue, fenceValue);

        // Wait until the fence has been processed.
        DXCall(_frameFence->Fence->SetEventOnCompletion(fenceValue, _frameFence->Event));
        WaitForSingleObjectEx(_frameFence->Event, INFINITE, FALSE);

        // Increment the fence value for the current frame.
        _fenceValues[_currentFrameIndex]++;
    }

    void AGpuRendererD3D12::BeginFrame()
    {
        // Prepare the command buffers to be used for the next frame
        DXCall(_commandAllocators[_currentFrameIndex]->Reset());
        DXCall(_commandList->Reset(_commandAllocators[_currentFrameIndex], nullptr));

        if (!_headless)
        {
            _mainSwapchain->backBufferIndex = _mainSwapchain->d3d12SwapChain->GetCurrentBackBufferIndex();

            BeginRenderPass(_mainSwapchain->backBufferFramebuffers[_mainSwapchain->backBufferIndex]);
        }
    }

    uint64_t AGpuRendererD3D12::Frame()
    {
        if (!_headless)
        {
            EndRenderPass();
        }

        // Send the command list off to the GPU for processing.
        DXCall(_commandList->Close());

        ID3D12CommandList* ppCommandLists[] = { _commandList };
        _graphicsQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

        // Present the frame.
        if (_mainSwapchain)
        {
            const bool vsync = true;
            uint32_t syncIntervals = vsync ? 1 : 0;
            DXCall(_mainSwapchain->d3d12SwapChain->Present(syncIntervals, syncIntervals == 0 ? DXGI_PRESENT_ALLOW_TEARING : 0));
        }

        // Prepare to render the next frame.
        // Schedule a Signal command in the queue.
        const uint64_t currentFenceValue = _fenceValues[_currentFrameIndex];
        SignalFence(_frameFence, _graphicsQueue, currentFenceValue);

        // Update frame index.
        _currentFrameIndex = (_currentFrameIndex + 1) % NumCmdAllocators;

        // If the next frame is not ready to be rendered yet, wait until it is ready.
        WaitFence(_frameFence, _fenceValues[_currentFrameIndex]);

        // Set the fence value for the next frame.
        _fenceValues[_currentFrameIndex] = currentFenceValue + 1;

        _RTVDescriptorHeap.EndFrame();
        //EndFrame_Helpers();

        // See if we have any deferred releases to process
        ProcessDeferredReleases(_currentFrameIndex);

        // Begin new frame.
        BeginFrame();

        // Return fence value.
        return _fenceValues[_currentFrameIndex];
    }

    void AGpuRendererD3D12::DeferredRelease_(IUnknown* resource, bool forceDeferred)
    {
        if (resource == nullptr)
            return;

        if ((forceDeferred == false)
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

    AgpuFence* AGpuRendererD3D12::CreateFence(uint64_t initialValue)
    {
        AgpuFence* fence = new AgpuFence();
        _d3dDevice->CreateFence(initialValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence->Fence));

        fence->Event = CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
        ALIMER_ASSERT(fence->Event != INVALID_HANDLE_VALUE);

        return fence;
    }

    void AGpuRendererD3D12::DestroyFence(AgpuFence* fence)
    {
        DeferredRelease(fence->Fence);
        delete fence;
        fence = nullptr;
    }

    void AGpuRendererD3D12::SignalFence(AgpuFence* fence, ID3D12CommandQueue* queue, uint64_t fenceValue)
    {
        ALIMER_ASSERT(fence->Fence != nullptr);
        DXCall(queue->Signal(fence->Fence, fenceValue));
    }

    void AGpuRendererD3D12::WaitFence(AgpuFence* fence, uint64_t fenceValue)
    {
        ALIMER_ASSERT(fence->Fence != nullptr);
        if (fence->Fence->GetCompletedValue() < fenceValue)
        {
            DXCall(fence->Fence->SetEventOnCompletion(fenceValue, fence->Event));
            WaitForSingleObjectEx(fence->Event, INFINITE, FALSE);
        }
    }

    bool AGpuRendererD3D12::IsFenceSignaled(AgpuFence* fence, uint64_t fenceValue)
    {
        ALIMER_ASSERT(fence->Fence != nullptr);
        return fence->Fence->GetCompletedValue() >= fenceValue;
    }

    void AGpuRendererD3D12::ClearFence(AgpuFence* fence, uint64_t fenceValue)
    {
        ALIMER_ASSERT(fence->Fence != nullptr);
        DXCall(fence->Fence->Signal(fenceValue));
    }

    AgpuSwapchain AGpuRendererD3D12::CreateSwapchain(const AgpuSwapchainDescriptor* descriptor)
    {
        // Create a descriptor for the swap chain.
        AgpuPixelFormat backBufferFormat = AGPU_PIXEL_FORMAT_BGRA8_UNORM;
        DXGI_FORMAT dxgiBackBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
        uint32_t backbufferCount = 2;
        if (descriptor->preferredColorFormat != AGPU_PIXEL_FORMAT_UNKNOWN)
        {
            switch (descriptor->preferredColorFormat)
            {
            case AGPU_PIXEL_FORMAT_BGRA8_UNORM_SRGB:
                backBufferFormat = AGPU_PIXEL_FORMAT_BGRA8_UNORM_SRGB;
                dxgiBackBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
                break;

            default:
                backBufferFormat = AGPU_PIXEL_FORMAT_BGRA8_UNORM;
                dxgiBackBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
                break;
            }
        }

        if (descriptor->bufferCount != 0)
        {
            backbufferCount = descriptor->bufferCount;
            if (backbufferCount > 3)
                backbufferCount = 3;
        }

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Width = descriptor->width;
        swapChainDesc.Height = descriptor->height;
        swapChainDesc.Format = dxgiBackBufferFormat;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = backbufferCount;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
        swapChainDesc.Flags =
            DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH |
            DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

        /* TODO: DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT*/

        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
        fsSwapChainDesc.Windowed = TRUE;

        // Create a swap chain for the window.
        IDXGISwapChain1* swapChain;
        DXCall(_dxgiFactory->CreateSwapChainForHwnd(
            _graphicsQueue,
            static_cast<HWND>(descriptor->windowHandle),
            &swapChainDesc,
            &fsSwapChainDesc,
            nullptr,
            &swapChain
        ));

        IDXGISwapChain3* swapChain3;
        DXCall(swapChain->QueryInterface(&swapChain3));
        swapChain->Release();

        // Create engine instance and assign.
        AgpuSwapchain swapchain = new AgpuSwapchain_T();
        swapchain->backBufferIndex = swapChain3->GetCurrentBackBufferIndex();
        swapchain->backbufferCount = backbufferCount;
        swapchain->backBufferFormat = backBufferFormat;
        swapchain->dxgiBackBufferFormat = dxgiBackBufferFormat;
        swapchain->d3d12SwapChain = swapChain3;

        AgpuTextureDescriptor textureDescriptor = {};
        textureDescriptor.type = AGPU_TEXTURE_TYPE_2D;
        textureDescriptor.width = descriptor->width;
        textureDescriptor.height = descriptor->height;
        textureDescriptor.depthOrArraySize = 1;
        textureDescriptor.mipLevels = 1;
        textureDescriptor.format = backBufferFormat;
        textureDescriptor.usage = AgpuTextureUsage(AGPU_TEXTURE_USAGE_OUTPUT_ATTACHMENT | AGPU_TEXTURE_USAGE_PRESENT);
        textureDescriptor.samples = AGPU_SAMPLE_COUNT1;

        AgpuFramebufferDescriptor fboDescriptor = {};

        for (uint32_t i = 0; i < swapchain->backbufferCount; i++)
        {
            ID3D12Resource* resource;
            DXCall(swapChain3->GetBuffer(i, IID_PPV_ARGS(&resource)));

            wchar_t name[25] = {};
            swprintf_s(name, L"Back Buffer %u", i);
            resource->SetName(name);

            // Create external texture.
            swapchain->backBufferTexture[i] = CreateTexture(&textureDescriptor, resource);

            // Create framebuffer.
            fboDescriptor.colorAttachments[0].texture = swapchain->backBufferTexture[i];
            swapchain->backBufferFramebuffers[i] = CreateFramebuffer(&fboDescriptor);
        }

        return swapchain;
    }

    void AGpuRendererD3D12::DestroySwapchain(AgpuSwapchain swapchain)
    {
        for (uint32_t i = 0; i < swapchain->backbufferCount; ++i)
        {
            DestroyTexture(swapchain->backBufferTexture[i]);
            DestroyFramebuffer(swapchain->backBufferFramebuffers[i]);
        }

        DeferredRelease(swapchain->d3d12SwapChain);
        delete swapchain;
        swapchain = nullptr;
    }

    AgpuTexture AGpuRendererD3D12::CreateTexture(const AgpuTextureDescriptor* descriptor, void* externalHandle)
    {
        AgpuTexture texture = new AgpuTexture_T();

        texture->dxgiFormat = agpuD3D12ConvertPixelFormat(descriptor->format);
        if (externalHandle == nullptr)
        {

        }
        else
        {
            texture->d3d12Resource = static_cast<ID3D12Resource*>(externalHandle);
        }

        if (descriptor->usage & AGPU_TEXTURE_USAGE_PRESENT)
        {
            texture->d3d12ResourceState = D3D12_RESOURCE_STATE_PRESENT;
        }
        else
        {
            if (descriptor->usage & AGPU_TEXTURE_USAGE_TRANSFER_SRC)
            {
                texture->d3d12ResourceState |= D3D12_RESOURCE_STATE_COPY_SOURCE;
            }

            if (descriptor->usage & AGPU_TEXTURE_USAGE_TRANSFER_DEST)
            {
                texture->d3d12ResourceState |= D3D12_RESOURCE_STATE_COPY_DEST;
            }

            if (descriptor->usage & AGPU_TEXTURE_USAGE_SAMPLED)
            {
                texture->d3d12ResourceState |= (D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
                    D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            }

            if (descriptor->usage & AGPU_TEXTURE_USAGE_STORAGE)
            {
                texture->d3d12ResourceState |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            }
            if (descriptor->usage & AGPU_TEXTURE_USAGE_OUTPUT_ATTACHMENT)
            {
                if (agpuIsDepthFormat(descriptor->format) || agpuIsStencilFormat(descriptor->format))
                {
                    texture->d3d12ResourceState |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
                }
                else
                {
                    texture->d3d12ResourceState |= D3D12_RESOURCE_STATE_RENDER_TARGET;
                }
            }
        }

        return texture;
    }

    void AGpuRendererD3D12::DestroyTexture(AgpuTexture texture)
    {
        DeferredRelease(texture->d3d12Resource);
        delete texture;
        texture = nullptr;
    }

    /* Framebuffer */
    AgpuFramebuffer AGpuRendererD3D12::CreateFramebuffer(const AgpuFramebufferDescriptor* descriptor)
    {
        AgpuFramebuffer framebuffer = new AgpuFramebuffer_T();
        framebuffer->numRTVs = 0;
        for (int i = 0; i < AGPU_MAX_COLOR_ATTACHMENTS; i++)
        {
            if (descriptor->colorAttachments[i].texture == nullptr)
                continue;

            framebuffer->colorAttachments[framebuffer->numRTVs] = descriptor->colorAttachments[i];
            framebuffer->d3d12RTVs[framebuffer->numRTVs] = _RTVDescriptorHeap.AllocatePersistent().Handles[0];

            D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = { };
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
            rtvDesc.Format = descriptor->colorAttachments[i].texture->dxgiFormat;
            rtvDesc.Texture2D.MipSlice = descriptor->colorAttachments[i].mipLevel;
            rtvDesc.Texture2D.PlaneSlice = 0;

            _d3dDevice->CreateRenderTargetView(
                descriptor->colorAttachments[i].texture->d3d12Resource,
                nullptr,
                framebuffer->d3d12RTVs[framebuffer->numRTVs]
            );

            framebuffer->numRTVs++;
        }

        return framebuffer;
    }

    void AGpuRendererD3D12::DestroyFramebuffer(AgpuFramebuffer framebuffer)
    {
        for (uint32_t i = 0; i < framebuffer->numRTVs; i++)
        {
            _RTVDescriptorHeap.FreePersistent(framebuffer->d3d12RTVs[i]);
        }

        delete framebuffer;
        framebuffer = nullptr;
    }

    void AGpuRendererD3D12::BeginRenderPass(AgpuFramebuffer framebuffer)
    {
        _currentFramebuffer = framebuffer;

        for (uint32_t i = 0; i < framebuffer->numRTVs; ++i)
        {
            // Indicate that the resource will be used as a render target.
            TransitionResource(_commandList,
                framebuffer->colorAttachments[i].texture->d3d12Resource,
                framebuffer->colorAttachments[i].texture->d3d12ResourceState,
                D3D12_RESOURCE_STATE_RENDER_TARGET);

            const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
            _commandList->ClearRenderTargetView(framebuffer->d3d12RTVs[i], clearColor, 0, nullptr);
        }

        if (framebuffer->d3d12DSV.ptr)
        {
            _commandList->OMSetRenderTargets(framebuffer->numRTVs, framebuffer->d3d12RTVs, FALSE, &framebuffer->d3d12DSV);
        }
        else
        {
            _commandList->OMSetRenderTargets(framebuffer->numRTVs, framebuffer->d3d12RTVs, FALSE, nullptr);
        }
    }

    void AGpuRendererD3D12::EndRenderPass()
    {
        for (uint32_t i = 0; i < _currentFramebuffer->numRTVs; ++i)
        {
            // Indicate that the back buffer will now be used to present.
            TransitionResource(_commandList,
                _currentFramebuffer->colorAttachments[i].texture->d3d12Resource,
                D3D12_RESOURCE_STATE_RENDER_TARGET,
                _currentFramebuffer->colorAttachments[i].texture->d3d12ResourceState);
        }

        _currentFramebuffer = nullptr;
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
        GetDXGIAdapter(&adapter);
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
                _dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;

                dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
                dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);

                Release(dxgiInfoQueue);
            }
        }
#endif

        // Create global dxgi factory.
        HRESULT hr = CreateDXGIFactory2(_dxgiFactoryFlags, IID_PPV_ARGS(&_dxgiFactory));
        if (FAILED(hr))
        {
            ALIMER_LOGERROR("Unable to create a DXGI 1.4 device. Make sure that your OS and driver support DirectX 12");
            return AGPU_ERROR;
        }

        AGpuRendererD3D12* renderD3D12 = new AGpuRendererD3D12();
        AgpuResult result = renderD3D12->Initialize(descriptor);
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
