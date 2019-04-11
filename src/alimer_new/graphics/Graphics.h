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

#include "graphics/CommandQueue.h"

namespace alimer
{
    class Window;
    class SwapChain;
    class CommandQueue;
    class CommandBuffer;
    class GraphicsImpl;

    class ALIMER_API Graphics final
    {
    public:
        /// Constructor.
        Graphics();

        /// Destructor.
        ~Graphics();

        bool BeginFrame();
        uint32_t EndFrame();

        /// Get queue.
        std::shared_ptr<CommandQueue> GetCommandQueue(CommandQueueType type = CommandQueueType::Direct) const;

        /// Get the device info.
        const GraphicsDeviceInfo& GetInfo() const;

        /// Get the device capabilities.
        const GraphicsDeviceCapabilities& GetCaps() const;

    private:
        GraphicsImpl* _impl = nullptr;
        uint32_t _frameCount = 0;

    protected:
        std::shared_ptr<CommandQueue> _directCommandQueue;
        std::shared_ptr<CommandQueue> _computeCommandQueue;
        std::shared_ptr<CommandQueue> _copyCommandQueue;
    };
}
