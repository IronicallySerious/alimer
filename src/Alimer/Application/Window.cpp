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
#include "../Graphics/GraphicsDevice.h"
#if ALIMER_SDL2
#include "../Application/SDL2/WindowSDL2.h"
#endif

namespace alimer
{
    Window::Window(const String& title, uint32_t width, uint32_t height, WindowFlags flags)
        : _title(title)
        , _size(width, height)
        , _flags(flags)
        , _impl(new WindowImpl(title, width, height, flags))
    {
    }

    Window::~Window()
    {
        delete _impl;
        _impl = nullptr;
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
        _impl->SetTitle(newTitle);
    }

    void Window::SetFullscreen(bool value)
    {
        _impl->SetFullscreen(value);
    }

    bool Window::IsVisible() const
    {
        return _impl->IsVisible();
    }

    bool Window::IsMinimized() const
    {
        return _impl->IsMinimized();
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
        _impl->Resize(width, height);
        OnSizeChanged(_size);
    }

    void Window::OnSizeChanged(const uvec2& newSize)
    {
        //_swapChain.Resize(newSize.x, newSize.y);
        WindowResizeEvent evt;
        evt.size = _size;
        resizeEvent(evt);
    }

    void Window::SetCursorVisible(bool visible)
    {
        _impl->SetCursorVisible(visible);
    }

    NativeHandle Window::GetNativeHandle() const
    {
        return _impl->GetNativeHandle();
    }
    
    NativeDisplay Window::GetNativeDisplay() const
    {
        return _impl->GetNativeDisplay();
    }

    void Window::OnCreated()
    {
        /*SwapChainDescriptor descriptor = {};
        descriptor.nativeConnection = _nativeConnection;
        descriptor.nativeWindow = _nativeWindow;
        descriptor.width = _size.x;
        descriptor.height = _size.y;
        descriptor.sRGB = false;
        descriptor.preferredDepthStencilFormat = PixelFormat::D24UNormS8;
        descriptor.preferredSamples = SampleCount::Count1;
        _swapChain.Define(&descriptor);*/
    }

    void Window::OnDestroyed()
    {
    }

    void Window::SwapBuffers()
    {
        //_swapChain.Present();
    }
}
