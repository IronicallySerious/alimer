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

#pragma once

#include "BackendD3D12.h"
#include "graphics/CommandQueue.h"
#include <atomic>
#include <cstdint>
#include <condition_variable>
#include "foundation/ThreadSafeQueue.h"

namespace alimer
{
    class ALIMER_API CommandQueueD3D12 final : public CommandQueue
    {
    public:
        CommandQueueD3D12(GraphicsDeviceD3D12* device, CommandQueueType type);
        ~CommandQueueD3D12() override;

        std::shared_ptr<CommandBuffer> GetCommandBuffer() override;
        uint64_t Submit(std::shared_ptr<CommandBuffer> commandBuffer) override;

        uint64_t Signal();
        bool IsFenceComplete(uint64_t fenceValue);
        void WaitForFenceValue(uint64_t fenceValue);
        void Flush();

        ID3D12CommandQueue* GetD3D12CommandQueue() const {
            return _d3d12CommandQueue.Get();
        }

    private:
        // Free any command lists that are finished processing on the command queue.
        void ProccessInFlightCommandLists();

        D3D12_COMMAND_LIST_TYPE _commandListType;
        ComPtr<ID3D12CommandQueue> _d3d12CommandQueue;
        ComPtr<ID3D12Fence> _d3d12Fence;
        std::atomic_uint64_t _fenceValue;

        using CommandListEntry = std::tuple<uint64_t, std::shared_ptr<CommandBuffer>>;

        ThreadSafeQueue<CommandListEntry> _inFlightCommandLists;
        ThreadSafeQueue<std::shared_ptr<CommandBuffer>>  _availableCommandBuffers;

        // A thread to process in-flight command lists.
        std::thread _processInFlightCommandListsThread;
        std::atomic_bool _processInFlightCommandLists;
        std::mutex _processInFlightCommandListsThreadMutex;
        std::condition_variable _processInFlightCommandListsThreadCV;
    };
}
