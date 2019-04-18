//
// Alimer is based on the Turso3D codebase.
// Copyright (c) 2018-2019 Amer Koleci and contributors.
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

#include "../../Debug/Log.h"
#include "../Input.h"
#include "../Application.h"
#include "../Window.h"

#ifndef NOMINMAX
#    define NOMINMAX 1
#endif
#ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN 1
#endif
#include <Windows.h>
#include <ShellScalingAPI.h>
#include <shellapi.h>

#ifndef DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
DECLARE_HANDLE(DPI_AWARENESS_CONTEXT);
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((DPI_AWARENESS_CONTEXT) -4
#endif

// user32.dll function pointer typedefs
typedef BOOL(WINAPI * PFN_SetProcessDPIAware)(void);
typedef BOOL(WINAPI * PFN_SetProcessDpiAwarenessContext)(DPI_AWARENESS_CONTEXT);

// shcore.dll function pointer typedefs
typedef HRESULT(WINAPI * PFN_SetProcessDpiAwareness)(PROCESS_DPI_AWARENESS);
typedef HRESULT(WINAPI * PFN_GetDpiForMonitor)(HMONITOR, MONITOR_DPI_TYPE, UINT*, UINT*);


#include "../../Debug/DebugNew.h"

namespace Turso3D
{
    static LRESULT CALLBACK WndProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);

    void Application::PlatformConstruct()
    {
        const bool highDPI = true;

        // Initialize COM API first.
        HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
        if (hr == RPC_E_CHANGED_MODE)
            hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

        if (hr == S_FALSE || FAILED(hr))
        {
            TURSO3D_LOGDEBUGF("Failed to initialize COM, error: %d", hr);
            return;
        }

        if (highDPI)
        {
            // Enable high DPI as SDL not support it on windows.
            HMODULE user32Library = LoadLibraryA("user32.dll");
            HMODULE shCore = LoadLibraryA("Shcore.dll");
            if (PFN_SetProcessDpiAwarenessContext SetProcessDpiAwarenessContextProc = (PFN_SetProcessDpiAwarenessContext)GetProcAddress(user32Library, "SetProcessDpiAwarenessContext"))
            {
                SetProcessDpiAwarenessContextProc(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
            }
            else if (shCore)
            {
                if (PFN_SetProcessDpiAwareness SetProcessDpiAwarenessProc = (PFN_SetProcessDpiAwareness)GetProcAddress(shCore, "SetProcessDpiAwareness"))
                {
                    SetProcessDpiAwarenessProc(PROCESS_PER_MONITOR_DPI_AWARE);
                }

                FreeLibrary(shCore);
            }
            else
            {
                if (PFN_SetProcessDPIAware SetProcessDPIAwareProc = (PFN_SetProcessDPIAware)GetProcAddress(user32Library, "SetProcessDPIAware"))
                {
                    SetProcessDPIAwareProc();
                }
            }

            FreeLibrary(user32Library);
        }

#ifdef _DEBUG
        if (!AllocConsole()) {
            TURSO3D_LOGINFO("Attached to console");
        }
#endif

        // Register main window class
        WNDCLASSEXW wc;
        wc.cbSize = sizeof(wc);
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wc.lpfnWndProc = WndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.hIcon = LoadIcon(0, IDI_APPLICATION);
        wc.hCursor = LoadCursor(0, IDC_ARROW);
        wc.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
        wc.lpszMenuName = nullptr;
        wc.lpszClassName = Window::AppWindowClass;
        wc.hIconSm = nullptr;

        if (!RegisterClassExW(&wc)) {
            MessageBox(NULL, TEXT("Win32 class registration failed!"), TEXT("Error"), MB_ICONEXCLAMATION | MB_OK);
            return;
        }
    }

    void Application::PlatformShutdown()
    {
        CoUninitialize();
    }

    int Application::PlatformRun()
    {
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

                Tick();
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
                    TURSO3D_LOGERROR("[Win32] - Failed to get message");
                    return EXIT_FAILURE;
                }
                else
                {
                    TranslateMessage(&msg);
                    DispatchMessageW(&msg);
                }

                input->Update();
            }
        }

        return EXIT_SUCCESS;
    }

    LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        Window* window = reinterpret_cast<Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        bool handled = false;
        // When the window is just opening and has not assigned the userdata yet, let the default procedure handle the messages
        if (window) {
            handled = window->OnWindowMessage(msg, (unsigned)wParam, (unsigned)lParam);
        }

        return handled ? 0 : DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}
