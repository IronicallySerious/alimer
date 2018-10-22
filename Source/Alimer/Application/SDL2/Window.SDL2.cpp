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

#include "../../Application/Window.h"
#include "../../Application/Application.h"
#include "../../Core/Log.h"
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_syswm.h>

namespace Alimer
{
    void Window::PlatformConstruct()
    {
        const bool resizable = any(_flags & WindowFlags::Resizable);
        bool fullscreen = any(_flags & WindowFlags::Fullscreen);

        const int x = SDL_WINDOWPOS_CENTERED;
        const int y = SDL_WINDOWPOS_CENTERED;
        uint32_t windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;
        if (resizable)
        {
            windowFlags |= SDL_WINDOW_RESIZABLE;
        }

        if (fullscreen)
        {
            windowFlags |= SDL_WINDOW_FULLSCREEN;
        }

        _window = SDL_CreateWindow(_title.c_str(),
            x, y,
            static_cast<int>(_size.x),
            static_cast<int>(_size.y),
            windowFlags);

        SDL_SysWMinfo wmInfo;
        SDL_VERSION(&wmInfo.version);
        SDL_GetWindowWMInfo(_window, &wmInfo);

#if ALIMER_PLATFORM_LINUX
        _handle.connection = wmInfo.info.x11.display;
        _handle.handle = wmInfo.info.x11.window;
#elif ALIMER_PLATFORM_APPLE_OSX
        _handle.connection = nullptr;
        _handle.handle = wmInfo.info.cocoa.window;;
#elif ALIMER_PLATFORM_WINDOWS
        _hInstance = wmInfo.info.win.hinstance;
        _handle = wmInfo.info.win.window;
#endif
    }

    void Window::PlatformDestroy()
    {
        if (_window != nullptr)
        {
            SDL_DestroyWindow(_window);
            _window = nullptr;
        }
    }

    void Window::Show()
    {
        if (_visible)
            return;

        SDL_ShowWindow(_window);
        _visible = true;
    }

    void Window::Hide()
    {
        if (_visible)
        {
            SDL_HideWindow(_window);
            _visible = false;
        }
    }

    void Window::Minimize()
    {
        SDL_MinimizeWindow(_window);
    }

    void Window::Maximize()
    {
        SDL_MaximizeWindow(_window);
    }

    void Window::Restore()
    {
        SDL_RestoreWindow(_window);
    }

    void Window::Close()
    {
        PlatformDestroy();
    }

    bool Window::IsMinimized() const
    {
        return (SDL_GetWindowFlags(_window) & SDL_WINDOW_MINIMIZED) != 0;
    }

    void Window::SetTitle(const std::string& newTitle)
    {
        _title = newTitle;
        SDL_SetWindowTitle(_window, newTitle.c_str());
    }
}
