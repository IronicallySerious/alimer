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
#include "graphics/Graphics.h"
#include "graphics/SwapChain.h"
#include "foundation/Utils.h"
#if defined(_WIN32) || defined(_WIN64)
#undef CreateWindow
#endif
using namespace std;

namespace alimer
{
    static Application* s_currentApplication = nullptr;
    Application::Application(const ApplicationConfiguration& config)
        : _config(config)
        , _exitCode(EXIT_SUCCESS)
    {
        _host = createPlatformHost(this);
        s_currentApplication = this;
    }

    Application::~Application()
    {
        _mainWindowSwapChain.reset();
        SafeDelete(_host);
        s_currentApplication = nullptr;
    }

    Application* Application::GetCurrent()
    {
        ALIMER_ASSERT_MSG(s_currentApplication, "Application has not been initialized");
        return s_currentApplication;
    }

    int Application::Run()
    {
#if !defined(__GNUC__) || __EXCEPTIONS
        try
        {
#endif
            _exitCode = _host->run();
            return _exitCode;

#if !defined(__GNUC__) || __EXCEPTIONS
        }
        catch (std::bad_alloc&)
        {
            _host->errorDialog(GetTypeName(), "An out-of-memory error occurred. The application will now exit.");
            return EXIT_FAILURE;
        }
#endif
    }

    void Application::RequestExit()
    {
        _host->requestExit();
    }

    void Application::InitializeBeforeRun()
    {
        if (_initialized)
        {
            return;
        }

        // Create graphics device.
        _graphics.reset(new Graphics());

        // Create main window.
        _mainWindow = _host->createWindow(_config.title,
            _config.width, _config.height,
            _config.resizable, _config.fullscreen);

        // Create swap chain from window.
        //_mainWindowSwapChain.reset(_graphics->CreateSwapChain(_mainWindow.get()));

        _initialized = true;
        Initialize();
        if (_exitCode) {
            ErrorExit();
        }
    }

    void Application::Tick()
    {
        ALIMER_ASSERT(_initialized);

        if (_exiting)
            return;

        // TODO: Apply frame time.
        Render();
    }

    void Application::ErrorExit(const std::string& message)
    {
        _host->requestExit();
        _exitCode = EXIT_FAILURE;

        if (message.empty())
        {
            //_host->ErrorDialog(GetTypeName(), startupErrors_.Length() ? startupErrors_ : "Application has been terminated due to unexpected error.");
        }
        else
        {
            _host->errorDialog(GetTypeName(), message);
        }
    }

    void Application::Render()
    {
        if (_headless)
            return;

        //ALIMER_PROFILE(Render);

        // Acquire next swap chain texture before rendering
        if (!_graphics->BeginFrame())
        {
            return;
        }

        _graphics->EndFrame();

        /*auto commandBuffer = _graphics->GetCommandQueue()->GetCommandBuffer();

        // TODO: Render scene
        // TODO: Render UI

        _graphics->GetCommandQueue()->Submit(commandBuffer);

        // Present content on screen.
        _mainWindowSwapChain->Present();*/
    }
}
