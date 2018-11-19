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

namespace Alimer
{
    struct FramebufferAttachment
    {
        Texture* texture = nullptr;
        uint32_t mipLevel = 0;
        uint32_t slice = 0;
    };

    struct FramebufferDescriptor
    {
        FramebufferAttachment colorAttachments[MaxColorAttachments];
        FramebufferAttachment depthStencilAttachment;
    };

    /// Defines a Framebuffer class.
    class ALIMER_API Framebuffer : public GraphicsResource
    {
    protected:
        /// Constructor.
        Framebuffer(Graphics* device, const FramebufferDescriptor* descriptor);

    public:
        /**
        * Attach a color texture.
        *
        * @param colorTexture The color texture.
        * @param index The render-target index to attach the texture to.
        * @param mipLevel The selected mip-level to attach.
        * @param firstArraySlice The first array slice to attach.
        * @param arraySize The number of array sliced to attach or RemainingArrayLayers for range [firstArraySlice, texture->GetArraySize()]
        */
        void AttachColorTarget(const SharedPtr<Texture>& colorTexture, uint32_t index, uint32_t mipLevel = 0, uint32_t firstArraySlice = 0, uint32_t arraySize = RemainingArrayLayers);

        /**
        * Attach a depth-stencil texture.
        *
        * @param depthStencil The depth-stencil texture.
        * @param mipLevel The selected mip-level to attach.
        * @param firstArraySlice The first array slice to attach.
        * @param arraySize The number of array sliced to attach or RemainingArrayLayers for range [firstArraySlice, texture->GetArraySize()]
        */
        void AttachDepthStencilTarget(const SharedPtr<Texture>& depthStencil, uint32_t mipLevel = 0, uint32_t firstArraySlice = 0, uint32_t arraySize = RemainingArrayLayers);

        /**
        * Get an attached color texture. If no texture is attached will return nullptr.
        */
        const Texture* GetColorTexture(uint32_t index) const;

        /**
        * Get the attached depth-stencil texture, or nullptr if no texture is attached.
        */
        const Texture* GetDepthStencilTexture() const;

        /**
        * Get the width of the framebuffer.
        */
        uint32_t GetWidth() const { return _width; }

        /**
        * Get the height of the framebuffer.
        */
        uint32_t GetHeight() const { return _height; }

        /**
        * Get the layers of the framebuffer.
        */
        uint32_t GetLayers() const { return _layers; }

    protected:
        std::vector<FramebufferAttachment> _colorAttachments;
        FramebufferAttachment _depthStencilAttachment;

        uint32_t _width = 0;
        uint32_t _height = 0;
        uint32_t _layers = 1;
    };
}
