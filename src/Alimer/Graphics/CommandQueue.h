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

#include "../foundation/ThreadSafeQueue.h"
#include "../Graphics/CommandBuffer.h"
#include "../Graphics/GPUResource.h"
#include "../Math/MathUtil.h"
#ifdef ALIMER_THREADING
#   include <thread>
#   include <atomic>
#   include <condition_variable>
#endif

namespace alimer
{
    /// Defines a queue that organizes command buffers to be executed by a GPU.
    class ALIMER_API CommandQueue : public GPUResource, public RefCounted
    {
        friend class CommandBuffer;

    public:
        /// Constructor.
        CommandQueue(GraphicsDevice* device, QueueType queueType);

        /// Destructor.
        ~CommandQueue() override;

        /// Unconditionally destroy the GPU resource.
        void Destroy() override;

        /// Request new command buffer.
        SharedPtr<CommandBuffer> GetCommandBuffer();

        /// Submit command buffer and optionally waith
        void Submit(SharedPtr<CommandBuffer> commandBuffer, bool waitForCompletion = false);

#if defined(ALIMER_VULKAN)
        VkCommandPool GetVkCommandPool() const { return _handle; }
#elif defined(ALIMER_D3D12)
#endif

    private:
#ifdef ALIMER_THREADING
        void CheckSubmittedCommandBuffers();
#endif


        void Create();

        QueueType _queueType;
        ThreadSafeQueue<SharedPtr<CommandBuffer>> _availableCommandBuffers;
        uint64_t _fenceValue;

#ifdef ALIMER_THREADING
        std::thread _thread;
        std::atomic_bool _runThread;
        std::mutex _threadMutex;
        std::condition_variable _threadCV;
#endif

#if defined(ALIMER_VULKAN)
        VkCommandPool   _handle = VK_NULL_HANDLE;
        VkFence         _fence = VK_NULL_HANDLE;
#elif defined(ALIMER_D3D12)
#endif
    };
}
