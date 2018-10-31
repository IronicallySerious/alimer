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

#ifndef _ALIMER_PLATFORM_H_
#define _ALIMER_PLATFORM_H_

#include <stdint.h>
#include <stdbool.h>

#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP)
#   define PLATFORM_UWP 1
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(_WIN64)
#   define PLATFORM_WINDOWS 1
#elif defined(__MACH__) || defined(__APPLE__)
#   define PLATFORM_OSX 1
#elif defined(__ANDROID__)
#   define PLATFORM_ANDROID 1
#elif defined(__linux__)
#   define PLATFORM_LINUX 1
#endif

#if PLATFORM_WINDOWS
    #define WIN32_LEAN_AND_MEAN
#   ifdef ALIMERC_EXPORTS
#       define EXPORT_API __declspec(dllexport)
#   else
#       define EXPORT_API __declspec(dllimport)
#   endif
#elif PLATFORM_ANDROID
#   define EXPORT_API __attribute__((visibility("default")))
#else
#   define EXPORT_API
#endif

#endif /* _ALIMER_PLATFORM_H_ */
