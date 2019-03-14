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
#include "ApplicationHostGLFW.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

// GLFW3 Error Callback, runs on GLFW3 error
static void glfwErrorCallback(int error, const char *description)
{
    //ALIMER_LOGERROR("[GLFW3 Error] Code: %i Decription: %s", error, description);
}

namespace alimer
{
    ApplicationHostGLFW::ApplicationHostGLFW(Application* application)
        : ApplicationHost(application)
    {
        glfwSetErrorCallback(glfwErrorCallback);
        if (!glfwInit())
        {
            //ALIMER_LOGERROR("Failed to initialize GLFW.");
            return;
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

#if ALIMER_PLATFORM_MACOS
        glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, GLFW_FALSE);
#endif
    }

    ApplicationHostGLFW::~ApplicationHostGLFW()
    {
        glfwTerminate();
    }

    void ApplicationHostGLFW::Run()
    {
        //_application->InitializeBeforeRun();

        // WIP
        auto window = glfwCreateWindow(800, 600, "Alimer Window", nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);

        while (!glfwWindowShouldClose(window))
        {
            //processInput(window);
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            {
                glfwSetWindowShouldClose(window, true);
            }

            glfwPollEvents();
        }
    }

    void ApplicationHostGLFW::RequestExit()
    {

    }

    ApplicationHost* ApplicationHost::Create(Application* application)
    {
        return new ApplicationHostGLFW(application);
    }
}
#endif
