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

#include <vector>
#include <string>
#include "../Core/Plugin.h"

namespace alimer
{
    class Engine;

    /// Class that manages runtime plugins.
    class ALIMER_API PluginManager final
    {
    public:
        ~PluginManager() = default;

        /// Create new plugin manager.
        static PluginManager* Create(Engine& engine);

        /// Destroy plugin manager instance.
        static void Destroy(PluginManager* manager);

        /// Load plugins from given path.
        void LoadPlugins(const std::string& pluginPath);
        bool LoadPlugin(const std::string& pluginName);
        void InstallPlugin(Plugin* plugin);

    private:
        /// Constructor.
        PluginManager(Engine& engine);

        Engine& _engine;
        std::vector<std::unique_ptr<Plugin>> _plugins;

    private:
        PluginManager(const PluginManager&) = delete;
        PluginManager& operator=(const PluginManager&) = delete;
    };
}
