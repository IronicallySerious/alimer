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

#include "alimer_config.h"

#if defined(_WIN64) || defined(_WIN32) 
struct HWND__;
struct HINSTANCE__;
#elif defined(__ANDROID__)
struct ANativeWindow;
#endif

namespace alimer
{
#if defined(_WIN64) || defined(_WIN32) 

    // Window handle is HWND (HWND__*) on Windows
    using WindowHandle = HWND__*;

    // Window connection is HINSTANCE (HINSTANCE__*) on Windows
    using WindowConnection = HINSTANCE__ * ;

#elif defined(__ANDROID__)

    // Window handle is ANativeWindow* (void*) on Android
    using WindowHandle = struct ANativeWindow*;

    // Window connection is dummy uint on Android
    using WindowConnection = unsigned;

#elif defined(__linux__) || defined(__linux) || defined(__BSD__) || defined(__FreeBSD__)

    // Window handle is Window (unsigned long) on Unix - X11
    using WindowHandle = unsigned long;

    // Window connection is Display (unsigned long) on Unix - X11
    using WindowConnection = unsigned long;

#elif defined(__APPLE__) 

    // Window handle is NSWindow or NSView (void*) on MacOS - Cocoa
    // Window handle is UIWindow (void*) on iOS/TVOS/WatchOS - UIKit
    using WindowHandle = void*;

    // Window connection is dummy uint on MacOS
    using WindowConnection = unsigned;

#elif defined(__EMSCRIPTEN__)

    // Window handle is dummy uint on emscripten
    using WindowHandle = unsigned;

    // Window connection is dummy uint on emscripten
    using WindowConnection = unsigned;

#endif
}
