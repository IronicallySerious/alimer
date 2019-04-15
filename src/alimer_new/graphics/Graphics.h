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
#include "engine/Configuration.h"

namespace alimer
{
    class Window;

    class ALIMER_API Graphics : public Object
    {
        ALIMER_OBJECT(Graphics, Object);

    protected:
        /// Constructor.
        Graphics(const ApplicationConfiguration& config);

    public:
        /// Destructor.
        ~Graphics();

        static Graphics* Create(const ApplicationConfiguration& config);

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

    protected:
        virtual void Shutdown();
        virtual bool BeginFrameImpl() { return true; }
        virtual void CommitFrame() {}

    protected:
        GraphicsDeviceInfo                                  _info = {};
        GraphicsDeviceCapabilities                          _caps = {};

        /// Main OS rendering window.
        Window* _renderWindow = nullptr;

        bool _initialized = false;

        uint32_t _sampleCount = 1;
        uint32_t _frameCount = 0;
    };
}
