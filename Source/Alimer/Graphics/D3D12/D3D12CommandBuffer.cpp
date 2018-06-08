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

#include "D3D12CommandBuffer.h"
#include "D3D12CommandListManager.h"
#include "D3D12Texture.h"
#include "D3D12GpuBuffer.h"
#include "D3D12Graphics.h"
#include "D3D12PipelineState.h"
#include "../../Core/Log.h"
#include "../../Core/Windows/EngineWindows.h"
#include "../../Util/Util.h"
#include "../../Util/HashMap.h"

namespace Alimer
{
	D3D12CommandBuffer::D3D12CommandBuffer(D3D12Graphics* graphics)
		: CommandBuffer(graphics)
		, _d3dDevice(graphics->GetD3DDevice())
		, _manager(graphics->GetCommandListManager())
		, _commandList(nullptr)
		, _currentAllocator(nullptr)
		, _type(D3D12_COMMAND_LIST_TYPE_DIRECT)
		, _numBarriersToFlush(0)
		, _boundRTVCount(0)
	{
		_manager->CreateNewCommandList(
			_type,
			&_commandList,
			&_currentAllocator);
	}

	D3D12CommandBuffer::~D3D12CommandBuffer()
	{
	}

	void D3D12CommandBuffer::Reset()
	{
		ALIMER_ASSERT(_commandList != nullptr && _currentAllocator == nullptr);
		_currentAllocator = _manager->GetQueue(_type).RequestAllocator();
		_commandList->Reset(_currentAllocator, nullptr);

		_numBarriersToFlush = 0;
		_currentPipeline = nullptr;
		_currentTopology = PrimitiveTopology::Count;
		// Reset cached state as well.
		CommandBuffer::ResetState();
	}

	uint64_t D3D12CommandBuffer::Commit(bool waitForCompletion)
	{
		FlushResourceBarriers();

		ALIMER_ASSERT(_currentAllocator != nullptr);

		// Execute the command list.
		auto& queue = _manager->GetQueue(_type);
		uint64_t fenceValue = queue.ExecuteCommandList(_commandList);
		queue.DiscardAllocator(fenceValue, _currentAllocator);
		_currentAllocator = nullptr;

		if (waitForCompletion)
		{
			_manager->WaitForFence(fenceValue);
		}

		static_cast<D3D12Graphics*>(_graphics)->RecycleCommandBuffer(this);

		//FreeCommandBuffer(this);

		return fenceValue;
	}

