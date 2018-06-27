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

#include "../Core/Ptr.h"
#include <string>

#if ALIMER_PLATFORM_WINDOWS
struct HWND__;
struct HDC__;
struct HINSTANCE__;
#elif ALIMER_PLATFORM_UWP
struct IUnknown;
#elif ALIMER_PLATFORM_ANDROID
typedef struct ANativeWindow ANativeWindow;
#endif

namespace Alimer
{
    enum WindowHandleType
    {
        WINDOW_HANDLE_UNKNOWN,
        WINDOW_HANDLE_WINDOWS,
        WINDOW_HANDLE_UWP,
        WINDOW_HANDLE_Android,
    };

    struct WindowHandle
    {
        WindowHandleType type;
        union
        {
#if ALIMER_PLATFORM_WINDOWS
            struct
            {
                /// The window handle
                HWND__* window;
                /// The window device context
                HDC__* hdc;

                /// The instance handle
                HINSTANCE__* hInstance;
            } win;

#elif ALIMER_PLATFORM_UWP
            struct
            {
                /// UWP CoreWindow
                IUnknown* window;
            } uwp;
#elif ALIMER_PLATFORM_ANDROID
            struct
            {
                ANativeWindow *window;
            } android;
#endif
            // Make sure this union is always 64 bytes (8 64-bit pointers)
            unsigned char dummy[64];
        } info;
    };

    /// OS Window class.
    class Window : public RefCounted
    {
    protected:
        /// Constructor.
        Window();

    public:
        /// Destructor.
        virtual ~Window();

        inline uint32_t GetWidth() const { return _width; }
        inline uint32_t GetHeight() const { return _height; }
        inline float GetAspectRatio() const { return static_cast<float>(_width) / _height; }
        inline const WindowHandle& GetHandle() const { return _handle; }

    protected:
        WindowHandle _handle;
        /// Window title.
        std::string _title;
        /// Window width.
        uint32_t _width;
        /// Window height.
        uint32_t _height;
        /// Resizable flag.
        bool _resizable;

    private:
        DISALLOW_COPY_MOVE_AND_ASSIGN(Window);
    };
}
