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
#include "../Graphics/Shader.h"

namespace Alimer
{
	class Graphics;

    struct VertexBufferLayoutDescriptor
    {
        uint32_t stride;
        VertexInputRate inputRate;
    };

    struct VertexAttributeDescriptor
    {
        uint32_t binding;
        VertexFormat format;
        uint32_t offset;
    };

    struct VertexDescriptor
    {
        VertexBufferLayoutDescriptor layouts[MaxVertexBufferBindings] = {};
        VertexAttributeDescriptor attributes[MaxVertexAttributes] = {};
    };

	struct RenderPipelineDescriptor
	{
		SharedPtr<Shader> shader;
        VertexDescriptor vertexDescriptor;
		
	};

	/// Defines a PipelineState class.
	class PipelineState : public RefCounted
	{
	protected:
		/// Constructor.
		PipelineState(Graphics* graphics, bool isGraphics);

	public:
		/// Destructor.
		virtual ~PipelineState() = default;

		bool IsGraphics() const { return _isGraphics; }

	protected:
		Graphics* _graphics;
		bool _isGraphics;

	private:
		DISALLOW_COPY_MOVE_AND_ASSIGN(PipelineState);
	};
}
