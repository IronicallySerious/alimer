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
#include "application.hpp"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#if defined(_WIN32)
#   include <Windows.h>
#endif

#if defined(VGPU_GL)
#   include <glad/glad.h>
#endif

#include "gpu/vgpu.h"

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

static GLFWwindow* _window = nullptr;

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
        _window = glfwCreateWindow(_width, _height, "vortice", 0, 0);

        glfwSetWindowUserPointer(_window, this);
        glfwSetKeyCallback(_window, vortice_glfw_key_callback);

#if defined(VGPU_GL)
        glfwMakeContextCurrent(_window);
        glfwSwapInterval(1);

        gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
#endif
    }

    void Application::platform_shutdown()
    {
        if (_window)
        {
            glfwDestroyWindow(_window);
           _window = nullptr;
        }

        glfwTerminate();

#if defined(_WIN32)
        CoUninitialize();
#endif
    }

    int Application::run_main_loop()
    {
        setup();

        // Setup vgpu using glfw window handle
        VGpuRendererSettings gpuDescriptor = {};
#if defined(_DEBUG)
        gpuDescriptor.validation = false;
#endif

#if defined(__linux__)
        gpuDescriptor.handle.connection = XGetXCBConnection(glfwGetX11Display());
        gpuDescriptor.handle.window = glfwGetX11Window(window);
#elif defined(_WIN32)
        gpuDescriptor.handle.hinstance = ::GetModuleHandleW(NULL);
        gpuDescriptor.handle.hwnd = glfwGetWin32Window(_window);
#endif
        gpuDescriptor.width = _width;
        gpuDescriptor.height = _height;
        gpuDescriptor.swapchain.image_count = 3;
        gpuDescriptor.swapchain.srgb = true;
        gpuDescriptor.swapchain.colorClearValue = { 0.0f, 0.0f, 0.2f, 1.0f };
        gpuDescriptor.swapchain.depthStencilFormat = VGPU_PIXEL_FORMAT_D32_FLOAT;
        vgpuInitialize("vortice", &gpuDescriptor);

        while (!glfwWindowShouldClose(_window))
        {
            if (_input.key_pressed(Key::A))
            {
                vortice_log_write(VORTICE_LOG_LEVEL_INFO, "Pressed 'A'.");
            }

            frame();

#ifdef SOKOL_GLCORE33
            glfwSwapBuffers(_window);
#endif
            glfwPollEvents();
        }

        return 0;
    }

} // namespace vortice

#endif /* VORTICE_GLFW */
