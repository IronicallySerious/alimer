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

#include "VulkanRenderPass.h"
#include "VulkanTexture.h"
#include "VulkanGraphics.h"
#include "VulkanConvert.h"
#include "../../Core/Log.h"
#include "../../Util/HashMap.h"

namespace Alimer
{
    VulkanRenderPass::VulkanRenderPass(VulkanGraphics* graphics, const RenderPassDescription* descriptor)
        : RenderPass(graphics, descriptor)
        , _logicalDevice(graphics->GetLogicalDevice())
        , _renderPass(graphics->GetVkRenderPass(descriptor))
    {
        VkImageView views[MaxColorAttachments + 1];
        uint32_t numViews = 0;

        for (uint32_t i = 0; i < MaxColorAttachments; i++)
        {
            const RenderPassAttachment& colorAttachment = descriptor->colorAttachments[i];
            Texture* texture = colorAttachment.texture;
            if (!texture)
                continue;

            _width = std::min(_width, texture->GetLevelWidth(colorAttachment.mipLevel));
            _height = std::min(_height, texture->GetLevelHeight(colorAttachment.mipLevel));
            views[numViews++] = static_cast<VulkanTexture*>(texture)->GetDefaultImageView();
        }

        VkFramebufferCreateInfo createInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.renderPass = _renderPass;
        createInfo.attachmentCount = numViews;
        createInfo.pAttachments = views;
        createInfo.width = _width;
        createInfo.height = _height;
        createInfo.layers = 1;

        VkResult result = vkCreateFramebuffer(_logicalDevice, &createInfo, nullptr, &_framebuffer);
        if (result != VK_SUCCESS)
        {
            ALIMER_LOGERROR("Vulkan - Failed to create framebuffer.");
            return;
        }
    }

    VulkanRenderPass::~VulkanRenderPass()
    {
        Destroy();
    }

    void VulkanRenderPass::Destroy()
    {
        if(_framebuffer != VK_NULL_HANDLE)
        {
            vkDestroyFramebuffer(_logicalDevice, _framebuffer, nullptr);
            _framebuffer = VK_NULL_HANDLE;
        }
    }
}
