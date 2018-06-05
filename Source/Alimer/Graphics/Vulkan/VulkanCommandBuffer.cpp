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
#include "VulkanGraphics.h"
#include "../../Core/Log.h"
#include "../../Util/HashMap.h"

namespace Alimer
{
	VulkanCommandBuffer::VulkanCommandBuffer(VulkanGraphics* graphics, VkCommandPool commandPool, VkCommandBuffer vkCommandBuffer)
		: CommandBuffer(graphics)
		, _logicalDevice(graphics->GetLogicalDevice())
		, _commandPool(commandPool)
		, _vkCommandBuffer(vkCommandBuffer)
	{
		
	}

	VulkanCommandBuffer::~VulkanCommandBuffer()
	{
		vkFreeCommandBuffers(_logicalDevice, _commandPool, 1, &_vkCommandBuffer);
	}

	void VulkanCommandBuffer::Begin()
	{
		VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		beginInfo.pNext = nullptr;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = nullptr;

		vkThrowIfFailed(vkBeginCommandBuffer(_vkCommandBuffer, &beginInfo));
	}

	void VulkanCommandBuffer::End()
	{
		vkThrowIfFailed(vkEndCommandBuffer(_vkCommandBuffer));
	}

	uint64_t VulkanCommandBuffer::Commit(bool waitForCompletion)
	{
		End();

		static_cast<VulkanGraphics*>(_graphics)->SubmitCommandBuffer(this);
		return _fenceValue;
	}

	void VulkanCommandBuffer::BeginRenderPass(const RenderPassDescriptor& descriptor)
	{
		Util::Hasher renderPassHasher;
		Util::Hasher framebufferHasher;
		uint64_t renderPassHash = 0;
		uint64_t framebufferHash = 0;

		VkClearValue clearValues[MaxColorAttachments + 1];
		uint32_t numClearValues = 0;

		for (uint32_t i = 0; i < MaxColorAttachments; i++)
		{
			const RenderPassColorAttachmentDescriptor& colorAttachment = descriptor.colorAttachments[i];
			Texture* texture = colorAttachment.texture;
			if (!texture)
				continue;

			renderPassHasher.u32(static_cast<uint32_t>(texture->GetFormat()));
			renderPassHasher.u32(static_cast<uint32_t>(colorAttachment.loadAction));
			renderPassHasher.u32(static_cast<uint32_t>(colorAttachment.storeAction));

			framebufferHasher.pointer(texture);

			if (colorAttachment.loadAction == LoadAction::Clear)
			{
				clearValues[i].color = { colorAttachment.clearColor.r, colorAttachment.clearColor.g, colorAttachment.clearColor.b, colorAttachment.clearColor.a };
				numClearValues = i + 1;
			}
		}

		if (descriptor.depthAttachment.texture != nullptr
			&& descriptor.depthAttachment.loadAction == LoadAction::Clear)
		{
			clearValues[numClearValues + 1].depthStencil = { descriptor.depthAttachment.clearDepth, descriptor.stencilAttachment.clearStencil };
			numClearValues++;
		}

		renderPassHash = renderPassHasher.get();
		framebufferHash = framebufferHasher.get();

		auto vulkanGraphics = static_cast<VulkanGraphics*>(_graphics);
		VkRenderPass renderPass = vulkanGraphics->GetVkRenderPass(descriptor, renderPassHash);
		VulkanFramebuffer* framebuffer = vulkanGraphics->GetFramebuffer(renderPass, descriptor, framebufferHash);

		VkRenderPassBeginInfo renderPassBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.framebuffer = framebuffer->framebuffer;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent = framebuffer->size;
		renderPassBeginInfo.clearValueCount = numClearValues;
		renderPassBeginInfo.pClearValues = clearValues;

		vkCmdBeginRenderPass(_vkCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void VulkanCommandBuffer::EndRenderPass()
	{
		vkCmdEndRenderPass(_vkCommandBuffer);
	}

	void VulkanCommandBuffer::SetPipeline(const PipelineStatePtr& pipeline)
	{

	}

	void VulkanCommandBuffer::DrawCore(PrimitiveTopology topology, uint32_t vertexCount, uint32_t instanceCount, uint32_t vertexStart, uint32_t baseInstance)
	{

	}

	void VulkanCommandBuffer::DrawIndexedCore(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount, uint32_t startIndex)
	{

	}
}
