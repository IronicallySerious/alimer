//
// Copyright (c) 2017-2019 Amer Koleci and contributors.
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

#include "CommandQueueD3D12.h"
#include "CommandBufferD3D12.h"
#include "GraphicsDeviceD3D12.h"
using namespace std;

namespace alimer
{
    CommandQueueD3D12::CommandQueueD3D12(GraphicsDeviceD3D12* device, CommandQueueType type)
        : CommandQueue(device, type)
        , _fenceValue(0)
    {
        D3D12_COMMAND_QUEUE_DESC desc = {};
        switch (type)
        {
        default:
        case CommandQueueType::Direct:
            _commandListType = D3D12_COMMAND_LIST_TYPE_DIRECT;
            break;
        case CommandQueueType::Compute:
            _commandListType = D3D12_COMMAND_LIST_TYPE_COMPUTE;
            break;
        case CommandQueueType::Copy:
            _commandListType = D3D12_COMMAND_LIST_TYPE_COPY;
            break;
        }

        desc.Type = _commandListType;
        desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask = 0;

        ThrowIfFailed(device->GetD3DDevice()->CreateCommandQueue(&desc, IID_PPV_ARGS(&_d3d12CommandQueue)));
        ThrowIfFailed(device->GetD3DDevice()->CreateFence(_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_d3d12Fence)));

        switch (_commandListType)
        {
        case D3D12_COMMAND_LIST_TYPE_COPY:
            _d3d12CommandQueue->SetName(L"Copy Command Queue");
            break;
        case D3D12_COMMAND_LIST_TYPE_COMPUTE:
            _d3d12CommandQueue->SetName(L"Compute Command Queue");
            break;
        case D3D12_COMMAND_LIST_TYPE_DIRECT:
            _d3d12CommandQueue->SetName(L"Direct Command Queue");
            break;
        }

        _processInFlightCommandListsThread = std::thread(&CommandQueueD3D12::ProccessInFlightCommandLists, this);
    }

    CommandQueueD3D12::~CommandQueueD3D12()
    {
        _processInFlightCommandLists = false;
        _processInFlightCommandListsThread.join();
    }

    shared_ptr<CommandBuffer> CommandQueueD3D12::GetCommandBuffer()
    {
        shared_ptr<CommandBuffer> commandBuffer;

        // Try to get existing command buffer.
        if (!_availableCommandBuffers.Empty())
        {
            _availableCommandBuffers.TryPop(commandBuffer);
        }
        else
        {
            // Otherwise create a new command list.
            commandBuffer = std::make_shared<CommandBufferD3D12>(static_cast<GraphicsDeviceD3D12*>(_graphicsDevice), _commandListType);
        }

        return commandBuffer;
    }

    uint64_t CommandQueueD3D12::Submit(shared_ptr<CommandBuffer> commandBuffer)
    {
        ID3D12GraphicsCommandList* d3dCommandList = static_pointer_cast<CommandBufferD3D12>(commandBuffer)->GetD3D12CommandList();
        d3dCommandList->Close();

        ID3D12CommandList* ppCommandLists[] = { d3dCommandList };
        _d3d12CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
        uint64_t fenceValue = Signal();

        _inFlightCommandLists.Push({ fenceValue, commandBuffer });

        return fenceValue;
    }

    uint64_t CommandQueueD3D12::Signal()
    {
        uint64_t fenceValue = ++_fenceValue;
        _d3d12CommandQueue->Signal(_d3d12Fence.Get(), fenceValue);
        return fenceValue;
    }

    bool CommandQueueD3D12::IsFenceComplete(uint64_t fenceValue)
    {
        return _d3d12Fence->GetCompletedValue() >= fenceValue;
    }

    void CommandQueueD3D12::WaitForFenceValue(uint64_t fenceValue)
    {
        if (!IsFenceComplete(fenceValue))
        {
            auto event = ::CreateEvent(NULL, FALSE, FALSE, NULL);
            ALIMER_ASSERT(event && "Failed to create fence event handle.");

            // Is this function thread safe?
            _d3d12Fence->SetEventOnCompletion(fenceValue, event);
            ::WaitForSingleObject(event, DWORD_MAX);

            ::CloseHandle(event);
        }
    }

    void CommandQueueD3D12::Flush()
    {
        std::unique_lock<std::mutex> lock(_processInFlightCommandListsThreadMutex);
        _processInFlightCommandListsThreadCV.wait(lock, [this] { return _inFlightCommandLists.Empty(); });

        // In case the command queue was signaled directly 
        // using the CommandQueue::Signal method then the 
        // fence value of the command queue might be higher than the fence
        // value of any of the executed command lists.
        WaitForFenceValue(_fenceValue);
    }

    void CommandQueueD3D12::ProccessInFlightCommandLists()
    {
        std::unique_lock<std::mutex> lock(_processInFlightCommandListsThreadMutex, std::defer_lock);

        while (_processInFlightCommandLists)
        {
            CommandListEntry commandListEntry;

            lock.lock();
            while (_inFlightCommandLists.TryPop(commandListEntry))
            {
                auto fenceValue = std::get<0>(commandListEntry);
                auto commandBuffer = std::get<1>(commandListEntry);

                WaitForFenceValue(fenceValue);

                static_pointer_cast<CommandBufferD3D12>(commandBuffer)->Reset();

                _availableCommandBuffers.Push(commandBuffer);
            }
            lock.unlock();
            _processInFlightCommandListsThreadCV.notify_one();

            std::this_thread::yield();
        }
    }
}
