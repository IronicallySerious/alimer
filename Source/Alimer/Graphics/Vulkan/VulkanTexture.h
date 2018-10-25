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

#include "../Texture.h"
#include "VulkanBackend.h"

namespace Alimer
{
    class VulkanGraphicsDevice;

	/// Vulkan Texture implementation.
	class VulkanTexture final : public Texture
	{
	public:
        VulkanTexture(VulkanGraphicsDevice* device, const TextureDescriptor* descriptor, const ImageLevel* initialData, VkImage existingImage = VK_NULL_HANDLE, VkImageUsageFlags usageFlags = 0);
        ~VulkanTexture();
        void Destroy() override;

        VkImage GetVkImage() const { return _vkImage; }

	private:
        SharedPtr<TextureView> CreateTextureView(const TextureViewDescriptor* descriptor) const override;
        VkDevice _logicalDevice;
        VmaAllocator _allocator;
        VkImage _vkImage = VK_NULL_HANDLE;
        bool _allocated = false;
	};

    /// Vulkan TextureView implementation.
    /*class VulkanTextureView final 
    {
    public:
        VulkanTextureView(VulkanGraphicsDevice* device, VulkanTexture* texture, const TextureViewDescriptor* descriptor);
        ~VulkanTextureView();

        VkImageView GetVkImageView() const { return _vkImageView; }
        uint64_t GetId() const { return _id; }

    private:
        VkDevice _logicalDevice;
        VkImageView _vkImageView = VK_NULL_HANDLE;
        uint64_t _id;
    };*/
}
