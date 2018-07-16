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

static_assert(sizeof(VkViewport) == sizeof(Alimer::Viewport), "VkViewport mismatch");
static_assert(offsetof(Alimer::Viewport, x) == offsetof(VkViewport, x), "VkViewport x offset mismatch");
static_assert(offsetof(Alimer::Viewport, y) == offsetof(VkViewport, y), "VkViewport y offset mismatch");
static_assert(offsetof(Alimer::Viewport, width) == offsetof(VkViewport, width), "VkViewport width offset mismatch");
static_assert(offsetof(Alimer::Viewport, height) == offsetof(VkViewport, height), "VkViewport height offset mismatch");
static_assert(offsetof(Alimer::Viewport, minDepth) == offsetof(VkViewport, minDepth), "VkViewport minDepth offset mismatch");
static_assert(offsetof(Alimer::Viewport, maxDepth) == offsetof(VkViewport, maxDepth), "VkViewport maxDepth offset mismatch");

namespace Alimer
{
    VulkanCommandBuffer::VulkanCommandBuffer(VulkanGraphics* graphics, VkCommandPool commandPool, bool secondary)
        : CommandBuffer()
        , _graphics(graphics)
        , _logicalDevice(_graphics->GetLogicalDevice())
        , _commandPool(commandPool)
    {
        VkCommandBufferAllocateInfo cmdBufAllocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
        cmdBufAllocateInfo.pNext = nullptr;
        cmdBufAllocateInfo.commandPool = _commandPool;
        cmdBufAllocateInfo.level = secondary ? VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmdBufAllocateInfo.commandBufferCount = 1;
        vkThrowIfFailed(vkAllocateCommandBuffers(
            _logicalDevice,
            &cmdBufAllocateInfo,
            &_vkCommandBuffer));

        BeginCompute();
    }

    VulkanCommandBuffer::~VulkanCommandBuffer()
    {
        vkFreeCommandBuffers(_logicalDevice, _commandPool, 1, &_vkCommandBuffer);
    }

    void VulkanCommandBuffer::BeginCompute()
    {
        _isCompute = true;
        BeginContext();
    }

    void VulkanCommandBuffer::BeginGraphics()
    {
        _isCompute = false;
        BeginContext();
    }

    void VulkanCommandBuffer::BeginContext()
    {
        _dirty = ~0u;
        _dirtySets = ~0u;
        _dirtyVbos = ~0u;
        memset(_vbo.buffers, 0, sizeof(_vbo.buffers));
        memset(&_indexState, 0, sizeof(_indexState));
        memset(&_bindings, 0, sizeof(_bindings));

        _currentVkPipeline = VK_NULL_HANDLE;
        _currentVkPipelineLayout = VK_NULL_HANDLE;
        _currentPipelineLayout = nullptr;
        _currentPipeline = nullptr;
    }

    void VulkanCommandBuffer::Begin(VkCommandBufferInheritanceInfo* inheritanceInfo)
    {
        VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        beginInfo.pNext = nullptr;
        if (inheritanceInfo)
        {
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
            beginInfo.pInheritanceInfo = inheritanceInfo;
        }

        vkThrowIfFailed(vkBeginCommandBuffer(_vkCommandBuffer, &beginInfo));
    }

    void VulkanCommandBuffer::End()
    {
        vkThrowIfFailed(vkEndCommandBuffer(_vkCommandBuffer));
    }

