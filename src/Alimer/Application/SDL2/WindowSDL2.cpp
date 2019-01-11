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

#include "WindowSDL2.h"
#include "../../Application/Application.h"
#include "../../Core/Log.h"

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_syswm.h>

namespace alimer
{
    WindowSDL2::WindowSDL2(GPUDevice* device, const String& title, uint32_t width, uint32_t height, WindowFlags flags)
        : Window(device, title, width, height, flags)
    {
        const bool resizable = any(flags & WindowFlags::Resizable);
        bool fullscreen = any(flags & WindowFlags::Fullscreen);

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

        _window = SDL_CreateWindow(title.CString(), x, y, int(width), int(height), windowFlags);

        SDL_SysWMinfo wmInfo;
        SDL_VERSION(&wmInfo.version);
        SDL_GetWindowWMInfo(_window, &wmInfo);

#if ALIMER_PLATFORM_LINUX
        _nativeWindow = wmInfo.info.x11.window;
        _nativeConnection = wmInfo.info.x11.display;
#elif ALIMER_PLATFORM_APPLE_OSX
        _nativeWindow = wmInfo.info.cocoa.window;
#elif ALIMER_PLATFORM_WINDOWS
        _nativeWindow = wmInfo.info.win.window;
        _nativeConnection = wmInfo.info.win.hinstance;
#endif

        OnCreated();
    }

    WindowSDL2::~WindowSDL2()
    {
        Destroy();
    }

    void WindowSDL2::Destroy()
    {
        if (_window != nullptr)
        {
            SDL_DestroyWindow(_window);
            _window = nullptr;

            OnDestroyed();
        }
    }

    void WindowSDL2::PlatformResize(uint32_t width, uint32_t height)
    {
        SDL_SetWindowSize(_window, (int)width, (int)height);
    }

    void WindowSDL2::Show()
    {
        if (_visible)
            return;

        SDL_ShowWindow(_window);
        _visible = true;
    }

    void WindowSDL2::Hide()
    {
        if (_visible)
        {
            SDL_HideWindow(_window);
            _visible = false;
        }
    }

    void WindowSDL2::Minimize()
    {
        SDL_MinimizeWindow(_window);
    }

    void WindowSDL2::Maximize()
    {
        SDL_MaximizeWindow(_window);
    }

    void WindowSDL2::Restore()
    {
        SDL_RestoreWindow(_window);
    }

    void WindowSDL2::Close()
    {
        Destroy();
    }

    bool WindowSDL2::IsVisible() const
    {
        return _visible;
    }

    bool WindowSDL2::IsMinimized() const
    {
        return (SDL_GetWindowFlags(_window) & SDL_WINDOW_MINIMIZED) != 0;
    }

    void WindowSDL2::SetTitle(const String& newTitle)
    {
        Window::SetTitle(newTitle);
        SDL_SetWindowTitle(_window, newTitle.CString());
    }

    bool WindowSDL2::IsCursorVisible() const
    {
        return SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE;
    }

    void WindowSDL2::SetCursorVisible(bool visible)
    {
        SDL_ShowCursor(visible ? SDL_ENABLE : SDL_DISABLE);
    }
}
