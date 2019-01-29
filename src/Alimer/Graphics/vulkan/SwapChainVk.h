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
#include <vector>

namespace alimer
{
    class TextureVk;

	/// Vulkan SwapChain implementation.
	class SwapChainVk final : public GPUSwapChain
	{
	public:
        SwapChainVk(GPUDeviceVk* device, VkSurfaceKHR surface, const SwapChainDescriptor* descriptor);
        ~SwapChainVk();

        uint32_t GetTextureCount() const override {
            return _imageCount;
        }
        uint32_t GetCurrentBackBuffer() const override {
            return _imageIndex;
        }
        GPUTexture* GetBackBufferTexture(uint32_t index) const override {
            return nullptr;
        }

        void Resize();
        void Resize(uint32_t width, uint32_t height);
        VkResult AcquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t *imageIndex);
        VkResult QueuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE);

	private:
        GPUDeviceVk* _device;
        VkSurfaceKHR _surface = VK_NULL_HANDLE;
        VkSwapchainKHR _handle = VK_NULL_HANDLE;
        uint32_t _width = 0;
        uint32_t _height = 0;
        VkFormat _colorFormat = VK_FORMAT_UNDEFINED;
        bool _vSync;
        PixelFormat _depthStencilFormat;
        SampleCount _samples;
        uint32_t _imageCount = 0;
        uint32_t _imageIndex = 0;
        std::vector<std::unique_ptr<TextureVk>> _swapchainTextures;
	};
}
