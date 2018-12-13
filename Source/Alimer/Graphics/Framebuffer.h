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
        Texture* texture;
        uint32_t baseMipLevel;
        uint32_t baseArrayLayer;
        uint32_t layerCount;
    };

    struct FramebufferDescriptor
    {
        FramebufferAttachment colorAttachments[MaxColorAttachments];
        FramebufferAttachment depthStencilAttachment;
    };

    /// Defines a Framebuffer class.
    class ALIMER_API Framebuffer : public GraphicsResource
    {
    public:
        /// Constructor.
        Framebuffer(Graphics* graphics);

        /// Set color attachment at index.
        void SetColorAttachment(uint32_t index, Texture* colorTexture, uint32_t baseMipLevel = 0, uint32_t baseArrayLayer = 0, uint32_t layerCount = RemainingArrayLayers);

        /// Set framebuffer depth stencil attachment.
        void SetDepthStencilAttachment(Texture* depthStencilTexture, uint32_t baseMipLevel = 0, uint32_t baseArrayLayer = 0, uint32_t layerCount = RemainingArrayLayers);

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

    private:
        virtual void ApplyColorAttachment(uint32_t index) = 0;
        virtual void ApplyDepthStencilAttachment() = 0;
        bool Create();

    protected:
        PODVector<FramebufferAttachment> _colorAttachments;
        FramebufferAttachment _depthStencilAttachment;

        uint32_t _width = 0;
        uint32_t _height = 0;
        uint32_t _layers = 1;
    };
}
