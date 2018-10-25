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

#include "VulkanGraphicsDevice.h"
#include "VulkanCommandBuffer.h"
#include "VulkanBuffer.h"
#include "VulkanShader.h"
#include "VulkanPipelineLayout.h"
#include "VulkanRenderPass.h"
#include "VulkanConvert.h"
#include "../../Math/Math.h"
#include "../../Core/Log.h"

namespace Alimer
{
    VulkanCommandBuffer::VulkanCommandBuffer(VulkanGraphicsDevice* device, VkQueue queue, VkCommandBufferLevel level)
        : CommandContext(device)
        , _logicalDevice(device->GetDevice())
        , _queue(queue)
    {
        _vkCommandPool = device->GetCommandPool();
        VkCommandBufferAllocateInfo allocateInfo = {};
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.pNext = nullptr;
        allocateInfo.commandPool = _vkCommandPool;
        allocateInfo.level = level;
        allocateInfo.commandBufferCount = 1;
        VkResult result = vkAllocateCommandBuffers(device->GetDevice(), &allocateInfo, &_handle);
        if (result != VK_SUCCESS)
        {
            ALIMER_LOGERRORF("[Vulkan] - Failed to allocate command buffer: %s", vkGetVulkanResultString(result));
            return;
        }

        _vkFence = device->AcquireFence();
        Begin();
    }

    VulkanCommandBuffer::~VulkanCommandBuffer()
    {
        Destroy();
    }

    void VulkanCommandBuffer::Destroy()
    {
        vkFreeCommandBuffers(_logicalDevice, _vkCommandPool, 1, &_handle);
        static_cast<VulkanGraphicsDevice*>(_device)->ReleaseFence(_vkFence);
    }

    void VulkanCommandBuffer::BeginContext()
    {
        _dirtyFlags = ~0u;
        _currentPipeline = VK_NULL_HANDLE;
        _currentPipelineLayout = VK_NULL_HANDLE;
        _currentTopology = PrimitiveTopology::Count;
        _currentSubpass = 0;
        _currentLayout = nullptr;
        _currentVkProgram = nullptr;
        //memset(bindings.cookies, 0, sizeof(bindings.cookies));
        memset(_currentVertexBuffers, 0, sizeof(_currentVertexBuffers));
        CommandContext::BeginContext();
    }

    void VulkanCommandBuffer::FlushImpl(bool waitForCompletion)
    {
        End();

        if (!waitForCompletion)
        {
            static_cast<VulkanGraphicsDevice*>(_device)->SubmitCommandBuffer(_handle);
        }
        else
        {
            static_cast<VulkanGraphicsDevice*>(_device)->SubmitCommandBuffer(_handle, _vkFence);
            Begin();
        }
    }

    void VulkanCommandBuffer::BeginRenderPassImpl(Framebuffer* framebuffer, const RenderPassBeginDescriptor* descriptor)
    {
       /* _currentFramebuffer = static_cast<VulkanGraphicsDevice*>(_device)->RequestFramebuffer(descriptor);
        _currentRenderPass = _currentFramebuffer->GetRenderPass();

        VkRect2D renderArea = { { 0, 0 }, { UINT32_MAX, UINT32_MAX } };
        renderArea.offset.x = min(_currentFramebuffer->GetWidth(), uint32_t(renderArea.offset.x));
        renderArea.offset.y = min(_currentFramebuffer->GetHeight(), uint32_t(renderArea.offset.y));
        renderArea.extent.width = min(_currentFramebuffer->GetWidth() - renderArea.offset.x, renderArea.extent.width);
        renderArea.extent.height = min(_currentFramebuffer->GetHeight() - renderArea.offset.y, renderArea.extent.height);

        VkClearValue clearValues[MaxColorAttachments + 1];
        uint32_t clearValueCount = 0;
        clearValueCount = _currentRenderPass->GetColorAttachmentsCount();
        for (uint32_t i = 0; i < clearValueCount; ++i)
        {
            clearValues[i].color.float32[0] = descriptor->colorAttachments[i].clearColor.r;
            clearValues[i].color.float32[1] = descriptor->colorAttachments[i].clearColor.g;
            clearValues[i].color.float32[2] = descriptor->colorAttachments[i].clearColor.b;
            clearValues[i].color.float32[3] = descriptor->colorAttachments[i].clearColor.a;
        }

        if (descriptor->depthStencil
            && (any(descriptor->depthOperation & RenderPassDepthOperation::ClearDepthStencil)))
        {
            clearValues[clearValueCount++].depthStencil = { descriptor->clearDepth, descriptor->clearStencil };
        }

        VkRenderPassBeginInfo renderPassBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
        renderPassBeginInfo.renderPass = _currentRenderPass->GetVkRenderPass();
        renderPassBeginInfo.framebuffer = _currentFramebuffer->GetVkFramebuffer();
        renderPassBeginInfo.renderArea = renderArea;
        renderPassBeginInfo.clearValueCount = clearValueCount;
        renderPassBeginInfo.pClearValues = clearValues;

        vkCmdBeginRenderPass(_handle, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        rect viewport(float(_currentFramebuffer->GetWidth()), float(_currentFramebuffer->GetHeight()));
        irect scissor(_currentFramebuffer->GetWidth(), _currentFramebuffer->GetHeight());
        SetViewport(viewport);
        SetScissor(scissor);
        BeginGraphics();*/
    }

