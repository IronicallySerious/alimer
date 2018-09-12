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

#include "VulkanCommandBuffer.h"
#include "VulkanBuffer.h"
#include "VulkanShader.h"
#include "VulkanPipelineLayout.h"
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
        : _graphics(graphics)
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
            &_handle));

        BeginCompute();
    }

    VulkanCommandBuffer::~VulkanCommandBuffer()
    {
        vkFreeCommandBuffers(_logicalDevice, _commandPool, 1, &_handle);
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
        _currentShader = nullptr;
        _currentTopology = PrimitiveTopology::Count;
    }

    bool VulkanCommandBuffer::BeginCore()
    {
        VkCommandBufferBeginInfo beginInfo = {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            nullptr,
            VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
            nullptr
        };

        return vkBeginCommandBuffer(_handle, &beginInfo) == VK_SUCCESS;
    }

    bool VulkanCommandBuffer::EndCore()
    {
        return vkEndCommandBuffer(_handle) == VK_SUCCESS;
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

        vkThrowIfFailed(vkBeginCommandBuffer(_handle, &beginInfo));
    }

    void VulkanCommandBuffer::BeginRenderPassCore(RenderPass* renderPass, const Color4* clearColors, uint32_t numClearColors, float clearDepth, uint8_t clearStencil)
    {
        if (!renderPass)
        {
            renderPass = _graphics->GetBackbufferRenderPass();
        }

        _currentRenderPass = static_cast<VulkanRenderPass*>(renderPass);

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
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent.width = renderPass->GetWidth();
        renderPassBeginInfo.renderArea.extent.height = renderPass->GetHeight();
        renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassBeginInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(_handle, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        //Viewport viewport(0.0f, 0.0f, float(renderPass->GetWidth()), float(renderPass->GetHeight()), 0.0f, 1.0f);
        //SetViewport(viewport);
        //SetScissor(setRenderArea);
        BeginGraphics();
    }

    void VulkanCommandBuffer::EndRenderPassCore()
    {
        vkCmdEndRenderPass(_handle);
    }

    /*void VulkanCommandBuffer::ExecuteCommandsCore(uint32_t commandBufferCount, CommandBuffer* const* commandBuffers)
    {
        std::vector<VkCommandBuffer> vkCommandBuffers;
        for (uint32_t i = 0; i < commandBufferCount; i++)
        {
            VulkanCommandBuffer* vulkanCmdBuffer = static_cast<VulkanCommandBuffer*>(commandBuffers[i]);
            ALIMER_ASSERT(vulkanCmdBuffer->IsSecondary());
            vkCommandBuffers[i] = vulkanCmdBuffer->GetVkCommandBuffer();
        }

        vkCmdExecuteCommands(_vkCommandBuffer, commandBufferCount, vkCommandBuffers.data());
    }*/

    void VulkanCommandBuffer::SetViewport(const Viewport& viewport)
    {
        // Flip to match DirectX coordinate system.
        _currentViewport = VkViewport{
            viewport.x, viewport.height + viewport.y,
            viewport.width,   -viewport.height,
            viewport.minDepth, viewport.maxDepth,
        };

        vkCmdSetViewport(_handle, 0, 1, &_currentViewport);
    }

    void VulkanCommandBuffer::SetScissor(const Rectangle& scissor)
    {
        VkRect2D vkScissor = {};
        vkScissor.offset = { scissor.x, scissor.y };
        vkScissor.extent = { static_cast<uint32_t>(scissor.width), static_cast<uint32_t>(scissor.height) };
        vkCmdSetScissor(_handle, 0, 1, &vkScissor);
    }

    void VulkanCommandBuffer::SetShaderProgramImpl(ShaderProgram* program)
    {
        if (_currentShader == program)
            return;

        _currentShader = static_cast<VulkanShader*>(program);
        _currentVkPipeline = VK_NULL_HANDLE;

        SetDirty(COMMAND_BUFFER_DIRTY_PIPELINE_BIT | COMMAND_BUFFER_DYNAMIC_BITS);

        /*if (_currentPipelineLayout == nullptr)
        {
            _dirtySets = ~0u;
            SetDirty(COMMAND_BUFFER_DIRTY_PUSH_CONSTANTS_BIT);

            _currentPipelineLayout = _currentShader->GetPipelineLayout();
            _currentVkPipelineLayout = _currentPipelineLayout->GetVkHandle();
        }
        else if (_currentShader->GetPipelineLayout() != _currentPipelineLayout)
        {
            const ResourceLayout &newLayout = _currentShader->GetPipelineLayout()->GetResourceLayout();
            const ResourceLayout &oldLayout = _currentPipelineLayout->GetResourceLayout();

            // TODO: Invalidate sets
            _currentPipelineLayout = _currentShader->GetPipelineLayout();
            _currentVkPipelineLayout = _currentPipelineLayout->GetVkHandle();
        }*/
    }

    void VulkanCommandBuffer::DrawCore(PrimitiveTopology topology, uint32_t vertexCount, uint32_t instanceCount, uint32_t vertexStart, uint32_t baseInstance)
    {
        PrepareDraw(topology);
        vkCmdDraw(_handle, vertexCount, instanceCount, vertexStart, baseInstance);
    }

    void VulkanCommandBuffer::DrawIndexedCore(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount, uint32_t startIndex)
    {
        ALIMER_ASSERT(_currentShader);
        ALIMER_ASSERT(!_isCompute);
        ALIMER_ASSERT(_indexState.buffer != nullptr);
        PrepareDraw(topology);
        vkCmdDrawIndexed(_handle, indexCount, instanceCount, startIndex, 0, 0);
    }

    void VulkanCommandBuffer::PrepareDraw(PrimitiveTopology topology)
    {
        ALIMER_ASSERT(_currentShader);
        ALIMER_ASSERT(!_isCompute);

        SetPrimitiveTopology(topology);
        FlushRenderState();

        uint32_t updateVboMask = _dirtyVbos & _activeVbos;
        ForEachBitRange(updateVboMask, [&](uint32_t binding, uint32_t count)
        {
#ifdef ALIMER_DEV
            for (uint32_t i = binding; i < binding + count; i++)
            {
                ALIMER_ASSERT(_vbo.buffers[i] != VK_NULL_HANDLE);
            }
#endif

            vkCmdBindVertexBuffers(_handle,
                binding,
                count,
                _vbo.buffers + binding,
                _vbo.offsets + binding);
        });
        _dirtyVbos &= ~updateVboMask;
    }

    void VulkanCommandBuffer::SetVertexAttribute(uint32_t attrib, uint32_t binding, VkFormat format, VkDeviceSize offset)
    {
        ALIMER_ASSERT(attrib < MaxVertexAttributes);

        auto &attr = _attribs[attrib];

        if (attr.binding != binding || attr.format != format || attr.offset != offset)
            SetDirty(COMMAND_BUFFER_DIRTY_STATIC_VERTEX_BIT);

        ALIMER_ASSERT(binding < MaxVertexBufferBindings);

        attr.binding = binding;
        attr.format = format;
        attr.offset = offset;
    }

    void VulkanCommandBuffer::BindVertexBufferImpl(GpuBuffer* buffer, uint32_t binding, uint32_t offset, uint32_t stride, VertexInputRate inputRate)
    {
        //VkBuffer vkBuffer = static_cast<VulkanBuffer*>(buffer)->GetVkHandle();
        //if (_vbo.buffers[binding] != vkBuffer
        //    || _vbo.offsets[binding] != offset)
        //{
         //   _dirtyVbos |= 1u << binding;

            // Bind vertex attributes as well
            /*uint32_t attrib = 0;
            uint32_t count = buffer->GetElementsCount();
            for (auto& element : buffer->GetElements())
            {
                SetVertexAttribute(attrib++, binding, vk::Convert(element.format), element.offset);
            }*/
        //}

        if (_vbo.strides[binding] != stride
            || _vbo.inputRates[binding] != inputRate)
        {
            SetDirty(COMMAND_BUFFER_DIRTY_STATIC_VERTEX_BIT);
        }

        //_vbo.buffers[binding] = vkBuffer;
        _vbo.offsets[binding] = offset;
        _vbo.strides[binding] = stride;
        _vbo.inputRates[binding] = inputRate;
    }

    void VulkanCommandBuffer::SetVertexInputFormatImpl(VertexInputFormat* format)
    {

    }

    void VulkanCommandBuffer::BindIndexBufferImpl(GpuBuffer* buffer, GpuSize offset, IndexType indexType)
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

        VkBuffer vkBuffer = static_cast<VulkanBuffer*>(buffer)->GetHandle();
        VkIndexType vkIndexType = vk::Convert(indexType);
        vkCmdBindIndexBuffer(_handle, vkBuffer, offset, vkIndexType);
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

    void VulkanCommandBuffer::FlushRenderState()
    {
        ALIMER_ASSERT(_currentPipelineLayout);
        //ALIMER_ASSERT(current_program);

        // We've invalidated pipeline state, update the VkPipeline.
        if (GetAndClear(
            COMMAND_BUFFER_DIRTY_STATIC_STATE_BIT
            | COMMAND_BUFFER_DIRTY_PIPELINE_BIT
            | COMMAND_BUFFER_DIRTY_STATIC_VERTEX_BIT))
        {
            VkPipeline oldPipeline = _currentVkPipeline;
            FlushGraphicsPipeline();
            if (oldPipeline != _currentVkPipeline)
            {
                vkCmdBindPipeline(_handle, VK_PIPELINE_BIND_POINT_GRAPHICS, _currentVkPipeline);
                SetDirty(COMMAND_BUFFER_DYNAMIC_BITS);
            }
        }

        FlushDescriptorSets();
    }

    void VulkanCommandBuffer::FlushGraphicsPipeline()
    {
        Hasher h;
        _activeVbos = 0;
        /*auto &layout = _currentPipelineLayout->GetResourceLayout();
        ForEachBit(layout.attributeMask, [&](uint32_t bit)
        {
            h.u32(bit);
            _activeVbos |= 1u << _attribs[bit].binding;
            h.u32(_attribs[bit].binding);
            h.u32(_attribs[bit].format);
            h.u32(_attribs[bit].offset);
        });*/

        ForEachBit(_activeVbos, [&](uint32_t bit)
        {
            h.u32(static_cast<uint32_t>(_vbo.inputRates[bit]));
            h.u32(_vbo.strides[bit]);
        });

        // TODO: use render pass hash.
        h.pointer(_currentRenderPass->GetVkRenderPass());
        //h.u64(_currentRenderPass->GetHash());
        h.u32(_currentSubpass);
        h.pointer(_currentShader);
        //h.u64(_currentPipeline->GetHash());
        h.u32(static_cast<uint32_t>(_currentTopology));

        auto hash = h.get();
        _currentVkPipeline = _currentShader->GetGraphicsPipeline(hash);
        if (_currentVkPipeline == VK_NULL_HANDLE)
        {
            // Create new.
            VkPipelineVertexInputStateCreateInfo vertexInputState = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
            VkVertexInputAttributeDescription vkAttribs[MaxVertexAttributes];
            vertexInputState.pVertexAttributeDescriptions = vkAttribs;

            uint32_t attributeMask = 0; // _currentPipelineLayout->GetResourceLayout().attributeMask;
            uint32_t bindingMask = 0;
            ForEachBit(attributeMask, [&](uint32_t bit) {
                auto &attr = vkAttribs[vertexInputState.vertexAttributeDescriptionCount++];
                attr.location = bit;
                attr.binding = _attribs[bit].binding;
                attr.format = _attribs[bit].format;
                attr.offset = _attribs[bit].offset;
                bindingMask |= 1u << attr.binding;
            });

            VkVertexInputBindingDescription vkVboBindings[MaxVertexBufferBindings];
            vertexInputState.pVertexBindingDescriptions = vkVboBindings;
            ForEachBit(bindingMask, [&](uint32_t bit) {
                auto &bind = vkVboBindings[vertexInputState.vertexBindingDescriptionCount++];
                bind.binding = bit;
                bind.inputRate = vk::Convert(_vbo.inputRates[bit]);
                bind.stride = _vbo.strides[bit];
            });

            // Viewport state
            VkPipelineViewportStateCreateInfo viewportState = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
            viewportState.viewportCount = 1;
            viewportState.scissorCount = 1;

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
            VkPipelineDynamicStateCreateInfo dynamicState = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
            dynamicState.dynamicStateCount = 2;
            VkDynamicState states[2] = {
                VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_VIEWPORT,
            };
            dynamicState.pDynamicStates = states;

            // Input assembly
            VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
            inputAssemblyState.pNext = nullptr;
            inputAssemblyState.flags = 0;
            inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            inputAssemblyState.primitiveRestartEnable = VK_FALSE;

            // Stages
            VkPipelineShaderStageCreateInfo stages[static_cast<uint32_t>(VulkanShaderStage::Count)];
            uint32_t numStages = 0;

            for (uint32_t i = 0; i < static_cast<uint32_t>(VulkanShaderStage::Count); i++)
            {
                auto vkModule = _currentShader->GetVkShaderModule(i);
                if (vkModule != VK_NULL_HANDLE)
                {
                    auto &stage = stages[numStages++];
                    stage = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
                    stage.module = vkModule;
                    stage.pName = "main";
                    stage.stage = static_cast<VkShaderStageFlagBits>(1u << i);
                }
            }

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
            createInfo.layout = _currentVkPipelineLayout;
            createInfo.renderPass = _currentRenderPass->GetVkRenderPass();
            createInfo.subpass = _currentSubpass;
            createInfo.basePipelineHandle = VK_NULL_HANDLE;
            createInfo.basePipelineIndex = 0;

            VkPipeline newPipeline;
            if (vkCreateGraphicsPipelines(
                _logicalDevice,
                _graphics->GetPipelineCache(),
                1,
                &createInfo,
                nullptr,
                &newPipeline) != VK_SUCCESS)
            {
                ALIMER_LOGCRITICAL("Vulkan - Failed to graphics pipeline");
            }

            _currentShader->AddPipeline(hash, newPipeline);
            _currentVkPipeline = newPipeline;
        }
    }

    void VulkanCommandBuffer::FlushDescriptorSet(uint32_t set)
    {
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

    void VulkanCommandBuffer::FlushDescriptorSets()
    {
        //auto &layout = _currentPipelineLayout->GetResourceLayout();
        //uint32_t updateSet = layout.descriptorSetMask & _dirtySets;
        //ForEachBit(updateSet, [&](uint32_t set) { FlushDescriptorSet(set); });
        //_dirtySets &= ~updateSet;
    }
}

