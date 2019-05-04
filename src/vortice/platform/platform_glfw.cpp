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

#if defined(VORTICE_GLFW)
#include "core/log.h"
#include "core/version.h"
#include "window.hpp"
#include "application.hpp"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#if defined(_WIN32)
#   include <Objbase.h>
#endif

#if defined(VGPU_GL)
#   include <glad/glad.h>
#endif

#if defined(__linux__)
#   define GLFW_EXPOSE_NATIVE_X11
#elif defined(_WIN32)
#   define GLFW_EXPOSE_NATIVE_WIN32
#endif
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

static void vortice_glfw_error(int code, const char* description) {
    //alimer_throw(description);
}

namespace vortice
{
    static Key vortice_glfw_convert_key(int key)
    {
#define k(glfw, vortice) case GLFW_KEY_##glfw: return Key::vortice
        switch (key)
        {
            k(A, A);
            k(B, B);
            k(C, C);
            k(D, D);
            k(E, E);
            k(F, F);
            k(G, G);
            k(H, H);
            k(I, I);
            k(J, J);
            k(K, K);
            k(L, L);
            k(M, M);
            k(N, N);
            k(O, O);
            k(P, P);
            k(Q, Q);
            k(R, R);
            k(S, S);
            k(T, T);
            k(U, U);
            k(V, V);
            k(W, W);
            k(X, X);
            k(Y, Y);
            k(Z, Z);
            k(LEFT_CONTROL, LeftCtrl);
            k(LEFT_ALT, LeftAlt);
            k(LEFT_SHIFT, LeftShift);
            k(ENTER, Return);
            k(SPACE, Space);
            k(ESCAPE, Escape);
            k(LEFT, Left);
            k(RIGHT, Right);
            k(UP, Up);
            k(DOWN, Down);
        default:
            return Key::Unknown;
        }
#undef k
    }

    struct CachedWindowState
    {
        int x = 0, y = 0, width = 0, height = 0;
    } cached_window;

