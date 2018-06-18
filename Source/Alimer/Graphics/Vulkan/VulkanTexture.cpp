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

#include "VulkanTexture.h"
#include "VulkanGraphics.h"
#include "VulkanConvert.h"
#include "../../Core/Log.h"
#include "../../Util/HashMap.h"

namespace Alimer
{
	VulkanTexture::VulkanTexture(VulkanGraphics* graphics, const TextureDescription& description, VkImage vkImage, VkImageUsageFlags usage)
		: Texture()
		, _logicalDevice(graphics->GetLogicalDevice())
		, _vkHandle(vkImage)
	{
		_description = description;

		if (usage & (VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT))
		{
			VkImageViewCreateInfo viewCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
			viewCreateInfo.image = vkImage;
			viewCreateInfo.format = vk::Convert(description.format);
			viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
			viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
			viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
			viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
			viewCreateInfo.subresourceRange.aspectMask = vk::FormatToAspectMask(viewCreateInfo.format);
			viewCreateInfo.subresourceRange.baseMipLevel = 0;
			viewCreateInfo.subresourceRange.baseArrayLayer = 0;
			viewCreateInfo.subresourceRange.levelCount = _description.mipLevels;
			viewCreateInfo.subresourceRange.layerCount = _description.arrayLayers;
			viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // get_image_view_type(tmpinfo, nullptr);

			if (vkCreateImageView(_logicalDevice, &viewCreateInfo, nullptr, &_defaultImageView) != VK_SUCCESS)
			{
				vkDestroyImage(_logicalDevice, _vkHandle, nullptr);
				return;
			}
		}
	}

	VulkanTexture::~VulkanTexture()
	{
		if (_defaultImageView != VK_NULL_HANDLE)
		{
			vkDestroyImageView(_logicalDevice, _defaultImageView, nullptr);
			_defaultImageView = VK_NULL_HANDLE;
		}
	}
}
