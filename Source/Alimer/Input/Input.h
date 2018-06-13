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

#include "../AlimerConfig.h"
#include <string>
#include <atomic>

namespace Alimer
{
    enum class CursorType : uint32_t
    {
        Arrow = 0,
        Cross,
        Hand,
        Count
    };

    /// Input system class.
    class ALIMER_API Input
    {
    protected:
        /// Constructor.
        Input();

    public:
        /// Destructor.
        virtual ~Input();

        /// Return the single instance of the Input.
        static Input* GetInstance();

        /// Update input state and poll devices.
        virtual void Update();

        /// Is cursor visible.
        virtual bool IsCursorVisible() const;

        /// Set cursor visibility.
        virtual void SetCursorVisible(bool visible);

    private:
        DISALLOW_COPY_MOVE_AND_ASSIGN(Input);
    };

    /// Access to Input module.
    ALIMER_API Input& gInput();
}