	void D3D12CommandBuffer::TransitionResource(D3D12Resource* resource, D3D12_RESOURCE_STATES newState, bool flushImmediate)
	{
		D3D12_RESOURCE_STATES oldState = resource->GetUsageState();

		if (_type == D3D12_COMMAND_LIST_TYPE_COMPUTE)
		{
			ALIMER_ASSERT((oldState & VALID_COMPUTE_QUEUE_RESOURCE_STATES) == oldState);
			ALIMER_ASSERT((newState & VALID_COMPUTE_QUEUE_RESOURCE_STATES) == newState);
		}

		if (oldState != newState)
		{
			ALIMER_ASSERT(_numBarriersToFlush < 16 && "Exceeded arbitrary limit on buffered barriers");
			D3D12_RESOURCE_BARRIER& barrierDesc = _resourceBarrierBuffer[_numBarriersToFlush++];

			barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrierDesc.Transition.pResource = resource->GetResource();
			barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			barrierDesc.Transition.StateBefore = oldState;
			barrierDesc.Transition.StateAfter = newState;

			// Check to see if we already started the transition
			if (newState == resource->GetTransitioningState())
			{
				barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
				resource->SetTransitioningState((D3D12_RESOURCE_STATES)-1);
			}
			else
			{
				barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			}

			resource->SetUsageState(newState);
		}
		else if (newState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
		{
			InsertUAVBarrier(resource, flushImmediate);
		}

		if (flushImmediate || _numBarriersToFlush == 16)
			FlushResourceBarriers();
	}

	void D3D12CommandBuffer::BeginResourceTransition(D3D12Resource* resource, D3D12_RESOURCE_STATES newState, bool flushImmediate)
	{
		// If it's already transitioning, finish that transition
		if (resource->GetTransitioningState() != (D3D12_RESOURCE_STATES)-1)
			TransitionResource(resource, resource->GetTransitioningState());

		D3D12_RESOURCE_STATES oldState = resource->GetUsageState();

		if (oldState != newState)
		{
			ALIMER_ASSERT(_numBarriersToFlush < 16 && "Exceeded arbitrary limit on buffered barriers");
			D3D12_RESOURCE_BARRIER& barrierDesc = _resourceBarrierBuffer[_numBarriersToFlush++];

			barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrierDesc.Transition.pResource = resource->GetResource();
			barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			barrierDesc.Transition.StateBefore = oldState;
			barrierDesc.Transition.StateAfter = newState;
			barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;

			resource->SetTransitioningState(newState);
		}

		if (flushImmediate || _numBarriersToFlush == 16)
			FlushResourceBarriers();
	}

	void D3D12CommandBuffer::InsertUAVBarrier(D3D12Resource* resource, bool flushImmediate)
	{
		ALIMER_ASSERT(_numBarriersToFlush < 16 && "Exceeded arbitrary limit on buffered barriers");
		D3D12_RESOURCE_BARRIER& barrierDesc = _resourceBarrierBuffer[_numBarriersToFlush++];

		barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrierDesc.UAV.pResource = resource->GetResource();

		if (flushImmediate)
			FlushResourceBarriers();
	}

	void D3D12CommandBuffer::FlushResourceBarriers()
	{
		if (_numBarriersToFlush > 0)
		{
			_commandList->ResourceBarrier(_numBarriersToFlush, _resourceBarrierBuffer);
			_numBarriersToFlush = 0;
		}
	}

	void D3D12CommandBuffer::BeginRenderPass(const RenderPassDescriptor& descriptor)
	{
		_boundRTVCount = 0;
		for (uint32_t i = 0; i < MaxColorAttachments; ++i)
		{
			const RenderPassColorAttachmentDescriptor& colorAttachment = descriptor.colorAttachments[i];
			Texture* texture = colorAttachment.texture;
			if (!texture)
				continue;

			D3D12Texture* d3dTexture = static_cast<D3D12Texture*>(texture);
			_boundRTV[_boundRTVCount] = d3dTexture->GetRTV();
			_boundRTVResources[_boundRTVCount++] = d3dTexture;

			// Transition to render target state.
			TransitionResource(d3dTexture, D3D12_RESOURCE_STATE_RENDER_TARGET, true);

			switch (colorAttachment.loadAction)
			{
				case LoadAction::Clear:
					_commandList->ClearRenderTargetView(
						d3dTexture->GetRTV(),
						colorAttachment.clearColor,
						0,
						nullptr);

					break;

				default:
					break;
			}
		}

		D3D12_VIEWPORT viewport;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = 800;
		viewport.Height = 600;
		viewport.MinDepth = D3D12_MIN_DEPTH;
		viewport.MaxDepth = D3D12_MAX_DEPTH;
		D3D12_RECT scissorRect;
		scissorRect.left = 0;
		scissorRect.top = 0;
		scissorRect.right = 800;
		scissorRect.bottom = 600;

		_commandList->RSSetViewports(1, &viewport);
		_commandList->RSSetScissorRects(1, &scissorRect);

		_commandList->OMSetRenderTargets(_boundRTVCount, _boundRTV, FALSE, nullptr);
	}

	void D3D12CommandBuffer::EndRenderPass()
	{
		for (uint32_t i = 0; i < _boundRTVCount; ++i)
		{
			TransitionResource(_boundRTVResources[i], D3D12_RESOURCE_STATE_COMMON, true);
		}
	}

	void D3D12CommandBuffer::SetPipeline(const PipelineStatePtr& pipeline)
	{
		auto d3dPipeline = std::static_pointer_cast<D3D12PipelineState>(pipeline);
        d3dPipeline->Bind(_commandList);
	}

	void D3D12CommandBuffer::DrawCore(PrimitiveTopology topology, uint32_t vertexCount, uint32_t instanceCount, uint32_t vertexStart, uint32_t baseInstance)
	{
		if (!PrepareDraw(topology))
			return;

		_commandList->DrawInstanced(vertexCount, instanceCount, vertexStart, baseInstance);
	}

	void D3D12CommandBuffer::DrawIndexedCore(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount, uint32_t startIndex)
	{
		if (!PrepareDraw(topology))
			return;

		_commandList->DrawIndexedInstanced(indexCount, instanceCount, startIndex, 0, 0);
	}

	void D3D12CommandBuffer::FlushGraphicsPipelineState()
	{
		using namespace Util;
		Hasher h;
		_activeVbos = 0;

		// TODO:
		_activeVbos |= 1u << 0;
		//auto &layout = current_layout->get_resource_layout();
		//ForEachBit(layout.attribute_mask, [&](uint32_t bit) {
		//	h.u32(bit);
			//_activeVbos |= 1u << attribs[bit].binding;
		//	h.u32(attribs[bit].binding);
		//	h.u32(attribs[bit].format);
		//	h.u32(attribs[bit].offset);
		//});

		ForEachBit(_activeVbos, [&](uint32_t bit) {
			h.u32(static_cast<uint32_t>(_vbo.inputRates[bit]));
			h.u32(_vbo.strides[bit]);
		});
	}

	bool D3D12CommandBuffer::PrepareDraw(PrimitiveTopology topology)
	{
		using namespace Util;

		// We've invalidated pipeline state, update the VkPipeline.
		if (GetAndClear(
			/*COMMAND_BUFFER_DIRTY_STATIC_STATE_BIT
			| COMMAND_BUFFER_DIRTY_PIPELINE_BIT
			| */COMMAND_BUFFER_DIRTY_STATIC_VERTEX_BIT))
		{
			ID3D12PipelineState* oldPipelineState = _currentPipeline;
			FlushGraphicsPipelineState();
			if (oldPipelineState != _currentPipeline)
			{
				//_commandList->SetGraphicsRootSignature(_testRootSignature.Get());
				//_commandList->SetPipelineState(_currentPipeline);
				//SetDirty(COMMAND_BUFFER_DYNAMIC_BITS);
			}
		}

		if (_currentTopology != topology)
		{
			_commandList->IASetPrimitiveTopology(d3d12::Convert(topology));
			_currentTopology = topology;
		}

		uint32_t updateVboMask = _dirtyVbos & _activeVbos;
		ForEachBitRange(updateVboMask, [&](uint32_t binding, uint32_t count)
		{
			static D3D12_VERTEX_BUFFER_VIEW views[MaxVertexBufferBindings] = {};

			for (uint32_t i = binding; i < binding + count; i++)
			{
#ifdef ALIMER_DEV
				ALIMER_ASSERT(_vbo.buffers[i] != nullptr);
#endif

				views[i].BufferLocation = static_cast<D3D12GpuBuffer*>(_vbo.buffers[i])->GetGpuVirtualAddress();
				views[i].StrideInBytes = _vbo.strides[i];
				views[i].SizeInBytes = _vbo.buffers[i]->GetSize();
			}

			_commandList->IASetVertexBuffers(binding, count, views);
		});
		_dirtyVbos &= ~updateVboMask;

		return true;
	}
}
