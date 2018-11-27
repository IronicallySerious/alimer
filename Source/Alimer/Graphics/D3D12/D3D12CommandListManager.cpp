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

#include "D3D12CommandListManager.h"
#include "D3D12Graphics.h"
#include "../../Debug/Log.h"

namespace Alimer
{
    /* D3D12CommandAllocatorPool */
    D3D12CommandAllocatorPool::D3D12CommandAllocatorPool(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type)
        : _device(device)
        , _type(type)
    {

    }

    D3D12CommandAllocatorPool::~D3D12CommandAllocatorPool()
    {
        Shutdown();
    }

    void D3D12CommandAllocatorPool::Shutdown()
    {
        for (size_t i = 0; i < _allocatorPool.size(); ++i)
        {
            _allocatorPool[i]->Release();
        }

        _allocatorPool.clear();
    }

    ID3D12CommandAllocator* D3D12CommandAllocatorPool::RequestAllocator(uint64_t completedFenceValue)
    {
        std::lock_guard<std::mutex> LockGuard(_mutex);

        ID3D12CommandAllocator* result = nullptr;

        if (!_readyAllocators.empty())
        {
            std::pair<uint64_t, ID3D12CommandAllocator*>& allocatorPair = _readyAllocators.front();

            if (allocatorPair.first <= completedFenceValue)
            {
                result = allocatorPair.second;
                ThrowIfFailed(result->Reset());
                _readyAllocators.pop();
            }
        }

        // If no allocator's were ready to be reused, create a new one
        if (result == nullptr)
        {
            ThrowIfFailed(_device->CreateCommandAllocator(_type, IID_PPV_ARGS(&result)));
            wchar_t name[32];
            swprintf(name, 32, L"CommandAllocator %zu", _allocatorPool.size());
            result->SetName(name);
            _allocatorPool.push_back(result);
        }

        return result;
    }

    void D3D12CommandAllocatorPool::DiscardAllocator(uint64_t fenceValue, ID3D12CommandAllocator* allocator)
    {
        std::lock_guard<std::mutex> LockGuard(_mutex);

        // That fence value indicates we are free to reset the allocator
        _readyAllocators.push(std::make_pair(fenceValue, allocator));
    }

    // D3D12CommandQueue
    D3D12CommandQueue::D3D12CommandQueue(D3D12CommandListManager* manager, D3D12_COMMAND_LIST_TYPE type)
        : _manager(manager)
        , _type(type)
        , _d3dCommandQueue(nullptr)
        , _d3dFence(nullptr)
        , _nextFenceValue((uint64_t)type << 56 | 1)
        , _lastCompletedFenceValue((uint64_t)type << 56)
        , _allocatorPool(manager->GetDevice(), type)
    {
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Type = type;
        queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.NodeMask = 0;

        if (FAILED(manager->GetDevice()->CreateCommandQueue(
            &queueDesc, IID_PPV_ARGS(&_d3dCommandQueue))))
        {
            ALIMER_LOGERROR("Failed to create D3D12 CommandQueue");
            return;
        }

        switch (type)
        {
        case D3D12_COMMAND_LIST_TYPE_DIRECT:
            _d3dCommandQueue->SetName(L"Main Gfx Queue");
            break;
        case D3D12_COMMAND_LIST_TYPE_COMPUTE:
            _d3dCommandQueue->SetName(L"Main Compute Queue");
            break;
        case D3D12_COMMAND_LIST_TYPE_COPY:
            _d3dCommandQueue->SetName(L"Main Copy Queue");
            break;
        default:
            break;
        }

        ThrowIfFailed(manager->GetDevice()->CreateFence(
            0, D3D12_FENCE_FLAG_NONE,
            IID_PPV_ARGS(&_d3dFence))
        );
        _d3dFence->SetName(L"D3D12CommandListManager::fence");
        _d3dFence->Signal((uint64_t)_type << 56);

        _fenceEventHandle = CreateEvent(nullptr, false, false, nullptr);
        ALIMER_ASSERT(_fenceEventHandle != INVALID_HANDLE_VALUE);
    }

    D3D12CommandQueue::~D3D12CommandQueue()
    {
        if (_d3dCommandQueue == nullptr)
            return;

        _allocatorPool.Shutdown();

        CloseHandle(_fenceEventHandle);

        _d3dFence->Release();
        _d3dFence = nullptr;

        _d3dCommandQueue->Release();
        _d3dCommandQueue = nullptr;
    }

