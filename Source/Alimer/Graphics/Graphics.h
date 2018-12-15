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
#include "../Graphics/GraphicsDeviceFeatures.h"
#include "../Graphics/CommandContext.h"
#include "../Graphics/GpuBuffer.h"
#include "../Graphics/Texture.h"
#include "../Graphics/Framebuffer.h"
#include "../Graphics/Shader.h"
#include "../Graphics/Sampler.h"
#include "../Graphics/Pipeline.h"
#include "../Graphics/RenderWindow.h"
#include <set>
#include <mutex>

namespace Alimer
{
    class GPUDevice;

    /// Low-level 3D graphics module.
    class ALIMER_API Graphics final: public Object
    {
        ALIMER_OBJECT(Graphics, Object);

    public:
        /// Constructor.
        Graphics(GraphicsBackend preferredBackend, bool validation);

        /// Destructor.
        ~Graphics() override;

        /// Register object.
        static void RegisterObject();

        /// Shutdown the Graphics module.
        static void Shutdown();

        /// Return the single instance of the Graphics.
        static Graphics* GetInstancePtr();
        
        /// Return the single instance of the Graphics.
        static Graphics& GetInstance();

        /// Check if backend is supported
        static bool IsBackendSupported(GraphicsBackend backend);

        /// Get all available backends.
        static std::set<GraphicsBackend> GetAvailableBackends();

        /// Get default best supported platform backend.
        static GraphicsBackend GetDefaultPlatformBackend();

        /// Initialize graphics with given settings.
        bool Initialize(const RenderWindowDescriptor* mainWindowDescriptor);

        /// Wait for a device to become idle.
        bool WaitIdle();

        /// Finishes the current frame and advances to next one.
        uint64_t Frame();

        /// Add a GraphicsResource to keep track of. 
        void AddGraphicsResource(GraphicsResource* resource);

        /// Remove a GraphicsResource.
        void RemoveGraphicsResource(GraphicsResource* resource);

        /// Get whether grapics has been initialized.
        bool IsInitialized() const { return _initialized; }

        /// Get the gpu device.
        GPUDevice* GetGPUDevice() const { return _device; }

        /// Get the backend.
        GraphicsBackend GetBackend() const { return _backend; }

        /// Get the device features.
        const GraphicsDeviceFeatures& GetFeatures() const;

        /// Get the main RenderWindow.
        RenderWindow* GetMainWindow() const;

        /// Get the immediate command context.
        CommandContext& GetImmediateContext() const { return *_immediateCommandContext; }

        static void NotifyValidationError(const char* message);

    private:
        virtual void OnFrame() {};

        static Graphics *_instance;
        GraphicsBackend _backend = GraphicsBackend::Empty;
        GPUDevice* _device = nullptr;
        bool _initialized = false;
        PODVector<GraphicsResource*> _gpuResources;
        std::mutex _gpuResourceMutex;
    
    protected:
        virtual void Finalize();

        bool _validation;
        uint64_t _frameIndex = 0;
        CommandContext* _immediateCommandContext = nullptr;
    };

    /// Singleton access for Graphics. 
    ALIMER_API Graphics& gGraphics();
}
