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

#include "../Graphics/CommandQueue.h"
#include "../Graphics/GraphicsDevice.h"

namespace alimer
{
    CommandQueue::CommandQueue(GraphicsDevice* device, QueueType queueType)
        : GPUResource(device, Type::CommandQueue)
        , _queueType(queueType)
        , _fenceValue(0)
    {
        Create();

#ifdef ALIMER_THREADING
        _runThread = true;
        _thread = std::thread(&CommandQueue::CheckSubmittedCommandBuffers, this);
#endif
    }

    CommandQueue::~CommandQueue()
    {
        Destroy();
    }

    SharedPtr<CommandBuffer> CommandQueue::GetCommandBuffer()
    {
        SharedPtr<CommandBuffer> commandBuffer;

        // If there is a command list on the queue.
        if (!_availableCommandBuffers.Empty())
        {
            _availableCommandBuffers.TryPop(commandBuffer);
        }
        else
        {
            // Otherwise create a new command list.
            commandBuffer = new CommandBuffer(this);
        }

        commandBuffer->Begin();

        return commandBuffer;
    }

#ifdef ALIMER_THREADING
    void CommandQueue::CheckSubmittedCommandBuffers()
    {
        std::unique_lock<std::mutex> lock(_threadMutex, std::defer_lock);

        while (_runThread)
        {
            lock.lock();

            _threadCV.notify_one();
            lock.unlock();
            std::this_thread::yield();
        }
    }
#endif
}
