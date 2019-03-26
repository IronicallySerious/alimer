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

#include "engine/Window.h"
#include "graphics/Types.h"

namespace alimer
{
    class SwapChain;

    class ALIMER_API GraphicsDevice
    {
    protected:
        /// Constructor.
        GraphicsDevice();

    public:
        /// Destructor.
        virtual ~GraphicsDevice() = default;

        static std::shared_ptr<GraphicsDevice> Create(const std::shared_ptr<Window>& window, const GraphicsDeviceDescriptor& desc);

        /// Begin next frame rendering.
        virtual bool BeginFrame();

        virtual void EndFrame();

        /// Get the device info.
        const GraphicsDeviceInfo& GetInfo() const;

        /// Get the device capabilities.
        const GraphicsDeviceCapabilities& GetCaps() const;

        /// Return whether graphics device has been initialized.
        bool IsInitialized() const { return _initialized; }

    protected:
        virtual bool Initialize(const std::shared_ptr<Window>& window, const GraphicsDeviceDescriptor& desc);

        /// Initialization state.
        bool _initialized = false;
        /// The main window.
        std::shared_ptr<Window> _window;
        /// The creation description.
        GraphicsDeviceDescriptor _descriptor = {};
        /// The main swap chain.
        std::unique_ptr<SwapChain> _swapChain = {};
        
        GraphicsDeviceInfo _info = {};
        GraphicsDeviceCapabilities  _caps = {};
    };

    extern ALIMER_API std::shared_ptr<GraphicsDevice> graphicsDevice;
}
