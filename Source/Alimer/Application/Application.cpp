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

#include "AlimerVersion.h"
#include "../Application/Application.h"
#include "../IO/Path.h"
#include "../Core/Platform.h"
#include "../Scene/Systems/CameraSystem.h"
#include "../Scene/Systems/RenderSystem.h"
#include "../Core/Log.h"
using namespace std;

namespace Alimer
{
    static Application* __appInstance = nullptr;

    Application::Application()
        : _running(false)
        , _paused(false)
        , _headless(false)
        , _settings{}
        , _log(new Logger())
        , _scene(_entities)
    {
        PlatformConstruct();

        __appInstance = this;
    }

    Application::~Application()
    {
        _paused = true;
        _running = false;

        PluginManager::DeleteInstance();

        __appInstance = nullptr;
    }

    Application* Application::GetInstance()
    {
        return __appInstance;
    }

    bool Application::InitializeBeforeRun()
    {
        SetCurrentThreadName("Main");

        ALIMER_LOGINFO("Initializing engine %s...", ALIMER_VERSION_STR);

        // Init Window and Gpu.
        if (!_headless)
        {
            _window = MakeWindow("Alimer", 800, 600);

            // Create and init graphics.
            _graphicsDevice = GraphicsDevice::Create(_settings.graphicsDeviceType, _settings.validation);
            GpuAdapter* adapter = nullptr;
            if (!_graphicsDevice->Initialize(adapter, _window))
            {
                ALIMER_LOGERROR("Failed to initialize GraphicsDevice.");
                return false;
            }
        }

        // Create per platform Input module.
        _input = CreateInput();

        // Create per platform Audio module.
        _audio = CreateAudio();

        // Load plugins
        LoadPlugins();

        // Initialize this instance and all systems.
        Initialize();

        // Setup and configure all systems.
        _systems.Add<CameraSystem>(_entities);
        _renderSystem = _systems.Add<RenderSystem>(_entities);

        ALIMER_LOGINFO("Engine initialized with success.");
        _running = true;
        //BeginRun();

        // Reset timer.
        _timer.Reset();

        // Run the first time an update
        //InternalUpdate();

        return true;
    }

    void Application::LoadPlugins()
    {
        PluginManager::GetInstance()->LoadPlugins(GetExecutableFolder());
    }

    void Application::RunFrame()
    {
        if (!_paused)
        {
            // Tick timer.
            double frameTime = _timer.Frame();
            double deltaTime = _timer.GetElapsed();

            // Update all systems.
            _systems.Update(deltaTime);

            // Render single frame.
            RenderFrame(frameTime, deltaTime);
        }

        // Update input, even when paused.
        _input->Update();
    }

    void Application::RenderFrame(double frameTime, double elapsedTime)
    {
        if (_headless)
            return;

        if (_graphicsDevice->BeginFrame())
        {
            CommandBuffer* commandBuffer = _graphicsDevice->GetDefaultCommandBuffer();

            // Begin recording.
            commandBuffer->Begin();
            commandBuffer->BeginRenderPass(nullptr, Color4(0.0f, 0.2f, 0.4f, 1.0f));

            // Call OnRenderFrame for custom rendering frame logic.
            OnRenderFrame(commandBuffer, frameTime, elapsedTime);

            // Render scene to default command buffer.
            _renderSystem->Render(commandBuffer);

            // End swap chain render pass.
            commandBuffer->EndRenderPass();

            // End recording.
            commandBuffer->End();

            // End rendering frame.
            _graphicsDevice->EndFrame();
        }
    }

    void Application::OnRenderFrame(CommandBuffer* commandBuffer, double frameTime, double elapsedTime)
    {
        ALIMER_UNUSED(frameTime);
        ALIMER_UNUSED(elapsedTime);

        // By default clear with some color.
        commandBuffer->BeginRenderPass(nullptr, Color4(0.0f, 0.2f, 0.4f, 1.0f));
        commandBuffer->EndRenderPass();
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

    Application& gApplication()
    {
        return *__appInstance;
    }
}
