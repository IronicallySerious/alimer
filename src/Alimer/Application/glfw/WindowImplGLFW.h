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

#include "../Window.h"
struct GLFWwindow;

namespace alimer
{
    class WindowImpl final 
    {
    public:
        WindowImpl(const std::string& title, uint32_t width, uint32_t height, WindowFlags flags);

        /// Destructor.
        ~WindowImpl();

        void Destroy();
        void Show();
        void Hide();
        void Minimize();
        void Maximize();
        void Restore();
        void Close();
        void Resize(uint32_t width, uint32_t height);
        bool IsVisible() const;
        bool IsMinimized() const;
        void SetTitle(const std::string& newTitle);
        void SetFullscreen(bool value);
        bool IsOpen() const;
        bool IsCursorVisible() const;
        void SetCursorVisible(bool visible);
        uintptr_t GetNativeHandle() const;
        uint64_t GetNativeDisplay() const;

        GLFWwindow* _window;

        /// Visibility flag.
        bool _visible = true;
    };
}