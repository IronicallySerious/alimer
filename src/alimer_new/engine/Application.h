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
#include "graphics/Types.h"
#include "math/vec2.h"
#include <memory>
#include <string>

namespace alimer
{
    class ApplicationHost;
    class Graphics;

    class ApplicationConfiguration
    {
    public:
        /// Preffered graphics backend.
        GpuPowerPreference preferredPowerPreference = GpuPowerPreference::Default;

        /// Main application window title.
        std::string title = "alimer";
        /// Main application window size.
        math::uint2 size = { 640u, 480u };
        /// Whether main application window is resizable.
        bool resizable = true;
        /// Whether main application window is fullscreen.
        bool fullscreen = false;
        /// Multisample count for graphics backend.
        uint32_t sampleCount = 1u;
        /// Whether to disable audio.
        bool disableAudio;
    };

    /// Base class for creating applications which initialize the engine and run a main loop until exited.
    class ALIMER_API Application : public Object
    {
        friend class ApplicationHost;

        ALIMER_OBJECT(Application, Object);

    protected:
        Application(const ApplicationConfiguration& config);

    public:
        /// Destructor.
        ~Application() override;

        /// Get the current active application.
        static Application* GetCurrent();

        /// Initialize all sub systems, graphics device, input, audio and run the main loop, then return the application exit code.
        int Run();

        /// Request the application to exit.
        void RequestExit();

        /// Tick one rendering frame.
        void Tick();

        /// Return whether application has been initialized.
        bool IsInitialized() const { return _initialized; }

        /// Return graphics subsystem.
        inline Graphics* GetGraphics() const { return _graphics; }

    protected:
        /// Setup after engine initialization and before running the main loop. Call ErrorExit() to terminate without running the main loop.
        virtual void Initialize() { }

        /// Cleanup after the main loop. Called by Application.
        virtual void Stop() { }

        /// Show an error message (last log message if empty), terminate the main loop, and set failure exit code.
        void ErrorExit(const std::string& message = "");

    private:
        /// Called by ApplicationHost to setup everything before running main loop.
        void InitializeBeforeRun();

        /// Perform frame rendering.
        void Render();

    protected:
        /// Configuration
        ApplicationConfiguration _config;

        /// Per platform application host.
        ApplicationHost* _host;

        /// GraphicsDevice.
        SharedPtr<Graphics> _graphics;

        bool _headless = false;
        bool _initialized = false;
        bool _exiting = false;

        /// Application exit code.
        int _exitCode;
    };
}
