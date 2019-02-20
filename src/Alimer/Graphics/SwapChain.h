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

#include "../Graphics/GPUBackend.h"
#include "../Graphics/CommandBuffer.h"
#include "../Graphics/GPUResource.h"
#include "../Math/MathUtil.h"
#include <vector>

namespace alimer
{
    /// Defines a queue that organizes command buffers to be executed by a GPU.
    class ALIMER_API SwapChain : public GPUResource, public RefCounted
    {
        friend class GraphicsDevice;

    private:
        /// Constructor.
        SwapChain(GraphicsDevice* device, const SwapChainDescriptor* descriptor);

    public:
        /// Destructor.
        ~SwapChain() override;

        /// Unconditionally destroy the GPU resource.
        void Destroy() override;

        bool Resize(uint32_t width, uint32_t height);

    private:
        bool ResizeImpl(uint32_t width, uint32_t height);

        uint32_t _width = 0;
        uint32_t _height = 0;
        bool _vsync;
        PixelFormat _colorFormat = PixelFormat::Undefined;
        uint64_t _nativeHandle = 0;

#if defined(ALIMER_VULKAN)
        VkSurfaceKHR                _surface = VK_NULL_HANDLE;
        VkSwapchainKHR              _handle = VK_NULL_HANDLE;
        uint32_t                    _presentQueueIndex;
        VkQueue                     _presentQueue = VK_NULL_HANDLE;
        uint32_t                    _imageIndex = 0;
        uint32_t                    _semaphoreIndex = 0;
        uint32_t                    _imageCount = 0;
        std::vector<VkImage>        _images;
        std::vector<VkSemaphore>    _imageSemaphores;
#elif defined(ALIMER_D3D12)
#endif
    };
}
