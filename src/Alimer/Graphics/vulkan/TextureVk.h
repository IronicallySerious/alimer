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

#include "BackendVk.h"
#include "../Texture.h"

namespace alimer
{
	/// Vulkan Texture implementation.
	class TextureVk final : public Texture
	{
	public:
        TextureVk(GraphicsDeviceVk* device, const TextureDescriptor* descriptor, VkImage nativeTexture, const void* pInitData);
        ~TextureVk() override;

        VkImage GetVkImage() const { return _handle; }
        VkImageView GetVkImageView() const { return _imageView; }

	private:
        bool Create(const void* pInitData) override;
        void Destroy() override;

        VkImage         _handle = VK_NULL_HANDLE;
        VmaAllocation   _allocation = VK_NULL_HANDLE;
        VkImageView     _imageView = VK_NULL_HANDLE;
        VkFormat        _vkFormat = VK_FORMAT_UNDEFINED;
        VkImageUsageFlags _vkUsage = 0;
	};
}
