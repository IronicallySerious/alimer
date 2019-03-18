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

#include "foundation/Platform.h"

#if defined(_WIN32) || ALIMER_PLATFORM_LINUX || ALIMER_PLATFORM_MACOS
#include "engine/Application.h"
#include "WindowGLFW.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace alimer
{
    static void Glfw_FramebufferSizeCallback(GLFWwindow *handle, int width, int height)
    {
        WindowGLFW* window = static_cast<WindowGLFW *>(glfwGetWindowUserPointer(handle));
        //ALIMER_ASSERT(width != 0 && height != 0);
        //window->NotifyResize(width, height);
    }

    static void Glfw_KeyCallback(GLFWwindow *handle, int key, int, int action, int mods)
    {
        if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE)
        {
            Application::Current()->RequestExit();
            //glfwSetWindowShouldClose(handle, GLFW_TRUE);
        }
    }

    WindowGLFW::WindowGLFW(const std::string& title, uint32_t width, uint32_t height, bool resizable, bool fullscreen)
        : Window(title, width, height, resizable, fullscreen)
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
        glfwSetFramebufferSizeCallback(_window, Glfw_FramebufferSizeCallback);
        glfwSetKeyCallback(_window, Glfw_KeyCallback);
        //glfwSetMouseButtonCallback(_window, Glfw_MouseButtonCallback);
        //glfwSetCursorPosCallback(_window, Glfw_MouseMoveCallback);
        //glfwSetScrollCallback(_window, Glfw_ScrollCallback);
        //glfwSetCharCallback(window, Glfw_CharCallback);
        //glfwSetCursorEnterCallback(window, Glfw_CursorEnterCallback);
        //glfwSetDropCallback(_window, Glfw_DropCallback);
    }

    WindowGLFW::~WindowGLFW()
    {
        Close();
    }

    void WindowGLFW::Close()
    {
        glfwSetWindowShouldClose(_window, GLFW_TRUE);
    }

    bool WindowGLFW::IsOpen() const
    {
        return !glfwWindowShouldClose(_window);
    }
}
#endif
