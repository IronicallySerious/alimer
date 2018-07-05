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

#include "Graphics/CommandBuffer.h"
#include "Graphics/CommandEncoder.h"
#include "D3D11Prerequisites.h"

namespace Alimer
{
    class D3D11RenderPass;

	/// D3D11 CommandBuffer implementation.
	class D3D11RenderPassCommandEncoder final : public RenderPassCommandEncoder
	{
	public:
		/// Constructor.
        explicit D3D11RenderPassCommandEncoder(CommandBuffer* commandBuffer, ID3D11DeviceContext1* context);

		/// Destructor.
		~D3D11RenderPassCommandEncoder() override;

        void BeginRenderPass(RenderPass* renderPass, const Color* clearColors, uint32_t numClearColors, float clearDepth, uint8_t clearStencil);
        void EndEncodingCore() override;

        void DrawCore(PrimitiveTopology topology, uint32_t vertexCount, uint32_t instanceCount, uint32_t vertexStart, uint32_t baseInstance) override;

    private:
        bool PrepareDraw(PrimitiveTopology topology);

	private:
        ID3D11DeviceContext1 * _context;
        D3D11RenderPass* _currentRenderPass = nullptr;
	};
}
