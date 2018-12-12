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

#include "../Base/Vector.h"
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

    class FramebufferImpl;

    /// Defines a Framebuffer class.
    class ALIMER_API Framebuffer : public GraphicsResource
    {
    public:
        /// Constructor.
        Framebuffer(Graphics* graphics);

        /// Constructor.
        Framebuffer(FramebufferImpl* impl);

        /// Desturctor
        ~Framebuffer() override;

        bool Define(const FramebufferDescriptor* descriptor);

        /// Get the number of color attachments.
        uint32_t GetColorAttachmentsCount() const { return _colorAttachments.Size(); }

        /// Get an attached color texture or null if no texture is attached.
        const Texture* GetColorTexture(uint32_t index) const;

        /// Get is framebuffer has depth stencil attachments
        bool HasDepthStencilAttachment() const;

        ///* Get the attached depth-stencil texture or null if no texture is attached.
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

        /// Return backend implementation, which holds the actual API-specific resources.
        FramebufferImpl* GetImpl() const { return _impl; }

    private:
        bool Create();
        void Destroy() override;

        FramebufferImpl* _impl;

        Vector<FramebufferAttachment> _colorAttachments;
        FramebufferAttachment _depthStencilAttachment;

        uint32_t _width = 0;
        uint32_t _height = 0;
        uint32_t _layers = 1;
    };
}
