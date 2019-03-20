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
#   include "WindowGLFW.h"
#   define GLFW_INCLUDE_NONE
#   include <GLFW/glfw3.h>

namespace alimer
{
    static void Glfw_WindowCloseCallback(GLFWwindow *window)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

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
            Application::Current()->requestExit();
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

        window = glfwCreateWindow(
            windowWidth,
            windowHeight,
            title.c_str(),
            monitor,
            nullptr);

        if (!fullscreen)
        {
            // Center on screen.
            monitor = glfwGetPrimaryMonitor();
            auto videoMode = glfwGetVideoMode(monitor);

            int windowWidth = 0;
            int windowHeight = 0;
            glfwGetWindowSize(window, &windowWidth, &windowHeight);

            int windowX = videoMode->width / 2 - windowWidth / 2;
            int windowY = videoMode->height / 2 - windowHeight / 2;
            glfwSetWindowPos(window, windowX, windowY);
        }

#if defined(ALIMER_OPENL)
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);
#endif

        glfwSetWindowUserPointer(window, this);
        glfwSetWindowCloseCallback(window, Glfw_WindowCloseCallback);
        glfwSetFramebufferSizeCallback(window, Glfw_FramebufferSizeCallback);
        glfwSetKeyCallback(window, Glfw_KeyCallback);
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
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    bool WindowGLFW::IsOpen() const
    {
        return !glfwWindowShouldClose(window);
    }

    void WindowGLFW::SwapBuffers()
    {
#if defined(ALIMER_OPENL)
        glfwSwapBuffers(_window);
#endif
    }
}
#endif
