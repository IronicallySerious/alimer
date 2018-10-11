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
#include "../Graphics/GraphicsImpl.h"
#include "../Graphics/ShaderCompiler.h"
#include "../Core/Log.h"

#if defined(_WIN32)
// Prefer the high-performance GPU on switchable GPU systems
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 1;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

namespace Alimer
{
    Graphics::Graphics(bool validation)
    {
#ifdef ALIMER_THREADING
        _cookie.store(0);
#endif
        _impl = CreateBackend(validation);
        AddSubsystem(this);
    }
    
    Graphics::~Graphics()
    {
        WaitIdle();

        // Destroy undestroyed resources.
        /*if (_gpuResources.size())
        {
            lock_guard<mutex> lock(_gpuResourceMutex);

            // Release all GPU objects that still exist
            std::sort(_gpuResources.begin(), _gpuResources.end(),
                [](const GpuResource* x, const GpuResource* y)
            {
                return x->GetResourceType() < y->GetResourceType();
            });

            for (size_t i = 0; i < _gpuResources.size(); ++i)
            {
                GpuResource* resource = _gpuResources.at(i);
                ALIMER_ASSERT(resource);
                auto type = resource->GetResourceType();
                resource->Destroy();
            }

            _gpuResources.clear();
        }*/

        ShutdownBackend();
        RemoveSubsystem(this);
    }

    GraphicsImpl* Graphics::CreateBackend(bool validation)
    {
        return new GraphicsImpl(validation);
    }

    bool Graphics::Initialize(const RenderingSettings& settings)
    {
        ALIMER_ASSERT_MSG(settings.windowHandle, "Invalid window handle for graphics creation.");

        if (_initialized)
        {
            ALIMER_LOGCRITICAL("Cannot Initialize Graphics if already initialized.");
        }

        _settings = settings;
        _initialized = BackendInitialize(settings);
        return _initialized;
    }

    void Graphics::ShutdownBackend()
    {
        SafeDelete(_impl);
    }

    bool Graphics::WaitIdle()
    {
        return _impl->WaitIdle();
    }

    bool Graphics::BackendInitialize(const RenderingSettings& settings)
    {
        return _impl->Initialize(settings);
    }

    bool Graphics::BeginFrame()
    {
        return _impl->BeginFrame();
    }

    void Graphics::EndFrame()
    {
        _impl->EndFrame();
    }

    GraphicsBackend Graphics::GetBackend() const
    {
        return _impl->GetBackend();
    }

    const GraphicsDeviceFeatures& Graphics::GetFeatures() const
    {
        return _impl->GetFeatures();
    }

    SharedPtr<TextureView> Graphics::GetSwapchainView() const
    {
        return _impl->GetSwapchainView();
    }

    SharedPtr<CommandBuffer> Graphics::GetMainCommandBuffer() const
    {
        return _impl->GetMainCommandBuffer();
    }

#if TODO
    GpuBuffer* Graphics::CreateBuffer(const BufferDescriptor* descriptor, const void* initialData)
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

