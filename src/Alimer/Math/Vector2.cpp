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

#include "../Math/Vector2.h"
#include <fmt/printf.h>

namespace alimer
{
    const IntVector2 IntVector2::Zero = { 0, 0 };
    const IntVector2 IntVector2::One = { 1, 1 };
    const IntVector2 IntVector2::UnitX = { 1, 0 };
    const IntVector2 IntVector2::UnitY = { 0, 1 };

    const Vector2 Vector2::Zero = { 0.0f, 0.0f };
    const Vector2 Vector2::One = { 1.0f, 1.0f };
    const Vector2 Vector2::UnitX = { 1.0f, 0.0f };
    const Vector2 Vector2::UnitY = { 0.0f, 1.0f };

    std::string IntVector2::ToString() const
    {
        return fmt::sprintf("%d %d", x, y);
    }

    std::string Vector2::ToString() const
    {
        return fmt::sprintf("%g %g", x, y);
    }
}