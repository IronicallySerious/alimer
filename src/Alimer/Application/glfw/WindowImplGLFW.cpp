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

#include "WindowImplGLFW.h"
#include "../Application.h"
#include "Core/Log.h"

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

namespace alimer
{
    inline uint64_t GetNativeWindowHandle(GLFWwindow* window) noexcept
    {
        ALIMER_UNUSED(window);
#if defined(GLFW_EXPOSE_NATIVE_WIN32)
        return (uint64_t)glfwGetWin32Window(window);
#elif defined(GLFW_EXPOSE_NATIVE_X11)
        return (uint64_t)glfwGetX11Window((window);
#elif defined(GLFW_EXPOSE_NATIVE_WAYLAND)
        return (uint64_t)glfwGetWaylandWindow((window);
#elif defined(GLFW_EXPOSE_NATIVE_COCOA)
        return (uint64_t)glfwGetCocoaWindow(window);
#else
        return 0;
#endif
    }

    inline uint64_t GetNativeDisplayHandle(GLFWwindow* window) noexcept
    {
        ALIMER_UNUSED(window);
#if defined(GLFW_EXPOSE_NATIVE_WIN32)
        return (uint64_t)GetModuleHandleW(nullptr);
#elif defined(GLFW_EXPOSE_NATIVE_X11)
        return (uint64_t)glfwGetX11Display(window);
#elif defined(GLFW_EXPOSE_NATIVE_WAYLAND)
        return (uint64_t)glfwGetWaylandDisplay(window);
#elif defined(GLFW_EXPOSE_NATIVE_COCOA)
        return 0;
#else
        return 0;
#endif
    }

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

    WindowImpl::WindowImpl(const std::string& title, uint32_t width, uint32_t height, WindowFlags flags)
    {
        const bool resizable = any(flags & WindowFlags::Resizable);
        bool fullscreen = any(flags & WindowFlags::Fullscreen);
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
        glfwSetWindowSizeCallback(_window, Glfw_WindowSizeCallback);
        glfwSetKeyCallback(_window, Glfw_KeyCallback);
        glfwSetMouseButtonCallback(_window, Glfw_MouseButtonCallback);
        glfwSetCursorPosCallback(_window, Glfw_MouseMoveCallback);
        glfwSetScrollCallback(_window, Glfw_ScrollCallback);
        //glfwSetCharCallback(window, Glfw_CharCallback);
        //glfwSetCursorEnterCallback(window, Glfw_CursorEnterCallback);
        //glfwSetDropCallback(pGLFWWindow, Glfw_DroppedFileCallback);

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
            glfwDestroyWindow(_window);
            _window = nullptr;

            //OnDestroyed();
        }
    }

    void WindowImpl::Resize(uint32_t width, uint32_t height)
    {
        glfwSetWindowSize(_window, (int)width, (int)height);
    }

    void WindowImpl::Show()
    {
        if (_visible)
            return;

        glfwShowWindow(_window);
        _visible = true;
    }

    void WindowImpl::Hide()
    {
        if (_visible)
        {
            glfwHideWindow(_window);
            _visible = false;
        }
    }

    void WindowImpl::Minimize()
    {
        glfwIconifyWindow(_window);
    }

    void WindowImpl::Maximize()
    {
        glfwIconifyWindow(_window);
    }

    void WindowImpl::Restore()
    {
        glfwRestoreWindow(_window);
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
        int iconified = glfwGetWindowAttrib(_window, GLFW_ICONIFIED);
        return iconified;
    }

    bool WindowImpl::IsOpen() const
    {
        return !glfwWindowShouldClose(_window);
    }

    void WindowImpl::SetTitle(const std::string& newTitle)
    {
        glfwSetWindowTitle(_window, newTitle.c_str());
    }

    void WindowImpl::SetFullscreen(bool value)
    {
       
    }

    bool WindowImpl::IsCursorVisible() const
    {
        return glfwGetInputMode(_window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL;
    }

    void WindowImpl::SetCursorVisible(bool visible)
    {
        glfwSetInputMode(_window, GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
    }

    uint64_t WindowImpl::GetNativeHandle() const
    {
        return GetNativeWindowHandle(_window);
    }

    uint64_t WindowImpl::GetNativeDisplay() const
    {
        return GetNativeDisplayHandle(_window);
    }
}
