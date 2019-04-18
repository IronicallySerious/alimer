//
// Alimer is based on the Turso3D codebase.
// Copyright (c) 2018-2019 Amer Koleci and contributors.
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

#include "../Math/IntVector2.h"
#include "../Object/Object.h"

#if defined(_WIN32) || defined(_WIN64)
struct HWND__;
#endif

namespace Turso3D
{
    /// %Window resized event.
    class TURSO3D_API WindowResizeEvent : public Event
    {
    public:
        /// New window size.
        IntVector2 size;
    };

    /// Operating system window, Win32 implementation.
    class TURSO3D_API Window : public Object
    {
        OBJECT(Window);

    public:
        /// Construct and register subsystem. The window is not yet opened.
        Window();
        /// Destruct. Close window if open.
        ~Window();

        /// Set window title.
        void SetTitle(const String& newTitle);
        /// Set window size. Open the window if not opened yet. Return true on success.
        bool SetSize(const IntVector2& size, bool fullscreen, bool resizable);
        /// Set window position.
        void SetPosition(const IntVector2& position);
        /// Set mouse cursor visible. Default is true. When hidden, the mouse cursor is confined to the window and kept centered; relative mouse motion can be read "endlessly" but absolute mouse position should not be used.
        void SetMouseVisible(bool enable);
        /// Move the mouse cursor to a window top-left relative position.
        void SetMousePosition(const IntVector2& position);
        /// Close the window.
        void Close();
        /// Minimize the window.
        void Minimize();
        /// Maximize the window.
        void Maximize();
        /// Restore window size.
        void Restore();

        /// Return window title.
        const String& Title() const { return title; }
        /// Return window client area size.
        const IntVector2& Size() const { return size; }
        /// Return window client area width.
        int Width() const { return size.x; }
        /// Return window client area height.
        int Height() const { return size.y; }
        /// Return window position.
        IntVector2 Position() const;
        /// Return last known mouse cursor position relative to window top-left.
        const IntVector2& MousePosition() const { return mousePosition; }
        /// Return whether is resizable.
        bool IsResizable() const { return resizable; }
        /// Return whether is fullscren.
        bool IsFullscreen() const { return fullscreen; }
        /// Return whether is currently minimized.
        bool IsMinimized() const { return minimized; }
        /// Return whether has input focus.
        bool HasFocus() const { return focus; }
        /// Return whether mouse cursor is visible.
        bool IsMouseVisible() const { return mouseVisible; }

#if defined(_WIN32) || defined(_WIN64)
        static const wchar_t AppWindowClass[];

        /// Return whether window is open.
        bool IsOpen() const { return _handle != nullptr; }

        /// Return window handle. Can be cast to a HWND.
        HWND__* Handle() const { return _handle; }

        /// Handle a window message. Return true if handled and should not be passed to the default window procedure.
        bool OnWindowMessage(unsigned msg, unsigned wParam, unsigned lParam);
#endif

        /// Close requested event.
        Event closeRequestEvent;
        /// Gained focus event.
        Event gainFocusEvent;
        /// Lost focus event.
        Event loseFocusEvent;
        /// Minimized event.
        Event minimizeEvent;
        /// Restored after minimization -event.
        Event restoreEvent;
        /// Size changed event.
        WindowResizeEvent resizeEvent;

    private:
        /// Change display mode. If width and height are zero, will restore desktop resolution.
        void SetDisplayMode(int width, int height);
        /// Update mouse visibility and clipping region to the OS.
        void UpdateMouseVisible();
        /// Update mouse clipping region.
        void UpdateMouseClipping();
        /// Refresh the internally tracked mouse cursor position.
        void UpdateMousePosition();
        /// Verify window size from the window client rect.
        IntVector2 ClientRectSize() const;

#if defined(_WIN32) || defined(_WIN64)
        /// Window handle.
        HWND__* _handle = nullptr;
#endif
        /// Window title.
        String title = "Turso3D Window";
        /// Current client area size.
        IntVector2 size = {};
        /// Last stored windowed mode position.
        IntVector2 savedPosition;
        /// Current mouse cursor position.
        IntVector2 mousePosition;
        /// Window style flags.
        unsigned windowStyle;
        /// Current minimization state.
        bool minimized;
        /// Current focus state.
        bool focus;
        /// Resizable flag.
        bool resizable;
        /// Fullscreen flag.
        bool fullscreen;
        /// Performing window resize flag. Used internally to suppress resize events during it.
        bool inResize;
        /// Mouse visible flag as requested by the application.
        bool mouseVisible;
        /// Internal mouse visible flag. The mouse is automatically shown when the window is unfocused, while mouseVisible represents the application's desired state. Used to prevent multiple calls to OS mouse visibility functions, which utilize a counter.
        bool mouseVisibleInternal;
    };
}

