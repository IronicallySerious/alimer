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
#include "../Core/Platform.h"
#include "../Core/Object.h"
#include "Math/math.h"
#include <string>

#if defined(ALIMER_GLFW)
struct GLFWwindow;
#endif

#if defined(_WIN32)
struct HWND__;
struct HINSTANCE__;
#elif defined(__ANDROID__)
struct ANativeWindow;
#endif

namespace alimer
{

#if defined(_WIN32)

    // Window handle is HWND (HWND__*) on Windows
    using WindowHandle = HWND__ * ;

    // Window connection is HINSTANCE (HINSTANCE__*) on Windows
    using WindowConnection = HINSTANCE__ * ;

#elif VORTICE_PLATFORM_LINUX || VORTICE_PLATFORM_BSD

    // Window handle is Window (unsigned long) on Unix - X11
    using WindowHandle = unsigned long;

    // Window connection is Display (unsigned long) on Unix - X11
    using WindowConnection = unsigned long;

#elif defined(VORTICE_PLATFORM_MACOS)

    // Window handle is NSWindow or NSView (void*) on MacOS - Cocoa
    using WindowHandle = void*;

    // Window connection is dummy uint on MacOS
    using WindowConnection = unsigned;

#elif VORTICE_PLATFORM_IOS || VORTICE_PLATFORM_TVOS || VORTICE_PLATFORM_WATCHOS

    // Window handle is UIWindow (void*) on iOS/TVOS/WatchOS - UIKit
    using WindowHandle = void*;

    // Window connection is dummy uint on iOS/TVOS/WatchOS
    using WindowConnection = unsigned;

#elif defined(__ANDROID__)

    // Window handle is ANativeWindow* (void*) on Android
    using WindowHandle = struct ANativeWindow*;

    // Window connection is dummy uint on Android
    using WindowConnection = unsigned;

#elif VORTICE_PLATFORM_EMSCRIPTEN

    // Window handle is dummy uint on emscripten
    using WindowHandle = unsigned;

    // Window connection is dummy uint on emscripten
    using WindowConnection = unsigned;

#endif

    /// Window resized event.
    struct WindowResizeEvent 
    {
        /// New window size.
        uvec2 size;
    };

    /// OS Window class.
    class ALIMER_API Window : public Object
    {
        ALIMER_OBJECT(Window, Object);

    public:
        /// Constructor.
        Window();

        /// Constructor.
        Window(const std::string& title, const uvec2& size, bool resizable = true, bool fullscreen = false);

        /// Destructor
        ~Window() override;

        /// Set window title.
        void SetTitle(const std::string& newTitle);
        /// Set window size. Open the window if not opened yet. Return true on success.
        bool SetSize(const uvec2& size, bool resizable = true, bool fullscreen = false);

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
        void Resize(const uvec2& size);

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
        const std::string& GetTitle() const { return _title; }

        /// Return window client area size.
        const uvec2& GetSize() const { return _size; }
        uint32_t GetWidth() const { return _size.x; }
        uint32_t GetHeight() const { return _size.y; }

        float GetAspectRatio() const { return static_cast<float>(_size.x) / _size.y; }

        /// Is cursor visible.
        bool IsCursorVisible() const { return true; }

        /// Set cursor visibility.
        void SetCursorVisible(bool visible);

        /// Gets the native window or view handle.
        WindowHandle GetNativeHandle() const;

        /// Gets the native display, connection or instance handle.
        WindowConnection GetNativeConnection() const;

        /// Size changed event.
        Event<void(const WindowResizeEvent&)> resizeEvent;

    protected:
        virtual void OnHandleCreated() {}
        virtual void OnHandleDestroyed() {}
        virtual void OnSizeChanged(const uvec2& newSize);
        /// Window title.
        std::string _title = "Alimer";
        /// Window height.
        uvec2 _size = { 0u, 0u };
        /// Fullscreen flag.
        bool _fullscreen;
        bool _visible = true;
        bool _focused = false;

    private:
#if defined(ALIMER_GLFW)
        /// Backend implementation.
        GLFWwindow* _window = nullptr;
#endif

    private:
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;
    };
}