    void VulkanCommandBuffer::BeginRenderPassCore(RenderPass* renderPass, const Rectangle& renderArea, const Color* clearColors, uint32_t numClearColors, float clearDepth, uint8_t clearStencil)
    {
        if (!renderPass)
        {
            renderPass = _graphics->GetBackbufferRenderPass();
        }

        _currentRenderPass = static_cast<VulkanRenderPass*>(renderPass);

        Rectangle setRenderArea = renderArea;
        if (renderArea.IsEmpty())
        {
            setRenderArea = Rectangle((int32_t)renderPass->GetWidth(), (int32_t)renderPass->GetHeight());
        }

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

        VkRenderPassBeginInfo renderPassBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
        renderPassBeginInfo.renderPass = _currentRenderPass->GetVkRenderPass();
        renderPassBeginInfo.framebuffer = _currentRenderPass->GetVkFramebuffer();
        renderPassBeginInfo.renderArea.offset.x = setRenderArea.x;
        renderPassBeginInfo.renderArea.offset.y = setRenderArea.y;
        renderPassBeginInfo.renderArea.extent.width = setRenderArea.width;
        renderPassBeginInfo.renderArea.extent.height = setRenderArea.height;
        renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassBeginInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(_vkCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        // 
        SetViewport(Viewport(setRenderArea));
        SetScissor(setRenderArea);
        BeginGraphics();
    }

    void VulkanCommandBuffer::EndRenderPassCore()
    {
        vkCmdEndRenderPass(_vkCommandBuffer);
    }

    void VulkanCommandBuffer::ExecuteCommandsCore(uint32_t commandBufferCount, CommandBuffer* const* commandBuffers)
    {
        std::vector<VkCommandBuffer> vkCommandBuffers;
        for (uint32_t i = 0; i < commandBufferCount; i++)
        {
            VulkanCommandBuffer* vulkanCmdBuffer = static_cast<VulkanCommandBuffer*>(commandBuffers[i]);
            ALIMER_ASSERT(vulkanCmdBuffer->IsSecondary());
            vkCommandBuffers[i] = vulkanCmdBuffer->GetVkCommandBuffer();
        }

        vkCmdExecuteCommands(_vkCommandBuffer, commandBufferCount, vkCommandBuffers.data());
    }

    void VulkanCommandBuffer::SetViewport(const Viewport& viewport)
    {
        // Flip to match DirectX coordinate system.
        _currentViewports[0] = VkViewport{
            viewport.x, viewport.height + viewport.y,
            viewport.width,   -viewport.height,
            viewport.minDepth, viewport.maxDepth,
        };

        vkCmdSetViewport(_vkCommandBuffer, 0, 1, &_currentViewports[0]);
    }

    void VulkanCommandBuffer::SetViewports(uint32_t numViewports, const Viewport* viewports)
    {
        for (uint32_t i = 0; i < numViewports; i++)
        {
            _currentViewports[i] = VkViewport{
                viewports[i].x, viewports[i].height + viewports[i].y,
                viewports[i].width,   -viewports[i].height,
                viewports[i].minDepth, viewports[i].maxDepth,
            };
        }

        vkCmdSetViewport(_vkCommandBuffer, 0, numViewports, _currentViewports);
    }

    void VulkanCommandBuffer::SetScissor(const Rectangle& scissor)
    {
        VkRect2D vkScissor = {};
        vkScissor.offset = { scissor.x, scissor.y };
        vkScissor.extent = { static_cast<uint32_t>(scissor.width), static_cast<uint32_t>(scissor.height) };
        vkCmdSetScissor(_vkCommandBuffer, 0, 1, &vkScissor);
    }

    void VulkanCommandBuffer::SetScissors(uint32_t numScissors, const Rectangle* scissors)
    {

    }

    void VulkanCommandBuffer::SetPipeline(PipelineState* pipeline)
    {
        _currentPipeline = static_cast<VulkanPipelineState*>(pipeline);
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

        vkCmdDraw(_vkCommandBuffer, vertexCount, instanceCount, vertexStart, baseInstance);
    }

    /*void VulkanCommandBuffer::DrawIndexedCore(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount, uint32_t startIndex)
    {

    }*/

    bool VulkanCommandBuffer::PrepareDraw(PrimitiveTopology topology)
    {
        if (!_currentPipeline)
            return false;

        VkPipeline oldPipeline = _currentVkPipeline;
        VkPipeline newPipeline = _currentPipeline->GetGraphicsPipeline(
            topology,
            _currentRenderPass->GetVkRenderPass()
        );

        if (oldPipeline != newPipeline)
        {
            vkCmdBindPipeline(_vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, newPipeline);
            _currentVkPipeline = newPipeline;
        }

        FlushDescriptorSets();

        uint32_t updateVboMask = _dirtyVbos & _currentPipeline->GetBindingMask();
        ForEachBitRange(updateVboMask, [&](uint32_t binding, uint32_t count)
        {
#ifdef ALIMER_DEV
            for (uint32_t i = binding; i < binding + count; i++)
            {
                ALIMER_ASSERT(_vbo.buffers[i] != VK_NULL_HANDLE);
            }
#endif

            vkCmdBindVertexBuffers(_vkCommandBuffer,
                binding,
                count,
                _vbo.buffers + binding,
                _vbo.offsets + binding);
        });
        _dirtyVbos &= ~updateVboMask;

        return true;
    }

    void VulkanCommandBuffer::SetVertexBufferCore(BufferHandle* buffer, uint32_t binding, uint64_t offset, uint32_t stride)
    {
        auto vkBuffer = static_cast<VulkanBuffer*>(buffer)->GetVkHandle();
        if (_vbo.buffers[binding] != vkBuffer
            || _vbo.offsets[binding] != offset)
        {
            _dirtyVbos |= 1u << binding;
        }

        if (_vbo.strides[binding] != stride)
        {
            SetDirty(COMMAND_BUFFER_DIRTY_STATIC_VERTEX_BIT);
        }

        _vbo.buffers[binding] = vkBuffer;
        _vbo.offsets[binding] = offset;
        _vbo.strides[binding] = stride;
    }

    /*void VulkanCommandBuffer::SetIndexBufferCore(GpuBuffer* buffer, uint32_t offset, IndexType indexType)
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

        VkBuffer vkBuffer = static_cast<VulkanBuffer*>(buffer)->GetVkHandle();
        VkIndexType vkIndexType = vk::Convert(indexType);
        vkCmdBindIndexBuffer(_vkHandle, vkBuffer, offset, vkIndexType);
    }
    */

    void VulkanCommandBuffer::SetUniformBufferCore(uint32_t set, uint32_t binding, BufferHandle* buffer, uint64_t offset, uint64_t range)
    {
        auto vkBuffer = static_cast<VulkanBuffer*>(buffer)->GetVkHandle();
        auto &b = _bindings.bindings[set][binding];

        if (b.buffer.buffer == vkBuffer
            && b.buffer.offset == offset
            && b.buffer.range == range)
        {
            return;
        }

        b.buffer = { vkBuffer, offset, range };
        _dirtySets |= 1u << set;
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
                buffer = _bindings.bindings[set][binding].buffer;
                buffer.offset = 0;
                write.pBufferInfo = &buffer;
            });

            vkUpdateDescriptorSets(_logicalDevice, writeCount, writes, 0, nullptr);
        }

        vkCmdBindDescriptorSets(
            _vkCommandBuffer,
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

