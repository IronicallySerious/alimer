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

#include "glfwWindow.h"
#include "../Application.h"
#include "../../Core/Log.h"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#if ALIMER_PLATFORM_LINUX || ALIMER_PLATFORM
#   define GLFW_EXPOSE_NATIVE_X11
#	define GLFW_EXPOSE_NATIVE_GLX
#elif ALIMER_PLATFORM_APPLE_OSX
#	define GLFW_EXPOSE_NATIVE_COCOA
#	define GLFW_EXPOSE_NATIVE_NSGL
#elif ALIMER_PLATFORM_WINDOWS
#	define GLFW_EXPOSE_NATIVE_WIN32
#	define GLFW_EXPOSE_NATIVE_WGL
#endif

#include "GLFW/glfw3native.h"

namespace Alimer
{
    glfwWindow::glfwWindow(const std::string& title, uint32_t width, uint32_t height, bool fullscreen)
	{
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        glfwWindowHint(GLFW_VISIBLE, 1);
        _window = glfwCreateWindow(width, height, title.c_str(), nullptr, 0);
        glfwSetWindowUserPointer(_window, this);

#if ALIMER_PLATFORM_LINUX || ALIMER_PLATFORM
        _handle.connection = glfwGetX11Display();
        _handle.handle = (void*)(uintptr_t)glfwGetX11Window(_window);
#elif ALIMER_PLATFORM_APPLE_OSX
        _handle.connection = nullptr;
        _handle.handle = glfwGetCocoaWindow(_window);
#elif ALIMER_PLATFORM_WINDOWS
        _handle.connection = GetModuleHandleW(NULL);
        _handle.handle = glfwGetWin32Window(_window);
#endif
	}

    glfwWindow::~glfwWindow()
	{
	}

	void glfwWindow::Show()
	{
		if (_visible)
			return;

        glfwShowWindow(_window);
		_visible = true;
	}

    void glfwWindow::Hide()
    {
        if (_visible)
        {
            glfwHideWindow(_window);
            _visible = false;
        }
    }

    void glfwWindow::Minimize()
    {
        glfwIconifyWindow(_window);
    }

    void glfwWindow::Maximize()
    {
        glfwMaximizeWindow(_window);
    }

    void glfwWindow::Restore()
    {
        glfwRestoreWindow(_window);
    }

	void glfwWindow::Close()
	{
        glfwWindowShouldClose(_window);
	}

	void glfwWindow::Destroy()
	{
        glfwDestroyWindow(_window);
	}

	void glfwWindow::Activate(bool focused)
	{
		if (_focused == focused)
			return;

		_focused = focused;
        glfwFocusWindow(_window);
	}

    bool glfwWindow::IsMinimized() const
    {
        return false;
    }

    bool glfwWindow::ShouldClose() const
    {
        return glfwWindowShouldClose(_window);
    }

    void glfwWindow::HandleResize(const uvec2& newSize)
    {
        _size = newSize;
        resizeEvent.size = newSize;
        SendEvent(resizeEvent);
    }
}
