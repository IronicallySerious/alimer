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

#include "../Core/Object.h"
#include "../Graphics/Types.h"
#include "../Graphics/GraphicsDeviceFeatures.h"
#include "../Graphics/GpuBuffer.h"
#include "../Graphics/Texture.h"
#include "../Graphics/Shader.h"
#include "../Graphics/CommandBuffer.h"
#include "../Graphics/VertexFormat.h"
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

        /// Add a GpuResource to keep track of. 
        void AddGpuResource(GpuResource* resource);

        /// Remove a GpuResource.
        void RemoveGpuResource(GpuResource* resource);

        /// Create new buffer with given descriptor and optional initial data.
        //GpuBuffer* CreateBuffer(const BufferDescriptor* descriptor, const void* initialData = nullptr);

        /// Create new vertex buffer.
        VertexBuffer* CreateVertexBuffer(uint32_t vertexCount, const std::vector<VertexElement>& elements, ResourceUsage resourceUsage = ResourceUsage::Default, const void* initialData = nullptr);

        /// Create new vertex buffer.
        VertexBuffer* CreateVertexBuffer(uint32_t vertexCount, size_t elementsCount, const VertexElement* elements, ResourceUsage resourceUsage = ResourceUsage::Default, const void* initialData = nullptr);

        /// Create new buffer with given descriptor and optional initial data.
        Texture* CreateTexture(const TextureDescriptor* descriptor, const ImageLevel* initialData = nullptr);

        /*
        /// Create new shader module using SPIRV bytecode.
        ShaderModule* CreateShaderModule(const std::vector<uint32_t>& spirv);

        /// Create new shader module from file and given entry point.
        ShaderModule* CreateShaderModule(const String& file, const String& entryPoint = "main");

        /// Create new shader program with descriptor.
        ShaderProgram* CreateShaderProgram(const ShaderProgramDescriptor* descriptor);

        /// Create new shader compute shader with descriptor.
        ShaderProgram* CreateShaderProgram(const ShaderStageDescriptor* stage);

        /// Create new shader program with vertex and fragment module.
        ShaderProgram* CreateShaderProgram(ShaderModule* vertex, ShaderModule* fragment);
        */

        /// Get whether grapics has been initialized.
        bool IsInitialized() const { return _initialized; }

        /// Get the type of device.
        GraphicsBackend GetBackend() const { return _backend; }

        /// Get the device features.
        const GraphicsDeviceFeatures& GetFeatures() const { return _features; }

        /// Get the main Swapchain current image view.
        virtual SharedPtr<TextureView> GetSwapchainView() const = 0;

        /// Get the main command buffer.
        virtual SharedPtr<CommandBuffer> GetMainCommandBuffer() const = 0;

        /// Request new command buffer.
        //SharedPtr<CommandBuffer> RequestCommandBuffer(CommandBuffer::Type type);

        void NotifyValidationError(const char* message);

    protected:
        virtual void Shutdown();
        //virtual GpuBuffer* CreateBufferImpl(const BufferDescriptor* descriptor, const void* initialData) = 0;
        virtual VertexBuffer* CreateVertexBufferImpl(uint32_t vertexCount, size_t elementsCount, const VertexElement* elements, ResourceUsage resourceUsage, const void* initialData) = 0;
        //virtual ShaderModule* CreateShaderModuleImpl(const std::vector<uint32_t>& spirv) = 0;
        //virtual ShaderProgram* CreateShaderProgramImpl(const ShaderProgramDescriptor* descriptor) = 0;
        virtual Texture* CreateTextureImpl(const TextureDescriptor* descriptor, const ImageLevel* initialData) = 0;

        GraphicsBackend _backend = GraphicsBackend::Empty;
        bool _validation;
        bool _initialized = false;
        RenderingSettings _settings = {};
        GraphicsDeviceFeatures _features = {};
        std::vector<GpuResource*> _gpuResources;
        std::mutex _gpuResourceMutex;

    private:
    };
}
