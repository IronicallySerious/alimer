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

#include "Application/Application.h"
#include "Scene/Systems/CameraSystem.h"
#include "IO/Path.h"
#include "Core/Platform.h"

namespace alimer
{
    Application* application = nullptr;

    Application::Application()
        : _exitCode(EXIT_SUCCESS)
        , _running(false)
        , _paused(false)
        , _scene()
    {
        PlatformConstruct();

        // Create the Engine, but do not initialize it yet. Subsystems except Graphics & Renderer are registered at this point
        _engine = new Engine();

        // Set current instance.
        application = this;
    }

    Application::~Application()
    {
        Shutdown();
        application = nullptr;
    }

    void Application::Shutdown()
    {
        if (!_running)
            return;

        _paused = true;
        _running = false;
        _engine.Reset();
    }

    bool Application::InitializeBeforeRun()
    {
        SetCurrentThreadName("Main");
        
        // Initialize engine.
        if (_engine->Initialize(_args) != EXIT_SUCCESS)
        {
            return false;
        }

        // Load plugins
        LoadPlugins();

        // Initialize this instance and all systems.
        Initialize();

        // Setup and configure all systems.
        //_systems.Add<CameraSystem>();

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
        _engine->GetPluginManager().LoadPlugins(FileSystem::GetExecutableFolder());
    }

    int Application::Run(int argc, char** argv)
    {
        for (int i = 1; i < argc; ++i)
        {
            _args.push_back(argv[i]);
        }

        Setup();
        if (_exitCode != EXIT_SUCCESS)
        {
            return _exitCode;
        }

        PlatformRun();
        return _exitCode;
    }

    void Application::RunFrame()
    {
        _engine->RunFrame();
    }

    void Application::RenderFrame(double frameTime, double elapsedTime)
    {
        if (_engine->IsHeadless()) {
            return;
        }

        /*auto context = _graphicsDevice->GetContext();

        RenderPassBeginDescriptor renderPass = {};
        renderPass.colors[0].clearColor = Color4(0.0f, 0.2f, 0.4f, 1.0f);
        context->BeginDefaultRenderPass(&renderPass);
        */

        // Call OnRenderFrame for custom rendering frame logic.
        OnRenderFrame(frameTime, elapsedTime);

        // Render scene to default command buffer.
        /*if (_sceneRenderPipeline)
        {
            //auto camera = _scene.GetActiveCamera()->GetComponent<CameraComponent>()->camera;
            //_renderPipeline->Render(_renderContext, { camera });
        }

        // End swap chain render pass.
        context->EndRenderPass();*/

        // Present rendering frame.
        //_mainWindow->SwapBuffers();

        // Advance to next frame.
        //_gpuDevice->Frame();
    }

    void Application::OnRenderFrame(double frameTime, double elapsedTime)
    {
        ALIMER_UNUSED(frameTime);
        ALIMER_UNUSED(elapsedTime);

        //Color4 clearColor(0.0f, 0.2f, 0.4f, 1.0f);
        //CommandContext& context = _gpuDevice->GetImmediateContext();
        //context.BeginRenderPass(_mainWindow->GetCurrentFramebuffer(), clearColor);
        //context.Draw(3, 0);
        //context.EndRenderPass();
        //context.Flush();
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
}
