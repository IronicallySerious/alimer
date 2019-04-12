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

#include "engine/window_handle.h"
#include "core/Object.h"
#include "math/vec2.h"

#if defined(ALIMER_GLFW)
struct GLFWwindow;
#endif

namespace alimer
{
    /// Operating system window.
    class ALIMER_API Window final : public Object
    {
        ALIMER_OBJECT(Window, Object);

    public:
        /// Constructor.
        Window();

        /// Constructor.
        Window(const std::string& title, const math::uint2& size, bool resizable = true, bool fullscreen = false);

        /// Destructor. Close window if open.
        ~Window();

        /// Set window size. Open the window if not opened yet. Return true on success.
        bool SetSize(const math::uint2& size, bool resizable = true, bool fullscreen = false);

        /// Close's the window
        void Close();
        bool IsOpen() const;

        /// Return window title.
        const std::string& GetTitle() const { return _title; }

        /// Return window size.
        const math::uint2& GetSize() const { return _size; }

        /// Return window width.
        uint32_t GetWidth() const { return _size.x; }
        /// Return window height.
        uint32_t GetHeight() const { return _size.y; }

        /// Gets the native window or view handle.
        WindowHandle GetNativeHandle() const;

        /// Gets the native display, connection or instance handle.
        WindowConnection GetNativeConnection() const;

    protected:
        std::string _title;
        math::uint2 _size;
        bool _resizable;
        bool _fullscreen;

#if defined(ALIMER_GLFW)
        GLFWwindow* _window = nullptr;
#endif
    };
}
