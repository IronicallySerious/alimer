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

#include "../Graphics/Types.h"
#include "../Graphics/PixelFormat.h"
#include "../Resource/Resource.h"

namespace Alimer
{
	class Graphics;

	/// Defines a Texture class.
	class Texture : public Resource
	{
	protected:
		/// Constructor.
		Texture(Graphics* graphics);

	public:
		/// Destructor.
		virtual ~Texture();

		inline TextureType GetTextureType() const { return _textureType; }
		inline PixelFormat GetFormat() const { return _format; }
		inline uint32_t GetWidth() const { return _width; }
		inline uint32_t GetHeight() const { return _height; }
		inline uint32_t GetDepth() const { return _depth; }
		inline uint32_t GetMipLevels() const { return _mipLevels; }
		inline uint32_t GetArrayLayers() const { return _arrayLayers; }
		inline SampleCount GetSamples() const { return _samples; }
		inline TextureUsage GetUsage() const { return _usage; }

	protected:
		Graphics* _graphics;
		TextureType _textureType;
		PixelFormat _format;
		uint32_t _width;
		uint32_t _height;
		uint32_t _depth;
		uint32_t _mipLevels;
		uint32_t _arrayLayers;
		SampleCount _samples;
		TextureUsage _usage;

	private:
		DISALLOW_COPY_MOVE_AND_ASSIGN(Texture);
	};
}
