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

#include "../../Turso3DConfig.h"

#ifdef TURSO3D_OPENGL

#include "../../Base/Ptr.h"

namespace Turso3D
{
    class Window;

    /// OpenGL context associated with a window, Win32 implementation.
    class TURSO3D_API GLContext
    {
    public:
        /// Construct. Associate with a window, but do not create the context yet.
        GLContext(Window* window);
        /// Destruct. Destroy the context if created.
        ~GLContext();

        /// Create context and initialize extensions. Return true on success. The pixel format can only be chosen once, so a context can not be created more than once to the same window.
        bool Create(int multisample);
        /// Present the backbuffer.
        void Present();
        /// Set vsync on/off.
        void SetVSync(bool enable);

        /// Return whether is initialized with a valid context.
        bool IsInitialized() const { return contextHandle != nullptr; }

    private:
        /// Destroy the context.
        void Release();

        /// Associated window.
        WeakPtr<Window> window;
        /// Window device context handle.
        void* dcHandle;
        /// OpenGL context handle.
        void* contextHandle;
    };
}

#endif
