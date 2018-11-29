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
#include "D3D12Prerequisites.h"
#include <vector>
#include <queue>
#include <mutex>

#define VALID_COMPUTE_QUEUE_RESOURCE_STATES \
	( D3D12_RESOURCE_STATE_UNORDERED_ACCESS \
	| D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE \
	| D3D12_RESOURCE_STATE_COPY_DEST \
	| D3D12_RESOURCE_STATE_COPY_SOURCE )

namespace Alimer
{
	class D3D12Graphics;
	class D3D12Texture;
    class D3D12Framebuffer;
	class D3D12CommandListManager;

	/// D3D12 CommandContext implementation.
	class D3D12CommandContext final : public CommandContext
	{
	public:
		/// Constructor.
        D3D12CommandContext(D3D12Graphics* graphics, D3D12_COMMAND_LIST_TYPE type);

		/// Destructor.
		~D3D12CommandContext() override;

        void Initialize();
		void Reset();

		void TransitionResource(D3D12Resource* resource, D3D12_RESOURCE_STATES newState, bool flushImmediate = false);
		void BeginResourceTransition(D3D12Resource* resource, D3D12_RESOURCE_STATES newState, bool flushImmediate = false);
		void InsertUAVBarrier(D3D12Resource* resource, bool flushImmediate = false);
		void FlushResourceBarriers();

        D3D12_COMMAND_LIST_TYPE GetCommandListType() const { return _type; }

	private:
        /// Commit for execution and optionally wait for completion.
        uint32_t Finish(bool waitForCompletion, bool releaseContext) override;

        void BeginRenderPassImpl(Framebuffer* framebuffer, const RenderPassBeginDescriptor* descriptor) override;
        void EndRenderPassImpl() override;

        D3D12Graphics*              _graphics;
		D3D12CommandListManager*    _manager;
		ID3D12GraphicsCommandList*  _commandList;
		ID3D12CommandAllocator*     _currentAllocator;
		D3D12_COMMAND_LIST_TYPE     _type;

        D3D12_RESOURCE_BARRIER      _resourceBarrierBuffer[16] = {};
		uint32_t                    _numBarriersToFlush = 0;

		uint32_t                    _boundRTVCount = 0;
        D3D12Resource*              _boundRTVResources[MaxColorAttachments] = {};
        D3D12_CPU_DESCRIPTOR_HANDLE _boundRTV[MaxColorAttachments] = {};

        D3D12Framebuffer*           _currentFramebuffer = nullptr;
		ID3D12PipelineState*        _currentD3DPipeline = nullptr;
		PrimitiveTopology           _currentTopology = PrimitiveTopology::Count;
	};
}
