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

#include "alimer_config.h"
#if defined(ALIMER_GLFW)
#   include "engine/Application.h"
#   include "engine/Window.h"
#   if defined(_WIN32)
#      define GLFW_EXPOSE_NATIVE_WIN32
#   elif defined(__linux__) || defined(__linux)
#      if !defined(ALIMER_GLFW_WAYLAND)
#          define GLFW_EXPOSE_NATIVE_X11
#      else
#          define GLFW_EXPOSE_NATIVE_WAYLAND
#   endif
#   elif defined(__APPLE__) 
#      define GLFW_EXPOSE_NATIVE_COCOA
#   endif
#   define GLFW_INCLUDE_NONE
#   include <GLFW/glfw3.h>
#   include <GLFW/glfw3native.h>

namespace alimer
{
    static void Glfw_WindowCloseCallback(GLFWwindow *window)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    static void Glfw_FramebufferSizeCallback(GLFWwindow *handle, int width, int height)
    {
        Window* window = static_cast<Window*>(glfwGetWindowUserPointer(handle));
        //ALIMER_ASSERT(width != 0 && height != 0);
        //window->NotifyResize(width, height);
    }

    static void Glfw_KeyCallback(GLFWwindow *handle, int key, int, int action, int mods)
    {
        if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE)
        {
            Application::GetCurrent()->RequestExit();
            //glfwSetWindowShouldClose(handle, GLFW_TRUE);
        }
    }

    Window::Window()
        : _title("Alimer")
        , _size(math::uint2::zero())
        , _resizable(false)
        , _fullscreen(false)
    {

    }

    Window::Window(const std::string& title, const math::uint2& size, bool resizable, bool fullscreen)
        : _title(title)
    {
        SetSize(size, resizable, fullscreen);
    }

    Window::~Window()
    {
        Close();
    }

    bool Window::SetSize(const math::uint2& size, bool resizable, bool fullscreen)
    {
        if (!_window)
        {
            GLFWmonitor* monitor = nullptr;
            int windowWidth;
            int windowHeight;
            if (fullscreen)
            {
                monitor = glfwGetPrimaryMonitor();
                const GLFWvidmode *mode = glfwGetVideoMode(monitor);
                windowWidth = mode->width;
                windowHeight = mode->height;
            }
            else
            {
                windowWidth = static_cast<int>(size.x);
                windowHeight = static_cast<int>(size.y);
                glfwWindowHint(GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);
            }

            _window = glfwCreateWindow(
                windowWidth,
                windowHeight,
                _title.c_str(),
                monitor,
                nullptr);

            if (!fullscreen)
            {
                // Center on screen.
                monitor = glfwGetPrimaryMonitor();
                auto videoMode = glfwGetVideoMode(monitor);

                int windowWidth = 0;
                int windowHeight = 0;
                glfwGetWindowSize(_window, &windowWidth, &windowHeight);

                int windowX = videoMode->width / 2 - windowWidth / 2;
                int windowY = videoMode->height / 2 - windowHeight / 2;
                glfwSetWindowPos(_window, windowX, windowY);
            }

#if defined(ALIMER_OPENL)
            glfwMakeContextCurrent(_window);
            glfwSwapInterval(1);
#endif

            glfwSetWindowUserPointer(_window, this);
            glfwSetWindowCloseCallback(_window, Glfw_WindowCloseCallback);
            glfwSetFramebufferSizeCallback(_window, Glfw_FramebufferSizeCallback);
            glfwSetKeyCallback(_window, Glfw_KeyCallback);
            //glfwSetMouseButtonCallback(_window, Glfw_MouseButtonCallback);
            //glfwSetCursorPosCallback(_window, Glfw_MouseMoveCallback);
            //glfwSetScrollCallback(_window, Glfw_ScrollCallback);
            //glfwSetCharCallback(window, Glfw_CharCallback);
            //glfwSetCursorEnterCallback(window, Glfw_CursorEnterCallback);
            //glfwSetDropCallback(_window, Glfw_DropCallback);
        }
        else
        {
        }

        _size = size;
        _resizable = resizable;
        _fullscreen = fullscreen;
        return true;
    }

    void Window::Close()
    {
        glfwSetWindowShouldClose(_window, GLFW_TRUE);
    }

    bool Window::IsOpen() const
    {
        return !glfwWindowShouldClose(_window);
    }

    WindowHandle Window::GetNativeHandle() const
    {
#if defined(GLFW_EXPOSE_NATIVE_WIN32)
        return glfwGetWin32Window(_window);
#elif defined(GLFW_EXPOSE_NATIVE_X11)
        return glfwGetX11Window((_window);
#elif defined(GLFW_EXPOSE_NATIVE_WAYLAND)
        return glfwGetWaylandWindow(_window);
#elif defined(GLFW_EXPOSE_NATIVE_COCOA)
        return glfwGetCocoaWindow(_window);
#else
        return 0;
#endif
    }

    WindowConnection Window::GetNativeConnection() const
    {
#if defined(GLFW_EXPOSE_NATIVE_WIN32)
        return GetModuleHandleW(nullptr);
#elif defined(GLFW_EXPOSE_NATIVE_X11)
        return glfwGetX11Display();
#elif defined(GLFW_EXPOSE_NATIVE_WAYLAND)
        return glfwGetWaylandDisplay();
#else
        return 0;
#endif
    }
}
#endif
