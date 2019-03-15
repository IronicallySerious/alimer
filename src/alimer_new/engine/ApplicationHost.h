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

#include "foundation/UniquePtr.h"
#include "core/Object.h"

namespace alimer
{
    class Window;
    class Application;

    /// Platform host for application.
    class ALIMER_API ApplicationHost
    {
    protected:
        ApplicationHost(Application* application);

    public:
        /// Destructor.
        virtual ~ApplicationHost() = default;

        /// Create host for application.
        static ApplicationHost* Create(Application* application);

        virtual void Run() = 0;
        virtual void RequestExit();

        virtual void ErrorDialog(const String& title, const String& message);

        virtual UniquePtr<Window> CreateWindow(const String& title, uint32_t width, uint32_t height, bool resizable, bool fullscreen) = 0;

    protected:
        void InitializeApplication();

    protected:
        Application* _application;
        bool _exitRequested = false;
    };
}