        return CreateBufferImpl(descriptor, initialData);
    }

    VertexInputFormat* Graphics::CreateVertexInputFormat(const VertexInputFormatDescriptor* descriptor)
    {
        ALIMER_ASSERT(descriptor);

        if (!descriptor->attributesCount)
        {
            ALIMER_LOGCRITICAL("Can not define VertexInputFormat with no vertex attributes");
        }

        return CreateVertexInputFormatImpl(descriptor);
    }

    ShaderModule* Graphics::CreateShaderModule(const std::vector<uint32_t>& spirv)
    {
        if (spirv.empty())
        {
            ALIMER_LOGCRITICAL("Cannot create shader module with empty bytecode");
        }

        return CreateShaderModuleImpl(spirv);
    }

    ShaderModule* Graphics::CreateShaderModule(const String& file, const String& entryPoint)
    {
        // Load from spirv bytecode.
        vector<uint32_t> spirv;

        /*if (gResources()->Exists(file + ".spv"))
        {
            auto vertexShaderStream = FileSystem::Get().Open(vertexShaderFile + ".spv");
            //auto vertexShaderStream = gResources()->Open(vertexShaderFile + ".spv");
            auto fragmentShaderStream = gResources()->Open(fragmentShaderFile + ".spv");

            // Lookup for SPIR-V compiled shader.
            if (!vertexShaderStream)
            {
                ALIMER_LOGERRORF("GLSL shader does not exists '%s'", vertexShaderFile.c_str());
                return nullptr;
            }

            if (!fragmentShaderStream)
            {
                ALIMER_LOGERRORF("GLSL shader does not exists '%s'", fragmentShaderFile.c_str());
                return nullptr;
            }

            vertexByteCode = vertexShaderStream->ReadBytes();
            fragmentByteCode = fragmentShaderStream->ReadBytes();
        }
        else*/
        {
            // Compile from source GLSL.
            String errorLog;
            if (!ShaderCompiler::Compile(file, entryPoint, spirv, errorLog))
            {
                ALIMER_LOGCRITICALF("Shader compilation failed: %s", errorLog.CString());
            }
        }

        return CreateShaderModule(spirv);
    }

    ShaderProgram* Graphics::CreateShaderProgram(const ShaderProgramDescriptor* descriptor)
    {
        ALIMER_ASSERT(descriptor);
        if (!descriptor->stageCount)
        {
            ALIMER_LOGCRITICAL("Cannot create shader program with empty stages");
        }

        for (uint32_t i = 0; i < descriptor->stageCount; i++)
        {
            if (!descriptor->stages[i].module)
            {
                ALIMER_LOGCRITICALF("Cannot create shader program with invalid shader module at index %s", i);
            }

            if (descriptor->stages[i].module->GetStage() == ShaderStage::Compute
                && descriptor->stageCount > 1)
            {
                ALIMER_LOGCRITICAL("Cannot create shader program with compute and graphics stages");
            }
        }

        return CreateShaderProgramImpl(descriptor);
    }

    ShaderProgram* Graphics::CreateShaderProgram(const ShaderStageDescriptor* stage)
    {
        ALIMER_ASSERT(stage);

        if (stage->module->GetStage() != ShaderStage::Compute)
        {
            ALIMER_LOGCRITICAL("Shader stage module is not compute");
        }

        ShaderProgramDescriptor descriptor = {};
        descriptor.stageCount = 1;
        descriptor.stages = stage;
        return CreateShaderProgramImpl(&descriptor);
    }

    ShaderProgram* Graphics::CreateShaderProgram(ShaderModule* vertex, ShaderModule* fragment)
    {
        ALIMER_ASSERT(vertex);
        ALIMER_ASSERT(fragment);

        if (vertex->GetStage() != ShaderStage::Vertex)
            ALIMER_LOGCRITICAL("Invalid vertex stage module");

        if (fragment->GetStage() != ShaderStage::Fragment)
            ALIMER_LOGCRITICAL("Invalid fragment stage module");

        std::array<ShaderStageDescriptor, 2> stages;
        stages[0].module = vertex;
        stages[1].module = fragment;
        ShaderProgramDescriptor descriptor = {};
        descriptor.stageCount = 2;
        descriptor.stages = stages.data();
        return CreateShaderProgramImpl(&descriptor);
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
#endif // TODO

    void Graphics::AddGpuResource(GpuResource* resource)
    {
        std::unique_lock<std::mutex> lock(_gpuResourceMutex);
        _gpuResources.push_back(resource);
    }

    void Graphics::RemoveGpuResource(GpuResource* resource)
    {
        std::unique_lock<std::mutex> lock(_gpuResourceMutex);
        auto it = std::find(_gpuResources.begin(), _gpuResources.end(), resource);
        if (it != _gpuResources.end())
        {
            _gpuResources.erase(it);
        }
    }

    uint64_t Graphics::GetNextUniqueId()
    {
#ifdef ALIMER_THREADING
        return _cookie.fetch_add(1, std::memory_order_relaxed) + 1;
#else
        _cookie++;
        return _cookie;
#endif
    }

    void Graphics::NotifyFalidationError(const char* message)
    {
        // TODO: Add callback.
        ALIMER_UNUSED(message);
    }

    
}