    uint64_t D3D12CommandQueue::ExecuteCommandList(ID3D12GraphicsCommandList* commandList)
    {
        std::lock_guard<std::mutex> LockGuard(_fenceMutex);

        ThrowIfFailed(commandList->Close());

        // Kickoff the command list.
        ID3D12CommandList* ppCommandLists[] = { commandList };
        _d3dCommandQueue->ExecuteCommandLists(1, ppCommandLists);

        // Signal the next fence value (with the GPU)
        _d3dCommandQueue->Signal(_d3dFence, _nextFenceValue);

        // And increment the fence value.  
        return _nextFenceValue++;
    }

    ID3D12CommandAllocator* D3D12CommandQueue::RequestAllocator()
    {
        uint64_t completedFenceValue = _d3dFence->GetCompletedValue();

        return _allocatorPool.RequestAllocator(completedFenceValue);
    }

    void D3D12CommandQueue::DiscardAllocator(uint64_t fenceValue, ID3D12CommandAllocator* allocator)
    {
        _allocatorPool.DiscardAllocator(fenceValue, allocator);
    }

    uint64_t D3D12CommandQueue::IncrementFence()
    {
        std::lock_guard<std::mutex> LockGuard(_fenceMutex);
        _d3dCommandQueue->Signal(_d3dFence, _nextFenceValue);
        return _nextFenceValue++;
    }

    bool D3D12CommandQueue::IsFenceComplete(uint64_t fenceValue)
    {
        // Avoid querying the fence value by testing against the last one seen.
        // The max() is to protect against an unlikely race condition that could cause the last
        // completed fence value to regress.
        if (fenceValue > _lastCompletedFenceValue)
            _lastCompletedFenceValue = std::max(_lastCompletedFenceValue, _d3dFence->GetCompletedValue());

        return fenceValue <= _lastCompletedFenceValue;
    }

    void D3D12CommandQueue::StallForFence(uint64_t fenceValue)
    {
        D3D12CommandQueue& producer = _manager->GetQueue((D3D12_COMMAND_LIST_TYPE)(fenceValue >> 56));
        _d3dCommandQueue->Wait(producer._d3dFence, fenceValue);
    }

    void D3D12CommandQueue::WaitForFence(uint64_t fenceValue)
    {
        if (IsFenceComplete(fenceValue))
            return;

        // TODO:  Think about how this might affect a multi-threaded situation.  Suppose thread A
        // wants to wait for fence 100, then thread B comes along and wants to wait for 99.  If
        // the fence can only have one event set on completion, then thread B has to wait for 
        // 100 before it knows 99 is ready.  Maybe insert sequential events?
        {
            std::lock_guard<std::mutex> LockGuard(_eventMutex);

            _d3dFence->SetEventOnCompletion(fenceValue, _fenceEventHandle);
            WaitForSingleObject(_fenceEventHandle, INFINITE);
            _lastCompletedFenceValue = fenceValue;
        }
    }

    // D3D12CommandListManager
    D3D12CommandListManager::D3D12CommandListManager(ID3D12Device* device)
        : _device(device)
        , _graphicsQueue(this, D3D12_COMMAND_LIST_TYPE_DIRECT)
        , _computeQueue(this, D3D12_COMMAND_LIST_TYPE_COMPUTE)
        , _copyQueue(this, D3D12_COMMAND_LIST_TYPE_COPY)
    {

    }

    D3D12CommandListManager::~D3D12CommandListManager()
    {

    }

    void D3D12CommandListManager::WaitIdle()
    {
        _graphicsQueue.WaitForIdle();
        _computeQueue.WaitForIdle();
        _copyQueue.WaitForIdle();
    }

    void D3D12CommandListManager::WaitForFence(uint64_t fenceValue)
    {
        D3D12CommandQueue& producer = GetQueue((D3D12_COMMAND_LIST_TYPE)(fenceValue >> 56));
        producer.WaitForFence(fenceValue);
    }

    void D3D12CommandListManager::CreateNewCommandList(
        D3D12_COMMAND_LIST_TYPE type,
        ID3D12GraphicsCommandList** commandList,
        ID3D12CommandAllocator** allocator)
    {
        ALIMER_ASSERT(type != D3D12_COMMAND_LIST_TYPE_BUNDLE && "Bundles are not yet supported");
        switch (type)
        {
        case D3D12_COMMAND_LIST_TYPE_DIRECT: *allocator = _graphicsQueue.RequestAllocator(); break;
        case D3D12_COMMAND_LIST_TYPE_BUNDLE: break;
        case D3D12_COMMAND_LIST_TYPE_COMPUTE: *allocator = _computeQueue.RequestAllocator(); break;
        case D3D12_COMMAND_LIST_TYPE_COPY: *allocator = _copyQueue.RequestAllocator(); break;
        }

        ThrowIfFailed(_device->CreateCommandList(1, type, *allocator, nullptr, IID_PPV_ARGS(commandList)));
        (*commandList)->SetName(L"CommandList");
    }
}

