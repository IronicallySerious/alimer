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

#include <foundation/cpp_macros.h>
#include "../Base/String.h"
#include "../Core/Platform.h"
#include "../Core/Object.h"
#include "../Math/Math.h"
#include "../Math/Vector2.h"

namespace alimer
{
    enum class WindowFlags : uint32_t
    {
        None = 0,
        Resizable = 1 << 0,
        Fullscreen = 1 << 1,
        Visible = 1 << 2,
        Default = Resizable | Visible
    };
    ALIMER_BITMASK(WindowFlags);

    /// Window resized event.
    struct WindowResizeEvent 
    {
        /// New window size.
        IntVector2 size;
    };

    using NativeHandle = void*;
    using NativeDisplay = void*;


    /// OS Window class.
    class ALIMER_API Window : public Object
    {
        ALIMER_OBJECT(Window, Object);

    public:
        /// Constructor.
        Window();

        /// Destructor
        ~Window() override;

        /// Defines window.
        bool Define(const String& title, const IntVector2& size, WindowFlags flags = WindowFlags::Default);

        /// Show the window.
        void Show();
        /// Hide the window.
        void Hide();
        /// Minimize the window.
        void Minimize();
        /// Maximize the window.
        void Maximize();
        /// Restore window size.
        void Restore();
        /// Close the window.
        void Close();

        /// Resize the window.
        void Resize(int width, int height);

        /// Set window title.
        void SetTitle(const String& newTitle);

        /// Set whether is fullscreen.
        void SetFullscreen(bool value);

        /// Return whether is visible.
        bool IsVisible() const;

        /// Return whether is currently minimized.
        bool IsMinimized() const;

        /// Return whether is fullscreen.
        bool IsFullscreen() const;

        /// Return is native window is valid.
        bool IsOpen() const;

        /// Return window title.
        const String& GetTitle() const { return _title; }

        /// Return window client area size.
        const IntVector2& GetSize() const { return _size; }
        int GetWidth() const { return _size.x; }
        int GetHeight() const { return _size.y; }

        float GetAspectRatio() const { return static_cast<float>(_size.x) / _size.y; }
        WindowFlags GetFlags() const { return _flags; }

        /// Is cursor visible.
        bool IsCursorVisible() const { return true; }

        /// Set cursor visibility.
        void SetCursorVisible(bool visible);

        /// Gets the native window or view handle.
        NativeHandle GetNativeHandle() const;

        /// Gets the native display, connection or instance handle.
        NativeDisplay GetNativeDisplay() const;

        /// Size changed event.
        Event<void(const WindowResizeEvent&)> resizeEvent;

    private:
        void OnSizeChanged(const IntVector2& newSize);

        /// Backend implementation.
        class WindowImpl* _impl = nullptr;
        /// Window title.
        String _title;
        /// Window size.
        IntVector2 _size;
        /// Flags
        WindowFlags _flags = WindowFlags::Default;
        /// Visibility flag.
        bool _focused = false;

    private:
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;
    };
}
