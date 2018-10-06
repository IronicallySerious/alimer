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
#include "../Graphics/GpuBuffer.h"
#include "../Graphics/Texture.h"
#include "../Graphics/RenderPass.h"
#include "../Graphics/Shader.h"
#include "../Graphics/CommandBuffer.h"
#include "../Graphics/VertexFormat.h"
#include <vector>
#include <set>
#include <mutex>

namespace Alimer
{
    /// Enum describing the Graphics backend.
    enum class GraphicsBackend : uint32_t
    {
        /// Best device supported for running platform.
        Default,
        /// Empty/Headless device type.
        Empty,
        /// Vulkan backend.
        Vulkan,
        /// DirectX 11.1+ backend.
        Direct3D11,
        /// DirectX 12 backend.
        Direct3D12,
    };

    enum class GpuVendor : uint8_t
    {
        Unknown,
        Arm,
        Nvidia,
        Amd,
        Intel,
        Warp,
        Count
    };

    class GraphicsImpl;

    class ALIMER_API RenderingSettings
    {
    public:


        // Main swap chain settings.
        uint32_t defaultBackBufferWidth = 1280;
        uint32_t defaultBackBufferHeight = 720;
        WindowHandle windowHandle = 0;
    };

    /// Low-level 3D graphics device class.
    class ALIMER_API GraphicsDevice final : public Object
    {
        friend class GpuResource;

        ALIMER_OBJECT(GraphicsDevice, Object);

    public:
        /// Constructor.
        GraphicsDevice(GraphicsBackend preferredGraphicsBackend = GraphicsBackend::Default, bool validation = false);

        /// Destructor.
        ~GraphicsDevice() override;

        /// Get supported graphics backends.
        static std::set<GraphicsBackend> GetAvailableBackends();

        /// Initialize graphics with given settings.
        bool Initialize(const RenderingSettings& settings);

        /// Wait for a device to become idle.
        void WaitIdle();

        /// Begin the rendering frame.
        bool BeginFrame();

        /// Finishes the current frame and schedules it for display.
        void EndFrame();

        /*
        /// Create new RenderPass given descriptor
        RenderPass* CreateRenderPass(const RenderPassDescription* descriptor);

        /// Create new buffer with given descriptor and optional initial data.
        GpuBuffer* CreateBuffer(const BufferDescriptor* descriptor, const void* initialData = nullptr);

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
        */

        /// Get whether grapics has been initialized.
        bool IsInitialized() const { return _initialized; }

        /// Get the type of device.
        GraphicsBackend GEetBackend() const { return _backend; }

        /// Get the device features.
        const GpuDeviceFeatures& GetFeatures() const { return _features; }

        /// Get the main command buffer.
        CommandBuffer* GetMainCommandBuffer() const { return _mainCommandBuffer.Get(); }

        /// Create new command buffer.
        //virtual CommandBuffer* CreateCommandBuffer() = 0;

        /// Get backend implementation.
        GraphicsImpl *GetImplementation() const { return _impl; }

    private:
        /// Add a GpuResource to keep track of. 
        void AddGpuResource(GpuResource* resource);

        /// Remove a GpuResource.
        void RemoveGpuResource(GpuResource* resource);
        //virtual RenderPass* CreateRenderPassImpl(const RenderPassDescription* descriptor) = 0;
        //virtual GpuBuffer* CreateBufferImpl(const BufferDescriptor* descriptor, const void* initialData) = 0;
        //virtual VertexInputFormat* CreateVertexInputFormatImpl(const VertexInputFormatDescriptor* descriptor) = 0;
        //virtual ShaderModule* CreateShaderModuleImpl(const std::vector<uint32_t>& spirv) = 0;
        //virtual ShaderProgram* CreateShaderProgramImpl(const ShaderProgramDescriptor* descriptor) = 0;
        //virtual Texture* CreateTextureImpl(const TextureDescriptor* descriptor, const ImageLevel* initialData = nullptr) = 0;

        GraphicsBackend _backend = GraphicsBackend::Empty;
        bool _initialized = false;
        RenderingSettings _settings{};
        GpuDeviceFeatures _features{};

        GraphicsImpl* _impl = nullptr;
        UniquePtr<CommandBuffer> _mainCommandBuffer;
        std::mutex _gpuResourceMutex;
        std::vector<GpuResource*> _gpuResources;
    };
}