    void VulkanCommandBuffer::EndRenderPassImpl()
    {
        vkCmdEndRenderPass(_handle);
    }

    void VulkanCommandBuffer::Begin()
    {
        VkCommandBufferBeginInfo beginInfo;
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext = nullptr;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        beginInfo.pInheritanceInfo = nullptr;
        VkResult result = vkBeginCommandBuffer(_handle, &beginInfo);
        if (result != VK_SUCCESS)
        {
            ALIMER_LOGERRORF("vkBeginCommandBuffer failed: %s", vkGetVulkanResultString(result));
        }

        _graphicsState.Reset();
    }

    void VulkanCommandBuffer::End()
    {
        VkResult result = vkEndCommandBuffer(_handle);
        if (result != VK_SUCCESS)
        {
            ALIMER_LOGERRORF("vkEndCommandBuffer failed: %s", vkGetVulkanResultString(result));
        }
    }

    void VulkanCommandBuffer::SetProgramImpl(Program* program)
    {
        SetDirty(COMMAND_BUFFER_DIRTY_PIPELINE_BIT | COMMAND_BUFFER_DYNAMIC_BITS);
        _currentVkProgram = static_cast<VulkanProgram*>(program);

        auto pipelineLayout = _currentVkProgram->GetPipelineLayout();
        if (_currentLayout == nullptr)
        {
            _dirtySets = ~0u;
            SetDirty(COMMAND_BUFFER_DIRTY_PUSH_CONSTANTS_BIT);

            _currentLayout = pipelineLayout;
            _currentPipelineLayout = _currentLayout->GetHandle();
        }
        else if (pipelineLayout->GetHash() != _currentLayout->GetHash())
        {
            // TODO: Handle descriptor change.
            //const VulkanResourceLayout &newLayout = pipelineLayout->GetResourceLayout();
            //const VulkanResourceLayout &oldLayout = _currentLayout->GetResourceLayout();

            _currentLayout = pipelineLayout;
            _currentPipelineLayout = _currentLayout->GetHandle();
        }
    }

    void VulkanCommandBuffer::SetVertexDescriptor(const VertexDescriptor* descriptor)
    {
        _graphicsState.SetVertexDescriptor(descriptor);
    }

    void VulkanCommandBuffer::SetVertexBufferImpl(GpuBuffer* buffer, uint32_t offset)
    {
        VkBuffer vkBuffer = static_cast<VulkanBuffer*>(buffer)->GetHandle();
        VkDeviceSize vkOffset = offset;
        vkCmdBindVertexBuffers(_handle, 0, 1, &vkBuffer, &vkOffset);
    }

    void VulkanCommandBuffer::SetVertexBuffersImpl(uint32_t firstBinding, uint32_t count, const GpuBuffer** buffers, const uint32_t* offsets)
    {
        for (uint32_t i = firstBinding; i < count; i++)
        {
            _currentVertexBuffers[i] = static_cast<const VulkanBuffer*>(buffers[i])->GetHandle();
            _vboOffsets[i] = offsets[i];
        }

        vkCmdBindVertexBuffers(_handle, firstBinding, count, _currentVertexBuffers, _vboOffsets);
    }

    void VulkanCommandBuffer::SetIndexBufferImpl(GpuBuffer* buffer, uint32_t offset, IndexType indexType)
    {
        VkBuffer vkBuffer = static_cast<VulkanBuffer*>(buffer)->GetHandle();
        VkIndexType vkIndexType = vk::Convert(indexType);
        vkCmdBindIndexBuffer(_handle, vkBuffer, offset, vkIndexType);
    }

    void VulkanCommandBuffer::DrawImpl(PrimitiveTopology topology, uint32_t vertexCount, uint32_t startVertexLocation)
    {
        DrawInstancedImpl(topology, vertexCount, 1, startVertexLocation, 0);
    }

