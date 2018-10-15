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

#include "../Core/Platform.h"
#include "../Core/Object.h"
#include "../Math/Math.h"
#include <string>

namespace Alimer
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
    class ALIMER_API WindowResizeEvent : public Event
    {
    public:
        /// New window size.
        uvec2 size;
    };

#if ALIMER_PLATFORM_WINDOWS
    class Win32OleDropTarget;
#endif

    /// OS Window class.
    class Window final : public Object
    {
        ALIMER_OBJECT(Window, Object);

    public:
        /// Constructor.
        Window();

        /// Destructor.
        ~Window() override;

        /// Set window title.
        void SetTitle(const std::string& newTitle);

        /// Define the window size and settings.
        bool Define(const uvec2& size, WindowFlags flags = WindowFlags::Default);

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

        /// Return whether is visible.
        bool IsVisible() const { return _visible; }

        /// Return whether is currently minimized.
        bool IsMinimized() const;

        /// Return whether is fullscreen.
        bool IsFullscreen() const;

        /// Return whether the backend handle is valid.
        bool IsValid() const { return _valid; }

        /// Return window title.
        const std::string& GetTitle() const { return _title; }

        /// Return window client area size.
        const uvec2& GetSize() const { return _size; }
        uint32_t GetWidth() const { return _size.x; }
        uint32_t GetHeight() const { return _size.y; }

        float GetAspectRatio() const { return static_cast<float>(_size.x) / _size.y; }
        WindowFlags GetFlags() const { return _flags; }

#if ALIMER_PLATFORM_UWP
        IUnknown* GetHandle() const { return _handle; }
#elif ALIMER_PLATFORM_WINDOWS
        LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

        HINSTANCE GetHInstance() const { return _hInstance; }
        HWND GetHandle() const { return _handle; }
#endif
        /// Size changed event.
        WindowResizeEvent resizeEvent;

    private:
        void PlatformConstruct();
        void PlatformDestroy();
        bool PlatformCreate();

#if ALIMER_PLATFORM_UWP
        /// Window handle is IUnknown on UWP
        IUnknown* _handle = nullptr;
#elif ALIMER_PLATFORM_WINDOWS
        void InitAfterCreation();
        void HandleResize(WPARAM wParam, LPARAM lParam);
        HINSTANCE _hInstance = nullptr;
        HWND _handle = nullptr;
        HMONITOR _monitor = nullptr;
        int _showCommand = SW_SHOW;
        Win32OleDropTarget* _dropTarget = nullptr;
#endif
        /// Window title.
        std::string _title;
        /// Window size.
        uvec2 _size;
        /// Flags
        WindowFlags _flags = WindowFlags::Default;
        /// Is backend handle valid.
        bool _valid = false;
        /// Visibility flag.
        bool _visible = true;
        /// Visibility flag.
        bool _focused = false;

    private:
        DISALLOW_COPY_MOVE_AND_ASSIGN(Window);
    };
}
