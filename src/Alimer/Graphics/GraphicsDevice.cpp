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

#include "../Graphics/GraphicsDevice.h"
#include "../Graphics/CommandContext.h"
#include "../Graphics/Texture.h"
#include "../Graphics/Sampler.h"
#include "../Application/Window.h"
#include "Core/Log.h"

#if defined(ALIMER_VULKAN)
#include "vulkan/GPUDeviceVk.h"
#endif

#if defined(ALIMER_D3D12)
#include "d3d12/GraphicsDeviceD3D12.h"
#endif

#if defined(_WIN32)
#include <windows.h>

// Prefer the high-performance GPU on switchable GPU systems
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 1;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif /* defined(_WIN32) */

using namespace std;

namespace alimer
{
    GraphicsDevice* graphics = nullptr;

    GraphicsDevice::GraphicsDevice(GraphicsBackend backend, PhysicalDevicePreference devicePreference, bool validation)
        : _backend(backend)
        , _devicePreference(devicePreference)
        , _validation(validation)
    {
        graphics = this;
        AddSubsystem(this);
    }

    GraphicsDevice::~GraphicsDevice()
    {
        // Destroy undestroyed resources.
        SafeDelete(_pointSampler);
        SafeDelete(_linearSampler);

        if (_gpuResources.size() > 0)
        {
            lock_guard<mutex> lock(_gpuResourceMutex);
            for (auto it = _gpuResources.begin(); it != _gpuResources.end(); ++it)
            {
                (*it)->Destroy();
            }

            _gpuResources.clear();
        }

        // Destroy backend.
        Finalize();

        RemoveSubsystem(this);
        graphics = nullptr;
    }

    GraphicsDevice* GraphicsDevice::Create(const char* applicationName, const GraphicsDeviceDescriptor* descriptor)
    {
        if (graphics != nullptr)
        {
            ALIMER_LOGCRITICAL("Cannot create multiple instance of GraphicsDevice");
        }

        GraphicsBackend preferredBackend = descriptor->preferredBackend;
        if (preferredBackend == GraphicsBackend::Default) {
            preferredBackend = GetDefaultPlatformBackend();
        }

        GraphicsDevice* device = nullptr;
        switch (preferredBackend)
        {
        case GraphicsBackend::Vulkan:
#if defined(ALIMER_VULKAN)
            if (GraphicsDeviceVk::IsSupported())
            {
                device = new GraphicsDeviceVk(applicationName, descriptor->devicePreference, descriptor->validation);
                ALIMER_LOGINFO("Vulkan backend created with success.");
            }
            else
#else
            {
                ALIMER_LOGERROR("Vulkan backend is not supported.");
            }
#endif

        case GraphicsBackend::Direct3D12:
#if defined(ALIMER_D3D12)
            if (GraphicsDeviceD3D12::IsSupported())
            {
                device = new GraphicsDeviceD3D12(descriptor->devicePreference, descriptor->validation);
                ALIMER_LOGINFO("Direct3D12 backend created with success.");
            }
            else
#else
            {
                ALIMER_LOGERROR("Direct3D12 backend is not supported.");
            }
#endif

        default:
            break;
        }

        return device;
    }

    GraphicsBackend GraphicsDevice::GetDefaultPlatformBackend()
    {
        switch (GetPlatformType())
        {
        case PlatformType::Windows:
        case PlatformType::UWP:
        case PlatformType::XboxOne:
            //if (IsBackendSupported(GraphicsBackend::Direct3D12))
            //{
            //    return GraphicsBackend::Direct3D12;
            //}

            if (IsBackendSupported(GraphicsBackend::Vulkan))
            {
                return GraphicsBackend::Vulkan;
            }

            return GraphicsBackend::Direct3D11;

        case PlatformType::Android:
        case PlatformType::Linux:
            if (IsBackendSupported(GraphicsBackend::Vulkan))
            {
                return GraphicsBackend::Vulkan;
            }
            return GraphicsBackend::OpenGL;

        case PlatformType::iOS:
        case PlatformType::MacOS:
        case PlatformType::AppleTV:
            return GraphicsBackend::OpenGL;

        case PlatformType::Web:
            return GraphicsBackend::OpenGL;

        default:
            return GraphicsBackend::Null;
        }
    }

    bool GraphicsDevice::IsBackendSupported(GraphicsBackend backend)
    {
        if (backend == GraphicsBackend::Default)
        {
            backend = GetDefaultPlatformBackend();
        }

        switch (backend)
        {
        case GraphicsBackend::Null:
            return true;

        case GraphicsBackend::Vulkan:
#if defined(ALIMER_VULKAN)
            return GraphicsDeviceVk::IsSupported();
#else
            return false;
#endif

        case GraphicsBackend::Direct3D12:
#if defined(ALIMER_D3D12)
            return GraphicsDeviceD3D12::IsSupported();
#else
            return false;
#endif

        case GraphicsBackend::Direct3D11:
#if defined(ALIMER_D3D11)
            return false; // GraphicsDeviceD3D11::IsSupported();
#else
            return false;
#endif

        case GraphicsBackend::OpenGL:
            return false;

        default:
            return false;
        }
    }

    bool GraphicsDevice::Initialize(const SwapChainDescriptor* descriptor)
    {
        if (_initialized) {
            return true;
        }

        if (!InitializeImpl(descriptor))
        {
            return false;
        }

        _initialized = true;
        return _initialized;
    }

    bool GraphicsDevice::BeginFrame()
    {
        if (!BeginFrameImpl()) {
            return false;
        }

        _frameId++;
        return true;
    }

    uint32_t GraphicsDevice::EndFrame()
    {
        EndFrame(_frameId);
        return _frameId;
    }

    void GraphicsDevice::OnAfterCreated()
    {
        // TODO: add stock samplers
        //SamplerDescriptor descriptor = {};
        //_pointSampler = CreateSampler(&descriptor);

        //descriptor.magFilter = SamplerMinMagFilter::Linear;
        //descriptor.minFilter = SamplerMinMagFilter::Linear;
        //descriptor.mipmapFilter = SamplerMipFilter::Linear;
        //_linearSampler = CreateSampler(&descriptor);
    }

    void GraphicsDevice::TrackResource(GPUResource* resource)
    {
        unique_lock<mutex> lock(_gpuResourceMutex);
        _gpuResources.push_back(resource);
    }

    void GraphicsDevice::UntrackResource(GPUResource* resource)
    {
        unique_lock<mutex> lock(_gpuResourceMutex);
        _gpuResources.erase(
            std::remove(_gpuResources.begin(), _gpuResources.end(), resource),
            end(_gpuResources)
        );
    }
}
