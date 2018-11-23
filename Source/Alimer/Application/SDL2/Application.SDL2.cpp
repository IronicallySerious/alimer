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
#include "../../Debug/Log.h"

#if ALIMER_PLATFORM_WINDOWS
#   include <ShellScalingAPI.h>
#   include <shellapi.h>

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

#endif // ALIMER_PLATFORM_WINDOWS

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_syswm.h>

namespace Alimer
{
    static inline MouseButton ConvertMouseButton(uint8_t sdlButton)
    {
        switch (sdlButton)
        {
        case SDL_BUTTON_LEFT:
            return MouseButton::Left;
        case SDL_BUTTON_MIDDLE:
            return MouseButton::Middle;
        case SDL_BUTTON_RIGHT:
            return MouseButton::Right;
        case SDL_BUTTON_X1:
            return MouseButton::X1;
        case SDL_BUTTON_X2:
            return MouseButton::X2;

        default:
            return MouseButton::None;
        }
    }

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
#endif // ALIMER_PLATFORM_WINDOWS
    }

    int Application::Run()
    {
        SDL_SetMainReady();
        int result = SDL_Init(
            SDL_INIT_VIDEO
            | SDL_INIT_GAMECONTROLLER
            | SDL_INIT_HAPTIC
            | SDL_INIT_TIMER);
        if (result < 0)
        {
            SDL_ShowSimpleMessageBox(
                SDL_MESSAGEBOX_ERROR,
                "SDL Init Errors",
                SDL_GetError(), nullptr);
            return EXIT_FAILURE;
        }

        SDL_EventState(SDL_DROPFILE, SDL_ENABLE);

        if (!InitializeBeforeRun())
        {
            return EXIT_FAILURE;
        }

        SDL_Event evt;
        //SDL2Window* sdlMainWindow = static_cast<SDL2Window*>(_window.Get());
        while (_running)
        {
            while (SDL_PollEvent(&evt))
            {
                switch (evt.type)
                {
                case SDL_QUIT:
                    _running = false;
                    break;

                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP:
                {
                    const SDL_MouseButtonEvent& mouseEvent = evt.button;
                    MouseButton button = ConvertMouseButton(mouseEvent.button);
                    _input.MouseButtonEvent(button,
                        static_cast<float>(mouseEvent.x),
                        static_cast<float>(mouseEvent.y),
                        mouseEvent.type == SDL_MOUSEBUTTONDOWN);

                    //OnMouseEvent(button,
                    //    static_cast<float>(mouseEvent.x),
                    //    static_cast<float>(mouseEvent.y),
                    //    mouseEvent.type == SDL_MOUSEBUTTONDOWN);
                }

                case SDL_MOUSEMOTION:
                {
                    const SDL_MouseMotionEvent& motionEvt = evt.motion;
                    MouseButton button = MouseButton::None;
                    if (motionEvt.state & SDL_BUTTON(SDL_BUTTON_LEFT))
                        button = MouseButton::Left;
                    else if (motionEvt.state &SDL_BUTTON(SDL_BUTTON_RIGHT))
                        button = MouseButton::Right;

                    _input.MouseMoveEvent(button,
                        static_cast<float>(motionEvt.x),
                        static_cast<float>(motionEvt.y));
                    //OnMouseMove(mx, my);
                }
                break;
                }
            }

            // Tick handles pause state.
            RunFrame();
        }

        OnExiting();

        // quit all subsystems and quit application.
        SDL_QuitSubSystem(SDL_INIT_EVERYTHING);
        SDL_Quit();
        return EXIT_SUCCESS;
    }
}

