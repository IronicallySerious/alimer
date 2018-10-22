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

#include "../Core/PluginManager.h"
#include "../IO/FileSystem.h"
#include "../Core/Log.h"
#include "../Core/Platform.h"

#if defined(_WIN32)
#   define PLUGIN_EXT ".dll"
#elif defined(__APPLE__)
#   define PLUGIN_EXT ".dylib"
#else
#   define PLUGIN_EXT ".so"
#endif

namespace Alimer
{
    PluginManager *PluginManager::_instance;

    PluginManager::PluginManager()
    {
    }

    PluginManager::~PluginManager()
    {

    }

    PluginManager *PluginManager::GetInstance()
    {
        if (!_instance)
            _instance = new PluginManager;

        return _instance;
    }

    void PluginManager::Shutdown()
    {
        delete _instance;
        _instance = nullptr;
    }

    void PluginManager::LoadPlugins(const String& pluginPath)
    {
        ALIMER_LOGTRACE("Initializing Plugin System...");
        ALIMER_LOGDEBUGF("Scanning for plugins in directory '%s'", pluginPath.CString());

        std::vector<String> files;
        ScanDirectory(files, pluginPath, PLUGIN_EXT, ScanDirFlags::Files, false);

        for (const String& pluginFile : files)
        {
            LoadPlugin(pluginFile);
        }

        // List enumerated plugins
        /*for (auto it : _plugins)
        {
            const PluginDesc& desc = it.second;
            ALIMER_LOGINFOF("Loaded plugin => Name: '%s', Version: '%u.%u'",
                desc.name,
                ALIMER_VERSION_MAJOR(desc.version),
                ALIMER_VERSION_MAJOR(desc.version));
        }*/
    }

    bool PluginManager::LoadPlugin(const String& pluginName)
    {
        void* libHandle = LoadNativeLibrary(pluginName.CString());
        if (!libHandle)
        {
            return false;
        }

        PluginLoadFunc loadFunc = (PluginLoadFunc)GetSymbol(libHandle, "AlimerPluginLoad");
        if (!loadFunc)
        {
            UnloadNativeLibrary(libHandle);
            return false;  // Not a plugin
        }

        // Try to instance the plugin being loaded
        try
        {
            Plugin* plugin = loadFunc();
            if (plugin)
            {
                InstallPlugin(plugin);
                //UnloadNativeLibrary(libHandle);
                return true;
            }
        }
        catch (...)
        {

        }

        return false;
    }

    void PluginManager::InstallPlugin(Plugin* plugin)
    {
        ALIMER_LOGINFOF("Installing plugin: %s", plugin->GetName().CString());

        _plugins.push_back(UniquePtr<Plugin>(plugin));
        plugin->Install();

        //if (_initialized)
        {
            plugin->Initialize();
        }

        ALIMER_LOGINFO("Plugin successfully installed");
    }
}
