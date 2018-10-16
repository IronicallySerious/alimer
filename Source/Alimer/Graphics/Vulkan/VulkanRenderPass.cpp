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

#include "VulkanGraphicsImpl.h"
#include "VulkanRenderPass.h"
#include "VulkanTexture.h"
#include "VulkanConvert.h"

namespace Alimer
{
    VulkanRenderPass::VulkanRenderPass(uint64_t hash, VulkanGraphicsDevice* device, const RenderPassDescriptor* descriptor)
        : _hash(hash)
        , _device(device)
    {
        uint32_t attachmentCount = 0;
        std::array<VkAttachmentDescription, MaxColorAttachments + 1> attachments = {};
        std::vector<VkAttachmentReference> colorReferences;
        VkAttachmentReference depthReference = {};
        bool hasDepth = false;

        for (uint32_t i = 0; i < MaxColorAttachments; i++)
        {
            const RenderPassColorAttachmentDescriptor& colorAttachment = descriptor->colorAttachments[i];
            auto attachment = colorAttachment.attachment;
            if (attachment == nullptr)
                continue;

            attachments[attachmentCount].format = vk::Convert(attachment->GetFormat());
            attachments[attachmentCount].samples = VK_SAMPLE_COUNT_1_BIT;
            attachments[attachmentCount].loadOp = vk::Convert(colorAttachment.loadAction);
            attachments[attachmentCount].storeOp = vk::Convert(colorAttachment.storeAction);
            attachments[attachmentCount].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[attachmentCount].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[attachmentCount].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[attachmentCount].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            colorReferences.push_back({ attachmentCount, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

            _colorAttachmentsCount++;
            attachmentCount++;
        }

        if (descriptor->depthStencil)
        {
            VkAttachmentLoadOp dsLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            VkAttachmentStoreOp dsStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

            if (any(descriptor->depthOperation & RenderPassDepthOperation::ClearDepthStencil))
                dsLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            else if (any(descriptor->depthOperation & RenderPassDepthOperation::LoadDepthStencil))
                dsLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

            if (any(descriptor->depthOperation & RenderPassDepthOperation::StoreDepthStencil))
                dsStoreOp = VK_ATTACHMENT_STORE_OP_STORE;

            attachments[attachmentCount].format = vk::Convert(descriptor->depthStencil->GetFormat());
            attachments[attachmentCount].samples = VK_SAMPLE_COUNT_1_BIT;
            attachments[attachmentCount].loadOp = dsLoadOp;
            attachments[attachmentCount].storeOp = dsStoreOp;
            attachments[attachmentCount].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[attachmentCount].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[attachmentCount].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[attachmentCount].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            depthReference.attachment = attachmentCount;
            depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            attachmentCount++;

            hasDepth = true;
        }

        VkSubpassDescription subpassDescription = {};
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
        subpassDescription.pColorAttachments = colorReferences.data();
        if (hasDepth)
        {
            subpassDescription.pDepthStencilAttachment = &depthReference;
        }
        else
        {
            subpassDescription.pDepthStencilAttachment = nullptr;
        }

        subpassDescription.inputAttachmentCount = 0;
        subpassDescription.pInputAttachments = nullptr;
        subpassDescription.preserveAttachmentCount = 0;
        subpassDescription.pPreserveAttachments = nullptr;
        subpassDescription.pResolveAttachments = nullptr;

        // Subpass dependencies for layout transitions
        std::array<VkSubpassDependency, 2> dependencies;

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo createInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.attachmentCount = attachmentCount;
        createInfo.pAttachments = attachments.data();
        createInfo.subpassCount = 1;
        createInfo.pSubpasses = &subpassDescription;
        createInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
        createInfo.pDependencies = dependencies.data();

        VkResult result = vkCreateRenderPass(_device->GetDevice(), &createInfo, nullptr, &_renderPass);
        if (result != VK_SUCCESS)
        {
            ALIMER_LOGERRORF("[Vulkan] - Failed to create render pass.");
        }

        ALIMER_LOGDEBUG("[Vulkan] - Created render pass");
    }

    VulkanRenderPass::~VulkanRenderPass()
    {
        if (_renderPass != VK_NULL_HANDLE)
        {
            vkDestroyRenderPass(_device->GetDevice(), _renderPass, nullptr);
            _renderPass = VK_NULL_HANDLE;
        }
    }

    VulkanFramebuffer::VulkanFramebuffer(VulkanGraphicsDevice* device, const VulkanRenderPass* renderPass, const RenderPassDescriptor* descriptor)
        : _id(device->AllocateCookie())
        , _device(device)
        , _renderPass(renderPass)
    {
        _width = UINT32_MAX;
        _height = UINT32_MAX;
        VkImageView views[MaxColorAttachments + 1];
        uint32_t attachmentCount = 0;

        for (uint32_t i = 0; i < MaxColorAttachments; i++)
        {
            const RenderPassColorAttachmentDescriptor& attachment = descriptor->colorAttachments[i];
            if (attachment.attachment == nullptr)
                continue;

            uint32_t mipLevel = attachment.attachment->GetBaseMipLevel();
            Texture* texture = attachment.attachment->GetTexture();
            _width = std::min(_width, texture->GetLevelWidth(mipLevel));
            _height = std::min(_height, texture->GetLevelHeight(mipLevel));
            views[attachmentCount++] = static_cast<VulkanTextureView*>(attachment.attachment)->GetVkImageView();
        }

        VkFramebufferCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.renderPass = _renderPass->GetVkRenderPass();
        createInfo.attachmentCount = attachmentCount;
        createInfo.pAttachments = views;
        createInfo.width = _width;
        createInfo.height = _height;
        createInfo.layers = 1;

        VkResult result = vkCreateFramebuffer(_device->GetDevice(), &createInfo, nullptr, &_framebuffer);
        if (result != VK_SUCCESS)
        {
            ALIMER_LOGERROR("[Vulkan] - Failed to create framebuffer.");
            return;
        }

        ALIMER_LOGDEBUGF("[Vulkan] - Created framebuffer : attachmentCount %u", attachmentCount);
    }

    VulkanFramebuffer::~VulkanFramebuffer()
    {
        if (_framebuffer != VK_NULL_HANDLE)
        {
            vkDestroyFramebuffer(_device->GetDevice(), _framebuffer, nullptr);
            _framebuffer = VK_NULL_HANDLE;
        }
    }
}
