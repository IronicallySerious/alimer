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

#include "graphics/Graphics.h"
#include "graphics/SwapChain.h"
#include "graphics/CommandBuffer.h"
#include "engine/Window.h"
#include "core/Log.h"

#if defined(_WIN64) || defined(_WIN32)
#   include <Windows.h>

// Prefer the high-performance GPU on switchable GPU systems
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 1;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

#include "GraphicsImpl.h"

namespace alimer
{
    Graphics::Graphics()
        : _impl(new GraphicsImpl())
    {

    }

    Graphics::~Graphics()
    {
    }

    bool Graphics::BeginFrame()
    {
        return _impl->BeginFrame();
    }

    uint32_t Graphics::EndFrame()
    {
        _impl->EndFrame();
        return _frameCount++;
    }

#if TODO
    std::shared_ptr<CommandQueue> GraphicsDevice::GetCommandQueue(CommandQueueType type) const
    {
        std::shared_ptr<CommandQueue> commandQueue;
        switch (type)
        {
        case CommandQueueType::Direct:
            commandQueue = _directCommandQueue;
            break;
        case CommandQueueType::Compute:
            commandQueue = _computeCommandQueue;
            break;
        case CommandQueueType::Copy:
            commandQueue = _copyCommandQueue;
            break;
        default:
            ALIMER_ASSERT(false && "Invalid command queue type.");
        }

        return commandQueue;
    }
#endif // TODO

    const GraphicsDeviceInfo& Graphics::GetInfo() const
    {
        return _impl->GetInfo();
    }

    const GraphicsDeviceCapabilities& Graphics::GetCaps() const
    {
        return _impl->GetCaps();
    }
}
