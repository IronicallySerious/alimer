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
#include "../Core/Object.h"
#include "../Core/Log.h"
#include "../Core/Timer.h"
#include "../Core/PluginManager.h"
#include "../Application/Window.h"
#include "../Serialization/Serializable.h"
#include "../IO/FileSystem.h"
#include "../Resource/ResourceManager.h"
#include "../Input/Input.h"
#include "../Audio/Audio.h"
#include "../Graphics/Graphics.h"
#include "../Scene/SceneManager.h"
struct enkiTaskScheduler;

namespace Alimer
{
    class ApplicationSettings : public Serializable
    {
    public:
        GraphicsDeviceType graphicsDeviceType = GraphicsDeviceType::Default;

#ifdef _DEBUG
        bool validation = true;
#else
        bool validation = false;
#endif
    };

    class SceneRenderer;

    /// Application for main loop and all modules and OS setup.
    class ALIMER_API Application : public Object
    {
        ALIMER_OBJECT(Application, Object);

    protected:
        /// Constructor.
        Application();

    public:
        /// Destructor.
        virtual ~Application();

        /// Return the single instance of the Application.
        static Application* GetInstance();

        /// Runs main loop.
        int Run();

        /// Run one frame.
        void RunOneFrame();

        /// Request application to exit.
        void Exit();

        /// Pause the main execution loop.
        void Pause();

        /// Resume the main execution loop.
        void Resume();

        WindowPtr MakeWindow(const std::string& title, uint32_t width = 1280, uint32_t height = 720, bool fullscreen = false);

        Timer &GetFrameTimer() { return _timer; }

        inline ResourceManager* GetResources() { return &_resources; }
        inline const Window* GetMainWindow() const { return _window.Get(); }
        inline const Graphics* GetGraphics() const { return _graphics.Get(); }
        inline Input* GetInput() const { return _input.get(); }
        inline Audio* GetAudio() const { return _audio.get(); }

        /// Sets the current scene to be active and rendered.
        void SetScene(Scene* scene);
        SceneManager* GetSceneManager() const { return _sceneManager.get(); }

    private:
        void PlatformConstruct();
        bool InitializeBeforeRun();
        void LoadPlugins();

        void RenderFrame(RenderPass* frameRenderPass, double frameTime, double elapsedTime);
        void UpdateScene(double frameTime, double elapsedTime);
        void RenderScene(RenderPass* frameRenderPass);

    protected:
        /// Called after setup and engine initialization with all modules initialized.
        virtual void Initialize() { }

        /// Cleanup after the main loop. 
        virtual void OnExiting() { }

        /// Render after frame update.
        void Render();

        /// Called during rendering single frame.
        virtual void OnRender(CommandBuffer* commandBuffer);

        virtual std::unique_ptr<Input> CreateInput();
        virtual std::unique_ptr<Audio> CreateAudio();

        std::vector<std::string> _args;
        std::atomic<bool> _running;
        std::atomic<bool> _paused;
        std::atomic<bool> _headless;
        ApplicationSettings _settings;

        std::unique_ptr<Logger> _log;
        Timer _timer;
        PluginManager _pluginManager;
        ResourceManager _resources;
        WindowPtr _window;
        SharedPtr<Graphics> _graphics;
        std::unique_ptr<Input> _input;
        std::unique_ptr<Audio> _audio;
        enkiTaskScheduler* _taskScheduler;

        std::unique_ptr<SceneManager> _sceneManager;
        std::unique_ptr<SceneRenderer> _renderer;

    private:
        DISALLOW_COPY_MOVE_AND_ASSIGN(Application);
    };

    /// Access to current application instance.
    ALIMER_API Application& gApplication();
}
