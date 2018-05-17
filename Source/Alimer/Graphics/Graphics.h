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

#include "../Core/Window.h"
#include "../Graphics/Texture.h"
#include <new>
#include <memory>
#include <atomic>

namespace Alimer
{
	enum class GraphicsDeviceType
	{
		Default,
		Empty,
		Direct3D12
	};

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
		virtual std::shared_ptr<Texture> BeginFrame() = 0;

		/// Present frame.
		virtual bool Present() = 0;
	protected:
		std::shared_ptr<Window> _window;

	private:
		DISALLOW_COPY_MOVE_AND_ASSIGN(Graphics);
	};

	extern Graphics* graphics;
}
