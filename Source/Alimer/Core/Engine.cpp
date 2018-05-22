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

#include "../Core/Engine.h"
#include "../Debug/Log.h"

namespace Alimer
{
	Alimer::Engine* engine = nullptr;

	Engine::Engine()
		: _running(false)
		, _paused(false)
		, _headless(false)
	{
		engine = this;
	}

	Engine::~Engine()
	{
		_paused = true;
		_running = false;
		engine = nullptr;
	}

	int Engine::Run()
	{
		return EXIT_SUCCESS;
	}

	void Engine::Tick()
	{
		if (_paused)
		{
			// When paused still update input logic.
			//_input->Update();
			return;
		}

		//_time->Update();
		Render();
		//_input->Update();
	}

	void Engine::Render()
	{
		if (_headless)
			return;

		auto frameTexture = _graphics->AcquireNextImage();
		auto commandBuffer = _graphics->CreateCommandBuffer();
		RenderPassDescriptor passDescriptor;
		passDescriptor.colorAttachments[0].texture = frameTexture.get();
		passDescriptor.colorAttachments[0].clearColor = { 0.0f, 0.2f, 0.4f, 1.0f };
		commandBuffer->BeginRenderPass(passDescriptor);
		commandBuffer->EndRenderPass();
		commandBuffer->Commit();

		//OnRender();
		_graphics->Present();
	}

	bool Engine::Initialize()
	{
		SetCurrentThreadName("Main");

		if (!_headless)
		{
			_window = CreateWindow();

			// Create and init graphics.
			_graphics.reset(Graphics::Create(_graphicsDeviceType));

			if (!_graphics->Initialize(_window))
			{
				ALIMER_LOGERROR("Failed to initialize Graphics.");
				return false;
			}
		}

		ALIMER_LOGINFO("Engine initialized with success.");
		_running = true;

		// TODO: Multithreading main
		RunMain();
		return true;
	}

	GpuBufferPtr vertexBuffer;

	void Engine::RunMain()
	{
		AlimerMain(_args);

		struct Vector3
		{
			float x;
			float y;
			float z;
		};

		struct Vector4
		{
			float x;
			float y;
			float z;
			float w;
		};

		struct Vertex
		{
			Vector3 position;
			Vector4 color;
		};

		const float aspectRatio = static_cast<float>(_window->GetWidth()) / _window->GetHeight();

		Vertex triangleVertices[] =
		{
			{ { 0.0f, 0.25f * aspectRatio, 0.0f },{ 1.0f, 0.0f, 0.0f, 1.0f } },
			{ { 0.25f, -0.25f * aspectRatio, 0.0f },{ 0.0f, 1.0f, 0.0f, 1.0f } },
			{ { -0.25f, -0.25f * aspectRatio, 0.0f },{ 0.0f, 0.0f, 1.0f, 1.0f } }
		};

		vertexBuffer = _graphics->CreateBuffer(BufferUsage::Vertex, 3, sizeof(Vertex), triangleVertices);
	}

	void Engine::Exit()
	{
		_paused = true;

		if (_running)
		{
			// TODO: Fire event.
			_running = false;
		}
	}

	void Engine::Pause()
	{
		if (_running && !_paused)
		{
			// TODO: Fire event.
			_paused = true;
		}
	}

	void Engine::Resume()
	{
		if (_running && _paused)
		{
			// TODO: Fire event.
			_paused = false;
		}
	}
}
