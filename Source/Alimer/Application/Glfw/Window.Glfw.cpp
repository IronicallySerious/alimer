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

#define GLFW_INCLUDE_NONE 
#include <GLFW/glfw3.h>
#if ALIMER_PLATFORM_WINDOWS
#   define GLFW_EXPOSE_NATIVE_WIN32
#elif ALIMER_PLATFORM_LINUX
#   if !defined(ALIMER_GLFW_WAYLAND)
#       define GLFW_EXPOSE_NATIVE_X11
#   else
#       define GLFW_EXPOSE_NATIVE_WAYLAND
#endif
#elif ALIMER_PLATFORM_MACOS
#   define GLFW_EXPOSE_NATIVE_COCOA
#endif
#include <GLFW/glfw3native.h>

namespace Alimer
{
    static void Glfw_WindowSizeCallback(GLFWwindow* glfwWindow, int width, int height)
    {
        // We also get here in case the window was minimized, so we need to ignore it
        if (width == 0 || height == 0)
        {
            return;
        }

        Window* window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
        window->Resize(width, height);
    }

    static void Glfw_KeyCallback(GLFWwindow* glfwWindow, int key, int scanCode, int action, int modifiers)
    {
        //Window* window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
        if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE)
        {
            glfwSetWindowShouldClose(glfwWindow, GLFW_TRUE);
        }
    }

    static void Glfw_MouseButtonCallback(GLFWwindow* glfwWindow, int button, int action, int)
    {
        //Window* window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
    }

    static void Glfw_MouseMoveCallback(GLFWwindow* glfwWindow, double mouseX, double mouseY)
    {
        //Window* window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
    }

    void Glfw_ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
    {
        //Window* window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
        //ImGuiIO& io = ImGui::GetIO();
        //io.MouseWheelH += (float)xoffset;
        //io.MouseWheel += (float)yoffset;
    }

    void Window::PlatformConstruct()
    {
        const bool resizable = any(_flags & WindowFlags::Resizable);
        bool fullscreen = any(_flags & WindowFlags::Fullscreen);
        GLFWmonitor* monitor = nullptr;
        if (fullscreen)
        {
            monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode *mode = glfwGetVideoMode(monitor);
            _size.x = mode->width;
            _size.y = mode->height;
        }
        else
        {
            glfwWindowHint(GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);
        }

        GLFWwindow* window = glfwCreateWindow(
            static_cast<int>(_size.x),
            static_cast<int>(_size.y),
            _title.CString(),
            monitor,
            nullptr);

        glfwSetWindowUserPointer(window, this);
        glfwSetWindowSizeCallback(window, Glfw_WindowSizeCallback);
        glfwSetKeyCallback(window, Glfw_KeyCallback);
        glfwSetMouseButtonCallback(window, Glfw_MouseButtonCallback);
        glfwSetCursorPosCallback(window, Glfw_MouseMoveCallback);
        glfwSetScrollCallback(window, Glfw_ScrollCallback);
        //glfwSetCharCallback(window, Glfw_CharCallback);
        //glfwSetCursorEnterCallback(window, Glfw_CursorEnterCallback);
        //glfwSetDropCallback(pGLFWWindow, Glfw_DroppedFileCallback);

        _window = window;
    }

    void Window::PlatformDestroy()
    {
        if (_window != nullptr)
        {
            glfwDestroyWindow((GLFWwindow*)_window);
            _window = nullptr;
        }
    }

    void Window::Show()
    {
        if (_visible)
            return;

        glfwShowWindow((GLFWwindow*)_window);
        _visible = true;
    }

    void Window::Hide()
    {
        if (_visible)
        {
            glfwHideWindow((GLFWwindow*)_window);
            _visible = false;
        }
    }

    void Window::Minimize()
    {
        glfwIconifyWindow((GLFWwindow*)_window);
    }

    void Window::Maximize()
    {
        glfwMaximizeWindow((GLFWwindow*)_window);
    }

    void Window::Restore()
    {
        glfwRestoreWindow((GLFWwindow*)_window);
    }

    void Window::Close()
    {
        PlatformDestroy();
    }

    bool Window::IsMinimized() const
    {
        int iconified = glfwGetWindowAttrib((GLFWwindow*)_window, GLFW_ICONIFIED);
        return iconified;
    }

    void Window::SetTitle(const String& newTitle)
    {
        _title = newTitle;
        glfwSetWindowTitle((GLFWwindow*)_window, newTitle.CString());
    }

    void Window::PlatformResize(uint32_t width, uint32_t height)
    {
        glfwSetWindowSize((GLFWwindow*)_window, width, height);
    }

    bool Window::IsOpen() const
    {
        return !glfwWindowShouldClose((GLFWwindow*)_window);
    }

    bool Window::IsCursorVisible() const
    {
        return glfwGetInputMode((GLFWwindow*)_window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL;
    }

    void Window::SetCursorVisible(bool visible)
    {
        glfwSetInputMode((GLFWwindow*)_window, GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
    }

    void* Window::GetNativeHandle() const
    {
#if ALIMER_PLATFORM_WINDOWS
        return glfwGetWin32Window((GLFWwindow*)_window);
#elif ALIMER_PLATFORM_LINUX
#   if !defined(ALIMER_GLFW_WAYLAND)
        return glfwGetX11Window((GLFWwindow*)_window);
#   else
        return glfwGetWaylandWindow((GLFWwindow*)_window);
#endif
#elif ALIMER_PLATFORM_MACOS
        return glfwGetCocoaWindow((GLFWwindow*)_window);
#endif
    }
}
