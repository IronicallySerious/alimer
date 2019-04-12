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
#   include "core/Log.h"
#   include "engine/Window.h"
#   include "engine/Application.h"
#   include "ApplicationHostGLFW.h"
#   include "graphics/Graphics.h"

#   if defined(_WIN64) || defined(_WIN32)
#       include <Windows.h>
#   endif

#   define GLFW_INCLUDE_NONE
#   include <GLFW/glfw3.h>

// GLFW3 Error Callback, runs on GLFW3 error
static void glfwErrorCallback(int error, const char *description)
{
    ALIMER_LOGERROR("GLFW Error (code %i): %s", error, description);
}

using namespace std;

namespace alimer
{
    ApplicationHostGLFW::ApplicationHostGLFW(Application* application)
        : ApplicationHost(application)
    {
#if defined(_WIN64) || defined(_WIN32)
        if (!AllocConsole())
        {
            throw std::runtime_error{ "AllocConsole error" };
        }

        FILE *fp;
        freopen_s(&fp, "conin$", "r", stdin);
        freopen_s(&fp, "conout$", "w", stdout);
        freopen_s(&fp, "conout$", "w", stderr);

        LPWSTR *szArgList;
        int     argCount;

        szArgList = CommandLineToArgvW(GetCommandLineW(), &argCount);

        // Ignore the first argument containing the application full path
        std::vector<std::wstring> arguments_w(szArgList + 1, szArgList + argCount);

        for (auto &arg_w : arguments_w)
        {
            _arguments.push_back(std::string(arg_w.begin(), arg_w.end()));
        }
#endif

        glfwSetErrorCallback(glfwErrorCallback);
        if (!glfwInit())
        {
            ALIMER_LOGERROR("Failed to initialize GLFW.");
            return;
        }

#if defined(ALIMER_OPENL)
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#else
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#endif

#if ALIMER_PLATFORM_MACOS
        glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, GLFW_FALSE);
#endif
    }

    ApplicationHostGLFW::~ApplicationHostGLFW()
    {
#if defined(_WIN64) || defined(_WIN32)
        FreeConsole();
#endif

        glfwSetErrorCallback(nullptr);
        glfwTerminate();
    }

    int ApplicationHostGLFW::Run()
    {
        InitializeApplication();

        while (!_exitRequested
            && _application->GetGraphics()->GetRenderWindow()->IsOpen())
        {
            _application->Tick();

            // Pool glfw events.
            glfwPollEvents();
        }

        return EXIT_SUCCESS;
    }

    ApplicationHost* CreatePlatformHost(Application* application)
    {
        return new ApplicationHostGLFW(application);
    }
}
#endif
