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
#include "../Core/Log.h"
#include <inttypes.h>

#if defined(_WIN32)
// Prefer the high-performance GPU on switchable GPU systems
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 1;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif /* defined(_WIN32) */

#if ALIMER_VULKAN
#   include "../Graphics/Vulkan/VulkanGraphicsDevice.h"
#endif

namespace Alimer
{
    GraphicsDevice::GraphicsDevice(GraphicsBackend backend, bool validation)
        : _backend(backend)
        , _validation(validation)
        , _shaderManager(this)
    {
        AddSubsystem(this);
    }

    GraphicsDevice::~GraphicsDevice()
    {
        RemoveSubsystem(this);
    }

    void GraphicsDevice::Shutdown()
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

        _shaders.Clear();
        _programs.Clear();
        _context.Reset();
    }

    GraphicsDevice* GraphicsDevice::Create(GraphicsBackend prefferedBackend, bool validation)
    {
        if (prefferedBackend == GraphicsBackend::Default)
        {
#if ALIMER_VULKAN
            prefferedBackend = GraphicsBackend::Vulkan;
#endif
        }

        GraphicsDevice* device = nullptr;
        switch (prefferedBackend)
        {
        case GraphicsBackend::Vulkan:
            device = new VulkanGraphicsDevice(validation);
            break;

        default:
            break;
        }

        return device;
    }

    bool GraphicsDevice::Initialize(const RenderingSettings& settings)
    {
        ALIMER_ASSERT_MSG(settings.windowHandle, "Invalid window handle for graphics creation.");

        if (_initialized)
        {
            ALIMER_LOGCRITICAL("Cannot Initialize Graphics if already initialized.");
        }

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

    ShaderModule* GraphicsDevice::RequestShader(const uint32_t* pCode, size_t size)
    {
        if (!size || pCode == nullptr)
        {
            ALIMER_LOGCRITICAL("Cannot create shader module with invalid bytecode");
        }

        Util::Hasher hasher;
        hasher.data(pCode, size);

        auto hash = hasher.get();
        auto *ret = _shaders.Find(hash);
        if (!ret)
        {
            ret = _shaders.Insert(hash, CreateShaderModuleImpl(hash, pCode, size));
            ALIMER_LOGDEBUGF("New %s shader created: '%s'", EnumToString(ret->GetStage()), std::to_string(hash).c_str());
        }

        return ret;
    }

    Program* GraphicsDevice::RequestProgram(ShaderModule* vertex, ShaderModule* fragment)
    {
        ALIMER_ASSERT(vertex);
        ALIMER_ASSERT(fragment);

        if (vertex->GetStage() != ShaderStage::Vertex)
            ALIMER_LOGCRITICAL("Invalid vertex stage module");

        if (fragment->GetStage() != ShaderStage::Fragment)
            ALIMER_LOGCRITICAL("Invalid fragment stage module");

        Util::Hasher hasher;
        hasher.u64(vertex->GetHash());
        hasher.u64(fragment->GetHash());

        auto hash = hasher.get();
        auto *ret = _programs.Find(hash);
        if (!ret)
        {
            std::vector<ShaderModule*> modules(2);
            modules[0] = vertex;
            modules[1] = fragment;
            ret = _programs.Insert(hash, CreateProgramImpl(hash, modules));
            ALIMER_LOGDEBUGF("New graphics program created: '%s'", std::to_string(hash).c_str());
        }
        return ret;
    }

    Program* GraphicsDevice::RequestProgram(ShaderModule* compute)
    {
        ALIMER_ASSERT(compute);

        if (compute->GetStage() != ShaderStage::Compute)
        {
            ALIMER_LOGCRITICAL("Shader stage module is not compute");
        }

        Util::Hasher hasher;
        hasher.u64(compute->GetHash());

        auto hash = hasher.get();
        auto *ret = _programs.Find(hash);
        if (!ret)
        {
            std::vector<ShaderModule*> modules(1);
            modules[0] = compute;
            ret = _programs.Insert(hash, CreateProgramImpl(hash, modules));
            ALIMER_LOGDEBUGF("New compute program created: '%s'", std::to_string(hash).c_str());
        }

        return ret;
    }

    Program* GraphicsDevice::RequestProgram(
        const uint32_t *vertexData, size_t vertexSize,
        const uint32_t *fragmentData, size_t fragmentSize)
    {
        ShaderModule* vertex = RequestShader(vertexData, vertexSize);
        ShaderModule* fragment = RequestShader(fragmentData, fragmentSize);
        return RequestProgram(vertex, fragment);
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

    ShaderManager &GraphicsDevice::GetShaderManager()
    {
        return _shaderManager;
    }
}
