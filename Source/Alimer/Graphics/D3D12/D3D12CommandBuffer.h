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

#include "Graphics/CommandBuffer.h"
#include "D3D12Helpers.h"
#include <vector>
#include <queue>
#include <mutex>

#define VALID_COMPUTE_QUEUE_RESOURCE_STATES \
	( D3D12_RESOURCE_STATE_UNORDERED_ACCESS \
	| D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE \
	| D3D12_RESOURCE_STATE_COPY_DEST \
	| D3D12_RESOURCE_STATE_COPY_SOURCE )

namespace Alimer
{
	class D3D12Graphics;
	class D3D12Texture;
	class D3D12CommandListManager;

	/// D3D12 CommandBuffer implementation.
	class D3D12CommandBuffer final
		: public CommandBuffer
		, public std::enable_shared_from_this<D3D12CommandBuffer>
	{
	public:
		/// Constructor.
		D3D12CommandBuffer(D3D12Graphics* graphics);

		/// Destructor.
		~D3D12CommandBuffer() override;

		void Reset();
		uint64_t Commit(bool waitForCompletion) override;

		void TransitionResource(D3D12Resource* resource, D3D12_RESOURCE_STATES newState, bool flushImmediate = false);
		void BeginResourceTransition(D3D12Resource* resource, D3D12_RESOURCE_STATES newState, bool flushImmediate = false);
		void InsertUAVBarrier(D3D12Resource* resource, bool flushImmediate = false);
		void FlushResourceBarriers();

		void BeginRenderPass(const RenderPassDescriptor& descriptor) override;
		void EndRenderPass() override;

		void SetPipeline(const PipelineStatePtr& pipeline) override;

		void DrawCore(PrimitiveTopology topology, uint32_t vertexCount, uint32_t instanceCount, uint32_t vertexStart, uint32_t baseInstance) override;
		void DrawIndexedCore(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount, uint32_t startIndex) override;

	private:
		void FlushGraphicsPipelineState();
		bool PrepareDraw(PrimitiveTopology topology);

	private:
		ID3D12Device * _d3dDevice;
		D3D12CommandListManager * _manager;
		ID3D12GraphicsCommandList* _commandList;
		ID3D12CommandAllocator* _currentAllocator;
		D3D12_COMMAND_LIST_TYPE _type;

		D3D12_RESOURCE_BARRIER _resourceBarrierBuffer[16];
		uint32_t _numBarriersToFlush;

		uint32_t _boundRTVCount;
		D3D12Resource* _boundRTVResources[MaxColorAttachments];
		D3D12_CPU_DESCRIPTOR_HANDLE _boundRTV[MaxColorAttachments];

		ID3D12PipelineState* _currentPipeline = nullptr;
		PrimitiveTopology _currentTopology = PrimitiveTopology::Count;
	};
}
