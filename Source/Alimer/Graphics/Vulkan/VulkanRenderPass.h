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

#include "../RenderPass.h"
#include "VulkanBackend.h"

namespace Alimer
{
    class VulkanGraphicsDevice;

    class VulkanRenderPass final 
    {
    public:
        VulkanRenderPass(uint64_t hash, VulkanGraphicsDevice* device, const RenderPassDescriptor* descriptor);
        ~VulkanRenderPass();

        VkRenderPass GetVkRenderPass() const { return _renderPass; }
        uint32_t GetColorAttachmentsCount() const { return _colorAttachmentsCount; }

        inline uint64_t GetHash() const { return _hash; }

    private:
        uint64_t _hash = 0;
        VulkanGraphicsDevice* _device;
        VkRenderPass _renderPass = VK_NULL_HANDLE;
        uint32_t _colorAttachmentsCount = 0;

        DISALLOW_COPY_MOVE_AND_ASSIGN(VulkanRenderPass);
    };

    class VulkanFramebuffer 
    {
    public:
        VulkanFramebuffer(VulkanGraphicsDevice* device, const VulkanRenderPass* renderPass, const RenderPassDescriptor* descriptor);
        ~VulkanFramebuffer();

        VkFramebuffer GetVkFramebuffer() const { return _framebuffer; }
        uint32_t GetWidth() const { return _width; }
        uint32_t GetHeight() const { return _height; }
        const VulkanRenderPass* GetRenderPass() const { return _renderPass; }
        uint64_t GetId() const { return _id; }

    private:
        VulkanGraphicsDevice* _device;
        VkFramebuffer _framebuffer = VK_NULL_HANDLE;
        const VulkanRenderPass* _renderPass;
        uint32_t _width = 0;
        uint32_t _height = 0;
        std::vector<VkImageView> _attachments;

        uint64_t _id;

    private:
        DISALLOW_COPY_MOVE_AND_ASSIGN(VulkanFramebuffer);
    };
}
