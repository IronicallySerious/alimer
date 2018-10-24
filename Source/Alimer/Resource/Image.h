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

#include "../Math/Math.h"
#include "../Resource/Resource.h"
#include "../Graphics/PixelFormat.h"

namespace Alimer
{
    /// Description of image mip level data.
    struct ALIMER_API ImageLevel
    {
        /// Pointer to pixel data.
        const void* data;

        /// Single row pitch.
        uint32_t rowPitch = 0;
    };

    enum class ImageFormat : uint32_t
    {
        Bmp,
        Png,
    };

	/// Defines an Image resource.
	class ALIMER_API Image final : public Resource
	{
        ALIMER_OBJECT(Image, Resource);

	public:
		/// Constructor.
        Image();

        /// Destructor.
        ~Image() = default;

        /// Set new image pixel dimensions and format. Setting a compressed format is not supported.
        void Define(const uvec2& newSize, PixelFormat newFormat);

        /// Set new pixel data.
        void SetData(const uint8_t* pixelData);

        /// Save the image to a stream in given format.
        bool Save(Stream* dest, ImageFormat format) const;

        /// Return image dimensions in pixels.
        const uvec2& GetSize() const { return _size; }
        /// Return image width in pixels.
        uint32_t GetWidth() const { return _size.x; }
        /// Return image height in pixels.
        uint32_t GetHeight() const { return _size.y; }

        /// Return pixel data.
        uint8_t* Data() const { return _data.Get(); }

    private:
        bool SaveBmp(Stream* dest) const;
        bool SavePng(Stream* dest) const;

        /// Image dimensions.
        uvec2 _size;
        /// Image format.
        PixelFormat _format = PixelFormat::Unknown;
        /// Number of mip levels. 1 for uncompressed images.
        uint32_t _mipLevels = 1;
        /// Image pixel data.
        AutoArrayPtr<uint8_t> _data;

        /// Memory size.
        size_t _memorySize = 0;
	};
}
