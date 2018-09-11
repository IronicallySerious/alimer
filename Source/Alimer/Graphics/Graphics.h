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
#include "../Application/Window.h"
#include "../Graphics/Types.h"
#include "../Graphics/GpuDeviceFeatures.h"
#include "../Graphics/GpuAdapter.h"
#include "../Graphics/GpuBuffer.h"
#include "../Graphics/Texture.h"
#include "../Graphics/RenderPass.h"
#include "../Graphics/Shader.h"
#include "../Graphics/CommandBuffer.h"
#include "../Graphics/VertexFormat.h"
#include <vector>
#include <set>
#include <queue>
#include <mutex>
#include <atomic>

namespace Alimer
{
    /// Low-level 3D graphics API class.
    class ALIMER_API Graphics : public Object
    {
        friend class GpuResource;

        ALIMER_OBJECT(Graphics, Object);

    protected:
        /// Constructor.
        Graphics(GraphicsDeviceType deviceType, bool validation = false);

    public:
        /// Destructor.
        virtual ~Graphics();

        /// Get supported graphics backends.
        static std::set<GraphicsDeviceType> GetAvailableBackends();

        /// Factory method for Graphics creation.
        static Graphics* Create(GraphicsDeviceType deviceType, bool validation = false, const String& applicationName = "Alimer");

        /// Initialize graphics with given adapter and window.
        bool Initialize(GpuAdapter* adapter, WindowPtr window);

        /// Wait for a device to become idle.
        virtual void WaitIdle() = 0;

        /// Begin frame rendering.
        virtual bool BeginFrame() = 0;

        /// End and present frame.
        virtual void EndFrame() = 0;

        /// Create new RenderPass given descriptor
        RenderPass* CreateRenderPass(const RenderPassDescription* descriptor);

        /// Create new buffer with given descriptor and optional initial data.
        GpuBuffer* CreateBuffer(MemoryFlags memoryFlags, const BufferDescriptor* descriptor, const void* initialData = nullptr);

        /// Create new VertexInputFormat with given descriptor.
        VertexInputFormat* CreateVertexInputFormat(const VertexInputFormatDescriptor* descriptor);

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

        /// Create new Texture with given descriptor and optional initial data.
        Texture* CreateTexture(const TextureDescriptor* descriptor, const ImageLevel* initialData = nullptr);

        /// Get whether grapics has been initialized.
        bool IsInitialized() const { return _initialized; }

        /// Get the type of device.
        GraphicsDeviceType GetDeviceType() const { return _deviceType; }

        /// Get supported adapters.
        const std::vector<GpuAdapter*>& GetAdapters() const { return _adapters; }

        /// Get the default and best adapter.
        GpuAdapter* GetDefaultAdapter() const { return _adapters[0]; }

        /// Get the device features.
        const GpuDeviceFeatures& GetFeatures() const { return _features; }

        /// Get the default command buffer.
        virtual CommandBuffer* GetDefaultCommandBuffer() const = 0;

        /// Create new command buffer.
        virtual CommandBuffer* CreateCommandBuffer() = 0;

    private:
        /// Add a GpuResource to keep track of. 
        void AddGpuResource(GpuResource* resource);

        /// Remove a GpuResource.
        void RemoveGpuResource(GpuResource* resource);

    protected:
        virtual void Finalize();
        void ClearAdapters();
        virtual bool BackendInitialize() = 0;

        virtual RenderPass* CreateRenderPassImpl(const RenderPassDescription* descriptor) = 0;
        virtual GpuBuffer* CreateBufferImpl(MemoryFlags memoryFlags, const BufferDescriptor* descriptor, const void* initialData) = 0;
        virtual VertexInputFormat* CreateVertexInputFormatImpl(const VertexInputFormatDescriptor* descriptor) = 0;
        virtual ShaderModule* CreateShaderModuleImpl(const std::vector<uint32_t>& spirv) = 0;
        virtual ShaderProgram* CreateShaderProgramImpl(const ShaderProgramDescriptor* descriptor) = 0;
        virtual Texture* CreateTextureImpl(const TextureDescriptor* descriptor, const ImageLevel* initialData = nullptr) = 0;

    protected:
        GraphicsDeviceType _deviceType;
        bool _validation;
        bool _initialized;
        std::vector<GpuAdapter*> _adapters;
        GpuDeviceFeatures _features;

        WindowPtr _window{};
        GpuAdapter* _adapter;

    private:
        std::mutex _gpuResourceMutex;
        std::vector<GpuResource*> _gpuResources;

    private:
        DISALLOW_COPY_MOVE_AND_ASSIGN(Graphics);
    };
}
