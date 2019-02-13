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

#pragma once

#include "../Core/Object.h"
#include "../Graphics/Types.h"
#include "../Graphics/GraphicsDevice.h"
#include "../Core/Log.h"
#include "../Math/Vector2.h"

namespace alimer
{
    class Window;
    class PluginManager;
    class ResourceManager;
    class Input;
    class Audio;
    class SceneManager;
    class Gui;

    struct EngineSettings
    {
        PhysicalDevicePreference    preferredDevice = PhysicalDevicePreference::Discrete;
        bool                        gpuValidation = false;

        /// Main window title.
        std::string     title = "Alimer";

        /// Main window size.
        IntVector2      size = { 800, 600 };
        bool            fullscreen = false;
        bool            resizable = false;
    };

    /// Alimer engine. Manages module setup and all engine logic.
    class ALIMER_API Engine final : public Object
    {
        ALIMER_OBJECT(Engine, Object);

    public:
        /// Constructor.
        Engine();

        /// Destructor.
        ~Engine() override;

        /// Initializes the engine
        bool Initialize(const std::vector<std::string>& args);

        /// Run one frame.
        void RunFrame();

        /// Render frame.
        void Render();

        /// Return whether engine has been initialized.
        bool IsInitialized() const { return _initialized; }

        /// Return whether engine is exiting.
        bool IsExiting() const { return _exiting; }

        /// Get the engine settings.
        const EngineSettings& GetSettings() const { return _settings; }

        inline bool IsHeadless() const { return _headless; }

        /// Get the engine content manager.
        inline PluginManager& GetPluginManager() { return *_pluginManager; }

        /// Get the engine resource manager.
        inline ResourceManager& GetResources() { return *_resources.Get(); }

        /// Get the main window.
        inline Window* GetRenderWindow() const { return _renderWindow.get(); }

        /// Get the input system.
        inline Input& GetInput() { return *_input.Get(); }

        /// Get the audio system.
        inline Audio& GetAudio() { return *_audio.Get(); }

        /// Get the graphics device.
        inline GraphicsDevice& GetGraphics() { return *_graphics.Get(); }

    private:
        /// Initialized flag.
        std::atomic_bool _initialized{ false };
        std::atomic_bool _exiting{ false };

        /// Engine settings.
        EngineSettings _settings;
        bool _headless = false;

        std::shared_ptr<spdlog::logger> _logger;
        PluginManager*              _pluginManager;
        SharedPtr<ResourceManager>  _resources;
        SharedPtr<Input>            _input;
        SharedPtr<Audio>            _audio;
        std::unique_ptr<Window>     _renderWindow;
        SharedPtr<GraphicsDevice>   _graphics;
        SharedPtr<SceneManager>     _sceneManager;

        // ImGui
        SharedPtr<Gui> _gui;
    };
}
