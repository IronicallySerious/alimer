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
#include "../Graphics/PipelineState.h"
#include "../Graphics/CommandBuffer.h"
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

        /// Return the single instance of the Graphics.
        static Graphics* GetInstance();

        /// Get supported graphics backends.
        static std::set<GraphicsDeviceType> GetAvailableBackends();

        /// Factory method for Graphics creation.
        static Graphics* Create(GraphicsDeviceType deviceType, bool validation = false, const std::string& applicationName = "Alimer");

        /// Initialize graphics with given adapter and window.
        bool Initialize(GpuAdapter* adapter, WindowPtr window);

        /// Wait for a device to become idle.
        virtual void WaitIdle() = 0;

        /// Begin frame rendering.
        virtual bool BeginFrame() = 0;

        /// End and present frame.
        virtual void EndFrame() = 0;

        /// Try to save screenshot with given file name.
        void SaveScreenshot(const std::string& fileName);

        virtual CommandBuffer* GetDefaultCommandBuffer() const = 0;

        // RenderPass
        virtual SharedPtr<RenderPass> CreateRenderPass(const RenderPassDescription& description) = 0;

        // Buffer
        virtual GpuBuffer* CreateBuffer(const GpuBufferDescription& description, const void* initialData = nullptr) = 0;

        // Shader
        Shader* CreateShader(
            const std::string& vertexShaderFile,
            const std::string& fragmentShaderFile);

        virtual Shader* CreateComputeShader(const void *pCode, size_t codeSize) = 0;
        virtual Shader* CreateShader(
            const void *pVertexCode, size_t vertexCodeSize,
            const void *pFragmentCode, size_t fragmentCodeSize) = 0;

        // PipelineState
        virtual PipelineState* CreateRenderPipelineState(const RenderPipelineDescription& description) = 0;

        inline GraphicsDeviceType GetDeviceType() const { return _deviceType; }

        /// Get supported adapters.
        const std::vector<GpuAdapter*>& GetAdapters() const { return _adapters; }

        /// Get the default and best adapter.
        GpuAdapter* GetDefaultAdapter() const { return _adapters[0]; }

        /// Get the device features.
        inline const GpuDeviceFeatures& GetFeatures() const { return _features; }

        /// Get default command buffer.
        //CommandBuffer* GetDefaultCommandBuffer() const { return _defaultCommandBuffer; }

    private:
        /// Add a GpuResource to keep track of. 
        void AddGpuResource(GpuResource* resource);

        /// Remove a GpuResource.
        void RemoveGpuResource(GpuResource* resource);

    protected:
        virtual void Finalize();
        virtual bool BackendInitialize() = 0;

        virtual void GenerateScreenshot(const std::string& fileName) {}

    protected:
        GraphicsDeviceType _deviceType;
        bool _validation;
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

    /// Access to current Graphics module.
    ALIMER_API Graphics& gGraphics();
}
