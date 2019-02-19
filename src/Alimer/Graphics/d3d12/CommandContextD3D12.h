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

#include "../CommandContext.h"
#include "D3D12GraphicsState.h"

namespace alimer
{
	class D3D12Texture;
    class D3D12Framebuffer;
	class D3D12CommandListManager;

	/// D3D12 CommandContext implementation.
	class CommandContextD3D12 final : public CommandContext
	{
	public:
		/// Constructor.
        CommandContextD3D12(GraphicsDeviceD3D12* device);

		/// Destructor.
		~CommandContextD3D12() override;

        void PushDebugGroupImpl(const std::string& name, const Color4& color) override;
        void PopDebugGroupImpl() override;
        void InsertDebugMarkerImpl(const std::string& name, const Color4& color) override;

        // Flush existing commands to the GPU but keep the context alive
        uint64_t Flush(bool waitForCompletion = false);

        // Flush existing commands and release the current context
        uint64_t Finish(bool waitForCompletion = false);

		void TransitionResource(D3D12Resource* resource, D3D12_RESOURCE_STATES newState, bool flushImmediate = false);
		void BeginResourceTransition(D3D12Resource* resource, D3D12_RESOURCE_STATES newState, bool flushImmediate = false);
		void InsertUAVBarrier(D3D12Resource* resource, bool flushImmediate = false);
		void FlushResourceBarriers();

        D3D12_COMMAND_LIST_TYPE GetCommandListType() const { return _type; }

	private:
        /// Commit for execution and optionally wait for completion.
        //uint64_t Finish(bool waitForCompletion, bool releaseContext) override;

        //void BeginRenderPassImpl(Framebuffer* framebuffer, const RenderPassBeginDescriptor* descriptor) override;
        //void EndRenderPassImpl() override;
        //void SetIndexBufferImpl(IndexBuffer* buffer, uint32_t offset, IndexType indexType) override;

        //void SetPrimitiveTopologyImpl(PrimitiveTopology topology) override;
        //void DrawImpl(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override;
        void FlushGraphicsState();

		D3D12CommandListManager*    _manager;
		ID3D12GraphicsCommandList*  _commandList;
		ID3D12CommandAllocator*     _currentAllocator;
		D3D12_COMMAND_LIST_TYPE     _type;
        uint64_t                    _fenceValue;

        D3D12_RESOURCE_BARRIER      _resourceBarrierBuffer[16] = {};
		uint32_t                    _numBarriersToFlush = 0;

		uint32_t                    _boundRTVCount = 0;
        D3D12Resource*              _boundRTVResources[MaxColorAttachments] = {};
        D3D12_CPU_DESCRIPTOR_HANDLE _boundRTV[MaxColorAttachments] = {};

        D3D12Framebuffer*           _currentFramebuffer = nullptr;
		ID3D12PipelineState*        _currentD3DPipeline = nullptr;

        D3D12GraphicsState          _graphicsState;
	};
}
