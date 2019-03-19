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
#include "core/Platform.h"
#if defined(_WIN32)
#   ifndef NOMINMAX
#       define NOMINMAX
#   endif
#   ifndef WIN32_LEAN_AND_MEAN
#       define WIN32_LEAN_AND_MEAN
#   endif
#   include <Windows.h>

// Prefer the high-performance GPU on switchable GPU systems
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 1;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

#if ALIMER_D3D12
#   include "graphics/d3d12/GraphicsDeviceD3D12.h"
#endif

#if ALIMER_OPENGL
#   include "graphics/opengl/GraphicsDeviceGL.h"
#endif

namespace alimer
{
    GraphicsDevice::GraphicsDevice()
    {
    }

    GraphicsDevice* GraphicsDevice::Create(const GraphicsDeviceDescriptor* descriptor)
    {
        GraphicsBackend preferredBackend = descriptor->preferredBackend;
        if (preferredBackend == GraphicsBackend::Default)
        {
            preferredBackend = GetDefaultGraphicsPlatform();
        }

        switch (preferredBackend)
        {
        case GraphicsBackend::Null:
            break;
        case GraphicsBackend::Direct3D12:
#if ALIMER_D3D12
            return new GraphicsDeviceD3D12(descriptor->powerPreference, descriptor->validation);
#else
            return nullptr;
#endif
            break;

        case GraphicsBackend::Direct3D11:
#if ALIMER_D3D11
            return new GraphicsDeviceD3D11(validation);
#else
            return nullptr;
#endif
            break;

        case GraphicsBackend::OpenGL:
#if ALIMER_OPENGL
            return new GraphicsDeviceGL(descriptor->validation);
#else
            return nullptr;
#endif
            break;
        default:
            ALIMER_UNREACHABLE();
            break;
        }

        return nullptr;
    }

    SwapChain* GraphicsDevice::CreateSwapChain(SwapChainSurface* surface, const SwapChainDescriptor* descriptor)
    {
        ALIMER_ASSERT(surface);
        ALIMER_ASSERT(descriptor);
        return CreateSwapChainImpl(surface, descriptor);
    }

    GraphicsBackend GetDefaultGraphicsPlatform()
    {
        switch (GetPlatformType())
        {
        case PlatformType::Windows:
            if (IsGraphicsBackendSupported(GraphicsBackend::Direct3D12))
            {
                return GraphicsBackend::Direct3D12;
            }

            if (IsGraphicsBackendSupported(GraphicsBackend::Direct3D11))
            {
                return GraphicsBackend::Direct3D11;
            }

            return GraphicsBackend::OpenGL;

        case PlatformType::UWP:
        case PlatformType::XboxOne:
            if (IsGraphicsBackendSupported(GraphicsBackend::Direct3D12))
            {
                return GraphicsBackend::Direct3D12;
            }

            return GraphicsBackend::Direct3D11;

        case PlatformType::Android:
        case PlatformType::Linux:
            return GraphicsBackend::OpenGL;

        case PlatformType::iOS:
        case PlatformType::MacOS:
            // Using MoltenVK
            return GraphicsBackend::Vulkan;
        default:
            ALIMER_UNREACHABLE();
        }
    }

    bool IsGraphicsBackendSupported(GraphicsBackend backend)
    {
        if (backend == GraphicsBackend::Default)
        {
            backend = GetDefaultGraphicsPlatform();
        }

        switch (backend)
        {
        case GraphicsBackend::Direct3D11:
#if ALIMER_D3D11
            return Platform.PlatformType == PlatformType.Windows
                || Platform.PlatformType == PlatformType.UWP;
#else
            return false;
#endif

        case GraphicsBackend::Direct3D12:
#if ALIMER_D3D12
            return false;
            //return D3D12.DeviceD3D12.IsSupported();
#else
            return false;
#endif

        case GraphicsBackend::Vulkan:
#if ALIMER_VULKAN
            return true;
#else
            return false;
#endif
        case GraphicsBackend::OpenGL:
#if ALIMER_OPENGL
            return true;
#else
            return false;
#endif

        default:
            return false;
        }
    }
}