    void VulkanCommandBuffer::DrawInstancedImpl(PrimitiveTopology topology, uint32_t vertexCount, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation)
    {
        FlushRenderState(topology);
        vkCmdDraw(_handle, vertexCount, instanceCount, startVertexLocation, startInstanceLocation);
    }

    void VulkanCommandBuffer::DrawIndexedImpl(PrimitiveTopology topology, uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation)
    {
        DrawIndexedInstancedImpl(topology, indexCount, 1, startIndexLocation, baseVertexLocation, 0);
    }

    void VulkanCommandBuffer::DrawIndexedInstancedImpl(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation)
    {
        FlushRenderState(topology);
        vkCmdDrawIndexed(_handle, indexCount, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
    }

    void VulkanCommandBuffer::DispatchImpl(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
    {
        vkCmdDispatch(_handle, groupCountX, groupCountY, groupCountZ);
    }

    void VulkanCommandBuffer::FlushRenderState(PrimitiveTopology topology)
    {
        ALIMER_ASSERT(_currentProgram);
        ALIMER_ASSERT(!_isCompute);
        ALIMER_ASSERT(_currentLayout);

        // We've invalidated pipeline state, update the VkPipeline.
        if (_graphicsState.IsDirty()
            || GetAndClear(
            COMMAND_BUFFER_DIRTY_STATIC_STATE_BIT
            | COMMAND_BUFFER_DIRTY_PIPELINE_BIT
            | COMMAND_BUFFER_DIRTY_STATIC_VERTEX_BIT))
        {
            VkPipeline oldPipeline = _currentPipeline;
            FlushGraphicsPipeline();
            if (oldPipeline != _currentPipeline)
            {
                vkCmdBindPipeline(_handle, VK_PIPELINE_BIND_POINT_GRAPHICS, _currentPipeline);
                SetDirty(COMMAND_BUFFER_DYNAMIC_BITS);
            }
        }

        FlushDescriptorSets();
    }

    void VulkanCommandBuffer::FlushDescriptorSets()
    {
        const VulkanResourceLayout& layout = _currentLayout->GetResourceLayout();
        uint32_t updateSet = layout.descriptorSetMask & _dirtySets;
        ForEachBit(updateSet, [&](uint32_t set) { FlushDescriptorSet(set); });
        _dirtySets &= ~updateSet;
    }

    void VulkanCommandBuffer::FlushDescriptorSet(uint32_t set)
    {
        ALIMER_UNUSED(set);
    }

    void VulkanCommandBuffer::FlushGraphicsPipeline()
    {
        Util::Hasher h;
        /*const VulkanResourceLayout& layout = _currentLayout->GetResourceLayout();
        ForEachBit(layout.vertexAttributeMask, [&](uint32_t bit)
        {
            h.u32(bit);
            _activeVbos |= 1u << _attribs[bit].binding;
            h.u32(_attribs[bit].binding);
            h.u32(static_cast<uint32_t>(_attribs[bit].format));
            h.u32(_attribs[bit].offset);
        });

        ForEachBit(_activeVbos, [&](uint32_t bit)
        {
            h.u32(static_cast<uint32_t>(_vbo.inputRates[bit]));
            h.u32(_vbo.strides[bit]);
        });*/

        h.u64(_currentRenderPass->GetHash());
        h.u32(_currentSubpass);
        h.u64(_currentProgram->GetHash());
        h.u32(static_cast<uint32_t>(_currentTopology));

        auto hash = h.get();
        _currentPipeline = _currentVkProgram->GetGraphicsPipeline(hash);
        if (_currentPipeline == VK_NULL_HANDLE)
        {
            // VertexInputState
            uint32_t vertexStride = 0;

            VkPipelineVertexInputStateCreateInfo vertexInputState = {};
            vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertexInputState.pNext = nullptr;
            vertexInputState.flags = 0;
            VkVertexInputBindingDescription vi_bindings[MaxVertexBufferBindings];
            vertexInputState.pVertexBindingDescriptions = vi_bindings;
            const auto& vertexDescriptor = _graphicsState.GetVertexDescriptor();
            for (uint32_t i = 0; i < MaxVertexBufferBindings; i++)
            {
                const auto& layoutInfo = vertexDescriptor.buffers[i];
                if (layoutInfo.stride == 0)
                    continue;

                auto& bindingDesc = vi_bindings[vertexInputState.vertexBindingDescriptionCount++];
                bindingDesc.binding = i;
                bindingDesc.stride = layoutInfo.stride;
                bindingDesc.inputRate = vk::Convert(layoutInfo.inputRate);
            }

            // Check if we need to auto offset.
            bool useAutoOffset = true;
            for (uint32_t i = 0; i < MaxVertexAttributes; i++)
            {
                if (vertexDescriptor.attributes[i].format == VertexFormat::Invalid)
                    continue;

                if (vertexDescriptor.attributes[i].offset != 0)
                {
                    useAutoOffset = false;
                    break;
                }
            }

            VkVertexInputAttributeDescription vi_attribs[MaxVertexAttributes];
            vertexInputState.pVertexAttributeDescriptions = vi_attribs;
            for (uint32_t i = 0; i < MaxVertexAttributes; i++)
            {
                const auto& attributeInfo = vertexDescriptor.attributes[i];
                if (attributeInfo.format == VertexFormat::Invalid)
                    continue;

                uint32_t location = static_cast<uint32_t>(attributeInfo.semantic);
                auto& attributeDesc = vi_attribs[vertexInputState.vertexAttributeDescriptionCount++];
                attributeDesc.location = location; //  i;
                attributeDesc.binding = attributeInfo.bufferIndex;
                attributeDesc.format = vk::Convert(attributeInfo.format);
                attributeDesc.offset = useAutoOffset ? vertexStride : attributeInfo.offset;
                vertexStride += GetVertexFormatSize(attributeInfo.format);
            }
            
            // Rasterization state
            VkPipelineRasterizationStateCreateInfo rasterizationState = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
            rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
            // Default clockwise as DirectX coordinate.
            rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
            rasterizationState.lineWidth = 1.0f;
            rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
            rasterizationState.depthBiasEnable = VK_FALSE;

            // Multisample
            VkPipelineMultisampleStateCreateInfo multpleSampleState = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
            multpleSampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            // Depth state
            VkPipelineDepthStencilStateCreateInfo depthStencilState = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
            depthStencilState.stencilTestEnable = VK_FALSE;
            depthStencilState.depthTestEnable = VK_FALSE;
            depthStencilState.depthWriteEnable = VK_FALSE;

            // Blend state
            VkPipelineColorBlendAttachmentState blendAttachments[MaxColorAttachments] = {};
            blendAttachments[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            blendAttachments[0].blendEnable = VK_FALSE;

            VkPipelineColorBlendStateCreateInfo blendState = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
            blendState.attachmentCount = 1;
            blendState.pAttachments = blendAttachments;

            // Dynamic state
            VkDynamicState dynamicStates[] = {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR,
                VK_DYNAMIC_STATE_LINE_WIDTH,
                VK_DYNAMIC_STATE_DEPTH_BIAS,
                VK_DYNAMIC_STATE_BLEND_CONSTANTS,
                VK_DYNAMIC_STATE_DEPTH_BOUNDS,
                VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
                VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
                VK_DYNAMIC_STATE_STENCIL_REFERENCE,
            };
            VkPipelineDynamicStateCreateInfo dynamicState;
            dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamicState.pNext = nullptr;
            dynamicState.flags = 0;
            dynamicState.dynamicStateCount = sizeof(dynamicStates) / sizeof(dynamicStates[0]);
            dynamicState.pDynamicStates = dynamicStates;

            // Input assembly
            VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
            inputAssemblyState.pNext = nullptr;
            inputAssemblyState.flags = 0;
            inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            inputAssemblyState.primitiveRestartEnable = VK_FALSE;

            // Stages
            VkPipelineShaderStageCreateInfo stages[static_cast<uint32_t>(ShaderStage::Count)];
            uint32_t numStages = 0;

            for (uint32_t i = 0; i < static_cast<uint32_t>(ShaderStage::Count); i++)
            {
                auto vkModule = _currentVkProgram->GetVkShaderModule(i);
                if (vkModule != VK_NULL_HANDLE)
                {
                    auto &stage = stages[numStages++];
                    stage = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
                    stage.module = vkModule;
                    stage.pName = "main";
                    stage.stage = static_cast<VkShaderStageFlagBits>(1u << i);
                }
            }

            // Viewport and scissor.
            VkPipelineViewportStateCreateInfo viewportState;
            viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportState.pNext = nullptr;
            viewportState.flags = 0;
            viewportState.viewportCount = 1;
            viewportState.pViewports = nullptr;
            viewportState.scissorCount = 1;
            viewportState.pScissors = nullptr;

            VkGraphicsPipelineCreateInfo createInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
            createInfo.pNext = nullptr;
            createInfo.flags = 0;
            createInfo.stageCount = numStages;
            createInfo.pStages = stages;
            createInfo.pVertexInputState = &vertexInputState;
            createInfo.pTessellationState = nullptr;
            createInfo.pViewportState = &viewportState;
            createInfo.pRasterizationState = &rasterizationState;
            createInfo.pMultisampleState = &multpleSampleState;
            createInfo.pDepthStencilState = &depthStencilState;
            createInfo.pColorBlendState = &blendState;
            createInfo.pDynamicState = &dynamicState;
            createInfo.pInputAssemblyState = &inputAssemblyState;
            createInfo.layout = _currentPipelineLayout;
            createInfo.renderPass = _currentRenderPass->GetVkRenderPass();
            createInfo.subpass = _currentSubpass;
            createInfo.basePipelineHandle = VK_NULL_HANDLE;
            createInfo.basePipelineIndex = 0;

            VkPipeline newPipeline;
            if (vkCreateGraphicsPipelines(
                static_cast<VulkanGraphicsDevice*>(_device)->GetDevice(),
                static_cast<VulkanGraphicsDevice*>(_device)->GetPipelineCache(),
                1,
                &createInfo,
                nullptr,
                &newPipeline) != VK_SUCCESS)
            {
                ALIMER_LOGCRITICAL("Vulkan - Failed to graphics pipeline");
            }

            _currentVkProgram->AddPipeline(hash, newPipeline);
            _currentPipeline = newPipeline;
        }

        _graphicsState.ClearDirty();
    }

    void VulkanCommandBuffer::SetViewport(const rect& viewport)
    {
        // Flip to match DirectX coordinate system.
        VkViewport vkViewport;
        vkViewport.x = viewport.x;
        vkViewport.y = viewport.height + viewport.y;
        vkViewport.width = viewport.width;
        vkViewport.height = -viewport.height;
        vkViewport.minDepth = 0.0f;
        vkViewport.maxDepth = 0.0f;
        vkCmdSetViewport(_handle, 0, 1, &vkViewport);
    }

    void VulkanCommandBuffer::SetScissor(const irect& scissor)
    {
        VkRect2D vkScissor;
        vkScissor.offset = { scissor.x, scissor.y };
        vkScissor.extent = { static_cast<uint32_t>(scissor.width), static_cast<uint32_t>(scissor.height) };
        vkCmdSetScissor(_handle, 0, 1, &vkScissor);
    }

    // GraphicsState
    template <typename T>
    static bool operator != (const T& a, const T& b)
    {
        constexpr bool typeSupported = std::is_base_of<VertexDescriptor, T>::value/* |
            std::is_base_of<RasterizationState, T>::value |
            std::is_base_of<MultisampleState, T>::value |
            std::is_base_of<DepthStencilState, T>::value*/;
        static_assert(typeSupported, "Invalid type");
        return (memcmp(&a, &b, sizeof(T)) != 0);
    }

    VulkanCommandBuffer::GraphicsState::GraphicsState()
    {
        Reset();
    }

    void VulkanCommandBuffer::GraphicsState::Reset()
    {
        _dirty = false;
    }

    void VulkanCommandBuffer::GraphicsState::SetVertexDescriptor(const VertexDescriptor* descriptor)
    {
        if (_vertexDescriptor != *descriptor)
        {
            memcpy(&_vertexDescriptor, descriptor, sizeof(VertexDescriptor));
            _dirty = true;
        }
    }

#if TODO
    void VulkanCommandBuffer::DrawIndexedCore(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount, uint32_t startIndex)
    {
        ALIMER_ASSERT(_currentShader);
        ALIMER_ASSERT(!_isCompute);
        ALIMER_ASSERT(_indexState.buffer != nullptr);
        PrepareDraw(topology);
        vkCmdDrawIndexed(_handle, indexCount, instanceCount, startIndex, 0, 0);
    }

    void VulkanCommandBuffer::BindBufferImpl(GpuBuffer* buffer, uint32_t offset, uint32_t range, uint32_t set, uint32_t binding)
    {
        auto vkBuffer = static_cast<VulkanBuffer*>(buffer)->GetHandle();
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

    void VulkanCommandBuffer::BindTextureImpl(Texture* texture, uint32_t set, uint32_t binding)
    {

    }

    void VulkanCommandBuffer::FlushDescriptorSet(uint32_t set)
    {
        ALIMER_UNUSED(set);
        /*auto &layout = _currentPipelineLayout->GetResourceLayout();
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
            _handle,
            _currentRenderPass ? VK_PIPELINE_BIND_POINT_GRAPHICS : VK_PIPELINE_BIND_POINT_COMPUTE,
            _currentVkPipelineLayout,
            set,
            1, &allocated.first,
            numDynamicOffsets,
            dynamicOffsets);*/
    }
#endif // TODO
}

