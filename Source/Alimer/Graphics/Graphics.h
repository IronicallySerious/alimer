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

#include "../Graphics/Types.h"
#include "../Core/Window.h"
#include "../Graphics/GpuBuffer.h"
#include "../Graphics/Texture.h"
#include "../Graphics/Shader.h"
#include "../Graphics/PipelineState.h"
#include "../Graphics/CommandBuffer.h"
#include <vector>
#include <atomic>

namespace Alimer
{
	/// Low-level 3D graphics API class.
	class Graphics
	{
	protected:
		/// Constructor.
		Graphics();

	public:
		/// Destructor.
		virtual ~Graphics();

		/// Factory method for Graphics creation.
		static Graphics* Create(GraphicsDeviceType deviceType);

		virtual bool Initialize(std::shared_ptr<Window> window);

		/// Wait for a device to become idle
		virtual bool WaitIdle() = 0;

		/// Begin rendering frame and return current backbuffer texture.
		virtual std::shared_ptr<Texture> AcquireNextImage() = 0;

		/// Present frame.
		virtual bool Present() = 0;

		// CommandBuffer
		virtual CommandBufferPtr CreateCommandBuffer() = 0;

		// Buffer
		virtual GpuBufferPtr CreateBuffer(BufferUsage usage, uint32_t elementCount, uint32_t elementSize, const void* initialData = nullptr) = 0;

		// PipelineLayout
		virtual PipelineLayoutPtr CreatePipelineLayout() = 0;

		// PipelineState
		virtual PipelineStatePtr CreateRenderPipelineState(const RenderPipelineDescriptor& descriptor) = 0;

	protected:
		std::shared_ptr<Window> _window;
		std::vector<std::shared_ptr<Texture>> _textures;

	private:
		DISALLOW_COPY_MOVE_AND_ASSIGN(Graphics);
	};

	// Direct access to Graphics module.
	extern Graphics* graphics;
}
