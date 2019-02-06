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

#include "../Application.h"
#include "WindowImplGLFW.h"
#define GLFW_INCLUDE_NONE 
#include <GLFW/glfw3.h>

namespace alimer
{
    // GLFW3 Error Callback, runs on GLFW3 error
    static void glfwErrorCallback(int error, const char *description)
    {
        ALIMER_LOGERROR("[GLFW3 Error] Code: {} Decription: {}", error, description);
    }

    void Application::PlatformConstruct()
    {
        glfwSetErrorCallback(glfwErrorCallback);

        if (!glfwInit())
        {
            ALIMER_LOGERROR("Failed to initialize GLFW");
            return;
        }

#if ALIMER_PLATFORM_MACOS
        glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, GLFW_FALSE);
#endif
    }

    void Application::PlatformRun()
    {
        if (_engine->GetSettings().preferredGraphicsBackend != GraphicsBackend::OpenGL)
        {
            // By default on non opengl context creation.
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        }
        else
        {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
        }

        if (!InitializeBeforeRun())
        {
            _exitCode = EXIT_FAILURE;
            return;
        }

        while (_engine->GetWindow()->IsOpen())
        {
            if (!_paused)
            {
                glfwPollEvents();
            }
            else
            {
                glfwWaitEvents();
            }

            // Tick handles pause state.
            RunFrame();
        }

        OnExiting();
        Shutdown();
        // quit all subsystems and quit application.
        glfwTerminate();
    }
}
