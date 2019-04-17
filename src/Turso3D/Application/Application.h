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

#include "../Base/AutoPtr.h"
#include "../Math/IntVector2.h"
#include "../Object/Object.h"

namespace Turso3D
{
    class Input;
    class Graphics;

    /// %Application subsystem which initialize rest of subdsystems and run a main loop until exited.
    class TURSO3D_API Application : public Object
    {
        OBJECT(Application);

    public:
        /// Construct and register subsystem.
        Application();

        /// Destruct.
        virtual ~Application();

        int Run();

    private:
        void HandleWindowCloseRequest(Event&);

    protected:
        /// Input subsystem.
        AutoPtr<Input> input;

        /// Graphics subsystem.
        AutoPtr<Graphics> graphics;
    };

}