    static void vortice_glfw_key_callback(GLFWwindow *window, int key, int, int action, int mods)
    {
        vortice::KeyState state;
        switch (action)
        {
        case GLFW_PRESS:
            state = KeyState::Pressed;
            break;

        default:
        case GLFW_RELEASE:
            state = KeyState::Released;
            break;

        case GLFW_REPEAT:
            state = KeyState::Repeat;
            break;
        }

        Key vorticeKey = vortice_glfw_convert_key(key);
        Application *app = static_cast<Application*>(glfwGetWindowUserPointer(window));

        if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE)
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        else if (action == GLFW_PRESS && key == GLFW_KEY_ENTER && mods == GLFW_MOD_ALT)
        {
            if (glfwGetWindowMonitor(window))
            {
                glfwSetWindowMonitor(window, nullptr, cached_window.x, cached_window.y, cached_window.width, cached_window.height, 0);
            }
            else
            {
                GLFWmonitor *primary = glfwGetPrimaryMonitor();
                if (primary)
                {
                    const GLFWvidmode *mode = glfwGetVideoMode(primary);
                    glfwGetWindowPos(window, &cached_window.x, &cached_window.y);
                    glfwGetWindowSize(window, &cached_window.width, &cached_window.height);
                    glfwSetWindowMonitor(window, primary, 0, 0, mode->width, mode->height, mode->refreshRate);
                }
            }
        }
        else
        {
            app->get_input().key_event(vorticeKey, state);
        }
    }

    void Application::platform_construct()
    {
#if defined(_WIN32)
        if (CoInitializeEx(NULL, COINIT_APARTMENTTHREADED) == RPC_E_CHANGED_MODE) {
            CoInitializeEx(NULL, COINIT_MULTITHREADED);
        }

#   if defined(_DEBUG)
        AllocConsole();

        freopen("conin$", "r", stdin);
        freopen("conout$", "w", stdout);
        freopen("conout$", "w", stderr);
#   endif /* defined(_DEBUG) */
#endif

        // Initialize glfw now.
        glfwSetErrorCallback(vortice_glfw_error);
        if (!glfwInit())
        {
            vortice_log_write(VORTICE_LOG_LEVEL_ERROR, "Failed to initialize GLFW.");
            return;
        }

#if defined(VGPU_GL) 
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#else
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#endif


#if defined(VGPU_GL) 
        glfwMakeContextCurrent(_window);
        glfwSwapInterval(1);

        gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
#endif
    }

    void Application::platform_shutdown()
    {
        _window.close();

        glfwTerminate();

#if defined(_WIN32)
        CoUninitialize();
#endif
    }

    int Application::run_main_loop()
    {
        setup();

        while (_window.is_open())
        {
            if (_input.key_pressed(Key::A))
            {
                vortice_log_write(VORTICE_LOG_LEVEL_INFO, "Pressed 'A'.");
            }

            frame();

#if defined(VGPU_GL) 
            glfwSwapBuffers(_window);
#endif
            glfwPollEvents();
        }

        return 0;
    }

    /* Window implementation */
    void Window::define(const char* title, int width, int height, bool resizable, bool fullscreen)
    {
        GLFWmonitor* monitor = nullptr;
        if (fullscreen)
        {
            monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode *mode = glfwGetVideoMode(monitor);
            width = mode->width;
            height = mode->height;
        }
        else
        {
            glfwWindowHint(GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);
        }

        if (_impl == nullptr)
        {
            GLFWwindow* window = glfwCreateWindow(width, height, title, monitor, nullptr);

            glfwSetWindowUserPointer(window, this);
            //glfwSetWindowSizeCallback(window, Glfw_Resize);
            glfwSetKeyCallback(window, vortice_glfw_key_callback);
            //glfwSetMouseButtonCallback(window, Glfw_MouseButtonCallback);
            //glfwSetCursorPosCallback(window, Glfw_MouseMoveCallback);
            //glfwSetScrollCallback(window, Glfw_ScrollCallback);
            //glfwSetCharCallback(window, Glfw_CharCallback);
            //glfwSetCursorEnterCallback(window, Glfw_CursorEnterCallback);
            //glfwSetDropCallback(_window, Glfw_DropCallback);

            _impl = window;
        }
        else
        {
            glfwSetWindowTitle((GLFWwindow*)_impl, title);
            glfwSetWindowSize((GLFWwindow*)_impl, width, height);
        }

        _width = width;
        _height = height;
    }

    void Window::close()
    {
        if (_impl != nullptr)
        {
            glfwDestroyWindow((GLFWwindow*)_impl);
            _impl = nullptr;
        }
    }

    void Window::show()
    {
        if (_impl != nullptr)
        {
            glfwShowWindow((GLFWwindow*)_impl);
        }
    }

    void Window::hide()
    {
        if (_impl != nullptr)
        {
            glfwHideWindow((GLFWwindow*)_impl);
        }
    }

    void Window::minimize()
    {
        if (_impl != nullptr)
        {
            glfwIconifyWindow((GLFWwindow*)_impl);
        }
    }

    void Window::maximize()
    {
        if (_impl != nullptr)
        {
            glfwMaximizeWindow((GLFWwindow*)_impl);
        }
    }

    void Window::restore()
    {
        if (_impl != nullptr)
        {
            glfwRestoreWindow((GLFWwindow*)_impl);
        }
    }

    void Window::set_title(const char* title)
    {
        if (_impl != nullptr)
        {
            glfwSetWindowTitle((GLFWwindow*)_impl, title);
        }
    }

    bool Window::is_open() const
    {
        return _impl != nullptr && !glfwWindowShouldClose((GLFWwindow*)_impl);
    }

#if defined(_WIN32)
    HWND Window::handle() const
    {
        if (_impl == nullptr) {
            return nullptr;
        }

        return glfwGetWin32Window((GLFWwindow*)_impl);
    }
#elif defined(__linux__)
    Window Window::handle() const
    {
        if (_impl == nullptr) {
            return nullptr;
        }

        return glfwGetX11Window((GLFWwindow*)_impl);
    }
#endif

} // namespace vortice

#endif /* VORTICE_GLFW */
