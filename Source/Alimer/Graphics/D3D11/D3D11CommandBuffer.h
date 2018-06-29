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
#include "D3D11Prerequisites.h"

namespace Alimer
{
	class D3D11Graphics;
    class D3D11RenderPass;

	/// D3D11 CommandBuffer implementation.
	class D3D11CommandBuffer final : public CommandBuffer
	{
	public:
		/// Constructor.
        D3D11CommandBuffer(D3D11Graphics* graphics, ID3D11DeviceContext1* context);

        /// Constructor.
        D3D11CommandBuffer(D3D11Graphics* graphics);

		/// Destructor.
		~D3D11CommandBuffer() override;

        void Destroy() override;
        void Enqueue() override;
        void Commit() override;

        void BeginRenderPass(RenderPass* renderPass, const Color* clearColors, uint32_t numClearColors, float clearDepth, uint8_t clearStencil) override;
        void EndRenderPass() override;

        void SetPipeline(const SharedPtr<PipelineState>& pipeline) override;

        void DrawCore(PrimitiveTopology topology, uint32_t vertexCount, uint32_t instanceCount, uint32_t vertexStart, uint32_t baseInstance) override;
        void DrawIndexedCore(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount, uint32_t startIndex) override;

    private:
        void ResetState() override;
        void OnSetVertexBuffer(GpuBuffer* buffer, uint32_t binding, uint64_t offset) override;
        void SetIndexBufferCore(GpuBuffer* buffer, uint32_t offset, IndexType indexType) override;
        bool PrepareDraw(PrimitiveTopology topology);

	private:
        Microsoft::WRL::ComPtr<ID3D11DeviceContext1> _context;
        bool _isImmediate;
        D3D11RenderPass* _currentRenderPass = nullptr;
	};
}
