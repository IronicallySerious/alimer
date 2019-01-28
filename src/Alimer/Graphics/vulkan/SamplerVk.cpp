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

#include "SamplerVk.h"
#include "GPUDeviceVk.h"
#include "../../Core/Log.h"

namespace alimer
{
    VkSamplerAddressMode GetVkSamplerAddressMode(SamplerAddressMode mode)
    {
        switch (mode) {
        case SamplerAddressMode::Repeat:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case SamplerAddressMode::MirrorRepeat:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case SamplerAddressMode::ClampToEdge:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case SamplerAddressMode::ClampToBorder:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        case SamplerAddressMode::MirrorClampToEdge:
            return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
        default:
            ALIMER_UNREACHABLE();
        }
    }

    VkFilter GetVkSamplerMinMagFilter(SamplerMinMagFilter filter)
    {
        switch (filter) {
        case SamplerMinMagFilter::Linear:
            return VK_FILTER_LINEAR;
        case SamplerMinMagFilter::Nearest:
            return VK_FILTER_NEAREST;
        default:
            ALIMER_UNREACHABLE();
        }
    }

    VkSamplerMipmapMode GetVkSamplerMipFilter(SamplerMipFilter filter)
    {
        switch (filter) {
        case SamplerMipFilter::Linear:
            return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        case SamplerMipFilter::Nearest:
            return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        default:
            ALIMER_UNREACHABLE();
        }
    }

    VkBorderColor GetVkBorderColor(SamplerBorderColor borderColor)
    {
        switch (borderColor)
        {
        case SamplerBorderColor::TransparentBlack:
            return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
        case SamplerBorderColor::OpaqueBlack:
            return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        case SamplerBorderColor::OpaqueWhite:
            return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        default:
            ALIMER_UNREACHABLE();
            return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
        }

        return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    }

    SamplerVk::SamplerVk(GPUDeviceVk* device, const SamplerDescriptor* descriptor)
        : _device(device)
    {
        VkSamplerCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.magFilter = GetVkSamplerMinMagFilter(descriptor->magFilter);
        createInfo.minFilter = GetVkSamplerMinMagFilter(descriptor->minFilter);
        createInfo.mipmapMode = GetVkSamplerMipFilter(descriptor->mipmapFilter);
        createInfo.addressModeU = GetVkSamplerAddressMode(descriptor->addressModeU);
        createInfo.addressModeV = GetVkSamplerAddressMode(descriptor->addressModeV);
        createInfo.addressModeW = GetVkSamplerAddressMode(descriptor->addressModeW);
        createInfo.mipLodBias = 0.0f;
        createInfo.anisotropyEnable = descriptor->maxAnisotropy > 1;
        createInfo.maxAnisotropy = static_cast<float>(descriptor->maxAnisotropy);
        createInfo.compareEnable = descriptor->compareFunction == CompareFunction::Never ? VK_FALSE : VK_TRUE;
        createInfo.compareOp = GetVkCompareOp(descriptor->compareFunction);
        createInfo.minLod = descriptor->lodMinClamp;
        createInfo.maxLod = descriptor->lodMaxClamp;
        createInfo.borderColor = GetVkBorderColor(descriptor->borderColor);
        createInfo.unnormalizedCoordinates = VK_FALSE;

        if (vkCreateSampler(device->GetVkDevice(), &createInfo, nullptr, &_handle) != VK_SUCCESS)
        {
            ALIMER_LOGERROR("Vulkan: Failed to create sampler");
        }
    }

    SamplerVk::~SamplerVk()
    {
        if (_handle != VK_NULL_HANDLE)
        {
            _device->DestroySampler(_handle);
            _handle = VK_NULL_HANDLE;
        }
    }

}
