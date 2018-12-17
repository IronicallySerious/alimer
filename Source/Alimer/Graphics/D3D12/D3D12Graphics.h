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

#pragma once

#include "../Graphics.h"
#include "D3D12DescriptorAllocator.h"
#include <queue>
#include <mutex>

namespace Alimer
{
    class D3D12CommandListManager;
    class D3D12Swapchain;
    class D3D12CommandContext;

    // Heap helpers
    const D3D12_HEAP_PROPERTIES* GetDefaultHeapProps();
    const D3D12_HEAP_PROPERTIES* GetUploadHeapProps();
    const D3D12_HEAP_PROPERTIES* GetReadbackHeapProps();

    struct UploadContext
    {
        ID3D12GraphicsCommandList* commandList;
        void* CPUAddress = nullptr;
        uint64_t resourceOffset = 0;
        ID3D12Resource* resource = nullptr;
        void* submission = nullptr;
    };


    /// D3D12 Low-level 3D graphics API class.
    class D3D12Graphics final : public GPUDevice
    {
    public:
        /// Is backend supported?
        static bool IsSupported();

        /// Constructor.
        D3D12Graphics(bool validation);

        /// Destructor.
        ~D3D12Graphics();

        //bool Initialize(const RenderWindowDescriptor* mainWindowDescriptor) override;

        //bool WaitIdle() override;
        //void Frame() override;

        D3D12CommandContext* AllocateContext(D3D12_COMMAND_LIST_TYPE type);
        void FreeContext(D3D12CommandContext* context);
        //CommandContext* AllocateContext() override;

        UploadContext ResourceUploadBegin(uint64_t size);
        void ResourceUploadEnd(UploadContext& context);

        //Framebuffer* GetSwapchainFramebuffer() const override;
        //FramebufferImpl* CreateFramebuffer(const Vector<FramebufferAttachment>& colorAttachments) override;
        //GpuBufferImpl* CreateBuffer(const BufferDescriptor* descriptor, const void* initialData, void* externalHandle) override;

        inline IDXGIFactory4* GetFactory() const { return _factory.Get(); }
        inline IDXGIAdapter1* GetAdapter() const { return _adapter.Get(); }
        inline ID3D12Device* GetD3DDevice() const { return _d3dDevice.Get(); }
        inline D3D12CommandListManager* GetCommandListManager() const { return _commandListManager; }

        inline D3D12DescriptorHandle AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT count = 1)
        {
            return _descriptorAllocator[type].Allocate(count);
        }

        D3D12_FEATURE_DATA_ROOT_SIGNATURE GetFeatureDataRootSignature() const { return _featureDataRootSignature; }

        ID3D12DescriptorHeap* RequestNewHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors);

    private:
        void InitializeFeatures();
        void InitializeUpload();
        void ShutdownUpload();
        void EndFrameUpload();
        void FlushUpload();
        void ClearFinishedUploads(uint64_t flushCount);
        

        Microsoft::WRL::ComPtr<IDXGIFactory4>   _factory;
        Microsoft::WRL::ComPtr<IDXGIAdapter1>   _adapter;
        Microsoft::WRL::ComPtr<ID3D12Device>    _d3dDevice;

        bool                                    _allowTearing = false;
        D3D_FEATURE_LEVEL                       _d3dFeatureLevel = D3D_FEATURE_LEVEL_11_0;
        D3D12_FEATURE_DATA_ROOT_SIGNATURE       _featureDataRootSignature;
        bool                                    _raytracingSupported = false;

        SharedPtr<D3D12Swapchain>               _mainSwapChain;
        
        std::mutex                              _heapAllocationMutex;
        std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> _descriptorHeapPool;
        D3D12DescriptorAllocator                _descriptorAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

        D3D12CommandListManager*                            _commandListManager;
        std::vector<std::unique_ptr<D3D12CommandContext> >  _contextPool[4];
        std::queue<D3D12CommandContext*>                    _availableCommandContexts[4];
        std::mutex                                          _contextAllocationMutex;

        struct UploadSubmission
        {
            ID3D12CommandAllocator* commandAllocator = nullptr;
            ID3D12GraphicsCommandList1* commandList = nullptr;
            uint64_t offset = 0;
            uint64_t size = 0;
            uint64_t fenceValue = 0;
            uint64_t padding = 0;

            void Reset()
            {
                offset = 0;
                size = 0;
                fenceValue = 0;
                padding = 0;
            }
        };
        UploadSubmission* AllocUploadSubmission(uint64_t size);

        static constexpr uint64_t UploadBufferSize = 256 * 1024 * 1024;
        static constexpr uint64_t MaxUploadSubmissions = 16;

        SRWLOCK                     _uploadSubmissionLock = SRWLOCK_INIT;
        SRWLOCK                     _uploadQueueLock = SRWLOCK_INIT;

        uint64_t                    _uploadBufferStart = 0;
        uint64_t                    _uploadBufferUsed = 0;
        UploadSubmission            _uploadSubmissions[MaxUploadSubmissions];
        uint64_t                    _uploadSubmissionStart = 0;
        uint64_t                    _uploadSubmissionUsed = 0;

        ID3D12CommandQueue*         _uploadCmdQueue = nullptr;
        D3D12Fence                  _uploadFence;
        uint64_t                    _uploadFenceValue = 0;
        ID3D12Resource*             _uploadBuffer = nullptr;
        uint8_t*                    _uploadBufferCPUAddress = nullptr;
    };
}
