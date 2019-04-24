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

#pragma once

#include "config.h"
#include <stdbool.h>

// Platform
#define VORTICE_PLATFORM_ANDROID    0
#define VORTICE_PLATFORM_BSD        0
#define VORTICE_PLATFORM_EMSCRIPTEN 0
#define VORTICE_PLATFORM_HURD       0
#define VORTICE_PLATFORM_IOS        0
#define VORTICE_PLATFORM_LINUX      0
#define VORTICE_PLATFORM_NX         0
#define VORTICE_PLATFORM_OSX        0
#define VORTICE_PLATFORM_PS4        0
#define VORTICE_PLATFORM_RPI        0
#define VORTICE_PLATFORM_STEAMLINK  0
#define VORTICE_PLATFORM_WINDOWS    0
#define VORTICE_PLATFORM_UWP        0   // Universal Windows Platform (WinRT)
#define VORTICE_PLATFORM_XBOXONE    0

#if defined(_DURANGO) || defined(_XBOX_ONE) // XboxOne
#   undef VORTICE_PLATFORM_XBOXONE
#   define VORTICE_PLATFORM_XBOXONE 1
#elif defined(_WIN32) || defined(_WIN64) // Windows
// http://msdn.microsoft.com/en-us/library/6sehtctf.aspx
#   ifndef NOMINMAX
#       define NOMINMAX
#   endif    // NOMINMAX
#   include <winapifamily.h>
#   if !defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
#       undef VORTICE_PLATFORM_WINDOWS
#       define VORTICE_PLATFORM_WINDOWS 1
#   else
#       undef VORTICE_PLATFORM_UWP
#       define VORTICE_PLATFORM_UWP 1
#   endif
#endif

// Misc
#define VORTICE_UNUSED(x) (void)(true ? (void)0 : ((void)(x)))

#if VORTICE_BUILD_SHARED
#   if defined(_MSC_VER)
#       ifdef VORTICE_EXPORTS
#           define VORTICE_API __declspec(dllexport)
#       else
#           define VORTICE_API __declspec(dllimport)
#       endif
#   else
#       define VORTICE_API __attribute__((visibility("default")))
#   endif
#else
#   define VORTICE_API
#endif
