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
#include "../Core/Log.h"

#if ALIMER_COMPILE_D3D11
#   include "../Graphics/D3D11/D3D11GraphicsDevice.h"
#endif

#if ALIMER_COMPILE_D3D12
// #   include "../Graphics/D3D12/D3D12GraphicsDevice.h"
#endif

#if ALIMER_COMPILE_VULKAN
#   include "../Graphics/Vulkan/VulkanGraphicsDevice.h"
#endif

namespace Alimer
{
    GraphicsDevice::GraphicsDevice(GraphicsBackend backend, bool validation)
        : _backend(backend)
        , _validation(validation)
    {
        AddSubsystem(this);
    }

    GraphicsDevice::~GraphicsDevice()
    {
        agpuShutdown();
        RemoveSubsystem(this);
    }

    void GraphicsDevice::Shutdown()
    {
        // Clear cached data.
        _shaders.Clear();

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

        _shaders.Clear();
        _context.Reset();
    }

    bool GraphicsDevice::IsBackendSupported(GraphicsBackend backend)
    {
        return agpuIsBackendSupported(static_cast<AgpuBackend>(backend));
    }

    std::set<GraphicsBackend> GraphicsDevice::GetAvailableBackends()
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

#if ALIMER_COMPILE_D3D12
            //backends.insert(GraphicsBackend::Direct3D12);
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

