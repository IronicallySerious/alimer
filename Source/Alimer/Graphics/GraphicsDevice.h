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

#pragma once

#include "../Base/Cache.h"
#include "../Core/Object.h"
#include "../Graphics/Types.h"
#include "../Graphics/GraphicsDeviceFeatures.h"
#include "../Graphics/VertexBuffer.h"
#include "../Graphics/IndexBuffer.h"
#include "../Graphics/Texture.h"
#include "../Graphics/Shader.h"
#include "../Graphics/ShaderCompiler.h"
#include "../Graphics/CommandBuffer.h"
#include <vector>
#include <mutex>

namespace Alimer
{
    /// Low-level 3D graphics module.
    class ALIMER_API GraphicsDevice : public Object
    {
        ALIMER_OBJECT(GraphicsDevice, Object);

    protected:
        /// Constructor.
        GraphicsDevice(GraphicsBackend backend, bool validation);

    public:
        static GraphicsDevice* Create(GraphicsBackend prefferedBackend = GraphicsBackend::Default, bool validation = false);

        /// Destructor.
        virtual ~GraphicsDevice() override;

        /// Initialize graphics with given settings.
        virtual bool Initialize(const RenderingSettings& settings);

        /// Wait for a device to become idle.
        virtual bool WaitIdle() = 0;

        /// Begin the rendering frame.
        virtual bool BeginFrame() = 0;

        /// Finishes the current frame and schedules it for display.
        virtual void EndFrame() = 0;

        /// Add a GraphicsResource to keep track of. 
        void AddGraphicsResource(GraphicsResource* resource);

        /// Remove a GraphicsResource.
        void RemoveGraphicsResource(GraphicsResource* resource);

        /// Create new buffer with given descriptor and optional initial data.
        //GpuBuffer* CreateBuffer(const BufferDescriptor* descriptor, const void* initialData = nullptr);

        /// Create new vertex buffer.
        VertexBuffer* CreateVertexBuffer(uint32_t vertexCount, const std::vector<VertexElement>& elements, ResourceUsage resourceUsage = ResourceUsage::Default, const void* initialData = nullptr);

        /// Create new vertex buffer.
        VertexBuffer* CreateVertexBuffer(uint32_t vertexCount, size_t elementsCount, const VertexElement* elements, ResourceUsage resourceUsage = ResourceUsage::Default, const void* initialData = nullptr);

        /// Create new index buffer.
        IndexBuffer* CreateIndexBuffer(uint32_t indexCount, IndexType indexType, ResourceUsage resourceUsage = ResourceUsage::Default, const void* initialData = nullptr);

        /// Create new buffer with given descriptor and optional initial data.
        Texture* CreateTexture(const TextureDescriptor* descriptor, const ImageLevel* initialData = nullptr);

        /// Request shader module using SPIRV bytecode.
        ShaderModule* RequestShader(const uint32_t* pCode, size_t size);

        /// Create new shader program with vertex and fragment module.
        Program* RequestProgram(ShaderModule* vertex, ShaderModule* fragment);

        /// Create new shader compute shader program.
        Program* RequestProgram(ShaderModule* compute);

        /// Create new shader program with descriptor.
        Program* RequestProgram(
            const uint32_t *vertexData, size_t vertexSize,
            const uint32_t *fragmentData, size_t fragmentSize);

        /// Get whether grapics has been initialized.
        bool IsInitialized() const { return _initialized; }

        /// Get the type of device.
        GraphicsBackend GetBackend() const { return _backend; }

        /// Get the device features.
        const GraphicsDeviceFeatures& GetFeatures() const { return _features; }

        /// Get the main Swapchain current image view.
        virtual TextureView* GetSwapchainView() const = 0;

        /// Get the main command buffer.
        virtual SharedPtr<CommandBuffer> GetMainCommandBuffer() const = 0;

        /// Request new command buffer.
        //SharedPtr<CommandBuffer> RequestCommandBuffer(CommandBuffer::Type type);

        ShaderManager &GetShaderManager();

        void NotifyValidationError(const char* message);

    protected:
        virtual void Shutdown();
        //virtual GpuBuffer* CreateBufferImpl(const BufferDescriptor* descriptor, const void* initialData) = 0;
        virtual VertexBuffer* CreateVertexBufferImpl(uint32_t vertexCount, size_t elementsCount, const VertexElement* elements, ResourceUsage resourceUsage, const void* initialData) = 0;
        virtual IndexBuffer* CreateIndexBufferImpl(uint32_t indexCount, IndexType indexType, ResourceUsage resourceUsage, const void* initialData) = 0;
        virtual std::unique_ptr<ShaderModule> CreateShaderModuleImpl(Util::Hash hash, const uint32_t* pCode, size_t size) = 0;
        virtual std::unique_ptr<Program> CreateProgramImpl(Util::Hash hash, const std::vector<ShaderModule*>& stages) = 0;
        virtual Texture* CreateTextureImpl(const TextureDescriptor* descriptor, const ImageLevel* initialData) = 0;

        GraphicsBackend _backend = GraphicsBackend::Empty;
        bool _validation;
        bool _initialized = false;
        RenderingSettings _settings = {};
        GraphicsDeviceFeatures _features = {};
        std::vector<GraphicsResource*> _gpuResources;
        std::mutex _gpuResourceMutex;

    private:
        ShaderManager _shaderManager;
        Cache<ShaderModule> _shaders;
        Cache<Program> _programs;
    };
}
