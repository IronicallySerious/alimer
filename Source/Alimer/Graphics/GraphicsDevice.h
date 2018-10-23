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
#include "../Graphics/GraphicsDeviceFeatures.h"
#include "../Graphics/CommandContext.h"
#include "../Graphics/GpuBuffer.h"
#include "../Graphics/Texture.h"
#include "../Graphics/Framebuffer.h"
#include "../Graphics/Shader.h"
#include "../Graphics/ShaderCompiler.h"
#include <vector>
#include <mutex>
#include <set>

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
        /// Destructor.
        virtual ~GraphicsDevice() override;

        /// Check if backend is supported
        static bool IsBackendSupported(GraphicsBackend backend);

        /// Get all available backends.
        static std::set<GraphicsBackend> GetAvailableBackends();

        /// Create new graphics device instance.
        static GraphicsDevice* Create(GraphicsBackend prefferedBackend = GraphicsBackend::Default, bool validation = false);

        /// Initialize graphics with given settings.
        virtual bool Initialize(const RenderingSettings& settings);

        /// Wait for a device to become idle.
        virtual bool WaitIdle() = 0;

        /// Finishes the current frame and schedules it for display.
        uint32_t Present();

        /// Add a GraphicsResource to keep track of. 
        void AddGraphicsResource(GraphicsResource* resource);

        /// Remove a GraphicsResource.
        void RemoveGraphicsResource(GraphicsResource* resource);

        /// Create new buffer with given descriptor and optional initial data.
        GpuBuffer* CreateBuffer(const BufferDescriptor* descriptor, const void* initialData = nullptr);

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

        /// Get the backend.
        GraphicsBackend GetBackend() const { return _backend; }

        /// Get the device features.
        const GraphicsDeviceFeatures& GetFeatures() const { return _features; }

        /// Get the main Swapchain current framebuffer.
        virtual Framebuffer* GetSwapchainFramebuffer() const = 0;

        /// Get the default command context.
        const SharedPtr<CommandContext>& GetContext() const { return _context; }

        ShaderManager &GetShaderManager();

        void NotifyValidationError(const char* message);

    protected:
        virtual void Shutdown();
        virtual void PresentImpl() = 0;
        virtual GpuBuffer* CreateBufferImpl(const BufferDescriptor* descriptor, const void* initialData) = 0;
        //virtual std::unique_ptr<ShaderModule> CreateShaderModuleImpl(Util::Hash hash, const uint32_t* pCode, size_t size) = 0;
        //virtual std::unique_ptr<Program> CreateProgramImpl(Util::Hash hash, const std::vector<ShaderModule*>& stages) = 0;
        virtual Texture* CreateTextureImpl(const TextureDescriptor* descriptor, const ImageLevel* initialData) = 0;

        GraphicsBackend _backend = GraphicsBackend::Empty;
        bool _validation;
        bool _initialized = false;
        RenderingSettings _settings = {};
        GraphicsDeviceFeatures _features = {};
        std::vector<GraphicsResource*> _gpuResources;
        std::mutex _gpuResourceMutex;
        SharedPtr<CommandContext> _context;

    private:
        ShaderManager _shaderManager;
        Cache<ShaderModule> _shaders;
        Cache<Program> _programs;
        uint32_t _frameIndex = 0;
    };
}
