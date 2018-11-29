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

#include "../Graphics/Graphics.h"

namespace Alimer
{
    class TextureImpl
    {
    protected:
        /// Constructor.
        TextureImpl() {}

    public:
        ~TextureImpl() = default;

        TextureType type;
        TextureUsage usage;
        PixelFormat format;
        uint32_t width;
        uint32_t height;
        uint32_t depth;
        uint32_t mipLevels;
        uint32_t arrayLayers;
        SampleCount samples;
    };

    class FramebufferImpl
    {
    protected:
        /// Constructor.
        FramebufferImpl() {}

    public:
        ~FramebufferImpl() = default;
    };

    class GpuBufferImpl
    {
    protected:
        /// Constructor.
        GpuBufferImpl() {}

    public:
        ~GpuBufferImpl() = default;
    };

    class GraphicsImpl
    {
    protected:
        /// Constructor.
        GraphicsImpl() {}

    public:
        ~GraphicsImpl() = default;

        virtual bool Initialize(const GraphicsSettings& settings) = 0;

        virtual bool WaitIdle() = 0;
        virtual void Frame() = 0;

        virtual CommandContext* AllocateContext() = 0;

        virtual Framebuffer* GetSwapchainFramebuffer() const = 0;
        virtual FramebufferImpl* CreateFramebuffer(const Vector<FramebufferAttachment>& colorAttachments) = 0;
        virtual GpuBufferImpl* CreateBuffer(const BufferDescriptor* descriptor, const void* initialData, void* externalHandle) = 0;

        GraphicsDeviceFeatures features = {};
    };
}
