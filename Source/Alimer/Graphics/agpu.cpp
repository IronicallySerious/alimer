//
// Copyright (c) 2018 Amer Koleci and contributors.
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

#include "../Base/Debug.h"
#define AGPU_IMPLEMENTATION
#include "agpu_backend.h"
#include "../Core/Log.h"
#include <vector>

#if defined(_WIN32)
#include <windows.h>

// Prefer the high-performance GPU on switchable GPU systems
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 1;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif /* defined(_WIN32) */

static AGpuRendererI* s_renderer = nullptr;

std::vector<AgpuBackend> agpuGetSupportedBackends()
{
    static std::vector<AgpuBackend> backends;

    if (backends.empty())
    {
        backends.push_back(AGPU_BACKEND_EMPTY);

#if AGPU_D3D11
        if (agpuIsD3D11Supported())
            backends.push_back(AGPU_BACKEND_D3D11);
#endif

#if AGPU_D3D12
        if (agpuIsD3D12Supported())
            backends.push_back(AGPU_BACKEND_D3D12);
#endif
    }

    return backends;
}

AgpuBackend agpuGetDefaultPlatformBackend()
{
#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP || ALIMER_PLATFORM_XBOX_ONE
    if (agpuIsBackendSupported(AGPU_BACKEND_D3D12))
    {
        return AGPU_BACKEND_D3D12;
    }

    return AGPU_BACKEND_D3D11;
#elif ALIMER_PLATFORM_LINUX || ALIMER_PLATFORM_ANDROID
    return AGPU_BACKEND_OPENGL;
#elif ALIMER_PLATFORM_MACOS || ALIMER_PLATFORM_IOS || ALIMER_PLATFORM_TVOS
    return AGPU_BACKEND_METAL;
#else
    return AGPU_BACKEND_OPENGL;
#endif
}

AgpuBool32 agpuIsBackendSupported(AgpuBackend backend)
{
    if (backend == AGPU_BACKEND_DEFAULT)
    {
        backend = agpuGetDefaultPlatformBackend();
    }

    switch (backend)
    {
    case AGPU_BACKEND_EMPTY:
        return AGPU_TRUE;
    case AGPU_BACKEND_VULKAN:
#if AGPU_VULKAN
        return AGPU_TRUE; // VulkanGraphicsDevice::IsSupported();
#else
        return AGPU_FALSE;
#endif
    case AGPU_BACKEND_D3D11:
#if AGPU_D3D11
        return agpuIsD3D11Supported();
#else
        return AGPU_FALSE;
#endif

    case AGPU_BACKEND_D3D12:
#if AGPU_D3D12
        return agpuIsD3D12Supported();
#else
        return AGPU_FALSE;
#endif

    case AGPU_BACKEND_OPENGL:
#if AGPU_OPENGL
        return AGPU_TRUE;
#else
        return AGPU_FALSE;
#endif
    default:
        return AGPU_FALSE;
    }
}

uint32_t agpuGetAvailableBackendsCount()
{
    return (uint32_t)agpuGetSupportedBackends().size();
}

AgpuBackend agpuGetAvailableBackend(uint32_t index)
{
    const std::vector<AgpuBackend> backends = agpuGetSupportedBackends();
    ALIMER_ASSERT(index < backends.size());
    return backends[index];
}

AgpuResult agpuInitialize(const AgpuDescriptor* descriptor)
{
    if (s_renderer != nullptr)
        return AGPU_ALREADY_INITIALIZED;

    AgpuBackend backend = descriptor->preferredBackend;
    if (backend == AGPU_BACKEND_DEFAULT)
    {
        backend = agpuGetDefaultPlatformBackend();
    }

    AgpuResult result = AGPU_ERROR;
    AGpuRendererI* renderer = nullptr;
    switch (backend)
    {
    case AGPU_BACKEND_EMPTY:
        break;
    case AGPU_BACKEND_VULKAN:
        break;
    case AGPU_BACKEND_D3D11:
#if AGPU_D3D11
        result = agpuCreateD3D11Backend(descriptor, &renderer);
#else
        ALIMER_LOGERROR("D3D11 backend is not supported");
        result = AGPU_ERROR;
#endif
        break;
    case AGPU_BACKEND_D3D12:
#if AGPU_D3D12
        result = agpuCreateD3D12Backend(descriptor, &renderer);
#else
        ALIMER_LOGERROR("D3D12 backend is not supported");
        result = AGPU_ERROR;
#endif
        break;
    case AGPU_BACKEND_METAL:
        break;
    case AGPU_BACKEND_OPENGL:
        break;
    default:
        break;
    }

    if (result == AGPU_OK)
    {
        s_renderer = renderer;
        return AGPU_OK;
    }

    return result;
}

void agpuShutdown()
{
    if (s_renderer != nullptr)
    {
        s_renderer->Shutdown();
        delete s_renderer;
        s_renderer = nullptr;
    }
}

uint64_t agpuFrame()
{
    return s_renderer->Frame();
}
