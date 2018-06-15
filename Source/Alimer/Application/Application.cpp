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

#include "../Application/Application.h"
#include "../Core/Log.h"

namespace Alimer
{
    static Application* __appInstance = nullptr;

    Application::Application()
		: _running(false)
		, _paused(false)
		, _headless(false)
        , _settings{}
        , _log(new Logger())
	{
        PlatformConstruct();
        __appInstance = this;
	}

    Application::~Application()
	{
		_paused = true;
		_running = false;
        __appInstance = nullptr;
	}

    Application* Application::GetInstance()
    {
        return __appInstance;
    }

	void Application::Tick()
	{
		if (_paused)
		{
			// When paused still update input logic.
			_input->Update();
		}
        else
        {
            //_time->Update();
            Render();
            _input->Update();
        }
	}

	void Application::Render()
	{
		if (_headless)
			return;

		// Acquire frame texture first.
		SharedPtr<Texture> frameTexture = _graphics->AcquireNextImage();
		// TODO: Render Scene.
        OnRender(frameTexture);
		_graphics->Present();
        _graphics->Frame();
	}

	bool Application::InitializeBeforeRun()
	{
		SetCurrentThreadName("Main");

		if (!_headless)
		{
			_window = MakeWindow("Alimer", 800, 600);

			// Create and init graphics.
            //_settings.graphicsDeviceType = GraphicsDeviceType::Direct3D12;
			//_settings.graphicsDeviceType  = GraphicsDeviceType::Vulkan;

			_graphics = Graphics::Create(_settings.graphicsDeviceType, _settings.validation);
			if (!_graphics->Initialize(_window))
			{
				ALIMER_LOGERROR("Failed to initialize Graphics.");
				return false;
			}
		}

		// Create per platform Input module.
		_input = CreateInput();

		// Create per platform Audio module.
		_audio = CreateAudio();

        // Initialize this instance and all systems.
        Initialize();

		ALIMER_LOGINFO("Engine initialized with success.");
		_running = true;
        //BeginRun();

        //_time.Reset();

        // Run the first time an update
        //InternalUpdate();
		return true;
	}

	void Application::Exit()
	{
		_paused = true;

		if (_running)
		{
			// TODO: Fire event.
			_running = false;
		}
	}

	void Application::Pause()
	{
		if (_running && !_paused)
		{
			// TODO: Fire event.
			_paused = true;
		}
	}

	void Application::Resume()
	{
		if (_running && _paused)
		{
			// TODO: Fire event.
			_paused = false;
		}
	}

	void Application::SetScene(Scene* scene)
	{
		_scene = scene;
	}

    Application& gApplication()
    {
        return *__appInstance;
    }
}
