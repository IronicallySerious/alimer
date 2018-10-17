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

namespace Alimer
{
	/// Defines pixel format.
	enum class PixelFormat : uint32_t
	{
		Undefined,
		R8UNorm,
		RG8UNorm,
		RGBA8UNorm,
		BGRA8UNorm,

        Depth16UNorm,
        Depth32Float,
        Depth24UNormStencil8,
        Depth32FloatStencil8,

        BC1,
        BC2,
        BC3,
        BC4UNorm,
        BC4SNorm,
        BC5UNorm,
        BC5SNorm,

        /// Compressed format with four floating-point components.
        BC6HSFloat,

        /// Compressed format with four unsigned floating-point components.
        BC6HUFloat,
	};

    ALIMER_API const char* EnumToString(PixelFormat format);

    /// Checks if given format is depth.
    ALIMER_API bool IsDepthFormat(PixelFormat format);

    /// Checks if given format is stencil.
    ALIMER_API bool IsStencilFormat(PixelFormat format);

    /// Checks if given format is depth-stencil.
    ALIMER_API bool IsDepthStencilFormat(PixelFormat format);

    /// Checks if given format is compressed.
    ALIMER_API bool IsCompressed(PixelFormat format);

    ALIMER_API uint32_t GetPixelFormatSize(PixelFormat format);

    /// Calculate the data size of an image level.
    ALIMER_API uint32_t CalculateDataSize(uint32_t width, uint32_t height, PixelFormat format, uint32_t* numRows = 0, uint32_t* rowPitch = 0);
}
