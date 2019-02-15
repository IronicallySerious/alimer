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

#include "FramebufferVk.h"
#include "GPUDeviceVk.h"
#include "../../Core/Log.h"

namespace alimer
{
    FramebufferVk::FramebufferVk(GPUDeviceVk* device, const FramebufferDescriptor* descriptor)
        : Framebuffer(device, descriptor)
    {
        VkImageView attachments[MaxColorAttachments + 1u];
        uint32_t attachmentCount = 0;

        for (uint32_t i = 0; i < _colorAttachments.size(); ++i)
        {
            if (descriptor->colorAttachments[i].texture == nullptr)
            {
                continue;
            }

            const uint32_t level = descriptor->colorAttachments[i].level;
            //attachments[attachmentCount++] = _colorAttachments[i].texture->GetView(level, descriptor->colorAttachments[i].slice);
        }

        if (_depthStencilAttachment.texture != nullptr) {
            const uint32_t level = descriptor->depthStencilAttachment.level;
            //attachments[attachmentCount++] = _depthStencilAttachment.texture->GetView(level, _depthStencilAttachment.slice);
        }

        VkFramebufferCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        //createInfo.renderPass = _renderPass->GetVkRenderPass();
        createInfo.attachmentCount = attachmentCount;
        createInfo.pAttachments = attachments;
        createInfo.width = _width;
        createInfo.height = _height;
        createInfo.layers = _layers;

        VkResult result = vkCreateFramebuffer(device->GetVkDevice(), &createInfo, nullptr, &_handle);
        if (result != VK_SUCCESS)
        {
            ALIMER_LOGERROR("[Vulkan] - Failed to create framebuffer.");
            return;
        }

        ALIMER_LOGDEBUG("[Vulkan] - Created framebuffer");
    }

    FramebufferVk::~FramebufferVk()
    {
        Destroy();
    }

    void FramebufferVk::Destroy()
    {

    }
}