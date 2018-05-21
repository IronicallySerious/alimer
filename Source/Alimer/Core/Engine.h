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

#pragma once

#include <new>
#include <memory>
#include <string>
#include <cstring>
#include <array>
#include <vector>
#include <string>
#include <atomic>
#include "../Core/Window.h"
#include "../IO/FileSystem.h"
#include "../Resource/ResourceManager.h"
#include "../Graphics/Graphics.h"

void AlimerMain(const std::vector<std::string>& args);

namespace Alimer
{
	/// Engine for main loop and all modules and OS setup.
	class Engine
	{
	protected:
		/// Constructor.
		Engine();

	public:
		/// Destructor.
		virtual ~Engine();

		/// Runs main loop.
		virtual int Run();

		/// Tick/Run one frame.
		void Tick();

		/// Request application to exit.
		void Exit();

		/// Pause the main execution loop.
		void Pause();

		/// Resume the main execution loop.
		void Resume();

		virtual std::shared_ptr<Window> CreateWindow() = 0;

		inline ResourceManager* GetResources() { return &_resources; }
		inline Window* GetMainWindow() const { return _window.get(); }
		inline Graphics* GetGraphics() const { return _graphics.get(); }

	protected:
		virtual bool Initialize();
		virtual void RunMain();
		/// Render after frame update.
		void Render();

		static bool SetCurrentThreadName(const std::string& name);

		std::vector<std::string> _args;
		std::atomic<bool> _running;
		std::atomic<bool> _paused;
		std::atomic<bool> _headless;

		ResourceManager _resources;
		std::shared_ptr<Window> _window;
		std::unique_ptr<Graphics> _graphics;

		GraphicsDeviceType _graphicsDeviceType{ GraphicsDeviceType::Default };

	private:
		DISALLOW_COPY_MOVE_AND_ASSIGN(Engine);
	};

	extern Engine* engine;
}
