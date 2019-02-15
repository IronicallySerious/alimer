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
//#   include "vulkan/GPUDeviceVk.h"
#endif

#if defined(ALIMER_D3D12)
#   include "d3d12/GraphicsDeviceD3D12.h"
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

    GraphicsDevice* GraphicsDevice::Create(GraphicsBackend preferredBackend, PhysicalDevicePreference devicePreference, bool validation)
    {
        if (graphics != nullptr)
        {
            ALIMER_LOGCRITICAL("Cannot create multiple instance of GraphicsDevice");
        }

        if (preferredBackend == GraphicsBackend::Default) {
            preferredBackend = GraphicsBackend::D3D12;
        }

        GraphicsDevice* device = nullptr;
        switch (preferredBackend)
        {
        case GraphicsBackend::D3D12:
#if defined(ALIMER_D3D12)
            if (GraphicsDeviceD3D12::IsSupported())
            {
                device = new GraphicsDeviceD3D12(devicePreference, validation);
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

        for (uint32_t i = 0u; i < 4u; ++i)
        {
            _contextPool[i].clear();
        }

        // Destroy backend.
        Finalize();

        RemoveSubsystem(this);
        graphics = nullptr;
    }

    bool GraphicsDevice::Initialize(const SwapChainDescriptor* descriptor)
    {
        if (_initialized) {
            return true;
        }

        if (!InitializeImpl("Alimer", descriptor))
        {
            return false;
        }

        _initialized = true;
        return _initialized;
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

    bool GraphicsDevice::BeginFrame()
    {
        if (_inBeginFrame)
        {
            ALIMER_LOGCRITICAL("Cannot nest BeginFrame calls, call EndFrame first.");
        }

        if (!BeginFrameImpl())
        {
            ALIMER_LOGCRITICAL("Failed to begin rendering frame.");
        }

        _inBeginFrame = true;
        return true;
    }

    uint64_t GraphicsDevice::Frame()
    {
        if (!_inBeginFrame)
        {
            ALIMER_LOGCRITICAL("BeginFrame must be called before EndFrame.");
        }

        // Tick backend
        EndFrameImpl();

        _inBeginFrame = false;
        return ++_frameIndex;
    }

    void GraphicsDevice::WaitIdle()
    {
        WaitIdleImpl();
    }

    Framebuffer* GraphicsDevice::GetDefaultFramebuffer() const {
        return nullptr;
    }

    CommandContext* GraphicsDevice::AllocateContext(QueueType type) {
        lock_guard<mutex> lock(_contextAllocationMutex);

        auto& availableContexts = _availableContexts[(uint32_t)type];

        CommandContext* ret = nullptr;
        if (availableContexts.empty())
        {
            ret = nullptr; // CreateCommandContext(type);
            _contextPool[(uint32_t)type].emplace_back(ret);
            ALIMER_LOGDEBUG("CommandContext allocated");
        }
        else
        {
            ret = availableContexts.front();
            availableContexts.pop();
            ret->Reset();
        }
        ALIMER_ASSERT(ret != nullptr);
        ALIMER_ASSERT(ret->GetQueueType() == type);

        return ret;
    }

    void GraphicsDevice::FreeContext(CommandContext* context)
    {
        ALIMER_ASSERT(context != nullptr);
        lock_guard<mutex> lock(_contextAllocationMutex);
        _availableContexts[(uint32_t)context->GetQueueType()].push(context);
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
