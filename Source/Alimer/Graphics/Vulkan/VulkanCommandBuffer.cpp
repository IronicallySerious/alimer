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

#include "VulkanCommandQueue.h"
#include "VulkanCommandBuffer.h"
#include "VulkanBuffer.h"
#include "VulkanShader.h"
#include "VulkanPipelineLayout.h"
#include "VulkanPipelineState.h"
#include "VulkanRenderPass.h"
#include "VulkanGraphics.h"
#include "VulkanConvert.h"
#include "../../Core/Log.h"
#include "../../Util/HashMap.h"

namespace Alimer
{
    VulkanCommandBuffer::VulkanCommandBuffer(VulkanCommandQueue* queue)
        : CommandBuffer(queue->GetGraphics())
        , _vkGraphics(static_cast<VulkanGraphics*>(queue->GetGraphics()))
        , _logicalDevice(_vkGraphics->GetLogicalDevice())
        , _queue(queue)
        , _enqueued(false)
    {
        VkCommandBufferAllocateInfo cmdBufAllocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
        cmdBufAllocateInfo.pNext = nullptr;
        cmdBufAllocateInfo.commandPool = queue->GetVkHandle();
        cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmdBufAllocateInfo.commandBufferCount = 1;
        vkThrowIfFailed(vkAllocateCommandBuffers(
            _logicalDevice,
            &cmdBufAllocateInfo,
            &_vkHandle));
    }

    VulkanCommandBuffer::~VulkanCommandBuffer()
    {
        vkFreeCommandBuffers(_logicalDevice, _queue->GetVkHandle(), 1, &_vkHandle);
    }

    void VulkanCommandBuffer::ResetState()
    {
        CommandBuffer::ResetState();
        memset(&_indexState, 0, sizeof(_indexState));

        _currentVkPipeline = VK_NULL_HANDLE;
        _currentVkPipelineLayout = VK_NULL_HANDLE;
        _currentPipelineLayout = nullptr;
        _currentPipeline.Reset();
    }

    void VulkanCommandBuffer::Enqueue()
    {
        if (_enqueued)
            return;

        _queue->Enqueue(_vkHandle);
        _enqueued = true;
    }

    void VulkanCommandBuffer::Commit()
    {
        End();
        Enqueue();
        _queue->Commit(this);
    }

    void VulkanCommandBuffer::Begin()
    {
        VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        beginInfo.pNext = nullptr;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        beginInfo.pInheritanceInfo = nullptr;

        vkThrowIfFailed(vkBeginCommandBuffer(_vkHandle, &beginInfo));

        ResetState();
    }

    void VulkanCommandBuffer::End()
    {
        vkThrowIfFailed(vkEndCommandBuffer(_vkHandle));
    }

    void VulkanCommandBuffer::Reset()
    {
        vkThrowIfFailed(vkResetCommandBuffer(_vkHandle, 0));
    }

