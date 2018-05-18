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

#include "../Graphics/PixelFormat.h"
#include <memory>

namespace Alimer
{
	class Graphics;

	enum class BufferUsage
	{
		Undefined = 0,
		Vertex = 1 << 0,
		Index = 1 << 1,
		Uniform = 1 << 2,
	};
	ALIMER_BITMASK(BufferUsage);

	/// Defines a GPU Buffer class.
	class GpuBuffer
	{
	protected:
		/// Constructor.
		GpuBuffer(Graphics* graphics, BufferUsage bufferUsage, uint32_t elementCount, uint32_t elementSize);

	public:
		/// Destructor.
		virtual ~GpuBuffer() = default;

		inline BufferUsage GetBufferUsage() const { return _bufferUsage; }
		inline uint32_t GetElementCount() const { return _elementCount; }
		inline uint32_t GetElementSize() const { return _elementSize; }
		inline uint64_t GetSize() const { return _size; }

	protected:
		Graphics* _graphics;
		BufferUsage _bufferUsage;
		uint32_t _elementCount;
		uint32_t _elementSize;
		uint64_t _size;

	private:
		DISALLOW_COPY_MOVE_AND_ASSIGN(GpuBuffer);
	};

	using GpuBufferPtr = std::shared_ptr<GpuBuffer>;
}
