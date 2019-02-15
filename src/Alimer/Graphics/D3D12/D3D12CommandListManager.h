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

#include "BackendD3D12.h"
#include <vector>
#include <queue>
#include <mutex>

namespace alimer
{
	class D3D12CommandListManager;

	class D3D12CommandAllocatorPool final
	{
	public:
		D3D12CommandAllocatorPool(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type);
		~D3D12CommandAllocatorPool();

		void Shutdown();

		ID3D12CommandAllocator* RequestAllocator(uint64_t completedFenceValue);
		void DiscardAllocator(uint64_t fenceValue, ID3D12CommandAllocator* allocator);
		inline size_t Size() { return _allocatorPool.size(); }

	private:
		ID3D12Device * _device;
		const D3D12_COMMAND_LIST_TYPE _type;
		std::vector<ID3D12CommandAllocator*> _allocatorPool;
		std::queue<std::pair<uint64_t, ID3D12CommandAllocator*>> _readyAllocators;
		std::mutex _mutex;
	};

	
	class D3D12CommandQueue final
	{
	public:
		D3D12CommandQueue(D3D12CommandListManager* manager, D3D12_COMMAND_LIST_TYPE type);
		~D3D12CommandQueue();

		uint64_t ExecuteCommandList(ID3D12GraphicsCommandList* commandList);
		ID3D12CommandAllocator* RequestAllocator();
		void DiscardAllocator(uint64_t fenceValue, ID3D12CommandAllocator* allocator);

		uint64_t IncrementFence();
		bool IsFenceComplete(uint64_t fenceValue);
		void StallForFence(uint64_t fenceValue);
		void WaitForFence(uint64_t fenceValue);

		void WaitForIdle() { WaitForFence(IncrementFence()); }
		inline bool IsReady() { return _d3dCommandQueue != nullptr; }
		ID3D12CommandQueue* GetCommandQueue() { return _d3dCommandQueue; }

		uint64_t GetNextFenceValue() { return _nextFenceValue; }

	private:
		D3D12CommandListManager* _manager;
		ID3D12CommandQueue* _d3dCommandQueue;
		const D3D12_COMMAND_LIST_TYPE _type;

		D3D12CommandAllocatorPool _allocatorPool;
		std::mutex _fenceMutex;
		std::mutex _eventMutex;

		// Lifetime of these objects is managed by the descriptor cache
		ID3D12Fence* _d3dFence;
		uint64_t _nextFenceValue;
		uint64_t _lastCompletedFenceValue;
		HANDLE _fenceEventHandle;
	};

	class D3D12CommandListManager final
	{
	public:
		D3D12CommandListManager(ID3D12Device* device);
		~D3D12CommandListManager();

        void WaitIdle();
		inline ID3D12Device* GetDevice() const { return _device; }
		D3D12CommandQueue& GetGraphicsQueue() { return _graphicsQueue; }
		D3D12CommandQueue& GetComputeQueue() { return _computeQueue; }
		D3D12CommandQueue& GetCopyQueue() { return _copyQueue; }

		D3D12CommandQueue& GetQueue(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT)
		{
			switch (type)
			{
				case D3D12_COMMAND_LIST_TYPE_COMPUTE: return _computeQueue;
				case D3D12_COMMAND_LIST_TYPE_COPY: return _copyQueue;
				default: return _graphicsQueue;
			}
		}

		ID3D12CommandQueue* GetD3DCommandQueue(D3D12_COMMAND_LIST_TYPE type)
		{
			switch (type)
			{
				case D3D12_COMMAND_LIST_TYPE_COMPUTE: return _computeQueue.GetCommandQueue();
				case D3D12_COMMAND_LIST_TYPE_COPY: return _copyQueue.GetCommandQueue();
				default: return _graphicsQueue.GetCommandQueue();
			}
		}

		// Test to see if a fence has already been reached
		bool IsFenceComplete(uint64_t fenceValue)
		{
			return GetQueue(D3D12_COMMAND_LIST_TYPE(fenceValue >> 56)).IsFenceComplete(fenceValue);
		}

		// The CPU will wait for a fence to reach a specified value
		void WaitForFence(uint64_t fenceValue);

		void CreateNewCommandList(
			D3D12_COMMAND_LIST_TYPE type,
			ID3D12GraphicsCommandList** commandList,
			ID3D12CommandAllocator** allocator);

	private:
		ID3D12Device* _device;

		D3D12CommandQueue _graphicsQueue;
		D3D12CommandQueue _computeQueue;
		D3D12CommandQueue _copyQueue;
	};
}
