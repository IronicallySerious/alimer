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

#include "Graphics/Graphics.h"
#include "D3D12DescriptorAllocator.h"
#include "D3D12Helpers.h"
#include <array>
#include <mutex>

namespace Alimer
{
	class D3D12CommandBuffer;
	class D3D12CommandListManager;
	class D3D12Texture;

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
		virtual ~D3D12Graphics() override;

		bool BackendInitialize() override;
		bool WaitIdle() override;
        CommandBuffer* GetDefaultContext() const override;

		SharedPtr<Texture> AcquireNextImage() override;
        void EndFrame() override;

        SharedPtr<GpuBuffer> CreateBuffer(const GpuBufferDescription& description, const void* initialData) override;

		SharedPtr<Shader> CreateComputeShader(const ShaderStageDescription& desc) override;
		SharedPtr<Shader> CreateShader(const ShaderStageDescription& vertex, const ShaderStageDescription& fragment) override;
        SharedPtr<PipelineState> CreateRenderPipelineState(const RenderPipelineDescriptor& descriptor) override;

		inline IDXGIFactory4* GetDXGIFactory() const { return _factory.Get(); }
		inline ID3D12Device* GetD3DDevice() const { return _d3dDevice.Get(); }
		inline D3D12CommandListManager* GetCommandListManager() const { return _commandListManager; }

		inline D3D12DescriptorHandle AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT count = 1)
		{
			return _descriptorAllocator[type].Allocate(count);
		}

        D3D12_FEATURE_DATA_ROOT_SIGNATURE GetFeatureDataRootSignature() const { return _featureDataRootSignature; }

	private:
		bool InitializeCaps();
        void HandleDeviceLost();
		void CreateSwapchain();
		SharedPtr<D3D12CommandBuffer> RetrieveCommandBuffer();
		void RecycleCommandBuffer(D3D12CommandBuffer* commandBuffer);

		static constexpr uint32_t FrameCount = 2u;
		bool _useWarpDevice{ false };

		ComPtr<IDXGIFactory4> _factory;
		ComPtr<ID3D12Device> _d3dDevice;

        D3D12_FEATURE_DATA_ROOT_SIGNATURE _featureDataRootSignature;

		ComPtr<IDXGISwapChain3> _swapChain;
		ComPtr<ID3D12Resource> _renderTargets[FrameCount];

		ID3D12DescriptorHeap* RequestNewHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors);

		std::mutex _heapAllocationMutex;
		std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> _descriptorHeapPool;
		D3D12DescriptorAllocator _descriptorAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

		D3D12CommandListManager* _commandListManager;
		static constexpr size_t CommandBufferRecycleCount = 16;
		std::mutex _commandBufferMutex;
		std::array<SharedPtr<D3D12CommandBuffer>, CommandBufferRecycleCount> _recycledCommandBuffers;
		size_t _commandBufferObjectId = 0;
        SharedPtr<D3D12CommandBuffer> _frameCommandBuffer;

		std::vector<SharedPtr<D3D12Texture>> _textures;
	};
}
