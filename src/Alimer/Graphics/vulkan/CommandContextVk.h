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

#include "BackendVk.h"
#include "../CommandContext.h"

namespace alimer
{
	/// Vulkan CommandContext implementation.
	class CommandContextVk final : public CommandContext
	{
	public:
        CommandContextVk(GraphicsDevice* device, VkCommandBuffer commandBuffer);
		~CommandContextVk() override;

        void Begin(VkCommandBufferUsageFlags flags);
        void End();
       
        VkCommandBuffer GetVkCommandBuffer() const { return _commandBuffer; }

        inline bool IsInsideRenderPass() const
        {
            return _state == State::InsideRenderPass;
        }

        inline bool IsOutsideRenderPass() const
        {
            return _state == State::InsideBegin;
        }

        inline bool IsSubmitted() const
        {
            return _state == State::Submitted;
        }

	private:
        void PushDebugGroupImpl(const std::string& name, const Color4& color) override;
        void PopDebugGroupImpl() override;
        void InsertDebugMarkerImpl(const std::string& name, const Color4& color) override;

        void BeginRenderPassImpl(const RenderPassDescriptor* descriptor) override;
        void EndRenderPassImpl() override;

        //void SetPipelineImpl(Pipeline* pipeline) override;

        //void SetVertexBufferImpl(GpuBuffer* buffer, uint32_t offset) override;
        //void SetVertexBuffersImpl(uint32_t firstBinding, uint32_t count, const GpuBuffer** buffers, const uint32_t* offsets) override;
        //void SetIndexBufferImpl(GpuBuffer* buffer, uint32_t offset, uint32_t stride) override;

        //void DrawImpl(PrimitiveTopology topology, uint32_t vertexCount, uint32_t startVertexLocation) override;
        //void DrawInstancedImpl(PrimitiveTopology topology, uint32_t vertexCount, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation) override;
        //void DrawIndexedImpl(PrimitiveTopology topology, uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation) override;
        //void DrawIndexedInstancedImpl(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation) override;

        //void SetViewport(const rect& viewport) override;
        //void SetScissor(const irect& scissor) override;

        //void DispatchImpl(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;
        void FlushRenderState(PrimitiveTopology topology);
        void FlushDescriptorSets();
        void FlushDescriptorSet(uint32_t set);
        void FlushGraphicsPipeline();

    private:
        VkDevice        _vkDevice;
        VkCommandBuffer _commandBuffer;
        bool            _supportsDebugUtils;
        bool            _supportsDebugMarker;

        enum class State : uint8_t
        {
            Ready,
            InsideBegin,
            InsideRenderPass,
            Ended,
            Submitted
        };

        // State
        class GraphicsState
        {
        public:
            GraphicsState();

            void Reset();

            //const VertexDescriptor& GetVertexDescriptor() const { return _vertexDescriptor; }
            //void SetVertexDescriptor(const VertexDescriptor* descriptor);

            bool IsDirty() const { return _dirty; }
            void ClearDirty() { _dirty = false; }

        private:
            //VertexDescriptor _vertexDescriptor;
            bool _dirty;
        };

        State _state = State::Ready;
        GraphicsState _graphicsState;
        //const VulkanFramebuffer* _currentFramebuffer = nullptr;
        //const VulkanRenderPass* _currentRenderPass = nullptr;
        
        uint32_t _currentSubpass = 0;
        PrimitiveTopology _currentTopology;

        VkPipeline _currentPipeline = VK_NULL_HANDLE;
        VkPipelineLayout _currentPipelineLayout = VK_NULL_HANDLE;
        //VulkanPipelineLayout* _currentLayout = nullptr;

        //VulkanProgram* _currentVkProgram = nullptr;

        enum CommandBufferDirtyBits
        {
            COMMAND_BUFFER_DIRTY_STATIC_STATE_BIT = 1 << 0,
            COMMAND_BUFFER_DIRTY_PIPELINE_BIT = 1 << 1,

            COMMAND_BUFFER_DIRTY_DEPTH_BIAS_BIT = 1 << 2,
            COMMAND_BUFFER_DIRTY_STENCIL_REFERENCE_BIT = 1 << 3,

            COMMAND_BUFFER_DIRTY_STATIC_VERTEX_BIT = 1 << 4,

            COMMAND_BUFFER_DIRTY_PUSH_CONSTANTS_BIT = 1 << 5,

            COMMAND_BUFFER_DYNAMIC_BITS = 
            COMMAND_BUFFER_DIRTY_DEPTH_BIAS_BIT |
            COMMAND_BUFFER_DIRTY_STENCIL_REFERENCE_BIT
        };
        using CommandBufferDirtyFlags = uint32_t;

        CommandBufferDirtyFlags _dirtyFlags = ~0u;
        VkBuffer _currentVertexBuffers[MaxVertexBufferBindings];
        VkDeviceSize _vboOffsets[MaxVertexBufferBindings];

        void SetDirty(CommandBufferDirtyFlags flags)
        {
            _dirtyFlags |= flags;
        }

        CommandBufferDirtyFlags GetAndClear(CommandBufferDirtyFlags flags)
        {
            auto mask = _dirtyFlags & flags;
            _dirtyFlags &= ~flags;
            return mask;
        }
	};
}
