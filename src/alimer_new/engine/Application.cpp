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

#include "engine/Application.h"
#include "engine/ApplicationHost.h"
#include "engine/Window.h"
#include "graphics/GraphicsDevice.h"
#include "graphics/GraphicsDeviceFactory.h"

namespace alimer
{
    static Application* s_currentApplication = nullptr;
    Application::Application(int argc, char** argv)
        : _exitCode(EXIT_SUCCESS)
    {
        for (int i = 0; i < argc; ++i)
        {
            _args.Push(argv[i]);
        }

        _host.Reset(ApplicationHost::Create(this));
        s_currentApplication = this;
    }

    Application::~Application()
    {
        _host = nullptr;
        s_currentApplication = nullptr;
    }

    Application* Application::Current()
    {
        return s_currentApplication;
    }

    int Application::Run()
    {
#if !defined(__GNUC__) || __EXCEPTIONS
        try
        {
#endif
            Setup();
            if (_exitCode) {
                return _exitCode;
            }

            // Create main window
            _mainWindow = _host->CreateWindow("alimer", 800, 600, true, false);

            _graphicsDeviceFactory = GraphicsDeviceFactory::Create(GraphicsBackend::Default, true);
            _host->Run();

            /*if (!_engine->Initialize(_args))
            {
                ErrorExit();
                return _exitCode;
            }*/

            Start();
            if (_exitCode) {
                return _exitCode;
            }

#if ALIMER_GLFW
            
#else
#endif

            return _exitCode;

#if !defined(__GNUC__) || __EXCEPTIONS
        }
        catch (std::bad_alloc&)
        {
            _host->ErrorDialog(GetTypeName(), "An out-of-memory error occurred. The application will now exit.");
            return EXIT_FAILURE;
        }
#endif
    }

    void Application::RequestExit()
    {
        _host->RequestExit();
    }

    void Application::InitializeBeforeRun()
    {
        AdapterDescriptor adapterDescription = {};
        _graphicsDevice = _graphicsDeviceFactory->CreateDevice(&adapterDescription);
    }

    void Application::Tick()
    {

    }

    void Application::ErrorExit(const String& message)
    {
        _host->RequestExit();
        _exitCode = EXIT_FAILURE;

        if (message.empty())
        {
            //_host->ErrorDialog(GetTypeName(), startupErrors_.Length() ? startupErrors_ : "Application has been terminated due to unexpected error.");
        }
        else
        {
            _host->ErrorDialog(GetTypeName(), message);
        }
    }

}
