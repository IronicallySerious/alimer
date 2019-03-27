//
// Copyright (c) 2017-2019 Amer Koleci and contributors.
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

#include "graphics/SwapChain.h"
#include "BackendVk.h"
#include <vector>

namespace alimer
{
    /// Vulkan SwapChain.
    class SwapChainVk final : public SwapChain
    {
    public:
        SwapChainVk(GraphicsDeviceVk* device, VkSurfaceKHR surface, const SwapChainDescriptor* descriptor);
        ~SwapChainVk();

        void Destroy();
        bool ResizeImpl(uint32_t width, uint32_t height) override;
        bool GetNextTextureImpl() override;

        uint32_t GetImageCount() const { return _imageCount; }

    private:
        VkInstance _instance;
        VkPhysicalDevice _physicalDevice;
        VkDevice _device;
        VkSurfaceKHR _surface = VK_NULL_HANDLE;
        VkSurfaceFormatKHR _format{};
        VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
        uint32_t _imageCount = 0;
        uint32_t _imageIndex = 0;
        std::vector<VkImage> _images;
        std::vector<VkSemaphore> _imageSemaphores;
        uint32_t _frameIndex = 0;
    };
}
