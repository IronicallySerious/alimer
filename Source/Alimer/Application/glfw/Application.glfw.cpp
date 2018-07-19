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

#include "../Application.h"
#include "glfwWindow.h"
#include "../../Core/Log.h"
#include "../../Input/glfw/glfwInput.h"
#include "../../Audio/WASAPI/AudioWASAPI.h"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

using namespace std;

namespace Alimer
{
    // GLFW3 Error Callback, runs on GLFW3 error
    static void ErrorCallback(int error, const char *description)
    {
        ALIMER_LOGWARN("[GLFW3 Error] Code: {} Decription: {}", error, description);
    }

#if ALIMER_PLATFORM_WINDOWS
    bool Win32PlatformInitialize()
    {
        HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
        if (hr == RPC_E_CHANGED_MODE)
            hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

        if (FAILED(hr))
        {
            ALIMER_LOGDEBUG("Failed to initialize COM, error: {}", hr);
            return false;
        }

        // Enable high DPI as SDL not support.
        /*if (HMODULE shCoreLibrary = ::LoadLibraryW(L"Shcore.dll"))
        {
            if (auto fn = (decltype(&SetProcessDpiAwareness))GetProcAddress(shCoreLibrary, "SetProcessDpiAwareness"))
            {
                fn(PROCESS_PER_MONITOR_DPI_AWARE);
            }

            FreeLibrary(shCoreLibrary);
        }
        else
        {
            if (HMODULE user32Library = ::LoadLibraryW(L"user32.dll"))
            {
                if (auto fn = (decltype(&SetProcessDPIAware))GetProcAddress(user32Library, "SetProcessDPIAware"))
                    fn();

                FreeLibrary(user32Library);
            }
        }*/

        static std::string osDescription;
        if (osDescription.empty())
        {
            osDescription = "Microsoft Windows";
            HMODULE ntdll = LoadLibraryW(L"ntdll.dll");
            if (ntdll)
            {
                typedef int (WINAPI * RtlGetVersion_FUNC) (OSVERSIONINFOEXW *);
                if (auto RtlGetVersion = (RtlGetVersion_FUNC)GetProcAddress(ntdll, "RtlGetVersion"))
                {
                    OSVERSIONINFOEX osvi = {};
                    osvi.dwOSVersionInfoSize = sizeof(osvi);

                    if (RtlGetVersion(&osvi) == 0)
                    {
                        //osDescription = osDescription + ""
                    }
                }

                FreeLibrary(ntdll);
            }
        }

        return true;
    }
#endif

    void Application::PlatformConstruct()
    {
#if ALIMER_PLATFORM_WINDOWS
        int argc;
        LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

        if (argc)
        {
            // Skip first one as its executable path.
            char temporaryCString[256];
            for (int i = 1; i < argc; i++)
            {
                WideCharToMultiByte(CP_UTF8, 0, argv[i], -1, temporaryCString, sizeof(temporaryCString), nullptr, nullptr);

                _args.push_back(temporaryCString);
            }

            LocalFree(argv);
        }
#endif // ALIMER_PLATFORM_WINDOWS
    }

    WindowPtr Application::MakeWindow(const std::string& title, uint32_t width, uint32_t height, bool fullscreen)
    {
        return MakeShared<glfwWindow>(title, width, height, fullscreen);
    }

    unique_ptr<Input> Application::CreateInput()
    {
        return make_unique<glfwInput>();
    }

    unique_ptr<Audio> Application::CreateAudio()
    {
        return make_unique<AudioWASAPI>();
    }

    int Application::Run()
    {
        glfwSetErrorCallback(ErrorCallback);

        if (!glfwInit())
        {
            ALIMER_LOGWARN("Failed to initialize GLFW");
            return EXIT_FAILURE;
        }

#if ALIMER_PLATFORM_WINDOWS
        if (!Win32PlatformInitialize())
        {
            ALIMER_LOGERROR("[Win32] - Failed to setup");
            return EXIT_FAILURE;
        }
#endif

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        if (!InitializeBeforeRun())
        {
            return EXIT_FAILURE;
        }

        glfwWindow* glfwMainWindow = static_cast<glfwWindow*>(_window.Get());
        while (_running
            && !glfwMainWindow->ShouldClose())
        {
            glfwPollEvents();

            // Tick handles pause state.
            RunFrame();
        }

        OnExiting();
        glfwTerminate();
        return EXIT_SUCCESS;
    }
}

