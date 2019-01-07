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

#include "../Graphics/RenderWindow.h"
#include "../Graphics/GPUDevice.h"
#include "../Graphics/GPUDeviceImpl.h"
#include "../Core/Log.h"

namespace alimer
{
    RenderWindow::RenderWindow(const RenderWindowDescriptor* descriptor)
        : Window(descriptor->title, descriptor->size, descriptor->windowFlags)
    {
        _swapChain = GetSubsystem<GPUDevice>()->GetImpl()->CreateSwapChain(
            GetNativeHandle(),
            descriptor->size.x,
            descriptor->size.y,
            descriptor->preferredDepthStencilFormat,
            descriptor->sRGB);

        const uint32_t backBufferCount = _swapChain->GetBackBufferCount();
        _backbufferTextures.Resize(backBufferCount);
        _framebuffers.Resize(backBufferCount);

        const bool hasDepthStencil = descriptor->preferredDepthStencilFormat != PixelFormat::Unknown;
        if (hasDepthStencil)
        {
            _depthStencilTexture = new Texture2D();
            _depthStencilTexture->Define(descriptor->size.x, descriptor->size.y, 1, 1, descriptor->preferredDepthStencilFormat, TextureUsage::OutputAttachment);
        }

        for (uint32_t i = 0; i < backBufferCount; ++i)
        {
            GPUTexture* gpuTexture = _swapChain->GetBackBufferTexture(i);
            _backbufferTextures[i] = new Texture2D(gpuTexture);
            _framebuffers[i] = new Framebuffer();
            _framebuffers[i]->SetColorAttachment(0, _backbufferTextures[i].Get(), 0, 0);
            if (hasDepthStencil)
            {
                _framebuffers[i]->SetDepthStencilAttachment(_depthStencilTexture.Get(), 0, 0);
            }
        }
    }

    RenderWindow::~RenderWindow()
    {
        Destroy();
    }

    void RenderWindow::Destroy()
    {
        _depthStencilTexture.Reset();
        _backbufferTextures.Clear();
        _framebuffers.Clear();
        SafeDelete(_swapChain);
    }

    void RenderWindow::SwapBuffers()
    {
        _swapChain->Present();
    }

    void RenderWindow::OnSizeChanged(const uvec2& newSize)
    {
        _swapChain->Configure(newSize.x, newSize.y);
        Window::OnSizeChanged(newSize);
    }

    Framebuffer* RenderWindow::GetCurrentFramebuffer() const
    {
        uint32_t currentBackBufferIndex = _swapChain->GetCurrentBackBuffer();
        return _framebuffers[currentBackBufferIndex].Get();
    }
}
