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
#include "../Core/Log.h"

namespace alimer
{
    class PluginManager;
    class ResourceManager;
    class SceneManager;
    class Gui;

    /// Alimer engine. Manages module setup and all engine logic.
    class ALIMER_API Engine final : public Object
    {
        ALIMER_OBJECT(Engine, Object);

    public:
        /// Constructor.
        Engine();

        /// Destructor.
        ~Engine();

        int Initialize(const Vector<String>& args);

        /// Return whether engine has been initialized.
        bool IsInitialized() const { return _initialized; }

        /// Return whether the engine has been created in headless mode.
        bool IsHeadless() const { return _headless; }

        /// Get the engine content manager.
        inline PluginManager& GetPluginManager() { return *_pluginManager; }

        /// Get the engine resource manager.
        inline ResourceManager& getResources() { return *_resources.Get(); }

    private:
        /// Initialized flag.
        bool _initialized = false;

        /// Headless flag
        bool _headless = false;

        std::shared_ptr<spdlog::logger> _logger;
        PluginManager* _pluginManager;
        SharedPtr<ResourceManager> _resources;
        SharedPtr<SceneManager> _sceneManager;

        // ImGui
        SharedPtr<Gui> _gui;
    };
}
