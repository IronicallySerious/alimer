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

#include "VulkanPipelineState.h"
#include "VulkanPipelineLayout.h"
#include "VulkanGraphics.h"
#include "VulkanShader.h"
#include "VulkanConvert.h"
#include "../../Core/Log.h"
#include "../../Util/HashMap.h"

namespace Alimer
{
    VulkanPipelineState::VulkanPipelineState(VulkanGraphics* graphics, const RenderPipelineDescriptor& descriptor)
        : PipelineState(graphics, true)
        , _logicalDevice(graphics->GetLogicalDevice())
        , _pipelineCache(graphics->GetPipelineCache())
    {
        _shader = StaticCast<VulkanShader>(descriptor.shader);

        // Stages
        for (uint32_t i = 0; i < static_cast<uint32_t>(ShaderStage::Count); i++)
        {
            auto stage = static_cast<ShaderStage>(i);
            if (_shader->GetVkShaderModule(stage) != VK_NULL_HANDLE)
            {
                auto &s = _stages[_stageCount++];
                s = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
                s.module = _shader->GetVkShaderModule(stage);
                s.pName = "main";
                s.stage = static_cast<VkShaderStageFlagBits>(1u << i);
            }
        }

        // Vertex input
        uint32_t attrMask = _shader->GetResourceLayout().attributeMask;
        _bindingMask = 0;
        ForEachBit(attrMask, [&](uint32_t bit)
        {
            auto &attr = _vkAttribs[_vkAttribsCount++];
            attr.location = bit;
            attr.binding = descriptor.vertexDescriptor.attributes[bit].binding;
            attr.format = vk::Convert(descriptor.vertexDescriptor.attributes[bit].format);
            attr.offset = descriptor.vertexDescriptor.attributes[bit].offset;
            _bindingMask |= 1u << attr.binding;
        });

        ForEachBit(_bindingMask, [&](uint32_t bit) {
            auto &bind = _vkBindings[_vkBindingsCount++];
            bind.binding = bit;
            bind.inputRate = vk::Convert(descriptor.vertexDescriptor.layouts[bit].inputRate);
            bind.stride = descriptor.vertexDescriptor.layouts[bit].stride;
        });

    }

    VulkanPipelineState::~VulkanPipelineState()
    {
    }

    VkPipeline VulkanPipelineState::GetGraphicsPipeline(PrimitiveTopology topology, VkRenderPass renderPass)
    {
        Hasher hasher;
        hasher.u32(static_cast<uint32_t>(topology));
        hasher.pointer(renderPass);

        auto hash = hasher.get();
        auto it = _graphicsPipelineCache.find(hash);
        if (it != _graphicsPipelineCache.end())
            return it->second;

        VkPipelineVertexInputStateCreateInfo vertexInputState = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
        vertexInputState.vertexBindingDescriptionCount = _vkBindingsCount;
        vertexInputState.pVertexBindingDescriptions = _vkBindings;
        vertexInputState.vertexAttributeDescriptionCount = _vkAttribsCount;
        vertexInputState.pVertexAttributeDescriptions = _vkAttribs;

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

        VkGraphicsPipelineCreateInfo createInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.stageCount = _stageCount;
        createInfo.pStages = _stages;
        createInfo.pVertexInputState = &vertexInputState;
        createInfo.pTessellationState = nullptr;
        createInfo.pViewportState = &viewportState;
        createInfo.pRasterizationState = &rasterizationState;
        createInfo.pMultisampleState = &multpleSampleState;
        createInfo.pDepthStencilState = &depthStencilState;
        createInfo.pColorBlendState = &blendState;
        createInfo.pDynamicState = &dynamicState;
        createInfo.pInputAssemblyState = &inputAssemblyState;
        createInfo.layout = _shader->GetPipelineLayout()->GetVkHandle();
        createInfo.renderPass = renderPass;
        createInfo.subpass = 0;
        createInfo.basePipelineHandle = VK_NULL_HANDLE;
        createInfo.basePipelineIndex = 0;

        VkPipeline pipeline;
        if (vkCreateGraphicsPipelines(
            _logicalDevice,
            _pipelineCache,
            1,
            &createInfo,
            nullptr,
            &pipeline) != VK_SUCCESS)
        {
            ALIMER_LOGCRITICAL("Vulkan - Failed to graphics pipeline");
        }

        _graphicsPipelineCache[hash] = pipeline;
        return pipeline;
    }
}
