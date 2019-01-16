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

#include "Graphics/SwapChain.h"
#include "Graphics/GPUDevice.h"
#include "Graphics/DeviceBackend.h"

namespace alimer
{
    SwapChain::SwapChain(GPUDevice* device)
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
        SafeDelete(_impl);
    }

    bool SwapChain::Define(const SwapChainDescriptor* descriptor)
    {
        ALIMER_ASSERT(descriptor);
        _width = descriptor->width;
        _height = descriptor->height;
        _backBufferFormat = descriptor->sRGB ? PixelFormat::BGRA8UNormSrgb : PixelFormat::BGRA8UNorm;
        _depthStencilFormat = descriptor->preferredDepthStencilFormat;

        Destroy();
        _impl = _device->GetImpl()->CreateSwapChain(descriptor);
        InitializeFramebuffer();
        return _impl != nullptr;
    }

    void SwapChain::Resize(uint32_t width, uint32_t height)
    {
        if (_width == width
            && _height == height)
        {
            return;
        }

        _impl->Resize(width, height);
        _width = width;
        _height = height;
    }

    void SwapChain::Present()
    {
        _impl->Present();
    }

    void SwapChain::InitializeFramebuffer()
    {
        const uint32_t textureCount = _impl->GetTextureCount();
        _backbufferTextures.Resize(textureCount);
        _framebuffers.Resize(textureCount);

        const bool hasDepthStencil = _depthStencilFormat != PixelFormat::Unknown;
        if (hasDepthStencil)
        {
            _depthStencilTexture = new Texture();
            _depthStencilTexture->Define2D(_width, _height, _depthStencilFormat, 1, 1, nullptr, TextureUsage::RenderTarget);
        }

        for (uint32_t i = 0; i < textureCount; ++i)
        {
            FramebufferDescriptor fboDescriptor = {};
            fboDescriptor.colorAttachments[0].texture = _backbufferTextures[i].Get();
            if (hasDepthStencil)
            {
                fboDescriptor.depthStencilAttachment.texture = _depthStencilTexture.Get();
            }

            //_framebuffers[i] = _device->CreateFramebuffer(&fboDescriptor);
        }
    }
}
