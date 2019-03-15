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

#include "core/Object.h"

namespace alimer
{
    /// Operating system window.
    class ALIMER_API Window : public Object
    {
        ALIMER_OBJECT(Window, Object);

    protected:
        /// Constructor.
        Window(const String& title, uint32_t width, uint32_t height, bool resizable, bool fullscreen);

    public:
        /// Destructor. Close window if open.
        virtual ~Window() = default;

        virtual void Close();

        bool IsOpen() const { return _isOpen; }

    protected:
        String _title;
        uint32_t _width;
        uint32_t _height;
        bool _resizable;
        bool _fullscreen;
        bool _isOpen;
    };
}
