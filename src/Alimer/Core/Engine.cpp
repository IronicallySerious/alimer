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

#include "../Core/Engine.h"
#include "../Core/PluginManager.h"
#include "../Resource/ResourceManager.h"
#include "../Input/Input.h"
#include "../Audio/Audio.h"
#include "../Graphics/GraphicsDevice.h"
#include "../Scene/SceneManager.h"
#include "../UI/Gui.h"
//#include <CLI/CLI.hpp>
//#include <fmt/printf.h>

namespace alimer
{
    Engine::Engine()
        : _pluginManager(nullptr)
    {
        // Register as subsystem.
        AddSubsystem(this);

        // Setup logger first.
        std::vector<std::shared_ptr<spdlog::sinks::sink>> sinks;

        sinks.emplace_back(std::make_shared<spdlog::sinks::platform_sink_mt>());
        sinks.emplace_back(std::make_shared<spdlog::sinks::daily_file_sink_mt>("AlimerLog", 23, 59));
#if ALIMER_PLATFORM_WINDOWS && ALIMER_DEV
        AllocConsole();
        sinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
#endif
        _logger = std::make_shared<spdlog::logger>(ALIMER_LOG, sinks.begin(), sinks.end());
        spdlog::register_logger(_logger);

#ifdef _DEBUG
        _logger->set_level(spdlog::level::debug);
#else
        _logger->set_level(spdlog::level::info);
#endif

        // Create plugin manager
        _pluginManager = PluginManager::Create(*this);

        // Create resources manager.
        _resources = new ResourceManager();

        // Create SceneManager.
        _sceneManager = new SceneManager();
    }

    Engine::~Engine()
    {
        PluginManager::Destroy(_pluginManager);
        _gui.Reset();
        _graphicsDevice.Reset();
        RemoveSubsystem(this);
    }

    int Engine::Initialize(const Vector<String>& args)
    {
        if (_initialized)
            return EXIT_SUCCESS;

        ALIMER_LOGINFO("Initializing engine {}", ALIMER_VERSION_STR);

#if TODO_CLI_ARGS
        // Define and parse command line parameters.
        {
            CLI::App app{ fmt::sprintf("Alimer %s.", ALIMER_VERSION_STR), "Alimer" };
            bool verboseOutput = false;
            app.add_flag("-v,--verbose", verboseOutput, "Enable verbose mode.");
            app.add_flag("--headless", _settings.graphicsDeviceDesc.headless, "Do not initialize windowing and graphics subsystem.");

            try
            {
                std::vector<std::string> cliArgs;
                cliArgs.reserve(args.Size());
                for (uint32_t i = 0; i < args.Size(); i++)
                {
                    cliArgs.emplace_back(args[i].CString());
                }

                app.parse(cliArgs);
            }
            catch (const CLI::ParseError &e) {
                return app.exit(e);
            }
        }
#endif // TODO_CLI


        // Register the rest of the subsystems
        _input = new Input();
        _audio = Audio::Create();
        _audio->Initialize();

        // Create GraphicsDevice.
        _graphicsDevice = GraphicsDevice::Create(_settings.validation, _settings.headless);
        if (_graphicsDevice == nullptr)
        {
            ALIMER_LOGERROR("Failed to create GraphicsDevice instance.");
            return false;
        }

        const bool vsync = true;
        const bool multisampling = false;
        if (!_graphicsDevice->SetMode(_settings.title, _settings.size, _settings.fullscreen, _settings.resizable, vsync, multisampling))
        {
            ALIMER_LOGERROR("Failed to setup GraphicsDevice.");
            return false;
        }

        // Create imgui system.
        //_gui = new Gui();

        ALIMER_LOGINFO("Engine initialized with success");
        _initialized = true;
        return EXIT_SUCCESS;
    }

    void Engine::RunFrame()
    {
        //ALIMER_PROFILE("RunFrame");
        {
            ALIMER_ASSERT(_initialized);

            // If not headless, and the graphics subsystem no longer has a window open, assume we should exit
            if (!IsHeadless() && _graphicsDevice.IsNull())
            {
                _exiting = true;
            }

            if (_exiting)
                return;
        }

        Render();

        /*if (!_paused)
        {
            // Tick timer.
            double frameTime = _timer.Frame();
            double deltaTime = _timer.GetElapsed();

            // Update all systems.
            //_systems.Update(deltaTime);

            // Render single frame if window is not minimzed.
            if (!_engine->IsHeadless()
                && !_mainWindow->IsMinimized())
            {
                RenderFrame(frameTime, deltaTime);
            }
        }*/

        // Update input, even when paused.
        _input->Update();
    }

    void Engine::Render()
    {
        if (IsHeadless())
            return;

        //ALIMER_PROFILE("Render");

        // If device is lost, BeginFrame will fail and we skip rendering
        if (!_graphicsDevice->BeginFrame())
            return;

        //Color4 clearColor(0.0f, 0.2f, 0.4f, 1.0f);
        //CommandContext& context = _graphicsDevice->GetContext();
        //context.BeginDefaultRenderPass(clearColor);
        //context.Draw(3, 0);
        //context.EndRenderPass();

        // TODO: Scene renderer
        // TODO: UI render.
        _graphicsDevice->EndFrame();
    }
}
