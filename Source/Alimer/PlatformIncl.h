//
// Alimer is based on the Turso3D codebase.
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

#pragma once

#include "PlatformDef.h"

#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP
#pragma warning(push)
#pragma warning(disable : 4005)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NODRAWTEXT
#define NOGDI
#define NOBITMAP
#define NOMCX
#define NOSERVICE
#define NOHELP
#pragma warning(pop)

#include <windows.h>

#if (defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP)) || (defined(_XBOX_ONE) && defined(_TITLE))
#pragma warning(push)
#pragma warning(disable: 4471)
#include <Windows.UI.Core.h>
#pragma warning(pop)
#endif

#if defined(_MSC_VER) && !defined(_M_ARM)
#include <intrin.h>
#define ANGLE_USE_SSE
#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#include <x86intrin.h>
#define ANGLE_USE_SSE
#endif

// Mips and arm devices need to include stddef for size_t.
#if defined(__mips__) || defined(__arm__) || defined(__aarch64__)
#include <stddef.h>
#endif

bool IsWindowsVersionOrGreater(WORD wMajorVersion, WORD wMinorVersion, WORD wServicePackMajor);

#define IsWindows8Point1OrGreater() IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WINBLUE), LOBYTE(_WIN32_WINNT_WINBLUE), 0);

#define IsWindows10OrGreater() IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WIN10), LOBYTE(_WIN32_WINNT_WIN10), 0);

#elif ALIMER_LINUX_FAMILY || ALIMER_PLATFORM_WEB
#   include <unistd.h>
#   include <signal.h>
#   include <dlfcn.h>

#	if ALIMER_PLATFORM_ANDROID
#		include <android/log.h>
#	elif ALIMER_PLATFORM_WEB
#		include <emscripten/emscripten.h>
#		include <emscripten/html5.h>
#		include <emscripten/threading.h>
#	endif

#endif
