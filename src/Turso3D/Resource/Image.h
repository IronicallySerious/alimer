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
#include "../Graphics/PixelFormat.h"
#include "Resource.h"

namespace Turso3D
{
    /// Description of image mip level data.
    struct TURSO3D_API ImageLevel
    {
        /// Default construct.
        ImageLevel() :
            data(nullptr),
            size(IntVector2::ZERO),
            rowSize(0),
            rows(0)
        {
        }

        /// Pointer to pixel data.
        uint8_t* data;
        /// Level size in pixels.
        IntVector2 size;
        /// Row size in bytes.
        size_t rowSize;
        /// Number of rows.
        size_t rows;
    };

    /// %Image resource.
    class TURSO3D_API Image : public Resource
    {
        OBJECT(Image);

    public:
        /// Construct.
        Image();
        /// Destruct.
        ~Image();

        /// Register object factory.
        static void RegisterObject();

        /// Set new image pixel dimensions and format. Setting a compressed format is not supported.
        void SetSize(const IntVector2& newSize, ImageFormat newFormat);
        /// Set new pixel data.
        void SetData(const uint8_t* pixelData);

        /// Load image from a stream. Return true on success.
        bool BeginLoad(Stream& source) override;
        /// Save the image to a stream. Regardless of original format, the image is saved as png. Compressed image data is not supported. Return true on success.
        bool Save(Stream& dest) override;

        /// Save in PNG format to file name. Return true if successful.
        bool SavePNG(const String& fileName) const;

        /// Save in PNG format to stream. Return true if successful.
        bool SavePNG(Stream& dest) const;

        /// Return image dimensions in pixels.
        const IntVector2& Size() const { return size; }
        /// Return image width in pixels.
        int Width() const { return size.x; }
        /// Return image height in pixels.
        int Height() const { return size.y; }
        /// Return number of components in a pixel. Will return 0 for formats which are not 8 bits per pixel.
        int Components() const { return components[format]; }
        /// Return byte size of a pixel. Will return 0 for block compressed formats.
        size_t PixelByteSize() const { return pixelByteSizes[format]; }
        /// Return pixel data.
        uint8_t* Data() const { return data.Get(); }
        /// Return the image format.
        ImageFormat Format() const { return format; }
        /// Return whether is a compressed image.
        bool IsCompressed() const { return format >= FMT_DXT1; }
        /// Return number of mip levels contained in the image data.
        size_t NumLevels() const { return numLevels; }
        /// Calculate the next mip image with halved width and height. Supports uncompressed 8 bits per pixel images only. Return true on success.
        bool GenerateMipImage(Image& dest) const;
        /// Return the data for a mip level. Images loaded from eg. PNG or JPG formats will only have one (index 0) level.
        ImageLevel Level(size_t index) const;
        /// Decompress a mip level as 8-bit RGBA. Supports compressed images only. Return true on success.
        bool DecompressLevel(uint8_t* dest, size_t levelIndex) const;

        /// Calculate the data size of an image level.
        static size_t CalculateDataSize(const IntVector2& size, ImageFormat format, size_t* numRows = 0, size_t* rowSize = 0);

        /// Pixel components per format.
        static const int components[];
        /// Pixel byte sizes per format.
        static const size_t pixelByteSizes[];

    private:
        /// Decode image pixel data using the stb_image library.
        static uint8_t* DecodePixelData(Stream& source, int& width, int& height, unsigned& components);
        /// Free the decoded pixel data.
        static void FreePixelData(uint8_t* pixelData);
        /// Check if image can be saved and return number of components, 0 failure.
        int CanSave() const;


        /// Image dimensions.
        IntVector2 size;
        /// Image format.
        ImageFormat format;
        /// Number of mip levels. 1 for uncompressed images.
        size_t numLevels;
        /// Image pixel data.
        AutoArrayPtr<uint8_t> data;
    };

}
