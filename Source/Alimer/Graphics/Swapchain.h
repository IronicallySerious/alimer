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

#include "../Graphics/Texture.h"
#include "../Math/Math.h"

namespace Alimer
{
    class SwapchainImpl;
    class GraphicsDevice;

    /// Defines a Swapchain class.
    class ALIMER_API Swapchain final
    {
    public:
        /// Constructor.
        Swapchain();

        /// Destructor.
        ~Swapchain();

        /// Native window handle.
        /// Define Swapchain. Return true on success.
        bool Define(void* windowHandle, const uvec2& size);

        /// Return image dimensions in pixels.
        const uvec2& GetSize() const { return _size; }
        uint32_t GetWidth() const { return _size.x; }
        uint32_t GetHeight() const { return _size.y; }
        PixelFormat GetFormat() const { return _format; }

        /// Get backend implementation.
        SwapchainImpl *GetImplementation() const { return _impl; }

    private:
        /// Graphics subsystem pointer.
        WeakPtr<GraphicsDevice> _graphics;

        /// Implementation.
        SwapchainImpl* _impl = nullptr;

        uvec2 _size;
        PixelFormat _format = PixelFormat::Undefined;
        uint32_t _textureCount = 0;
        uint32_t _textureIndex = 0;
        std::vector<SharedPtr<Texture>> _textures;
    };
}
