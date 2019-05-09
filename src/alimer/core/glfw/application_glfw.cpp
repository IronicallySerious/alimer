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

//#include "core/log.h"
#include "application_glfw.h"

#if defined(__linux__)
#   define GLFW_EXPOSE_NATIVE_X11
#elif defined(_WIN32)
#   define GLFW_EXPOSE_NATIVE_WIN32
#endif
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <vgpu.h>

#if defined(VGPU_GL) || defined(VGPU_GLES)
#   include <glad.h>
#endif

static inline void alimer_glfw_error(int code, const char* description) {
    //alimer_throw(description);
}

namespace alimer
{
    ApplicationGlfw::ApplicationGlfw()
    {
        // Initialize glfw now.
        glfwSetErrorCallback(alimer_glfw_error);
    }

    ApplicationGlfw::~ApplicationGlfw()
    {
        vgpu_shutdown();
        glfwTerminate();
    }

    bool ApplicationGlfw::init(uint32_t width, uint32_t height, bool fullscreen)
    {
        if (!glfwInit())
        {
            //vortice_log_write(VORTICE_LOG_LEVEL_ERROR, "Failed to initialize GLFW.");
            return false;
        }

        const bool opengl = vgpu_get_backend() == VGPU_BACKEND_OPENGL;

        if (opengl)
        {
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        }
        else
        {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        }

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
            const bool resizable = false;
            glfwWindowHint(GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);
        }

        _window = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), "Alimer Window", monitor, nullptr);
        glfwSetWindowUserPointer(_window, this);
        //glfwSetWindowSizeCallback(window, Glfw_Resize);
        //glfwSetKeyCallback(_window, vortice_glfw_key_callback);
        //glfwSetMouseButtonCallback(window, Glfw_MouseButtonCallback);
        //glfwSetCursorPosCallback(window, Glfw_MouseMoveCallback);
        //glfwSetScrollCallback(window, Glfw_ScrollCallback);
        //glfwSetCharCallback(window, Glfw_CharCallback);
        //glfwSetCursorEnterCallback(window, Glfw_CursorEnterCallback);
        //glfwSetDropCallback(_window, Glfw_DropCallback);

#if defined(VGPU_GL) || defined(VGPU_GLES)
        glfwMakeContextCurrent(_window);
        glfwSwapInterval(1);

        gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
#endif

        // Setup vgpu.
        VGpuRendererSettings gpuDescriptor = {};
#if defined(_DEBUG)
        gpuDescriptor.validation = false;
#endif
#if defined(__linux__)
        gpuDescriptor.handle.connection = XGetXCBConnection(glfwGetX11Display());
        gpuDescriptor.handle.window = glfwGetX11Window(_window);
#elif defined(_WIN32)
        gpuDescriptor.handle.hinstance = ::GetModuleHandleW(NULL);
        gpuDescriptor.handle.hwnd = glfwGetWin32Window(_window);
#endif
        gpuDescriptor.width = _width;
        gpuDescriptor.height = _height;
        gpuDescriptor.swapchain.imageCount = 3;
        gpuDescriptor.swapchain.srgb = true;
        gpuDescriptor.swapchain.colorClearValue = { 0.0f, 0.0f, 0.2f, 1.0f };
        gpuDescriptor.swapchain.depthStencilFormat = VGPU_PIXEL_FORMAT_D32_FLOAT;
        gpuDescriptor.swapchain.sampleCount = VGPU_SAMPLE_COUNT1;
        vgpu_initialize("vortice", &gpuDescriptor);

        return true;
    }

    int ApplicationGlfw::run()
    {
        int exit_code = 0;

#if defined(ALIMER_TOOLS)
        bool compile = false;
#endif

        if (!init(1280, 720, false)) {
            return 1;
        }

        while (!glfwWindowShouldClose(_window))
        {
#if defined(VGPU_GL) || defined(VGPU_GLES)
            glfwSwapBuffers(_window);
#endif
            glfwPollEvents();
        }

        return exit_code;
    }
}
