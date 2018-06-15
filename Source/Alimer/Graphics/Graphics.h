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

#include "../Core/Ptr.h"
#include "../Graphics/Types.h"
#include "../Application/Window.h"
#include "../Graphics/GpuBuffer.h"
#include "../Graphics/Texture.h"
#include "../Graphics/Shader.h"
#include "../Graphics/PipelineState.h"
#include "../Graphics/CommandQueue.h"
#include <vector>
#include <set>
#include <mutex>
#include <atomic>

namespace Alimer
{
	/// Low-level 3D graphics API class.
	class ALIMER_API Graphics : public RefCounted
	{
        friend class GpuResource;

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

        /// Initialize graphics with given window.
		virtual bool Initialize(const SharedPtr<Window>& window);

		/// Wait for a device to become idle
		virtual bool WaitIdle() = 0;

		/// Begin rendering frame and return current backbuffer texture.
		virtual SharedPtr<Texture> AcquireNextImage() = 0;

		/// Present frame.
		virtual bool Present() = 0;

        virtual void Frame() = 0;

		// Buffer
		virtual SharedPtr<GpuBuffer> CreateBuffer(const GpuBufferDescription& description, const void* initialData = nullptr) = 0;

		// Shader
		SharedPtr<Shader> CreateShader(const std::string& vertexShaderFile, const std::string& fragmentShaderFile);
		virtual SharedPtr<Shader> CreateComputeShader(const ShaderStageDescription& desc) = 0;
		virtual SharedPtr<Shader> CreateShader(
			const ShaderStageDescription& vertex,
			const ShaderStageDescription& fragment) = 0;

		// PipelineState
		virtual SharedPtr<PipelineState> CreateRenderPipelineState(const RenderPipelineDescriptor& descriptor) = 0;

        inline GraphicsDeviceType GetDeviceType() const { return _deviceType; }

        /// Get default command queue.
        CommandQueue* GetCommandQueue() const { return _commandQueue.Get(); }

    private:
        /// Add a GpuResource to keep track of. 
        void AddGpuResource(GpuResource* resource);

        /// Remove a GpuResource.
        void RemoveGpuResource(GpuResource* resource);

	protected:
		virtual void Finalize();

	protected:
        GraphicsDeviceType _deviceType;
        bool _validation;
        SharedPtr<Window> _window{};
        SharedPtr<CommandQueue> _commandQueue;

    private:
        std::mutex _gpuResourceMutex;
        std::vector<GpuResource*> _gpuResources;

	private:
		DISALLOW_COPY_MOVE_AND_ASSIGN(Graphics);
	};

    /// Access to current Graphics module.
    ALIMER_API Graphics& gGraphics();
}
