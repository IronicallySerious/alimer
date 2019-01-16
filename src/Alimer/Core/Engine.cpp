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
#include "../Graphics/GPUDevice.h"
#include "../Scene/SceneManager.h"
#include "../UI/Gui.h"
#include <CLI/CLI.hpp>
#include <fmt/printf.h>

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

        // Register modules
        GPUDevice::RegisterObject();

        // Create SceneManager.
        _sceneManager = new SceneManager();
    }

    Engine::~Engine()
    {
        PluginManager::Destroy(_pluginManager);
        _gui.Reset();
        RemoveSubsystem(this);
    }

    int Engine::Initialize(const Vector<String>& args)
    {
        if (_initialized)
            return EXIT_SUCCESS;

        ALIMER_LOGINFO("Initializing engine {}", ALIMER_VERSION_STR);

        // Define and parse command line parameters.
        {
            CLI::App app{ fmt::sprintf("Alimer %s.", ALIMER_VERSION_STR), "Alimer" };
            bool verboseOutput = false;
            app.add_flag("-v,--verbose", verboseOutput, "Enable verbose mode.");
            app.add_flag("--headless", _headless, "Do not initialize windowing and graphics subsystem.");

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

        // Create imgui system.
        //_gui = new Gui();

        ALIMER_LOGINFO("Engine initialized with success");
        _initialized = true;
        return EXIT_SUCCESS;
    }
}
