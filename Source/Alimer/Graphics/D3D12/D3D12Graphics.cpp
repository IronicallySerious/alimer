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
#include "D3D12CommandBuffer.h"
#include "D3D12Texture.h"
#include "D3D12Framebuffer.h"
#include "D3D12GpuBuffer.h"
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

    using namespace Microsoft::WRL;

    const D3D12_HEAP_PROPERTIES* GetDefaultHeapProps()
    {
        static D3D12_HEAP_PROPERTIES heapProps =
        {
            D3D12_HEAP_TYPE_DEFAULT,
            D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
            D3D12_MEMORY_POOL_UNKNOWN,
            0,
            0,
        };

        return &heapProps;
    }

    const D3D12_HEAP_PROPERTIES* GetUploadHeapProps()
    {
        static D3D12_HEAP_PROPERTIES heapProps =
        {
            D3D12_HEAP_TYPE_UPLOAD,
            D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
            D3D12_MEMORY_POOL_UNKNOWN,
            0,
            0,
        };

        return &heapProps;
    }

    const D3D12_HEAP_PROPERTIES* GetReadbackHeapProps()
    {
        static D3D12_HEAP_PROPERTIES heapProps =
        {
            D3D12_HEAP_TYPE_READBACK,
            D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
            D3D12_MEMORY_POOL_UNKNOWN,
            0,
            0,
        };

        return &heapProps;
    }

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
        : _descriptorAllocator{
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

        // Destroy all command contexts.
        for (uint32_t i = 0; i < 4; ++i)
        {
            _contextPool[i].clear();
        }

        // Delete command list manager.
        SafeDelete(_commandListManager);

        // Delete main swap chain if created.
        _mainSwapChain.Reset();

        // Shutdown upload logic.
        ShutdownUpload();

        // Clear DescriptorHeap Pools.
        {
            std::lock_guard<std::mutex> guard(_heapAllocationMutex);
            _descriptorHeapPool.clear();
        }
    }

    void D3D12Graphics::InitializeFeatures()
    {
        DXGI_ADAPTER_DESC1 desc = { };
        _adapter->GetDesc1(&desc);

        features.SetDeviceId(desc.DeviceId);
        features.SetVendorId(desc.VendorId);
        features.SetDeviceName(String(desc.Description));
        features.SetMultithreading(true);
        features.SetMaxColorAttachments(D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT);

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

        InitializeUpload();

        return true;
    }

   
    void D3D12Graphics::InitializeUpload()
    {
        for (uint64_t i = 0; i < MaxUploadSubmissions; ++i)
        {
            UploadSubmission& submission = _uploadSubmissions[i];
            ThrowIfFailed(_d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&submission.commandAllocator)));
            ThrowIfFailed(_d3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, submission.commandAllocator, nullptr, IID_PPV_ARGS(&submission.commandList)));
            ThrowIfFailed(submission.commandList->Close());

            wchar_t name[25] = {};
            swprintf_s(name, L"Upload Command List %u", i);
            submission.commandList->SetName(name);
        }

        D3D12_COMMAND_QUEUE_DESC queueDesc = { };
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
        ThrowIfFailed(_d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&_uploadCmdQueue)));
        _uploadCmdQueue->SetName(L"Upload Copy Queue");

        _uploadFence.Initialize(_d3dDevice.Get(), 0);

        D3D12_RESOURCE_DESC resourceDesc = { };
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resourceDesc.Width = UploadBufferSize;
        resourceDesc.Height = 1;
        resourceDesc.DepthOrArraySize = 1;
        resourceDesc.MipLevels = 1;
        resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
        resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
        resourceDesc.SampleDesc.Count = 1;
        resourceDesc.SampleDesc.Quality = 0;
        resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        resourceDesc.Alignment = 0;

        ThrowIfFailed(_d3dDevice->CreateCommittedResource(
            GetUploadHeapProps(),
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr, IID_PPV_ARGS(&_uploadBuffer))
        );

        D3D12_RANGE readRange = { };
        ThrowIfFailed(_uploadBuffer->Map(0, &readRange, reinterpret_cast<void**>(&_uploadBufferCPUAddress)));
    }

    void D3D12Graphics::ShutdownUpload()
    {
        //for (uint64_t i = 0; i < ArraySize_(TempFrameBuffers); ++i)
        //    Release(TempFrameBuffers[i]);

        SafeRelease(_uploadBuffer);
        SafeRelease(_uploadCmdQueue);
        _uploadFence.Shutdown();
        for (uint64_t i = 0; i < MaxUploadSubmissions; ++i)
        {
            SafeRelease(_uploadSubmissions[i].commandAllocator);
            SafeRelease(_uploadSubmissions[i].commandList);
        }

        /*Release(convertCmdAllocator);
        Release(convertCmdList);
        Release(convertCmdQueue);
        Release(convertPSO);
        Release(convertArrayPSO);
        Release(convertCubePSO);
        Release(convertRootSignature);
        convertFence.Shutdown();

        Release(readbackCmdAllocator);
        Release(readbackCmdList);
        readbackFence.Shutdown();*/
    }

    void D3D12Graphics::EndFrameUpload()
    {
        // If we can grab the lock, try to clear out any completed submissions
        if (TryAcquireSRWLockExclusive(&_uploadSubmissionLock))
        {
            ClearFinishedUploads(0);

            ReleaseSRWLockExclusive(&_uploadSubmissionLock);
        }

        {
            AcquireSRWLockExclusive(&_uploadQueueLock);

            // Make sure to sync on any pending uploads
            ClearFinishedUploads(0);
            //_commandListManager->GetD3DCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT).WaitForFence(_uploadFence->Fence, _uploadFenceValue);

            ReleaseSRWLockExclusive(&_uploadQueueLock);
        }

        //TempFrameUsed = 0;
    }

    void D3D12Graphics::FlushUpload()
    {
        AcquireSRWLockExclusive(&_uploadSubmissionLock);

        ClearFinishedUploads(uint64_t(-1));

        ReleaseSRWLockExclusive(&_uploadSubmissionLock);
    }

    void D3D12Graphics::ClearFinishedUploads(uint64_t flushCount)
    {
        const uint64_t start = _uploadSubmissionStart;
        const uint64_t used = _uploadSubmissionUsed;
        for (uint64_t i = 0; i < used; ++i)
        {
            const uint64_t idx = (start + i) % MaxUploadSubmissions;
            UploadSubmission& submission = _uploadSubmissions[idx];
            ALIMER_ASSERT(submission.size > 0);
            ALIMER_ASSERT(_uploadBufferUsed >= submission.size);

            // If the submission hasn't been sent to the GPU yet we can't wait for it
            if (submission.fenceValue == uint64_t(-1))
                return;

            if (i < flushCount)
            {
                _uploadFence.Wait(submission.fenceValue);
            }

            if (_uploadFence.Signaled(submission.fenceValue))
            {
                _uploadSubmissionStart = (_uploadSubmissionStart + 1) % MaxUploadSubmissions;
                _uploadSubmissionUsed -= 1;
                _uploadBufferStart = (_uploadBufferStart + submission.padding) % UploadBufferSize;
                ALIMER_ASSERT(submission.offset == _uploadBufferStart);
                ALIMER_ASSERT(_uploadBufferStart + submission.size <= UploadBufferSize);
                _uploadBufferStart = (_uploadBufferStart + submission.size) % UploadBufferSize;
                _uploadBufferUsed -= (submission.size + submission.padding);
                submission.Reset();

                if (_uploadBufferUsed == 0)
                    _uploadBufferStart = 0;
            }
        }
    }

    D3D12Graphics::UploadSubmission* D3D12Graphics::AllocUploadSubmission(uint64_t size)
    {
        ALIMER_ASSERT(_uploadSubmissionUsed <= MaxUploadSubmissions);
        if (_uploadSubmissionUsed == MaxUploadSubmissions)
            return nullptr;

        const uint64_t submissionIdx = (_uploadSubmissionStart + _uploadSubmissionUsed) % MaxUploadSubmissions;
        ALIMER_ASSERT(_uploadSubmissions[submissionIdx].size == 0);

        ALIMER_ASSERT(_uploadBufferUsed <= UploadBufferSize);
        if (size > (UploadBufferSize - _uploadBufferUsed))
            return nullptr;

        const uint64_t start = _uploadBufferStart;
        const uint64_t end = _uploadBufferStart + _uploadBufferUsed;
        uint64_t allocOffset = uint64_t(-1);
        uint64_t padding = 0;
        if (end < UploadBufferSize)
        {
            const uint64_t endAmt = UploadBufferSize - end;
            if (endAmt >= size)
            {
                allocOffset = end;
            }
            else if (start >= size)
            {
                // Wrap around to the beginning
                allocOffset = 0;
                _uploadBufferUsed += endAmt;
                padding = endAmt;
            }
        }
        else
        {
            const uint64_t wrappedEnd = end % UploadBufferSize;
            if ((start - wrappedEnd) >= size)
                allocOffset = wrappedEnd;
        }

        if (allocOffset == uint64_t(-1))
            return nullptr;

        _uploadSubmissionUsed += 1;
        _uploadBufferUsed += size;

        UploadSubmission* submission = &_uploadSubmissions[submissionIdx];
        submission->offset = allocOffset;
        submission->size = size;
        submission->fenceValue = uint64_t(-1);
        submission->padding = padding;

        return submission;
    }

    UploadContext D3D12Graphics::ResourceUploadBegin(uint64_t size)
    {
        ALIMER_ASSERT(_d3dDevice != nullptr);

        size = AlignTo(size, 512);
        ALIMER_ASSERT(size <= UploadBufferSize);
        ALIMER_ASSERT(size > 0);

        UploadSubmission* submission = nullptr;

        {
            AcquireSRWLockExclusive(&_uploadSubmissionLock);

            ClearFinishedUploads(0);

            submission = AllocUploadSubmission(size);
            while (submission == nullptr)
            {
                ClearFinishedUploads(1);
                submission = AllocUploadSubmission(size);
            }

            ReleaseSRWLockExclusive(&_uploadSubmissionLock);
        }

        ThrowIfFailed(submission->commandAllocator->Reset());
        ThrowIfFailed(submission->commandList->Reset(submission->commandAllocator, nullptr));

        UploadContext context;
        context.commandList = submission->commandList;
        context.resource = _uploadBuffer;
        context.CPUAddress = _uploadBufferCPUAddress + submission->offset;
        context.resourceOffset = submission->offset;
        context.submission = submission;

        return context;

    }

    void D3D12Graphics::ResourceUploadEnd(UploadContext& context)
    {
        ALIMER_ASSERT(context.commandList != nullptr);
        ALIMER_ASSERT(context.submission != nullptr);
        UploadSubmission* submission = reinterpret_cast<UploadSubmission*>(context.submission);

        {
            AcquireSRWLockExclusive(&_uploadQueueLock);

            // Finish off and execute the command list
            ThrowIfFailed(submission->commandList->Close());
            ID3D12CommandList* cmdLists[1] = { submission->commandList };
            _uploadCmdQueue->ExecuteCommandLists(1, cmdLists);

            ++_uploadFenceValue;
            _uploadFence.Signal(_uploadCmdQueue, _uploadFenceValue);
            submission->fenceValue = _uploadFenceValue;

            ReleaseSRWLockExclusive(&_uploadQueueLock);
        }

        context = UploadContext();
    }

    bool D3D12Graphics::WaitIdle()
    {
        _commandListManager->WaitIdle();
        return true;
    }
    
    void D3D12Graphics::Frame()
    {
        /* End frame for upload */
        EndFrameUpload();

        // Present the frame.
        if (_mainSwapChain)
        {
            _mainSwapChain->Present();
        }

    }

    D3D12CommandContext* D3D12Graphics::AllocateContext(D3D12_COMMAND_LIST_TYPE type)
    {
        std::lock_guard<std::mutex> lock(_contextAllocationMutex);

        auto& availableContexts = _availableCommandContexts[type];

        D3D12CommandContext* context = nullptr;
        if (availableContexts.empty())
        {
            context = new D3D12CommandContext(this, type);
            _contextPool[type].emplace_back(context);
            context->Initialize();
        }
        else
        {
            context = availableContexts.front();
            availableContexts.pop();
            context->Reset();
        }
        ALIMER_ASSERT(context != nullptr);
        ALIMER_ASSERT(context->GetCommandListType() == type);
        return context;
    }

    void D3D12Graphics::FreeContext(D3D12CommandContext* context)
    {
        ALIMER_ASSERT(context != nullptr);
        std::lock_guard<std::mutex> lock(_contextAllocationMutex);
        _availableCommandContexts[context->GetCommandListType()].push(context);
    }

    CommandContext* D3D12Graphics::AllocateContext()
    {
        return AllocateContext(D3D12_COMMAND_LIST_TYPE_DIRECT);
    }

    Framebuffer* D3D12Graphics::GetSwapchainFramebuffer() const
    {
        return _mainSwapChain->GetFramebuffer();
    }

    FramebufferImpl* D3D12Graphics::CreateFramebuffer(const Vector<FramebufferAttachment>& colorAttachments)
    {
        return new D3D12Framebuffer(this, colorAttachments);
    }

    GpuBufferImpl* D3D12Graphics::CreateBuffer(const BufferDescriptor* descriptor, const void* initialData, void* externalHandle)
    {
        return new D3D12Buffer(this, descriptor, initialData, externalHandle);
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
        heapDesc.NodeMask = 0;

        ComPtr<ID3D12DescriptorHeap> heap;
        ThrowIfFailed(_d3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heap)));
        _descriptorHeapPool.emplace_back(heap);
        return heap.Get();
    }

    /* D3D12Fence */
    D3D12Fence::~D3D12Fence()
    {
        ALIMER_ASSERT(D3DFence == nullptr);
        Shutdown();
    }

    void D3D12Fence::Initialize(ID3D12Device* device, uint64_t initialValue)
    {
        ThrowIfFailed(device->CreateFence(initialValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&D3DFence)));
        FenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
        //Win32Call(FenceEvent != 0);
    }

    void D3D12Fence::Shutdown()
    {
        SafeRelease(D3DFence);
        //DeferredRelease(D3DFence);
    }

    void D3D12Fence::Signal(ID3D12CommandQueue* queue, uint64_t fenceValue)
    {
        ALIMER_ASSERT(D3DFence != nullptr);
        ThrowIfFailed(queue->Signal(D3DFence, fenceValue));
    }

    void D3D12Fence::Wait(uint64_t fenceValue)
    {
        ALIMER_ASSERT(D3DFence != nullptr);
        if (D3DFence->GetCompletedValue() < fenceValue)
        {
            ThrowIfFailed(D3DFence->SetEventOnCompletion(fenceValue, FenceEvent));
            WaitForSingleObject(FenceEvent, INFINITE);
        }
    }

    bool D3D12Fence::Signaled(uint64_t fenceValue)
    {
        ALIMER_ASSERT(D3DFence != nullptr);
        return D3DFence->GetCompletedValue() >= fenceValue;
    }

    void D3D12Fence::Clear(uint64_t fenceValue)
    {
        ALIMER_ASSERT(D3DFence != nullptr);
        D3DFence->Signal(fenceValue);
    }

}
