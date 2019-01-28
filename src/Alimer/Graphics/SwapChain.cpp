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

#include "../Graphics/SwapChain.h"
#include "../Graphics/GraphicsDevice.h"
#include "../Graphics/Backend.h"

namespace alimer
{
    SwapChain::SwapChain(GraphicsDevice* device)
        : GPUResource(device, Type::SwapChain)
    {
    }

    SwapChain::~SwapChain()
    {
        Destroy();
    }

    void SwapChain::Destroy()
    {
        _backbufferTextures.Clear();
        _depthStencilTexture.Reset();
        _framebuffers.Clear();

        // Destroy backend.
        SafeDelete(_impl);
    }

    void SwapChain::Resize(uint32_t width, uint32_t height)
    {
        if (_width == width
            && _height == height)
        {
            return;
        }

        _width = width;
        _height = height;
    }

    void SwapChain::Present()
    {
    }

    void SwapChain::InitializeFramebuffer()
    {
        const uint32_t textureCount = _backbufferTextures.Size();
        _framebuffers.Resize(textureCount);

        const bool hasDepthStencil = _depthStencilFormat != PixelFormat::Unknown;
        if (hasDepthStencil)
        {
            _depthStencilTexture = _graphicsDevice->Create2DTexture(_width, _height, _depthStencilFormat, 1, 1, TextureUsage::RenderTarget);
        }

        for (uint32_t i = 0; i < textureCount; ++i)
        {
            FramebufferDescriptor fboDescriptor = {};
            fboDescriptor.colorAttachments[0].texture = _backbufferTextures[i].Get();
            fboDescriptor.colorAttachments[0].level = 0;
            fboDescriptor.colorAttachments[0].slice = 0;
            if (hasDepthStencil)
            {
                fboDescriptor.depthStencilAttachment.texture = _depthStencilTexture.Get();
            }

            _framebuffers[i] = _graphicsDevice->CreateFramebuffer(&fboDescriptor);
        }
    }
}
