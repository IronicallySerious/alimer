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

#include "../Application/Window.h"
#include "../Graphics/Framebuffer.h"
#include "../Graphics/SwapChain.h"
#include "../Graphics/GPUDevice.h"

namespace alimer
{
    Window::Window(GPUDevice* device, const String& title, uint32_t width, uint32_t height, WindowFlags flags)
        : _device(device)
        , _title(title)
        , _size(width, height)
        , _flags(flags)
        , _swapChain(nullptr)
    {
    }

    void Window::Show()
    {
    }

    void Window::Hide()
    {
    }

    void Window::Minimize()
    {
    }

    void Window::Maximize()
    {
    }

    void Window::Restore()
    {
    }

    void Window::Close()
    {
    }

    void Window::SetTitle(const String& newTitle)
    {
        _title = newTitle;
    }

    bool Window::IsFullscreen() const
    {
        return any(_flags & WindowFlags::Fullscreen);
    }

    void Window::Resize(uint32_t width, uint32_t height)
    {
        if (_size.x == width
            && _size.y == height)
        {
            return;
        }

        _size.x = width;
        _size.y = height;
        PlatformResize(width, height);
        OnSizeChanged(_size);
    }

    void Window::OnSizeChanged(const uvec2& newSize)
    {
        _swapChain->Resize(newSize.x, newSize.y);
        WindowResizeEvent evt;
        evt.size = _size;
        resizeEvent(evt);
    }

    void Window::SetCursorVisible(bool visible)
    {

    }

    void Window::OnCreated()
    {
        SwapChainDescriptor descriptor = {};
        descriptor.nativeConnection = _nativeConnection;
        descriptor.nativeWindow = _nativeWindow;
        descriptor.width = _size.x;
        descriptor.height = _size.y;
        descriptor.sRGB = false;
        descriptor.preferredDepthStencilFormat = PixelFormat::D24UNormS8;
        descriptor.preferredSamples = SampleCount::Count1;
        _swapChain = _device->CreateSwapChain(&descriptor);
    }

    void Window::OnDestroyed()
    {
        SafeDelete(_swapChain);
    }

    Framebuffer* Window::GetCurrentFramebuffer() const
    {
        return _swapChain->GetCurrentFramebuffer();
    }

    void Window::SwapBuffers()
    {
        _swapChain->Present();
    }
}
