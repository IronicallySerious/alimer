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
#include "../IO/Path.h"
#include "../Core/Log.h"
#include "AlimerVersion.h"
#include "enkiTS/src/TaskScheduler_c.h"
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
        , _scene(new Scene())
    {
        PlatformConstruct();

        // Init enkiTS
        _taskScheduler = enkiNewTaskScheduler();
        enkiInitTaskScheduler(_taskScheduler);

        __appInstance = this;
    }

    Application::~Application()
    {
        _paused = true;
        _running = false;

        // Delete enkiTS
        enkiDeleteTaskScheduler(_taskScheduler);

        __appInstance = nullptr;
    }

    Application* Application::GetInstance()
    {
        return __appInstance;
    }

    bool Application::InitializeBeforeRun()
    {
        SetCurrentThreadName("Main");

        ALIMER_LOGINFO("Initializing engine {}...", ALIMER_VERSION_STR);

        // Init resource paths.
        static const string AssetsFolderName = "assets";
        static const string ShadersFolderName = "shaders";

        string executablePath = GetExecutableFolder();
        string assetsDirectory = Path::Join(GetParentPath(executablePath), AssetsFolderName);
        string shadersDirectory = Path::Join(assetsDirectory, ShadersFolderName);

        if (DirectoryExists(assetsDirectory))
        {
            _resources.AddResourceDir(assetsDirectory);
            if (DirectoryExists(shadersDirectory))
                _resources.AddResourceDir(shadersDirectory);
        }
        else
        {
            assetsDirectory = Path::Join(executablePath, AssetsFolderName);
            shadersDirectory = Path::Join(assetsDirectory, ShadersFolderName);

            if (DirectoryExists(assetsDirectory))
            {
                _resources.AddResourceDir(assetsDirectory);

                if (DirectoryExists(shadersDirectory))
                    _resources.AddResourceDir(shadersDirectory);
            }
        }

        // Init Window and Gpu.
        if (!_headless)
        {
            _window = MakeWindow("Alimer", 800, 600);

            // Create and init graphics.
            //_settings.graphicsDeviceType = GraphicsDeviceType::Direct3D12;
            _settings.graphicsDeviceType = GraphicsDeviceType::Direct3D11;
            //_settings.graphicsDeviceType  = GraphicsDeviceType::Vulkan;

            _graphics = Graphics::Create(_settings.graphicsDeviceType, _settings.validation);
            GpuAdapter* adapter = nullptr;
            if (!_graphics->Initialize(adapter, _window))
            {
                ALIMER_LOGERROR("Failed to initialize Graphics.");
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

        ALIMER_LOGINFO("Engine initialized with success.");
        _running = true;
        //BeginRun();

        // Reset timer.
        _timer.Reset();

        // Run the first time an update
        //InternalUpdate();
        _pluginManager.Update();

        return true;
    }

    void Application::LoadPlugins()
    {
        _pluginManager.Initialize(GetExecutableFolder());
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
            Render();
            _input->Update();
        }
    }

    void Application::Render()
    {
        if (_headless)
            return;

        // Acquire frame texture first.
        SharedPtr<RenderPass> frameRenderPass = _graphics->BeginFrame();
        if (frameRenderPass.IsNotNull())
        {
            // Tick timer.
            double frameTime = _timer.Frame();
            double elapsedTime = _timer.GetElapsed();

            // Render single frame.
            RenderFrame(frameRenderPass.Get(), frameTime, elapsedTime);

            // End frame.
            _graphics->EndFrame();
        }
    }

    void Application::UpdateScene(double frameTime, double elapsedTime)
    {
        _scene->UpdateCachedTransforms();
    }

    void Application::RenderScene(RenderPass* frameRenderPass)
    {
        // TODO: Add Scene renderer.
        CommandBuffer* commandBuffer = _graphics->GetDefaultCommandBuffer();
        //RenderPassDescriptor passDescriptor = {};
        //passDescriptor.colorAttachments[0].texture = frameTexture;
        //passDescriptor.colorAttachments[0].clearColor = { 0.0f, 0.2f, 0.4f, 1.0f };
        auto encoder = commandBuffer->GetRenderPassCommandEncoder(frameRenderPass);
        _scene->Render(commandBuffer);
        encoder->Close();
        commandBuffer->Commit();
    }

    void Application::RenderFrame(RenderPass* frameRenderPass, double frameTime, double elapsedTime)
    {
        UpdateScene(frameTime, elapsedTime);
        RenderScene(frameRenderPass);
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
