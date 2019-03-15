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
#include "foundation/Vector.h"
#include "foundation/UniquePtr.h"

namespace alimer
{
    class Window;
    class ApplicationHost;
    class GraphicsDevice;
    class GraphicsDeviceFactory;

    /// Base class for creating applications which initialize the engine and run a main loop until exited.
    class ALIMER_API Application : public Object
    {
        friend class ApplicationHost;

        ALIMER_OBJECT(Application, Object);

    protected:
        Application(int argc, char** argv);

    public:
        /// Destructor.
        ~Application() override;

        /// Get the current active application.
        static Application* Current();

        /// Initialize all sub systems, graphics device, input, audio and run the main loop, then return the application exit code.
        int Run();

        /// Request the application to exit.
        void RequestExit();

        /// Tick one rendering frame.
        void Tick();

        /// Get the main window.
        Window* GetMainWindow() const { return _mainWindow.Get(); }

    protected:
        /// Setup before engine initialization. This is a chance to eg. modify the engine parameters. Call ErrorExit() to terminate without initializing the engine.
        virtual void Setup() { }

        /// Setup after engine initialization and before running the main loop. Call ErrorExit() to terminate without running the main loop.
        virtual void Start() { }

        /// Cleanup after the main loop. Called by Application.
        virtual void Stop() { }

        /// Show an error message (last log message if empty), terminate the main loop, and set failure exit code.
        void ErrorExit(const String& message = "");

    private:
        /// Called by ApplicationHost to setup everything before running main loop.
        void InitializeBeforeRun();

    protected:
        /// Command line arguments.
        Vector<String> _args;

        /// Per platform application host.
        UniquePtr<ApplicationHost> _host;

        /// Main window.
        UniquePtr<Window> _mainWindow;

        /// GraphicsDevice factory.
        UniquePtr<GraphicsDeviceFactory> _graphicsDeviceFactory;
        UniquePtr<GraphicsDevice> _graphicsDevice;

        /// Application exit code.
        int _exitCode;
    };
}