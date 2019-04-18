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

#include "../Debug/Log.h"
#include "../Debug/Profiler.h"
#include "Texture.h"

#include "../Debug/DebugNew.h"

namespace Turso3D
{
    void Texture::RegisterObject()
    {
        RegisterFactory<Texture>();
    }

    bool Texture::BeginLoad(Stream& source)
    {
        loadImages.Clear();
        loadImages.Push(new Image());
        if (!loadImages[0]->Load(source))
        {
            loadImages.Clear();
            return false;
        }

        // If image uses unsupported format, decompress to RGBA now
        if (loadImages[0]->Format() >= FMT_ETC1)
        {
            Image* rgbaImage = new Image();
            rgbaImage->SetSize(loadImages[0]->Size(), FMT_RGBA8);
            loadImages[0]->DecompressLevel(rgbaImage->Data(), 0);
            loadImages[0] = rgbaImage; // This destroys the original compressed image
        }

        // Construct mip levels now if image is uncompressed
        if (!loadImages[0]->IsCompressed())
        {
            Image* mipImage = loadImages[0];

            while (mipImage->Width() > 1 || mipImage->Height() > 1)
            {
                loadImages.Push(new Image());
                mipImage->GenerateMipImage(*loadImages.Back());
                mipImage = loadImages.Back();
            }
        }

        return true;
    }

    bool Texture::EndLoad()
    {
        if (loadImages.IsEmpty())
            return false;

        Vector<ImageLevel> initialData;

        for (size_t i = 0; i < loadImages.Size(); ++i)
        {
            for (size_t j = 0; j < loadImages[i]->NumLevels(); ++j)
                initialData.Push(loadImages[i]->Level(j));
        }

        Image* image = loadImages[0];
        bool success = Define(TextureType::Type2D, ResourceUsage::Immutable, image->Size(), image->Format(), initialData.Size(), &initialData[0]);
        /// \todo Read a parameter file for the sampling parameters
        success &= DefineSampler(FILTER_TRILINEAR, ADDRESS_WRAP, ADDRESS_WRAP, ADDRESS_WRAP);

        loadImages.Clear();
        return success;
    }

    size_t Texture::NumFaces() const
    {
        return type == TextureType::TypeCube ? MAX_CUBE_FACES : 1;
    }
}
