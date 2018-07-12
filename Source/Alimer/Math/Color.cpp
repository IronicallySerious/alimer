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

#include "../Math/Color.h"
#include <cstdio>

namespace Alimer
{
    const Color Color::Black = { 0.0f, 0.0f, 0.0f, 1.0f };
    const Color Color::White = { 1.0f, 1.0f, 1.0f, 1.0f };
    const Color Color::WhiteSmoke = { 0.960784376f, 0.960784376f, 0.960784376f, 1.0f };
    const Color Color::Red = { 1.000000000f, 0.000000000f, 0.000000000f, 1.000000000f };
    const Color Color::Green = { 0.0f, 0.501960814f, 0.0f, 1.0f };
    const Color Color::Blue = { 0.0f, 0.0f, 1.0f, 1.0f };
    const Color Color::Yellow = { 1.0f, 1.0f, 0.0f, 1.0f };
    const Color Color::YellowGreen = { 0.603921592f, 0.803921640f, 0.196078449f, 1.0f };
    const Color Color::Lime = { 0.000000000f, 1.000000000f, 0.000000000f, 1.000000000f };

    std::string Color::ToString() const
    {
        char tempBuffer[128];
        sprintf(tempBuffer, "%g %g %g %g", r, g, b, a);
        return std::string(tempBuffer);
    }
}
