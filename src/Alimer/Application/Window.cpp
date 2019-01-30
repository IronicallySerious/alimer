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
#include "../Graphics/GraphicsDevice.h"
#if defined(ALIMER_GLFW)
#include "glfw/window_glfw.h"
#elif defined(ALIMER_SDL2)
#include "SDL2/WindowSDL2.h"
#endif

namespace alimer
{
    Window::Window()
        : _title("Alimer")
        , _size(800, 600)
        , _flags(WindowFlags::Default)
    {
    }

    Window::~Window()
    {
        SafeDelete(_impl);
    }

    bool Window::Define(const String& title, const IntVector2& size, WindowFlags flags)
    {
        SafeDelete(_impl);
        _size = size;
        _impl = new WindowImpl(title, size.x, size.y, flags);
        return _impl != nullptr;
    }

    void Window::Show()
    {
        _impl->Show();
    }

    void Window::Hide()
    {
        _impl->Hide();
    }

    void Window::Minimize()
    {
        _impl->Minimize();
    }

    void Window::Maximize()
    {
        _impl->Maximize();
    }

    void Window::Restore()
    {
        _impl->Restore();
    }

    void Window::Close()
    {
        _impl->Close();
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

    bool Window::IsOpen() const
    {
        return _impl != nullptr && _impl->IsOpen();
    }

    void Window::Resize(int width, int height)
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

    void Window::OnSizeChanged(const IntVector2& newSize)
    {
        WindowResizeEvent evt;
        evt.size = _size;
        resizeEvent(evt);
    }

    void Window::SetCursorVisible(bool visible)
    {
        _impl->SetCursorVisible(visible);
    }

    uint64_t Window::GetNativeHandle() const
    {
        return _impl->GetNativeHandle();
    }
    
    uint64_t Window::GetNativeDisplay() const
    {
        return _impl->GetNativeDisplay();
    }
}
