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
#include "../Graphics/GpuBuffer.h"
#include "../Graphics/RenderPass.h"
#include "../Graphics/PipelineState.h"
#include "../Math/Color.h"

namespace Alimer
{
	class Graphics;

	/// Defines a command buffer for storing recorded gpu commands.
	class ALIMER_API CommandBuffer : public GpuResource, public RefCounted
	{
	protected:
		/// Constructor.
		CommandBuffer(Graphics* graphics);

	public:
		/// Destructor.
		virtual ~CommandBuffer();

        /// Reserves a place for this command buffer on its associated command queue.
        virtual void Enqueue() = 0;

        /// Commits this command buffer for execution as soon as possible.
        virtual void Commit() = 0;

		virtual void BeginRenderPass(RenderPass* renderPass,
            const Color& clearColor = Color::Black,
            float clearDepth = 1.0f, uint8_t clearStencil = 0);

        virtual void BeginRenderPass(RenderPass* renderPass,
            const Color* clearColors, uint32_t numClearColors,
            float clearDepth = 1.0f, uint8_t clearStencil = 0) = 0;

		virtual void EndRenderPass() = 0;

		void SetVertexBuffer(GpuBuffer* buffer, uint32_t binding, uint64_t offset = 0, VertexInputRate inputRate = VertexInputRate::Vertex);
        void SetIndexBuffer(GpuBuffer* buffer, uint32_t offset = 0, IndexType indexType = IndexType::UInt16);
		virtual void SetPipeline(const SharedPtr<PipelineState>& pipeline) = 0;

        void SetUniformBuffer(uint32_t set, uint32_t binding, const GpuBuffer* buffer);

		void Draw(PrimitiveTopology topology, uint32_t vertexCount, uint32_t instanceCount = 1u, uint32_t vertexStart = 0u, uint32_t baseInstance = 0u);
		void DrawIndexed(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount = 1u, uint32_t startIndex = 0u);

	protected:
		virtual void ResetState();
		virtual void DrawCore(PrimitiveTopology topology, uint32_t vertexCount, uint32_t instanceCount, uint32_t vertexStart, uint32_t baseInstance) = 0;
		virtual void DrawIndexedCore(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount, uint32_t startIndex) = 0;
        virtual void OnSetVertexBuffer(GpuBuffer* buffer, uint32_t binding, uint64_t offset) {}
        virtual void SetIndexBufferCore(GpuBuffer* buffer, uint32_t offset, IndexType indexType) = 0;

		enum CommandBufferDirtyBits
		{
			COMMAND_BUFFER_DIRTY_STATIC_VERTEX_BIT = 1 << 0,
		};
		using CommandBufferDirtyFlags = uint32_t;
		void SetDirty(CommandBufferDirtyFlags flags)
		{
			_dirty |= flags;
		}

		CommandBufferDirtyFlags GetAndClear(CommandBufferDirtyFlags flags)
		{
			auto mask = _dirty & flags;
			_dirty &= ~flags;
			return mask;
		}


		struct VertexBindingState
		{
			GpuBuffer* buffers[MaxVertexBufferBindings];
			uint64_t offsets[MaxVertexBufferBindings];
			uint64_t strides[MaxVertexBufferBindings];
			VertexInputRate inputRates[MaxVertexBufferBindings];
		};

        struct ResourceBindingBufferInfo {
            const GpuBuffer*  buffer;
            uint64_t    offset;
            uint64_t    range;
        };

        struct ResourceBinding
        {
            union {
                ResourceBindingBufferInfo buffer;
            };
        };

        struct ResourceBindings
        {
            ResourceBinding bindings[MaxDescriptorSets][MaxBindingsPerSet];
            uint8_t push_constant_data[MaxDescriptorSets];
        };

		VertexBindingState _vbo = {};
        ResourceBindings _bindings;

		CommandBufferDirtyFlags _dirty = ~0u;
        uint32_t _dirtySets = 0;
		uint32_t _dirtyVbos = 0;

	private:
		DISALLOW_COPY_MOVE_AND_ASSIGN(CommandBuffer);
	};
}
