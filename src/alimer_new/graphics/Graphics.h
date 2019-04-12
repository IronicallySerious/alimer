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

#include "core/Object.h"
#include "graphics/CommandQueue.h"
#include "math/vec2.h"

namespace alimer
{
    class Window;
    class GraphicsImpl;

    class ALIMER_API Graphics final : public Object
    {
        ALIMER_OBJECT(Graphics, Object);

    public:
        /// Constructor.
        Graphics();

        /// Destructor.
        ~Graphics();

        /// Set graphics mode. Create the window and rendering context if not created yet. Return true on success.
        bool SetMode(const std::string& title, const math::uint2& size, bool resizable = false, bool fullscreen = false, uint32_t multisample = 1);

        bool BeginFrame();
        uint32_t EndFrame();

        /// Return whether graphics backend has been initialized.
        bool IsInitialized() const;

        /// Get the device info.
        const GraphicsDeviceInfo& GetInfo() const;

        /// Get the device capabilities.
        const GraphicsDeviceCapabilities& GetCaps() const;

        /// Return the rendering window.
        Window* GetRenderWindow() const;

    private:
        GraphicsImpl* _impl = nullptr;
        /// Main OS rendering window.
        Window* _renderWindow = nullptr;

        uint32_t _sampleCount = 1;
        uint32_t _frameCount = 0;

    protected:
        std::shared_ptr<CommandQueue> _directCommandQueue;
        std::shared_ptr<CommandQueue> _computeCommandQueue;
        std::shared_ptr<CommandQueue> _copyCommandQueue;
    };
}
