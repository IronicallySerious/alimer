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

#pragma once

#include "../AlimerConfig.h"

#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP)
struct IUnknown;
#elif defined(_WIN32)
struct HWND__;
#endif

namespace Alimer
{
#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP)
    /// Window handle is IUnknown on UWP
    using WindowHandle = IUnknown*;
#elif defined(_WIN32)
    /// Window handle is HWND (HWND__*) on Windows
    using WindowHandle = HWND__*;
#elif defined(__APPLE__)
    /// Window handle is NSWindow or NSView (void*) on Mac OS X - Cocoa or UIWindow (void*) on iOS - UIKit.
    using WindowHandle = void*;
#elif defined(__linux__)
    // Window handle is Window (unsigned long) on Unix - X11
    using WindowHandle = unsigned long;
#elif defined(__ANDROID__)
    // Window handle is ANativeWindow* (void*) on Android
    using WindowHandle = void*;
#endif
}
