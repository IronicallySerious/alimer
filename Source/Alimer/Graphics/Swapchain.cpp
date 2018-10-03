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

#include "../Graphics/Swapchain.h"
#include "../Graphics/GraphicsDevice.h"
#include "../Graphics/GraphicsImpl.h"
#include "../Core/Log.h"

namespace Alimer
{
    Swapchain::Swapchain()
	{
        _graphics = Object::GetSubsystem<GraphicsDevice>();
	}

    Swapchain::~Swapchain()
    {
        for (uint32_t i = 0; i < _textureCount; i++)
        {
            //vkDestroyImageView(device, buffers[i].view, nullptr);
        }

        SafeDelete(_impl);
    }

    bool Swapchain::Define(void* windowHandle, const uvec2& size)
    {
        SafeDelete(_impl);

        if (!size.x || !size.y)
        {
            ALIMER_LOGERROR("Can not define Swapchain with invalid width or height.");
            return false;
        }

        if (!windowHandle)
        {
            ALIMER_LOGERROR("Can not define Swapchain with invalid window handle.");
            return false;
        }

        _size = size;

        if (_graphics && _graphics->IsInitialized())
        {
            _impl = _graphics->GetImplementation()->CreateSwapchain(windowHandle, _size);
            if (!_impl)
            {
                ALIMER_LOGERROR("Failed to create backend Swapchain");
                return false;
            }

            ALIMER_LOGDEBUG("Swapchain defined with {%u, %u}", size.x, size.y);
            _textureCount = _impl->GetTextureCount();
            _format = _impl->GetFormat();
        }

        return true;
    }

    Texture* Swapchain::GetNextTexture()
    {
        if (!_impl->AcquireNextTexture(&_textureIndex))
        {
            ALIMER_LOGERROR("Failed to acquire Swapchain texture");
            return nullptr;
        }

        return _textures[_textureIndex];
    }

    void Swapchain::Present()
    {
        _impl->Present();
    }
}
