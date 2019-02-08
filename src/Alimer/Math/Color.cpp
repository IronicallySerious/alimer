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

#include "../Math/Color.h"
#include <fmt/printf.h>

namespace alimer
{
    const Color4 Color4::Black = { 0.0f, 0.0f, 0.0f, 1.0f };
    const Color4 Color4::White = { 1.0f, 1.0f, 1.0f, 1.0f };
    const Color4 Color4::WhiteSmoke = { 0.960784376f, 0.960784376f, 0.960784376f, 1.0f };
    const Color4 Color4::Red = { 1.000000000f, 0.000000000f, 0.000000000f, 1.000000000f };
    const Color4 Color4::Green = { 0.0f, 0.501960814f, 0.0f, 1.0f };
    const Color4 Color4::Blue = { 0.0f, 0.0f, 1.0f, 1.0f };
    const Color4 Color4::Yellow = { 1.0f, 1.0f, 0.0f, 1.0f };
    const Color4 Color4::YellowGreen = { 0.603921592f, 0.803921640f, 0.196078449f, 1.0f };
    const Color4 Color4::Lime = { 0.000000000f, 1.000000000f, 0.000000000f, 1.000000000f };

    std::string Color4::ToString() const
    {
        return fmt::sprintf("%g %g %g %g", r, g, b, a);
    }
}
