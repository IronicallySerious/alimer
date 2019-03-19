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

#if defined(ALIMER_GLFW)
#include "ApplicationHostGLFW.h"
#include "WindowGLFW.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

// GLFW3 Error Callback, runs on GLFW3 error
static void glfwErrorCallback(int error, const char *description)
{
    //ALIMER_LOGERROR("[GLFW3 Error] Code: %i Decription: %s", error, description);
}

using namespace std;

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

#if ALIMER_PLATFORM_MACOS
        glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, GLFW_FALSE);
#endif
    }

    ApplicationHostGLFW::~ApplicationHostGLFW()
    {
        glfwTerminate();
    }

    int ApplicationHostGLFW::Run()
    {
        InitializeApplication();

        while (!_exitRequested
            && _application->GetMainWindow()->IsOpen())
        {
            _application->Tick();

            // Pool glfw events.
            glfwPollEvents();
        }

        return EXIT_SUCCESS;
    }

    unique_ptr<Window> ApplicationHostGLFW::CreateWindow(const string& title, uint32_t width, uint32_t height, bool resizable, bool fullscreen, bool opengl)
    {
        return make_unique<WindowGLFW>(title, width, height, resizable, fullscreen, opengl);
    }

    ApplicationHost* ApplicationHost::Create(Application* application)
    {
        return new ApplicationHostGLFW(application);
    }
}
#endif
