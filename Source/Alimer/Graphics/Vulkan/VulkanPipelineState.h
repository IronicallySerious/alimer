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

#include "../PipelineState.h"
#include "VulkanPrerequisites.h"
#include "../../Util/HashMap.h"

namespace Alimer
{
	class VulkanGraphics;
    class VulkanShader;

	/// Vulkan PipelineState implementation.
	class VulkanPipelineState final : public PipelineState
	{
	public:
        VulkanPipelineState(VulkanGraphics* graphics, const RenderPipelineDescription& description);
		~VulkanPipelineState() override;

        VulkanShader* GetShader() const { return _shader.Get(); }
        uint32_t GetBindingMask() const { return _bindingMask; }
        VkPipeline GetGraphicsPipeline(PrimitiveTopology topology, VkRenderPass renderPass);

	private:
		VkDevice _logicalDevice;
        VkPipelineCache _pipelineCache;
        SharedPtr<VulkanShader> _shader;
        uint32_t _bindingMask = 0;

        uint32_t _stageCount = 0;
        uint32_t _vkBindingsCount = 0;
        uint32_t _vkAttribsCount = 0;

        VkPipelineShaderStageCreateInfo _stages[static_cast<uint32_t>(ShaderStage::Count)] = {};
        VkVertexInputBindingDescription _vkBindings[MaxVertexBufferBindings] = {};
        VkVertexInputAttributeDescription _vkAttribs[MaxVertexAttributes] = {};
        HashMap<VkPipeline> _graphicsPipelineCache;
	};
}
