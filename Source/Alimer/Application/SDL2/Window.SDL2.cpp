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

#include "Window.SDL2.h"
#include "../Application.h"
#include "../../Core/Log.h"
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_syswm.h>

namespace Alimer
{
    SDL2Window::SDL2Window(const std::string& title, uint32_t width, uint32_t height, bool fullscreen)
	{
        const int x = SDL_WINDOWPOS_CENTERED;
        const int y = SDL_WINDOWPOS_CENTERED;
        uint32_t windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
        if (fullscreen)
        {
            windowFlags |= SDL_WINDOW_FULLSCREEN;
        }

        _window = SDL_CreateWindow(title.c_str(),
            x, y,
            static_cast<int>(width),
            static_cast<int>(height),
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
        _handle.connection = wmInfo.info.win.hinstance;
        _handle.handle = wmInfo.info.win.window;
#endif
	}

    SDL2Window::~SDL2Window()
	{
        Destroy();
	}

	void SDL2Window::Show()
	{
		if (_visible)
			return;

        SDL_ShowWindow(_window);
		_visible = true;
	}

    void SDL2Window::Hide()
    {
        if (_visible)
        {
            SDL_HideWindow(_window);
            _visible = false;
        }
    }

    void SDL2Window::Minimize()
    {
        SDL_MinimizeWindow(_window);
    }

    void SDL2Window::Maximize()
    {
        SDL_MaximizeWindow(_window);
    }

    void SDL2Window::Restore()
    {
        SDL_RestoreWindow(_window);
    }

	void SDL2Window::Close()
	{
        Destroy();
	}

	void SDL2Window::Destroy()
	{
        if (_window != nullptr)
        {
            SDL_DestroyWindow(_window);
            _window = nullptr;
        }
	}

	void SDL2Window::Activate(bool focused)
	{
		if (_focused == focused)
			return;

		_focused = focused;
        SDL_RaiseWindow(_window);
	}

    bool SDL2Window::IsMinimized() const
    {
        return false;
    }

    void SDL2Window::HandleResize(const uvec2& newSize)
    {
        _size = newSize;
        resizeEvent.size = newSize;
        SendEvent(resizeEvent);
    }
}
