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
#include "D3D12Helpers.h"
#include <array>
#include <mutex>

namespace Alimer
{
    class D3D12CommandListManager;
    class D3D12Swapchain;
    class D3D12CommandBuffer;

    /// D3D12 Low-level 3D graphics API class.
    class D3D12Graphics final : public Graphics
    {
        friend class D3D12DescriptorAllocator;
        friend class D3D12CommandBuffer;

    public:
        /// Is backend supported?
        static bool IsSupported();

        /// Constructor.
        D3D12Graphics(bool validation);

        /// Destructor.
        ~D3D12Graphics() override;

        bool Initialize(const GraphicsSettings& settings) override;

        bool WaitIdle() override;
        void Frame() override;

        /*CommandBuffer* GetDefaultCommandBuffer() const override;

        SharedPtr<RenderPass> CreateRenderPass(const RenderPassDescription& description) override;
        SharedPtr<GpuBuffer> CreateBuffer(const GpuBufferDescription& description, const void* initialData) override;

        SharedPtr<Shader> CreateComputeShader(const ShaderStageDescription& desc) override;
        SharedPtr<Shader> CreateShader(const ShaderStageDescription& vertex, const ShaderStageDescription& fragment) override;
        SharedPtr<PipelineState> CreateRenderPipelineState(const RenderPipelineDescriptor& descriptor) override;
        */

        inline IDXGIFactory4* GetFactory() const { return _factory.Get(); }
        inline IDXGIAdapter1* GetAdapter() const { return _adapter.Get(); }
        inline ID3D12Device* GetD3DDevice() const { return _d3dDevice.Get(); }
        inline D3D12CommandListManager* GetCommandListManager() const { return _commandListManager; }

        inline D3D12DescriptorHandle AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT count = 1)
        {
            return _descriptorAllocator[type].Allocate(count);
        }

        D3D12_FEATURE_DATA_ROOT_SIGNATURE GetFeatureDataRootSignature() const { return _featureDataRootSignature; }

    private:
        void InitializeFeatures();

        /*
        SharedPtr<RenderPass> BeginFrameCore() override;
        void EndFrameCore() override;
        
        void HandleDeviceLost();
        
        SharedPtr<D3D12CommandBuffer> RetrieveCommandBuffer();
        void RecycleCommandBuffer(D3D12CommandBuffer* commandBuffer);
        */

        static constexpr uint32_t               RenderLatency = 2u;

        ComPtr<IDXGIFactory4>                   _factory;
        ComPtr<IDXGIAdapter1>                   _adapter;
        ComPtr<ID3D12Device>                    _d3dDevice;

        bool                                    _allowTearing = false;
        D3D_FEATURE_LEVEL                       _d3dFeatureLevel = D3D_FEATURE_LEVEL_11_0;
        D3D12_FEATURE_DATA_ROOT_SIGNATURE       _featureDataRootSignature;
        bool                                    _raytracingSupported = false;

        SharedPtr<D3D12Swapchain>               _mainSwapChain;

        ID3D12DescriptorHeap* RequestNewHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors);

        std::mutex _heapAllocationMutex;
        std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> _descriptorHeapPool;
        D3D12DescriptorAllocator _descriptorAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

        D3D12CommandListManager* _commandListManager;
    };
}
