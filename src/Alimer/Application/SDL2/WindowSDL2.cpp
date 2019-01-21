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
#include "Application/Application.h"
#include "Core/Log.h"

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_syswm.h>

namespace alimer
{
    inline NativeHandle GetNativeWindowHandle(const SDL_SysWMinfo& wmi) noexcept
    {
        (void)wmi;
#if defined(SDL_VIDEO_DRIVER_WINDOWS)
        return wmi.info.win.window;
#elif defined(SDL_VIDEO_DRIVER_WINRT)
        return wmi.info.winrt.window;
#elif defined(SDL_VIDEO_DRIVER_X11)
        return (void*)(uintptr_t)wmi.info.x11.window;
#elif defined(SDL_VIDEO_DRIVER_DIRECTFB)
        return wmi.info.dfb.window;
#elif defined(SDL_VIDEO_DRIVER_COCOA)
        return wmi.info.cocoa.window;
#elif defined(SDL_VIDEO_DRIVER_UIKIT)
        return wmi.info.uikit.window;
#elif defined(SDL_VIDEO_DRIVER_WAYLAND)
        return wmi.info.wl.surface;
#elif defined(SDL_VIDEO_DRIVER_ANDROID)
        return wmi.info.android.window;
#elif defined(SDL_VIDEO_DRIVER_VIVANTE)
        return (void*)(uintptr_t)wmi.info.vivante.window;
#else
        return nullptr;
#endif
    }

    inline NativeDisplay GetNativeDisplayHandle(const SDL_SysWMinfo& wmi) noexcept
    {
        (void)wmi;
#if defined(SDL_VIDEO_DRIVER_WINDOWS)
        return wmi.info.win.hinstance;
#elif defined(SDL_VIDEO_DRIVER_WINRT)
        return nullptr;
#elif defined(SDL_VIDEO_DRIVER_X11)
        return wmi.info.x11.display;
#elif defined(SDL_VIDEO_DRIVER_DIRECTFB)
        return nullptr;
#elif defined(SDL_VIDEO_DRIVER_COCOA)
        return nullptr;
#elif defined(SDL_VIDEO_DRIVER_UIKIT)
        return nullptr;
#elif defined(SDL_VIDEO_DRIVER_WAYLAND)
        return wmi.info.wl.display;
#elif defined(SDL_VIDEO_DRIVER_ANDROID)
        return nullptr;
#elif defined(SDL_VIDEO_DRIVER_VIVANTE)
        return (void*)(uintptr_t)wmi.info.vivante.display;
#else
        return nullptr;
#endif
    }

    WindowImpl::WindowImpl(const String& title, uint32_t width, uint32_t height, WindowFlags flags)
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
        //OnCreated();
    }

    WindowImpl::~WindowImpl()
    {
        Destroy();
    }

    void WindowImpl::Destroy()
    {
        if (_window != nullptr)
        {
            SDL_DestroyWindow(_window);
            _window = nullptr;

            //OnDestroyed();
        }
    }

    void WindowImpl::Resize(uint32_t width, uint32_t height)
    {
        SDL_SetWindowSize(_window, (int)width, (int)height);
    }

    void WindowImpl::Show()
    {
        if (_visible)
            return;

        SDL_ShowWindow(_window);
        _visible = true;
    }

    void WindowImpl::Hide()
    {
        if (_visible)
        {
            SDL_HideWindow(_window);
            _visible = false;
        }
    }

    void WindowImpl::Minimize()
    {
        SDL_MinimizeWindow(_window);
    }

    void WindowImpl::Maximize()
    {
        SDL_MaximizeWindow(_window);
    }

    void WindowImpl::Restore()
    {
        SDL_RestoreWindow(_window);
    }

    void WindowImpl::Close()
    {
        Destroy();
    }

    bool WindowImpl::IsVisible() const
    {
        return _visible;
    }

    bool WindowImpl::IsMinimized() const
    {
        return (SDL_GetWindowFlags(_window) & SDL_WINDOW_MINIMIZED) != 0;
    }

    void WindowImpl::SetTitle(const String& newTitle)
    {
        SDL_SetWindowTitle(_window, newTitle.CString());
    }

    void WindowImpl::SetFullscreen(bool value)
    {
        if (SDL_SetWindowFullscreen(_window, value ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0))
        {
            // TODO: Log error
        }
    }

    bool WindowImpl::IsCursorVisible() const
    {
        return SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE;
    }

    void WindowImpl::SetCursorVisible(bool visible)
    {
        SDL_ShowCursor(visible ? SDL_ENABLE : SDL_DISABLE);
    }

    NativeHandle WindowImpl::GetNativeHandle() const
    {
        SDL_SysWMinfo wmi;
        SDL_VERSION(&wmi.version);
        if (!SDL_GetWindowWMInfo(_window, &wmi))
        {
            return {};
        }

        return GetNativeWindowHandle(wmi);
    }

    NativeDisplay WindowImpl::GetNativeDisplay() const
    {
        SDL_SysWMinfo wmi;
        SDL_VERSION(&wmi.version);
        if (!SDL_GetWindowWMInfo(_window, &wmi))
        {
            return {};
        }

        return GetNativeDisplayHandle(wmi);
    }
}
