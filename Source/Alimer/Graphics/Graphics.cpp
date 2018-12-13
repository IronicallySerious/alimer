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

#include "../Graphics/Graphics.h"
#include "../Graphics/ShaderCompiler.h"
#include "../IO/FileSystem.h"
#include "../Debug/Log.h"

#if defined(ALIMER_D3D11)
#   include "../Graphics/D3D11/D3D11GraphicsDevice.h"
#endif

#if ALIMER_COMPILE_VULKAN
#   include "../Graphics/Vulkan/VulkanGraphicsDevice.h"
#endif

#if defined(ALIMER_D3D12)
#include "../Graphics/D3D12/D3D12Graphics.h"
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

namespace Alimer
{
    Graphics *Graphics::_instance;

    Graphics::Graphics(GraphicsBackend backend, bool validation)
        : _backend(backend)
        , _validation(validation)
    {
        AddSubsystem(this);
        _instance = this;
    }

    Graphics::~Graphics()
    {
        Finalize();
        RemoveSubsystem(this);
        _instance = nullptr;
    }

    Graphics* Graphics::Create(GraphicsBackend preferredBackend, bool validation)
    {
        if (_instance != nullptr)
        {
            ALIMER_LOGERROR("Cannot create multiple instance of Graphics module");
            return nullptr;
        }

        GraphicsBackend backend = preferredBackend;
        if (backend == GraphicsBackend::Default)
        {
            backend = GetDefaultPlatformBackend();
        }

        if (!IsBackendSupported(backend))
        {
            ALIMER_LOGERROR("Backend is not supported");
            return nullptr;
        }

        Graphics* device = nullptr;
        switch (backend)
        {
        case GraphicsBackend::Empty:
            break;
        case GraphicsBackend::Vulkan:
            break;
        case GraphicsBackend::D3D11:
#if defined(ALIMER_D3D11)
            device = new D3D11Graphics(validation);
            ALIMER_LOGINFO("D3D11 backend created with success.");
#else
            ALIMER_LOGERROR("D3D11 backend is not supported.");
#endif
            break;
        case GraphicsBackend::D3D12:
#if defined(ALIMER_D3D12)
            //device = new D3D12Graphics(validation);
            ALIMER_LOGINFO("D3D12 backend created with success.");
#else
            ALIMER_LOGERROR("D3D12 backend is not supported.");
#endif
            break;
        case GraphicsBackend::Metal:
            break;
        case GraphicsBackend::OpenGL:
            break;
        default:
            ALIMER_UNREACHABLE();
        }

        return device;
    }

    void Graphics::Shutdown()
    {
        SafeDelete(_instance);
    }

    Graphics& Graphics::GetInstance()
    {
        return *_instance;
    }

    void Graphics::Finalize()
    {
        // Destroy undestroyed resources.
        if (_gpuResources.Size())
        {
            std::lock_guard<std::mutex> lock(_gpuResourceMutex);
            for (auto it = _gpuResources.Begin(); it != _gpuResources.End(); ++it)
            {
                (*it)->Destroy();
            }

            _gpuResources.Clear();
        }

        SafeDelete(_immediateCommandContext);
    }

    bool Graphics::IsBackendSupported(GraphicsBackend backend)
    {
        if (backend == GraphicsBackend::Default)
        {
            backend = GetDefaultPlatformBackend();
        }

        switch (backend)
        {
        case GraphicsBackend::Empty:
            return true;
        case GraphicsBackend::Vulkan:
#if AGPU_VULKAN
            return true; // VulkanGraphicsDevice::IsSupported();
#else
            return false;
#endif
        case GraphicsBackend::D3D11:
#if defined(ALIMER_D3D11)
            return D3D11Graphics::IsSupported();
#else
            return false;
#endif

        case GraphicsBackend::D3D12:
#if defined(ALIMER_D3D12)
            return false;
            //return D3D12Graphics::IsSupported();
#else
            return false;
#endif

        case GraphicsBackend::OpenGL:
#if AGPU_OPENGL
            return true;
#else
            return false;
#endif
        default:
            return false;
        }
        }

    std::set<GraphicsBackend> Graphics::GetAvailableBackends()
    {
        static std::set<GraphicsBackend> backends;

        if (backends.empty())
        {
            backends.insert(GraphicsBackend::Empty);

#if ALIMER_COMPILE_OPENGL
            backends.insert(GraphicsBackend::OpenGL);
#endif

#if defined(ALIMER_D3D11)
            if (D3D11Graphics::IsSupported())
            {
                backends.insert(GraphicsBackend::D3D11);
            }
#endif


#if defined(ALIMER_D3D12)
            /*if (D3D12Graphics::IsSupported())
            {
                backends.insert(GraphicsBackend::D3D12);
            }*/
#endif

#if ALIMER_COMPILE_VULKAN
            if (VulkanGraphicsDevice::IsSupported())
            {
                backends.insert(GraphicsBackend::Vulkan);
            }
#endif
        }

        return backends;
    }

    GraphicsBackend Graphics::GetDefaultPlatformBackend()
    {
#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP || ALIMER_PLATFORM_XBOX_ONE
        if (IsBackendSupported(GraphicsBackend::D3D12))
        {
            return GraphicsBackend::D3D12;
        }

        return GraphicsBackend::D3D11;
#elif ALIMER_PLATFORM_LINUX || ALIMER_PLATFORM_ANDROID
        return GraphicsBackend::OpenGL;
#elif ALIMER_PLATFORM_MACOS || ALIMER_PLATFORM_IOS || ALIMER_PLATFORM_TVOS
        return GraphicsBackend::Metal;
#else
        return GraphicsBackend::OpenGL;
#endif
    }

    bool Graphics::Initialize(const RenderWindowDescriptor* mainWindowDescriptor)
    {
        if (_initialized)
        {
            ALIMER_LOGCRITICAL("Cannot Initialize Graphics if already initialized.");
        }

        _initialized = true;
        _frameIndex = 0;
        return _initialized;
    }

    bool Graphics::WaitIdle()
    {
        return true;
    }

    uint64_t Graphics::Frame()
    {
        OnFrame();
        return ++_frameIndex;
    }

    void Graphics::AddGraphicsResource(GraphicsResource* resource)
    {
        std::unique_lock<std::mutex> lock(_gpuResourceMutex);
        _gpuResources.Push(resource);
    }

    void Graphics::RemoveGraphicsResource(GraphicsResource* resource)
    {
        std::unique_lock<std::mutex> lock(_gpuResourceMutex);
        _gpuResources.Remove(resource);
    }

    void Graphics::NotifyValidationError(const char* message)
    {
        // TODO: Add callback.
        ALIMER_UNUSED(message);
    }

    const GraphicsDeviceFeatures& Graphics::GetFeatures() const
    {
        return _features;
    }

    void Graphics::RegisterObject()
    {
        static bool registered = false;
        if (registered)
            return;
        registered = true;

        ShaderModule::RegisterObject();
        Shader::RegisterObject();
        Texture::RegisterObject();
    }

    Graphics& gGraphics()
    {
        return Graphics::GetInstance();
    }
}
