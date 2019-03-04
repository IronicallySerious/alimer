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

#include "../Graphics/Window.h"

#if defined(ALIMER_GLFW)
#   define GLFW_INCLUDE_NONE 
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
#   include <GLFW/glfw3.h>
#   include <GLFW/glfw3native.h>

static void Glfw_Resize(GLFWwindow* handle, int w, int h) {
    auto window = reinterpret_cast<alimer::Window*>(glfwGetWindowUserPointer(handle));
    window->Resize(w, h);
}

static void Glfw_KeyCallback(GLFWwindow* handle, int key, int scanCode, int action, int modifiers)
{
    //auto window = static_cast<alimer::Window*>(glfwGetWindowUserPointer(handle));
    if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE)
    {
        glfwSetWindowShouldClose(handle, GLFW_TRUE);
    }
}

static void Glfw_MouseButtonCallback(GLFWwindow* handle, int button, int action, int)
{
    auto window = static_cast<alimer::Window*>(glfwGetWindowUserPointer(handle));
}

static void Glfw_MouseMoveCallback(GLFWwindow* handle, double mouseX, double mouseY)
{
    //auto window = static_cast<alimer::Window*>(glfwGetWindowUserPointer(handle));
}
static void Glfw_ScrollCallback(GLFWwindow* handle, double xoffset, double yoffset) {
    //auto window = static_cast<alimer::Window*>(glfwGetWindowUserPointer(handle));
    //ImGuiIO& io = ImGui::GetIO();
    //io.MouseWheelH += (float)xoffset;
    //io.MouseWheel += (float)yoffset;
}

static void Glfw_DropCallback(GLFWwindow* window, int count, const char** paths) {
    // auto window = static_cast<alimer::Window*>(glfwGetWindowUserPointer(handle));
}

using namespace std;

namespace alimer
{
   
    Window::Window(const string& title, uint32_t width, uint32_t height, bool resizable, bool fullscreen)
        : _title(title)
        , _size(width, height)
        , _fullscreen(fullscreen)
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
            windowWidth = static_cast<int>(width);
            windowHeight = static_cast<int>(height);
            glfwWindowHint(GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);
        }

        _window = glfwCreateWindow(
            windowWidth,
            windowHeight,
            title.c_str(),
            monitor,
            nullptr);

        glfwSetWindowUserPointer(_window, this);
        glfwSetWindowSizeCallback(_window, Glfw_Resize);
        glfwSetKeyCallback(_window, Glfw_KeyCallback);
        glfwSetMouseButtonCallback(_window, Glfw_MouseButtonCallback);
        glfwSetCursorPosCallback(_window, Glfw_MouseMoveCallback);
        glfwSetScrollCallback(_window, Glfw_ScrollCallback);
        //glfwSetCharCallback(window, Glfw_CharCallback);
        //glfwSetCursorEnterCallback(window, Glfw_CursorEnterCallback);
        glfwSetDropCallback(_window, Glfw_DropCallback);
    }

    Window::~Window()
    {
        if (_window != nullptr)
        {
            glfwDestroyWindow(_window);
            _window = nullptr;

            OnHandleDestroyed();
        }
    }

    void Window::Show()
    {
        if (_visible)
            return;

        glfwShowWindow(_window);
        _visible = true;
    }

    void Window::Hide()
    {
        if (_visible)
        {
            glfwHideWindow(_window);
            _visible = false;
        }
    }

    void Window::Minimize()
    {
        glfwIconifyWindow(_window);
    }

    void Window::Maximize()
    {
        glfwIconifyWindow(_window);
    }

    void Window::Restore()
    {
        glfwRestoreWindow(_window);
    }

    void Window::Close()
    {
        glfwSetWindowShouldClose(_window, GLFW_TRUE);
    }

    void Window::SetTitle(const std::string& newTitle)
    {
        _title = newTitle;
        glfwSetWindowTitle(_window, newTitle.c_str());
    }

    void Window::SetFullscreen(bool value)
    {
    }

    bool Window::IsVisible() const
    {
        return _visible;
    }

    bool Window::IsMinimized() const
    {
        int iconified = glfwGetWindowAttrib(_window, GLFW_ICONIFIED);
        return iconified;
    }

    bool Window::IsFullscreen() const
    {
        return _fullscreen;
    }

    bool Window::IsOpen() const
    {
        return !glfwWindowShouldClose(_window);
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
        glfwSetWindowSize(_window, (int)width, (int)height);
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
        glfwSetInputMode(_window, GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
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

    void Window::SwapBuffers()
    {
        glfwSwapBuffers(_window);
    }
}
#elif defined(ALIMER_SDL2)
#include "SDL2/WindowSDL2.h"

namespace alimer
{

}

#endif
