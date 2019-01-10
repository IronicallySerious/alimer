//
// Copyright (c) 2017-2019 Amer Koleci and contributors.
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

#include "../Application/Window.h"
#include "../Graphics/Framebuffer.h"

namespace alimer
{
    struct ALIMER_API RenderWindowDescriptor
    {
        String title = "Alimer";
        uvec2 size = { 800, 600 };
        WindowFlags windowFlags = WindowFlags::Default;

        /// sRGB color space.
        bool sRGB  = false;

        /// Preferred depth stencil format.
        PixelFormat preferredDepthStencilFormat = PixelFormat::D24UNormS8;

        /// Preferred samples.
        SampleCount preferredSamples = SampleCount::Count1;
    };

    /// Defines a RenderWindow class.
    class ALIMER_API RenderWindow final : public Window
    {
        friend class GPUDevice;
        ALIMER_OBJECT(RenderWindow, Window);

    public:
        /// Constructor.
        RenderWindow(const RenderWindowDescriptor* descriptor);

        /// Desturctor
        ~RenderWindow() override;

        void Destroy();

        /// Swaps the frame buffers to display the next frame. 
        void SwapBuffers();

        /// Get the current framebuffer.
        Framebuffer* GetCurrentFramebuffer() const;

    private:
        
        SharedPtr<Texture2D> _depthStencilTexture;
        Vector<SharedPtr<Framebuffer>> _framebuffers;
    };
}
