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
#include "D3D12Graphics.h"
#include "../../Debug/Log.h"

namespace Alimer
{
	D3D12CommandBuffer::D3D12CommandBuffer(D3D12Graphics* graphics)
		: CommandBuffer(graphics)
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
		assert(_commandList != nullptr && _currentAllocator == nullptr);
		_currentAllocator = _manager->GetQueue(_type).RequestAllocator();
		_commandList->Reset(_currentAllocator, nullptr);

		_numBarriersToFlush = 0;
	}

	uint64_t D3D12CommandBuffer::Commit(bool waitForCompletion)
	{
		FlushResourceBarriers();

		assert(_currentAllocator != nullptr);

		// Execute the command list.
		auto& queue = _manager->GetQueue(_type);
		uint64_t fenceValue = queue.ExecuteCommandList(_commandList);
		queue.DiscardAllocator(fenceValue, _currentAllocator);
		_currentAllocator = nullptr;

		if (waitForCompletion)
		{
			_manager->WaitForFence(fenceValue);
		}

		static_cast<D3D12Graphics*>(_graphics)->RecycleCommandBuffer(shared_from_this());

		//FreeCommandBuffer(this);

		return fenceValue;
	}

	void D3D12CommandBuffer::TransitionResource(D3D12Resource* resource, D3D12_RESOURCE_STATES newState, bool flushImmediate)
	{
		D3D12_RESOURCE_STATES oldState = resource->GetUsageState();

		if (_type == D3D12_COMMAND_LIST_TYPE_COMPUTE)
		{
			assert((oldState & VALID_COMPUTE_QUEUE_RESOURCE_STATES) == oldState);
			assert((newState & VALID_COMPUTE_QUEUE_RESOURCE_STATES) == newState);
		}

		if (oldState != newState)
		{
			assert(_numBarriersToFlush < 16 && "Exceeded arbitrary limit on buffered barriers");
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
			assert(_numBarriersToFlush < 16 && "Exceeded arbitrary limit on buffered barriers");
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
		assert(_numBarriersToFlush < 16 && "Exceeded arbitrary limit on buffered barriers");
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

		_commandList->OMSetRenderTargets(_boundRTVCount, _boundRTV, FALSE, nullptr);
	}

	void D3D12CommandBuffer::EndRenderPass()
	{
		for (uint32_t i = 0; i < _boundRTVCount; ++i)
		{
			TransitionResource(_boundRTVResources[i], D3D12_RESOURCE_STATE_COMMON, true);
		}
	}

	void D3D12CommandBuffer::DrawCore(PrimitiveTopology topology, uint32_t vertexCount, uint32_t instanceCount, uint32_t vertexStart, uint32_t baseInstance)
	{
		if (!PrepareDraw(topology))
			return;
		
	}

	void D3D12CommandBuffer::DrawIndexedCore(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount, uint32_t startIndex)
	{
		if (!PrepareDraw(topology))
			return;

		_commandList->DrawIndexedInstanced(indexCount, instanceCount, startIndex, 0,  0);
	}

	bool D3D12CommandBuffer::PrepareDraw(PrimitiveTopology topology)
	{
		if (_currentTopology != topology)
		{
			_commandList->IASetPrimitiveTopology(d3d12::Convert(topology));
			_currentTopology = topology;
		}

		return true;
	}
}
