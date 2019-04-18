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

#include "Turso3DConfig.h"

namespace Turso3D
{
    /// Image formats.
    enum ImageFormat
    {
        FMT_NONE = 0,
        FMT_R8,
        FMT_RG8,
        FMT_RGBA8,
        FMT_A8,
        FMT_R16,
        FMT_RG16,
        FMT_RGBA16,
        FMT_R16F,
        FMT_RG16F,
        FMT_RGBA16F,
        FMT_R32F,
        FMT_RG32F,
        FMT_RGB32F,
        FMT_RGBA32F,
        FMT_D16,
        FMT_D32,
        FMT_D24S8,
        FMT_DXT1,
        FMT_DXT3,
        FMT_DXT5,
        FMT_ETC1,
        FMT_PVRTC_RGB_2BPP,
        FMT_PVRTC_RGBA_2BPP,
        FMT_PVRTC_RGB_4BPP,
        FMT_PVRTC_RGBA_4BPP
    };
}
