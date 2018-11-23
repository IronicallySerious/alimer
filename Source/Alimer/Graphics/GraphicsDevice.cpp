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

#include "../Graphics/GraphicsDevice.h"
#include "../Graphics/ShaderCompiler.h"
#include "../IO/FileSystem.h"
#include "../Debug/Log.h"

#if ALIMER_COMPILE_D3D11
#   include "../Graphics/D3D11/D3D11GraphicsDevice.h"
#endif

#if ALIMER_COMPILE_VULKAN
#   include "../Graphics/Vulkan/VulkanGraphicsDevice.h"
#endif

namespace Alimer
{
    Graphics::Graphics(GraphicsBackend backend, bool validation)
        : _backend(backend)
        , _validation(validation)
    {
        AddSubsystem(this);
    }

    Graphics::~Graphics()
    {
        agpuShutdown();
        RemoveSubsystem(this);
    }

    void Graphics::Shutdown()
    {
        // Destroy undestroyed resources.
        if (_gpuResources.size())
        {
            std::lock_guard<std::mutex> lock(_gpuResourceMutex);
            for (size_t i = 0; i < _gpuResources.size(); ++i)
            {
                GraphicsResource* resource = _gpuResources.at(i);
                ALIMER_ASSERT(resource);
                resource->Destroy();
            }

            _gpuResources.clear();
        }

        _context.Reset();
    }

    bool Graphics::IsBackendSupported(GraphicsBackend backend)
    {
        return agpuIsBackendSupported(static_cast<AgpuBackend>(backend));
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

#if ALIMER_COMPILE_D3D11
            backends.insert(GraphicsBackend::D3D11);
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

    Graphics* Graphics::Create(GraphicsBackend prefferedBackend, bool validation)
    {
        if (prefferedBackend == GraphicsBackend::Default)
        {
            auto availableBackends = Graphics::GetAvailableBackends();

            if (availableBackends.find(GraphicsBackend::Vulkan) != availableBackends.end())
            {
                prefferedBackend = GraphicsBackend::Vulkan;
            }
            else if (availableBackends.find(GraphicsBackend::Metal) != availableBackends.end())
            {
                prefferedBackend = GraphicsBackend::Metal;
            }
            else if (availableBackends.find(GraphicsBackend::D3D12) != availableBackends.end())
            {
                prefferedBackend = GraphicsBackend::D3D12;
            }
            else if (availableBackends.find(GraphicsBackend::D3D11) != availableBackends.end())
            {
                prefferedBackend = GraphicsBackend::D3D11;
            }
            else if (availableBackends.find(GraphicsBackend::OpenGL) != availableBackends.end())
            {
                prefferedBackend = GraphicsBackend::OpenGL;
            }
            else
            {
                prefferedBackend = GraphicsBackend::Empty;
            }
        }

        Graphics* device = nullptr;
        switch (prefferedBackend)
        {
        case GraphicsBackend::Vulkan:
#if ALIMER_COMPILE_VULKAN
            device = new VulkanGraphicsDevice(validation);
#else
            ALIMER_LOGCRITICAL("Vulkan backend is not supported");
#endif
            break;

        case GraphicsBackend::D3D11:
#if ALIMER_COMPILE_D3D11
            device = new D3D11GraphicsDevice(validation);
#else
            ALIMER_LOGCRITICAL("Direct3D11 backend is not supported");
#endif
            break;

        default:
            break;
        }

        return device;
    }

    bool Graphics::Initialize(const RenderingSettings& settings)
    {
        if (_initialized)
        {
            ALIMER_LOGCRITICAL("Cannot Initialize Graphics if already initialized.");
        }

        AgpuDescriptor descriptor;
        descriptor.validation = AGPU_TRUE;
        descriptor.preferredBackend = AGPU_BACKEND_DEFAULT;
        agpuInitialize(&descriptor);

        _settings = settings;
        _initialized = true;
        _frameIndex = 0;
        return _initialized;
    }

    uint32_t Graphics::Present()
    {
        _context->Flush();
        PresentImpl();
        return ++_frameIndex;
    }

    Texture* Graphics::CreateTexture(const TextureDescriptor* descriptor, const ImageLevel* initialData)
    {
        ALIMER_ASSERT(descriptor);

        if (descriptor->usage == TextureUsage::Unknown)
        {
            ALIMER_LOGCRITICAL("Invalid texture usage");
        }

        if (descriptor->width == 0
            || descriptor->height == 0
            || descriptor->depth == 0
            || descriptor->arrayLayers == 0
            || descriptor->mipLevels == 0)
        {
            ALIMER_LOGCRITICAL("Cannot create an empty texture");
        }

        //if (initialData && !(descriptor->usage & TextureUsage::TransferDest))
        //{
        //    ALIMER_LOGCRITICAL("Texture needs the transfer dest usage when creating with initial data.");
        //}

        return CreateTextureImpl(descriptor, initialData);
    }

    Framebuffer* Graphics::CreateFramebuffer(const FramebufferDescriptor* descriptor)
    {
        ALIMER_ASSERT(descriptor);

        return CreateFramebufferImpl(descriptor);
    }

    void Graphics::AddGraphicsResource(GraphicsResource* resource)
    {
        std::unique_lock<std::mutex> lock(_gpuResourceMutex);
        _gpuResources.push_back(resource);
    }

    void Graphics::RemoveGraphicsResource(GraphicsResource* resource)
    {
        std::unique_lock<std::mutex> lock(_gpuResourceMutex);
        auto it = std::find(_gpuResources.begin(), _gpuResources.end(), resource);
        if (it != _gpuResources.end())
        {
            _gpuResources.erase(it);
        }
    }

    void Graphics::NotifyValidationError(const char* message)
    {
        // TODO: Add callback.
        ALIMER_UNUSED(message);
    }

    void Graphics::RegisterObject()
    {
        static bool registered = false;
        if (registered)
            return;
        registered = true;

        Shader::RegisterObject();
        Texture::RegisterObject();
    }

    void RegisterGraphicsLibrary()
    {
        Graphics::RegisterObject();
    }
}
