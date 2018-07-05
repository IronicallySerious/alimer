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
#include "WindowWindows.h"
#include "../../Core/Log.h"
#include "../../Input/Windows/InputWindows.h"
#include "../../Audio/WASAPI/AudioWASAPI.h"
#include <ShellScalingAPI.h>
#include <shellapi.h>
#include <Ole2.h>
#include <oleidl.h>

static const DWORD MS_VC_EXCEPTION = 0x406D1388;
#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
    DWORD dwType; // Must be 0x1000.
    LPCSTR szName; // Pointer to name (in user addr space).
    DWORD dwThreadID; // Thread ID (-1=caller thread).
    DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

// Prefer the high-performance GPU on switchable GPU systems
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 1;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

namespace Alimer
{
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
        if (HMODULE shCoreLibrary = ::LoadLibraryW(L"Shcore.dll"))
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
        }

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

    void Application::PlatformConstruct()
    {
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
    }

    SharedPtr<Window> Application::MakeWindow(const std::string& title, uint32_t width, uint32_t height, bool fullscreen)
    {
        return MakeShared<WindowWindows>(title, width, height, fullscreen);
    }

    Input* Application::CreateInput()
    {
        return new InputWindows();
    }

    Audio* Application::CreateAudio()
    {
        return new AudioWASAPI();
    }

    int Application::Run()
    {
        if (!Win32PlatformInitialize())
        {
            ALIMER_LOGERROR("[Win32] - Failed to setup");
            return EXIT_FAILURE;
        }

        if (!InitializeBeforeRun())
        {
            return EXIT_FAILURE;
        }

        MSG msg;
        while (_running)
        {
            if (!_paused)
            {
                if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
                {
                    TranslateMessage(&msg);
                    DispatchMessageW(&msg);

                    if (msg.message == WM_QUIT)
                    {
                        Exit();
                        break;
                    }
                }

                // Tick handles pause state.
                RunOneFrame();
            }
            else
            {
                BOOL ret = GetMessageW(&msg, nullptr, 0, 0);
                if (ret == 0)
                {
                    Exit();
                    break;
                }
                else if (ret == -1)
                {
                    Exit();
                    ALIMER_LOGERROR("[Win32] - Failed to get message");
                    return EXIT_FAILURE;
                }
                else
                {
                    TranslateMessage(&msg);
                    DispatchMessageW(&msg);
                }

                // Tick handles pause state.
                RunOneFrame();
            }
        }

        OnExiting();

        return EXIT_SUCCESS;
    }

    bool Application::SetCurrentThreadName(const std::string& name)
    {
        THREADNAME_INFO info;
        info.dwType = 0x1000;
        info.szName = name.c_str();
        info.dwThreadID = static_cast<DWORD>(-1);
        info.dwFlags = 0;

        __try
        {
            RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), reinterpret_cast<ULONG_PTR*>(&info));
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
        }
        return true;
    }
}

