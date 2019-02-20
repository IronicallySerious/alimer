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
#include "../Graphics/Texture.h"
#include "../Graphics/Sampler.h"
#include "../Application/Window.h"
#include "Core/Log.h"

#if defined(ALIMER_VULKAN)
#include "vulkan/GraphicsDeviceVk.h"
#elif defined(ALIMER_D3D12)
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

    GraphicsDevice::GraphicsDevice(const char* applicationName, const GraphicsDeviceDescriptor* descriptor)
        : _devicePreference(descriptor->devicePreference)
        , _validation(descriptor->validation)
        , _headless(descriptor->headless)
        , _impl(new GraphicsImpl(applicationName, descriptor))
    {
        _backend = _impl->GetBackend();

        // Create command queue's
        _directCommandQueue = MakeShared<CommandQueue>(this, QueueType::Direct);
        _computeCommandQueue = MakeShared<CommandQueue>(this, QueueType::Compute);
        _copyCommandQueue = MakeShared<CommandQueue>(this, QueueType::Copy);

        graphics = this;
        AddSubsystem(this);
    }

    GraphicsDevice::~GraphicsDevice()
    {
        _impl->WaitIdle();

        _swapChain.Reset();
        _directCommandQueue.Reset();
        _computeCommandQueue.Reset();
        _copyCommandQueue.Reset();

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
        SafeDelete(_impl);

        RemoveSubsystem(this);
        graphics = nullptr;
    }

    GraphicsDevice* GraphicsDevice::Create(const char* applicationName, const GraphicsDeviceDescriptor* descriptor)
    {
        if (graphics != nullptr)
        {
            ALIMER_LOGCRITICAL("Cannot create multiple instance of GraphicsDevice");
        }

        if (!GraphicsDevice::IsSupported())
        {
            ALIMER_LOGERROR("Vulkan backend is not supported.");
            return nullptr;
        }

        GraphicsDevice* device = new GraphicsDevice(applicationName, descriptor);
        ALIMER_LOGINFO("Vulkan backend created with success.");
        return device;
    }

    bool GraphicsDevice::Initialize(const SwapChainDescriptor* descriptor)
    {
        if (_initialized) {
            return true;
        }

        _swapChain = new SwapChain(this, descriptor);

        OnAfterCreated();
        _initialized = true;
        return _initialized;
    }

    bool GraphicsDevice::BeginFrame()
    {
        if (!_impl->BeginFrame()) {
           return false;
        }

        _frameId++;
        return true;
    }

    uint32_t GraphicsDevice::EndFrame()
    {
        _impl->EndFrame();
        _frameId++;
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

    const GraphicsDeviceFeatures& GraphicsDevice::GetFeatures() const
    {
        return _impl->GetFeatures();
    }

    const GraphicsDeviceLimits& GraphicsDevice::GetLimits() const
    {
        return _impl->GetLimits();
    }

    SharedPtr<CommandQueue> GraphicsDevice::GetCommandQueue(QueueType queueType) const
    {
        SharedPtr<CommandQueue> commandQueue;
        switch (queueType)
        {
        case QueueType::Direct:
            commandQueue = _directCommandQueue;
            break;
        case QueueType::Compute:
            commandQueue = _computeCommandQueue;
            break;
        case QueueType::Copy:
            commandQueue = _copyCommandQueue;
            break;
        default:
            assert(false && "Invalid command queue type.");
        }

        return commandQueue;
    }
}
