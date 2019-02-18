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
#include "../GraphicsDevice.h"
#include <vector>

namespace alimer
{
    class TextureVk;

	/// Vulkan SwapChain implementation.
	class SwapChainVk final 
	{
	public:
        SwapChainVk(GraphicsDeviceVk* device, VkSurfaceKHR surface, const SwapChainDescriptor* descriptor);
        ~SwapChainVk();


        VkResult AcquireNextImage(VkSemaphore semaphore, uint32_t *imageIndex);
        VkResult QueuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore);

        VkSwapchainKHR GetVkHandle() const { return _handle; }
        uint32_t GetImageCount() const { return _imageCount; }
        uint32_t GetImageIndex() const { return _imageIndex; }
        Framebuffer* GetFramebuffer() const { return _framebuffers[_imageIndex].Get(); };

        void Resize();
        bool Resize(uint32_t width, uint32_t height);

	private:
        GraphicsDeviceVk*           _device;
        VkSurfaceKHR                _surface = VK_NULL_HANDLE;
        VkSwapchainKHR              _handle = VK_NULL_HANDLE;
        uint32_t                    _width = 0;
        uint32_t                    _height = 0;
        PixelFormat                 _colorFormat = PixelFormat::Undefined;
        bool                        _vSync;
        bool                        _tripleBuffer;
        PixelFormat                 _depthStencilFormat = PixelFormat::Undefined;
        SampleCount                 _samples;
        uint32_t                    _imageIndex = 0;
        uint32_t                    _imageCount = 0;
        std::vector<VkImage>        _images;
        std::vector<std::unique_ptr<TextureVk>> _swapchainTextures;
        std::vector<SharedPtr<Framebuffer>> _framebuffers;
	};
}
