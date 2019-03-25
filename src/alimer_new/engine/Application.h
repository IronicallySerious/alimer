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
#include <memory>
#include <string>
#include <vector>

namespace alimer
{
    class Window;
    class ApplicationHost;
    class GraphicsDevice;

    class ApplicationConfiguration
    {
    public:
        /// Main application window title.
        std::string title = "alimer";
        /// Main application window width.
        uint32_t width = 640;
        /// Main application window height.
        uint32_t height = 480;
        /// Whether main application window is resizable.
        bool resizable = true;
        /// Whether main application window is fullscreen.
        bool fullscreen = false;
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
        static Application* getCurrent();

        /// Initialize all sub systems, graphics device, input, audio and run the main loop, then return the application exit code.
        int run();

        /// Request the application to exit.
        void requestExit();

        /// Tick one rendering frame.
        void tick();

        /// Return whether application has been initialized.
        bool isInitialized() const { return _initialized; }

        /// Get the main window.
        Window* getMainWindow() const { return _mainWindow.get(); }

    protected:
        /// Setup before engine initialization. This is a chance to eg. modify the engine parameters. Call ErrorExit() to terminate without initializing the engine.
        virtual void setup() { }

        /// Setup after engine initialization and before running the main loop. Call ErrorExit() to terminate without running the main loop.
        virtual void initialize() { }

        /// Cleanup after the main loop. Called by Application.
        virtual void stop() { }

        /// Show an error message (last log message if empty), terminate the main loop, and set failure exit code.
        void errorExit(const std::string& message = "");

    private:
        /// Called by ApplicationHost to setup everything before running main loop.
        void initializeBeforeRun();

        /// Perform frame rendering.
        void render();

    protected:
        /// Configuration
        ApplicationConfiguration _config;

        /// Command line arguments.
        //std::vector<std::string> _args;

        /// Per platform application host.
        ApplicationHost* _host;

        /// Main window.
        std::shared_ptr<Window> _mainWindow;

        /// GraphicsDevice.
        std::shared_ptr<GraphicsDevice> _graphics = nullptr;

        bool _headless = false;
        bool _initialized = false;
        bool _exiting = false;

        /// Application exit code.
        int _exitCode;
    };
}
