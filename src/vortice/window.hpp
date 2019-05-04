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

#include "core/platform.h"
#if defined(_WIN32)
#   if !defined(NOMINMAX)
#       define NOMINMAX
#   endif
#   if !defined(WIN32_LEAN_AND_MEAN)
#       define WIN32_LEAN_AND_MEAN
#   endif
#   include <Windows.h>
#elif defined(__linux__)
#   include <X11/Xlib-xcb.h>
#endif

namespace vortice
{
    /// Define OS window class.
    class VORTICE_API Window final
    {
    public:
        Window();

        /// Destructor.
        ~Window();

        /// Opens/defines the window.
        void define(const char* title, int width, int height, bool resizable = true, bool fullscreen = false);

        /// Closes the window.
        void close();

        /// Shows the window.
        void show();

        /// Hides the window.
        void hide();

        /// Minimizes the window.
        void minimize();

        /// Maximizes the window.
        void maximize();

        /// Restores the window.
        void restore();

        /// Sets the title of the window.
        void set_title(const char* title);

        /// Check if window is open and valid.
        bool is_open() const;

        /// Returns the native window handle.
#if defined(_WIN32)
        HWND handle() const;
#elif defined(__linux__)
        Window handle() const;
#endif

    private:
        /// Native implementation handle
        void* _impl = nullptr;

        int _width;
        int _height;
    };
} // namespace vortice
