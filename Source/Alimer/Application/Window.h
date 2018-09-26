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

#include "../Core/Object.h"
#include "../Math/Math.h"
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
    struct WindowHandle
    {
        /// Native connection, display or instance type.
        void* connection;
        /// Native window handle.
        void* handle;
    };

    /// Window resized event.
    class ALIMER_API WindowResizeEvent : public Event
    {
    public:
        /// New window size.
        uvec2 size;
    };

    /// OS Window class.
    class Window : public Object
    {
        ALIMER_OBJECT(Window, Object);

    protected:
        /// Constructor.
        Window();

    public:
        /// Destructor.
        virtual ~Window();

        /// Show the window.
        virtual void Show();
        /// Hide the window.
        virtual void Hide();
        /// Minimize the window.
        virtual void Minimize();
        /// Maximize the window.
        virtual void Maximize();
        /// Restore window size.
        virtual void Restore();
        /// Close the window.
        virtual void Close();

        /// Return whether is visible.
        virtual bool IsVisible() const;

        /// Return whether is currently minimized.
        virtual bool IsMinimized() const;

        /// Set window title.
        virtual void SetTitle(const std::string& newTitle);

        /// Return window title.
        const std::string& GetTitle() const { return _title; }

        /// Return window client area size.
        const uvec2& GetSize() const { return _size; }
        uint32_t GetWidth() const { return _size.x; }
        uint32_t GetHeight() const { return _size.y; }

        float GetAspectRatio() const { return static_cast<float>(_size.x) / _size.y; }
        const WindowHandle& GetHandle() const { return _handle; }

        /// Size changed event.
        WindowResizeEvent resizeEvent;

    protected:
        WindowHandle _handle;
        /// Window title.
        std::string _title;
        /// Window size.
        uvec2 _size;
        /// Resizable flag.
        bool _resizable;

    private:
        DISALLOW_COPY_MOVE_AND_ASSIGN(Window);
    };
    using WindowPtr = SharedPtr<Window>;
}
