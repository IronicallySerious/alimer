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

#include "Graphics/Graphics.h"
#include "../Core/Log.h"

#if ALIMER_D3D12
#include "Graphics/D3D12/D3D12Graphics.h"
#endif

namespace Alimer
{
	Graphics::Graphics()
		: _window(nullptr)
	{
	}

	Graphics::~Graphics()
	{
		_textures.clear();
	}

	Graphics* Graphics::Create(GraphicsDeviceType deviceType)
	{
		if (deviceType == GraphicsDeviceType::Default)
		{
			// TODO: Find best device type
			deviceType = GraphicsDeviceType::Direct3D12;
		}

		Graphics* graphics = nullptr;
		switch (deviceType)
		{
			case GraphicsDeviceType::Direct3D12:
#if ALIMER_D3D12
				ALIMER_LOGINFO("Using Direct3D 12 graphics backend");
				graphics = new D3D12Graphics();
#else
				ALIMER_LOGERROR("Direct3D 12 graphics backend not supported");
#endif
				break;

			case GraphicsDeviceType::Default:
				break;

			case GraphicsDeviceType::Empty:
			default:
				break;
		}

		return graphics;
	}

	bool Graphics::Initialize(std::shared_ptr<Window> window)
	{
		_window = window;
		return true;
	}
}
