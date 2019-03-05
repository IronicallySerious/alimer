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

#if ALIMER_D3D11
#   include "d3d11/GraphicsDeviceD3D11.h"
#endif

#if defined(ALIMER_D3D12)
#include "d3d12/GraphicsDeviceD3D12.h"
#endif

#if defined(ALIMER_VULKAN)
#   include "vulkan/GraphicsDeviceVk.h"
#endif

#if defined(ALIMER_GLFW)
#   define GLFW_INCLUDE_NONE 
#   include <GLFW/glfw3.h>
#endif

using namespace std;

namespace alimer
{
    GraphicsDevice::GraphicsDevice(GraphicsBackend backend, const GraphicsDeviceDescriptor* descriptor)
        : _backend(backend)
        , _devicePreference(descriptor->devicePreference)
        , _validation(descriptor->validation)
    {
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
        _renderWindow.reset();

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

    GraphicsDevice* GraphicsDevice::Create(const char* applicationName, const GraphicsDeviceDescriptor* descriptor)
    {
        auto graphics = GetSubsystem<GraphicsDevice>();
        if (graphics != nullptr)
        {
            ALIMER_LOGCRITICAL("Cannot create multiple instance of GraphicsDevice");
        }

        GraphicsBackend preferredBackend = descriptor->preferredBackend;
        if (preferredBackend == GraphicsBackend::Count) {
            preferredBackend = GraphicsBackend::Direct3D11;
        }

#if defined(ALIMER_GLFW)
        if (preferredBackend != GraphicsBackend::OpenGL)
        {
            // Disable opengl context by default creation.
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        }
        else
        {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        }
#endif

        GraphicsDevice* device = nullptr;
        switch (preferredBackend)
        {
        case GraphicsBackend::Direct3D11:
#if ALIMER_D3D11
            if (!GraphicsDeviceD3D11::IsSupported())
            {
                ALIMER_LOGERROR("Direct3D11 backend is not supported.");
                return nullptr;
            }

            device = new GraphicsDeviceD3D11(descriptor);
            ALIMER_LOGINFO("Direct3D11 backend created with success.");
#else
            ALIMER_LOGERROR("Direct3D11 backend is not supported.");
#endif
            break;

        default:
            break;
        }
        
        return device;
    }

    bool GraphicsDevice::Initialize(const SwapChainDescriptor* descriptor)
    {
        if (_initialized) {
            return true;
        }

        _initialized = InitializeImpl(descriptor);
        return _initialized;
    }

    bool GraphicsDevice::BeginFrame()
    {
        return true;
    }

    void GraphicsDevice::EndFrame()
    {
        Tick();

        // Present to screen.
        _renderWindow->SwapBuffers();
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
        return CreateBufferImpl(descriptor, pInitData);
    }

    ShaderHandle* GraphicsDevice::CreateShader(const ShaderDescriptor* descriptor)
    {
        ALIMER_ASSERT(descriptor);
        return CreateShaderImpl(descriptor);
    }
}
