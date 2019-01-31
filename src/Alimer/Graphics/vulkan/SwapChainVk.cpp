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

#include "SwapChainVk.h"
#include "TextureVk.h"
#include "GPUDeviceVk.h"
#include "../../Core/Log.h"

namespace alimer
{
    SwapChainVk::SwapChainVk(GPUDeviceVk* device, VkSurfaceKHR surface, const SwapChainDescriptor* descriptor)
        : _device(device)
        , _surface(surface)
        , _vSync(descriptor->vSync)
        , _depthStencilFormat(descriptor->preferredDepthStencilFormat)
        , _samples(descriptor->preferredSamples)
    {
        Resize(descriptor->width, descriptor->height);
    }

    SwapChainVk::~SwapChainVk()
    {
        if (_handle != VK_NULL_HANDLE)
        {
            //for (uint32_t i = 0; i < imageCount; i++)
            //{
            //    vkDestroyImageView(_device->GetVkDevice(), buffers[i].view, nullptr);
            //}

            vkDestroySwapchainKHR(_device->GetVkDevice(), _handle, nullptr);
            _handle = VK_NULL_HANDLE;
        }

        if (_surface != VK_NULL_HANDLE)
        {
            vkDestroySurfaceKHR(_device->GetVkInstance(), _surface, nullptr);
            _surface = VK_NULL_HANDLE;
        }

    }

    void SwapChainVk::Resize()
    {
        Resize(_width, _height);
    }

    void SwapChainVk::Resize(uint32_t width, uint32_t height)
    {
        /*VkPhysicalDevice physicalDevice = _device->GetVkPhysicalDevice();
        
        
        

        _width = swapchainSize.width;
        _height = swapchainSize.height;
        _colorFormat = format.format;

        // Create backend textures
        _swapchainTextures.resize(_imageCount);
        TextureDescriptor textureDescriptor = {};
        textureDescriptor.width = _width;
        textureDescriptor.height = _height;
        textureDescriptor.depth = 1;
        textureDescriptor.arraySize = 1;
        textureDescriptor.mipLevels = 1;
        textureDescriptor.samples = SampleCount::Count1;
        textureDescriptor.type = TextureType::Type2D;
        textureDescriptor.format = vk::Convert(_colorFormat);
        textureDescriptor.usage = textureUsage;

        const bool canClear = createInfo.imageUsage & VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        VkCommandBuffer clearImageCmdBuffer = canClear ? _device->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true) : VK_NULL_HANDLE;
        for (uint32_t i = 0; i < _imageCount; ++i)
        {
            _swapchainTextures[i] = std::make_unique<TextureVk>(_device, &textureDescriptor, swapchainImages[i], nullptr);
            
            // Clear with default value if supported.
            if (canClear)
            {
                // Clear images with default color.
                VkClearColorValue clearColor = {};
                clearColor.float32[3] = 1.0f;
                //clearColor.float32[0] = 1.0f;

                VkImageSubresourceRange clearRange = {};
                clearRange.layerCount = 1;
                clearRange.levelCount = 1;

                // Clear with default color.
                _device->ClearImageWithColor(
                    clearImageCmdBuffer,
                    swapchainImages[i],
                    clearRange,
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                    &clearColor);
            }
        }

        if (canClear)
        {
            _device->FlushCommandBuffer(clearImageCmdBuffer, true);
        }*/
    }
}
