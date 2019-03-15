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

#include "graphics/GraphicsDeviceFactory.h"
#if defined(_WIN32)
#   ifndef NOMINMAX
#       define NOMINMAX
#   endif
#   ifndef WIN32_LEAN_AND_MEAN
#       define WIN32_LEAN_AND_MEAN
#   endif
#   include <Windows.h>
#endif

#if ALIMER_D3D12
#   include "graphics/d3d12/GraphicsDeviceFactoryD3D12.h"
#endif

namespace alimer
{
    GraphicsDeviceFactory::GraphicsDeviceFactory(GraphicsBackend backend)
        : _backend(backend)
    {
    }

    GraphicsDeviceFactory* GraphicsDeviceFactory::Create(GraphicsBackend preferredBackend, bool validation)
    {
        if (preferredBackend == GraphicsBackend::Default)
        {
#if ALIMER_D3D12
            preferredBackend = GraphicsBackend::Direct3D12;
#endif
        }

        switch (preferredBackend)
        {
        case GraphicsBackend::Null:
            break;
        case GraphicsBackend::Direct3D12:
#if ALIMER_D3D12
            return new GraphicsDeviceFactoryD3D12(validation);
#else
            return nullptr;
#endif
            break;
        default:
            ALIMER_UNREACHABLE();
            break;
        }
    }

    GraphicsDevice* GraphicsDeviceFactory::CreateDevice(const AdapterDescriptor* adapterDescription)
    {
        ALIMER_ASSERT(adapterDescription);

        return CreateDeviceImpl(adapterDescription);
    }

    SwapChainSurface* GraphicsDeviceFactory::CreateSurfaceFromWin32(void *hInstance, void *hWnd)
    {
#if defined(_WIN32)
        if (!IsWindow((HWND)hWnd))
        {
            ALIMER_ASSERT_MSG(false, "Invalid hWnd handle");
        }

        return CreateSurfaceFromWin32Impl(hInstance, hWnd);
#else
        ALIMER_ASSERT_MSG(false, "CreateSurfaceFromWin32 is supported only on Win32 operating system");
        return nullptr;
#endif
    }
}
