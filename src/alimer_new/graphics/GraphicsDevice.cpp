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

#include "graphics/GraphicsDevice.h"
#include "graphics/SwapChain.h"
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

#if defined(ALIMER_VULKAN)
#   include "vulkan/GraphicsDeviceVk.h"
#endif

namespace alimer
{
    std::shared_ptr<GraphicsDevice> graphicsDevice;

    GraphicsDevice::GraphicsDevice()
    {

    }

    std::shared_ptr<GraphicsDevice> GraphicsDevice::Create(const std::shared_ptr<Window>& window, const GraphicsDeviceDescriptor& desc)
    {
        if (graphicsDevice)
        {
            LOGE("Currently can create only one GraphicsDevice instance");
            return nullptr;
        }


#if defined(ALIMER_VULKAN)
        graphicsDevice = std::shared_ptr<GraphicsDevice>(new GraphicsDeviceVk());
#endif

        if (graphicsDevice->Initialize(window, desc) == false) {
            graphicsDevice = nullptr;
        }

        return graphicsDevice;
    }

    bool GraphicsDevice::Initialize(const std::shared_ptr<Window>& window, const GraphicsDeviceDescriptor& desc)
    {
        if (_initialized)
        {
            return true;
        }

        _window = window;
        _descriptor = desc;
        _initialized = true;
        return _initialized;
    }

    bool GraphicsDevice::BeginFrame()
    {
        return true;
    }

    void GraphicsDevice::EndFrame()
    {
    }

    const GraphicsDeviceInfo& GraphicsDevice::GetInfo() const
    {
        return _info;
    }

    const GraphicsDeviceCapabilities& GraphicsDevice::GetCaps() const
    {
        return _caps;
    }
}
