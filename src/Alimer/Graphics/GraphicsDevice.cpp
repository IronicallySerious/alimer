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
#include "Core/Log.h"

#if defined(_WIN32)
#include <windows.h>

// Prefer the high-performance GPU on switchable GPU systems
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 1;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif /* defined(_WIN32) */

#include "AlimerConfig.h"

#if ALIMER_D3D11
#   include "d3d11/GraphicsDeviceD3D11.h"
#endif

#if defined(ALIMER_D3D12)
#include "d3d12/GraphicsDeviceD3D12.h"
#endif

#if defined(ALIMER_VULKAN)
#   include "vulkan/GraphicsDeviceVk.h"
#endif

using namespace std;

namespace alimer
{
    GraphicsDevice::GraphicsDevice(const char* applicationName, GpuPreference devicePreference)
    {
        _impl = new GraphicsImpl(applicationName, devicePreference);
        _renderWindow = new Window();

        RegisterObject();
        AddSubsystem(this);
    }

    GraphicsDevice::~GraphicsDevice()
    {
        Finalize();
        RemoveSubsystem(this);
    }

    void GraphicsDevice::Finalize()
    {
        if (!_initialized) {
            return;
        }

        _initialized = false;
        _renderContext.Reset();
        _renderWindow.Reset();

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
    }

    void GraphicsDevice::RegisterObject()
    {
        static bool registered = false;
        if (registered) {
            return;
        }

        registered = true;
        Shader::RegisterObject();
        Texture::RegisterObject();
        Sampler::RegisterObject();
    }

    GraphicsDevice* GraphicsDevice::Create(const char* applicationName, GpuPreference devicePreference)
    {
        auto graphics = GetSubsystem<GraphicsDevice>();
        if (graphics != nullptr)
        {
            ALIMER_LOGCRITICAL("Cannot create multiple instance of GraphicsDevice");
        }

        if (!GraphicsImpl::IsSupported())
        {
            ALIMER_LOGERROR("Vulkan backend is not supported.");
            return nullptr;
        }

        return new GraphicsDevice(applicationName, devicePreference);
    }

    bool GraphicsDevice::SetMode(const IntVector2& size, bool resizable, bool fullscreen, SampleCount samples)
    {
        if (!_renderWindow->SetSize(size, resizable, fullscreen)) {
            return false;
        }

        if (!_initialized)
        {
            _initialized = _impl->Initialize(_renderWindow.Get(), samples);
            return true;
        }

        return _initialized;
    }

    bool GraphicsDevice::BeginFrame()
    {
        return true;
    }

    void GraphicsDevice::EndFrame()
    {
        //Tick();
    }

    GraphicsBackend GraphicsDevice::GetBackend() const
    {
        return _impl->GetInfo().backend;
    }

    const GraphicsDeviceInfo& GraphicsDevice::GetInfo() const
    {
        return _impl->GetInfo();
    }

    const GraphicsDeviceCapabilities& GraphicsDevice::GetCaps() const
    {
        return _impl->GetCaps();
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

    BufferHandle* GraphicsDevice::CreateBuffer(const BufferDescriptor* descriptor, const void* pInitData)
    {
        ALIMER_ASSERT(descriptor);
        return nullptr;
        //return CreateBufferImpl(descriptor, pInitData);
    }

    ShaderHandle* GraphicsDevice::CreateShader(ShaderStage stage, const std::string& code, const std::string& entryPoint)
    {
        //return CreateShaderImpl(stage, code, entryPoint);
        return nullptr;
    }

    PipelineHandle* GraphicsDevice::CreateRenderPipeline(const RenderPipelineDescriptor* descriptor)
    {
        ALIMER_ASSERT(descriptor);
        //return CreateRenderPipelineImpl(descriptor);
        return nullptr;
    }
}
