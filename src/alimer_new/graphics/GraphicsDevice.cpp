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

#if defined(ALIMER_VULKAN)
#   include "vulkan/GraphicsDeviceVk.h"
#endif

#if defined(ALIMER_D3D12)
#   include "d3d12/GraphicsDeviceD3D12.h"
#endif

namespace alimer
{
    std::shared_ptr<GraphicsDevice> graphicsDevice;

    GraphicsDevice::GraphicsDevice()
    {

    }

    std::shared_ptr<GraphicsDevice> GraphicsDevice::Create(GraphicsBackend preferredBackend, GpuPowerPreference powerPreference)
    {
        if (graphicsDevice)
        {
            ALIMER_LOGERROR("Currently can create only one GraphicsDevice instance");
            return nullptr;
        }

        if (preferredBackend == GraphicsBackend::Default)
        {
#if defined(ALIMER_D3D12)
            preferredBackend = GraphicsBackend::Direct3D12;
#elif defined(ALIMER_VULKAN)
            preferredBackend = GraphicsBackend::Vulkan;
#endif
        }

        switch (preferredBackend)
        {
        case GraphicsBackend::Null:
            break;
        case GraphicsBackend::Vulkan:
#if defined(ALIMER_VULKAN)
            graphicsDevice = std::shared_ptr<GraphicsDevice>(new GraphicsDeviceVk(powerPreference));
            ALIMER_LOGINFO("Using Vulkan graphics backend.");
#endif
            break;
        case GraphicsBackend::Direct3D12:
#if defined(ALIMER_D3D12)
            graphicsDevice = std::shared_ptr<GraphicsDevice>(new GraphicsDeviceD3D12(powerPreference));
            ALIMER_LOGINFO("Using Direct3D12 graphics backend.");
#endif
            break;
        case GraphicsBackend::Direct3D11:
            break;
        default:
            break;
        }

        return graphicsDevice;
    }

    SwapChain* GraphicsDevice::CreateSwapChain(const SwapChainSurface* surface, const SwapChainDescriptor* descriptor)
    {
        ALIMER_ASSERT(surface);
        ALIMER_ASSERT(descriptor);

#if defined(_WIN64) || defined(_WIN32)
        if (!IsWindow(reinterpret_cast<HWND>(surface->window)))
        {
            ALIMER_ASSERT_MSG(false, "Invalid hWnd handle");
        }
#endif

        return CreateSwapChainImpl(surface, descriptor);
    }

    SwapChain* GraphicsDevice::CreateSwapChain(Window* window)
    {
        SwapChainSurface surface = {};
#if defined(_WIN64) || defined(_WIN32)
        surface.hInstance = window->GetNativeConnection();
        surface.window = window->GetNativeHandle();
#elif defined(__ANDROID__)
        surface.window = window->GetNativeHandle();
#elif defined(__linux__)
        surface.display = window->GetNativeConnection();
        surface.window = window->GetNativeHandle();
#elif defined(__APPLE__)
        surface.layer = window->GetNativeHandle();
#endif

        SwapChainDescriptor descriptor = {};
        descriptor.width = window->GetWidth();
        descriptor.height = window->GetHeight();
        return CreateSwapChain(&surface, &descriptor);
    }

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

    const GraphicsDeviceInfo& GraphicsDevice::GetInfo() const
    {
        return _info;
    }

    const GraphicsDeviceCapabilities& GraphicsDevice::GetCaps() const
    {
        return _caps;
    }
}