    GraphicsDevice* GraphicsDevice::Create(GraphicsBackend prefferedBackend, bool validation)
    {
        if (prefferedBackend == GraphicsBackend::Default)
        {
            auto availableBackends = GraphicsDevice::GetAvailableBackends();

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

        GraphicsDevice* device = nullptr;
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

    bool GraphicsDevice::Initialize(const RenderingSettings& settings)
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

    GpuBuffer* GraphicsDevice::CreateBuffer(const BufferDescriptor* descriptor, const void* initialData)
    {
        ALIMER_ASSERT(descriptor);

        if (descriptor->usage == BufferUsage::None)
        {
            ALIMER_LOGCRITICAL("Invalid buffer usage");
        }

        if (!descriptor->size)
        {
            ALIMER_LOGCRITICAL("Cannot create empty buffer");
        }

        if (descriptor->resourceUsage == ResourceUsage::Immutable
            && initialData == nullptr)
        {
            ALIMER_LOGCRITICAL("Immutable Buffer needs valid initial data.");
        }

        if (any(descriptor->usage & BufferUsage::Index))
        {
            if (descriptor->stride != 2 && descriptor->stride != 4)
            {
                ALIMER_LOGCRITICAL("IndexBuffer needs to be created with stride 2 or 4.");
            }
        }

        GpuBuffer* buffer = CreateBufferImpl(descriptor, initialData);
        if (buffer == nullptr)
        {
            ALIMER_LOGERROR("Failed to create buffer");
            return nullptr;
        }

        ALIMER_LOGDEBUGF("Created %s %s buffer [size: %llu, stride: %u]",
            EnumToString(descriptor->resourceUsage),
            EnumToString(descriptor->usage),
            descriptor->size,
            descriptor->stride
        );
        return buffer;
    }

    uint32_t GraphicsDevice::Present()
    {
        _context->Flush();
        PresentImpl();
        return ++_frameIndex;
    }

    Texture* GraphicsDevice::CreateTexture(const TextureDescriptor* descriptor, const ImageLevel* initialData)
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

    Framebuffer* GraphicsDevice::CreateFramebuffer(const FramebufferDescriptor* descriptor)
    {
        ALIMER_ASSERT(descriptor);

        return CreateFramebufferImpl(descriptor);
    }

    ShaderModule* GraphicsDevice::RequestShader(const uint32_t* pCode, size_t size)
    {
        if (!size || pCode == nullptr)
        {
            ALIMER_LOGCRITICAL("Cannot create shader module with invalid bytecode");
        }

        Hasher hasher;
        hasher.Data(pCode, size);

        auto hash = hasher.GetValue();
        auto *ret = _shaders.Find(hash);
        if (!ret)
        {
            //ret = _shaders.Insert(hash, CreateShaderModuleImpl(hash, pCode, size));
            ALIMER_LOGDEBUGF("New %s shader created: '%s'", EnumToString(ret->GetStage()), std::to_string(hash).c_str());
        }

        return ret;
    }

    ShaderModule* GraphicsDevice::RequestShader(const ShaderBlob& blob)
    {
        if (blob.size <= 0 || blob.data == nullptr)
        {
            ALIMER_LOGCRITICAL("Cannot create shader module with invalid bytecode");
        }

        Hasher hasher;
        hasher.Data(blob.data, blob.size);

        auto hash = hasher.GetValue();
        auto *ret = _shaders.Find(hash);
        if (!ret)
        {
            ret = _shaders.Insert(hash, UniquePtr<ShaderModule>(new ShaderModule(hash, blob)));
            ALIMER_LOGDEBUGF("New %s shader created: '%s'", EnumToString(ret->GetStage()), std::to_string(hash).c_str());
        }

        return ret;
    }

    ShaderModule* GraphicsDevice::RequestShader(const String& url)
    {
        // TODO: Shader cache.
        if (!FileSystem::Get().Exists("assets://" + url))
        {
            ALIMER_LOGCRITICALF("Shader does not exists: '%s'", url.CString());
        }

        return nullptr;

        /*ShaderStage stage = ShaderStage::Count;
        String ext = FileSystem::GetExtension(url);
        if (ext == ".vert")
        {
            stage = ShaderStage::Vertex;
        }
        else if (ext == ".frag")
        {
            stage = ShaderStage::Fragment;
        }
        else if (ext == ".tesc")
        {
            stage = ShaderStage::TessControl;
        }
        else if (ext == ".tese")
        {
            stage = ShaderStage::TessEvaluation;
        }
        else if (ext == ".geom")
        {
            stage = ShaderStage::Geometry;
        }
        else if (ext == ".comp")
        {
            stage = ShaderStage::Geometry;
        }
        else
        {
            ALIMER_LOGCRITICALF("Invalid shader extension: '%s'", ext.CString());
        }

        

        auto stream = FileSystem::Get().Open("assets://" + url);
        auto shaderSource = stream->ReadAllText();
        ShaderCompiler compiler;
        ShaderBlob blob = compiler.Compile(
            shaderSource.CString(),
            "main",
            ShaderLanguage::GLSL,
            stage,
            stream->GetName().CString());

        return RequestShader(blob);*/
    }

    Shader* GraphicsDevice::CreateShader(const ShaderDescriptor* descriptor)
    {
        ALIMER_ASSERT(descriptor);
        return CreateShaderImpl(descriptor);
    }

    Shader* GraphicsDevice::CreateShader(const ShaderBlob& compute)
    {
        ALIMER_ASSERT(compute.size);
        ShaderDescriptor descriptor;
        descriptor.stages[static_cast<uint32_t>(ShaderStage::Compute)] = compute;
        return CreateShader(&descriptor);
    }

    Pipeline* GraphicsDevice::CreateRenderPipeline(const RenderPipelineDescriptor* descriptor)
    {
        ALIMER_ASSERT(descriptor);
#ifdef _DEBUG
        if (descriptor->shaders[static_cast<unsigned>(ShaderStage::Vertex)] == nullptr)
        {
            ALIMER_LOGERROR("CreateRenderPipeline: Invalid vertex shader.");
        }

        if (descriptor->shaders[static_cast<unsigned>(ShaderStage::Compute)] != nullptr)
        {
            ALIMER_LOGERROR("CreateRenderPipeline: Cannot contain compute shader.");
        }
#endif

        return CreateRenderPipelineImpl(descriptor);
    }

    void GraphicsDevice::AddGraphicsResource(GraphicsResource* resource)
    {
        std::unique_lock<std::mutex> lock(_gpuResourceMutex);
        _gpuResources.push_back(resource);
    }

    void GraphicsDevice::RemoveGraphicsResource(GraphicsResource* resource)
    {
        std::unique_lock<std::mutex> lock(_gpuResourceMutex);
        auto it = std::find(_gpuResources.begin(), _gpuResources.end(), resource);
        if (it != _gpuResources.end())
        {
            _gpuResources.erase(it);
        }
    }

    void GraphicsDevice::NotifyValidationError(const char* message)
    {
        // TODO: Add callback.
        ALIMER_UNUSED(message);
    }
}