    void VulkanCommandBuffer::BeginRenderPass(RenderPass* renderPass, const Color* clearColors, uint32_t numClearColors, float clearDepth, uint8_t clearStencil)
    {
        std::vector<VkClearValue> clearValues(numClearColors + 1);
        uint32_t i = 0;
        for (; i < numClearColors; ++i)
        {
            clearValues[i].color.float32[0] = clearColors[i].r;
            clearValues[i].color.float32[1] = clearColors[i].g;
            clearValues[i].color.float32[2] = clearColors[i].b;
            clearValues[i].color.float32[3] = clearColors[i].a;
        }
        clearValues[i].depthStencil.depth = clearDepth;
        clearValues[i].depthStencil.stencil = clearStencil;

        _currentRenderPass = static_cast<VulkanRenderPass*>(renderPass);

        VkRenderPassBeginInfo renderPassBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
        renderPassBeginInfo.renderPass = _currentRenderPass->GetVkRenderPass();
        renderPassBeginInfo.framebuffer = _currentRenderPass->GetVkFramebuffer();
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent.width = _currentRenderPass->GetWidth();
        renderPassBeginInfo.renderArea.extent.height = _currentRenderPass->GetHeight();
        renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassBeginInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(_vkHandle, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        // TODO: Set later
        VkViewport viewport = { 0.0f, 0.0f, renderPassBeginInfo.renderArea.extent.width, renderPassBeginInfo.renderArea.extent.height, 0.0f, 1.0f };

        // Flip to match DirectX coordinate system.
        viewport = VkViewport{
            viewport.x, viewport.height + viewport.y,
            viewport.width,   -viewport.height,
            viewport.minDepth, viewport.maxDepth,
        };

        VkRect2D scissor = {};
        scissor.extent = renderPassBeginInfo.renderArea.extent;

        vkCmdSetViewport(_vkHandle, 0, 1, &viewport);
        vkCmdSetScissor(_vkHandle, 0, 1, &scissor);
    }

    void VulkanCommandBuffer::EndRenderPass()
    {
        vkCmdEndRenderPass(_vkHandle);
    }

    void VulkanCommandBuffer::SetPipeline(const SharedPtr<PipelineState>& pipeline)
    {
        _currentPipeline = StaticCast<VulkanPipelineState>(pipeline);
        _currentVkPipeline = VK_NULL_HANDLE;

        auto newPipelineLayout = _currentPipeline->GetShader()->GetPipelineLayout();
        if (_currentPipelineLayout == nullptr)
        {
            _dirtySets = ~0u;

            _currentPipelineLayout = newPipelineLayout;
            _currentVkPipelineLayout = newPipelineLayout->GetVkHandle();
        }
        else if (newPipelineLayout != _currentPipelineLayout)
        {
            const ResourceLayout &newLayout = newPipelineLayout->GetResourceLayout();
            const ResourceLayout &oldLayout = _currentPipelineLayout->GetResourceLayout();

            // TODO: Invalidate sets
            _currentPipelineLayout = newPipelineLayout;
            _currentVkPipelineLayout = newPipelineLayout->GetVkHandle();
        }
    }

    void VulkanCommandBuffer::DrawCore(PrimitiveTopology topology, uint32_t vertexCount, uint32_t instanceCount, uint32_t vertexStart, uint32_t baseInstance)
    {
        if (!PrepareDraw(topology))
            return;

        vkCmdDraw(_vkHandle, vertexCount, instanceCount, vertexStart, baseInstance);
    }

    void VulkanCommandBuffer::DrawIndexedCore(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount, uint32_t startIndex)
    {

    }

    void VulkanCommandBuffer::OnSetVertexBuffer(GpuBuffer* buffer, uint32_t binding, uint64_t offset)
    {
        _currentVkBuffers[binding] = static_cast<VulkanBuffer*>(buffer)->GetVkHandle();
    }

    void VulkanCommandBuffer::SetIndexBufferCore(GpuBuffer* buffer, uint32_t offset, IndexType indexType)
    {
        if (_indexState.buffer == buffer
            && _indexState.offset == offset
            && _indexState.indexType == indexType)
        {
            return;
        }

        _indexState.buffer = buffer;
        _indexState.offset = offset;
        _indexState.indexType = indexType;

        VkBuffer vkBuffer = static_cast<VulkanBuffer*>(buffer)->GetVkHandle();;
        VkIndexType vkIndexType = vk::Convert(indexType);
        vkCmdBindIndexBuffer(_vkHandle, vkBuffer, offset, vkIndexType);
    }

    bool VulkanCommandBuffer::PrepareDraw(PrimitiveTopology topology)
    {
        if (_currentPipeline.IsNull())
            return false;

        VkPipeline oldPipeline = _currentVkPipeline;
        VkPipeline newPipeline = _currentPipeline->GetGraphicsPipeline(
            topology,
            _currentRenderPass->GetVkRenderPass()
        );
        if (oldPipeline != newPipeline)
        {
            vkCmdBindPipeline(_vkHandle, VK_PIPELINE_BIND_POINT_GRAPHICS, newPipeline);
            _currentVkPipeline = newPipeline;
        }

        FlushDescriptorSets();

        uint32_t updateVboMask = _dirtyVbos & _currentPipeline->GetBindingMask();
        ForEachBitRange(updateVboMask, [&](uint32_t binding, uint32_t count)
        {
#ifdef ALIMER_DEV
            for (uint32_t i = binding; i < binding + count; i++)
            {
                ALIMER_ASSERT(_currentVkBuffers[i] != VK_NULL_HANDLE);
            }
#endif

            vkCmdBindVertexBuffers(_vkHandle,
                binding,
                count,
                _currentVkBuffers + binding,
                _vbo.offsets + binding);
        });
        _dirtyVbos &= ~updateVboMask;

        return true;
    }

    void VulkanCommandBuffer::FlushDescriptorSet(uint32_t set)
    {
        auto &layout = _currentPipelineLayout->GetResourceLayout();
        auto &setLayout = layout.sets[set];
        uint32_t numDynamicOffsets = 0;
        uint32_t dynamicOffsets[MaxBindingsPerSet];

        Hasher h;

        // UBOs
        ForEachBit(setLayout.uniformBufferMask, [&](uint32_t binding) {
            h.pointer(_bindings.bindings[set][binding].buffer.buffer);
            h.u32(_bindings.bindings[set][binding].buffer.range);
            ALIMER_ASSERT(_bindings.bindings[set][binding].buffer.buffer != nullptr);

            dynamicOffsets[numDynamicOffsets++] = _bindings.bindings[set][binding].buffer.offset;
        });

        auto hash = h.get();
        std::pair<VkDescriptorSet, bool> allocated = _currentPipelineLayout->GetAllocator(set)->Find(hash);
        if (!allocated.second)
        {
            uint32_t writeCount = 0;
            uint32_t bufferInfoCount = 0;

            VkWriteDescriptorSet writes[MaxBindingsPerSet];
            VkDescriptorBufferInfo bufferInfo[MaxBindingsPerSet];

            ForEachBit(setLayout.uniformBufferMask, [&](uint32_t binding) {
                auto &write = writes[writeCount++];
                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.pNext = nullptr;
                write.descriptorCount = 1;
                write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                write.dstArrayElement = 0;
                write.dstBinding = binding;
                write.dstSet = allocated.first;

                // Offsets are applied dynamically.
                auto &buffer = bufferInfo[bufferInfoCount++];
                buffer.buffer = static_cast<const VulkanBuffer*>(_bindings.bindings[set][binding].buffer.buffer)->GetVkHandle();
                buffer.range = _bindings.bindings[set][binding].buffer.range;
                buffer.offset = 0;
                write.pBufferInfo = &buffer;
            });

            vkUpdateDescriptorSets(_logicalDevice, writeCount, writes, 0, nullptr);
        }

        vkCmdBindDescriptorSets(
            _vkHandle,
            _currentRenderPass ? VK_PIPELINE_BIND_POINT_GRAPHICS : VK_PIPELINE_BIND_POINT_COMPUTE,
            _currentVkPipelineLayout,
            set,
            1, &allocated.first,
            numDynamicOffsets,
            dynamicOffsets);
    }

    void VulkanCommandBuffer::FlushDescriptorSets()
    {
        auto &layout = _currentPipelineLayout->GetResourceLayout();
        uint32_t updateSet = layout.descriptorSetMask & _dirtySets;
        ForEachBit(updateSet, [&](uint32_t set) { FlushDescriptorSet(set); });
        _dirtySets &= ~updateSet;
    }
}
