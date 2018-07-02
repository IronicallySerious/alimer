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


using namespace std;

#if WIN32
#   define PLUGIN_EXT ".dll"
#elif APPLE
#   define PLUGIN_EXT ".dylib"
#else
#   define PLUGIN_EXT ".so"
#endif

namespace Alimer
{
    PluginManager::PluginManager()
    {
    }

    static bool LoadPlugin(const std::string& pluginPath, void** pDllHandle, AlimerPluginApi** pApi)
    {
        void* libHandle = LoadNativeLibrary(pluginPath.c_str());
        if (!libHandle)
        {
            return false;
        }

        GetPluginApiFunc getPluginApi = (GetPluginApiFunc)GetSymbol(libHandle, "AlimerGetPluginApi");
        if (!getPluginApi)
        {
            UnloadNativeLibrary(libHandle);
            return false;  // Not a plugin
        }

        AlimerPluginApi* pluginApi = (AlimerPluginApi*)getPluginApi(PLUGIN_API_ID, 0);
        if (!pluginApi) {
            UnloadNativeLibrary(libHandle);
            return false;  // Incompatible plugin
        }

        *pDllHandle = libHandle;
        *pApi = pluginApi;

        return true;
    }

    static bool ValidatePlugin(const std::string& pluginPath, PluginDesc* desc)
    {
        void* dllHandle;
        AlimerPluginApi* api;
        if (!LoadPlugin(pluginPath, &dllHandle, &api))
        {
            return false;
        }

        memcpy(desc, api->GetDesc(), sizeof(PluginDesc));
        UnloadNativeLibrary(dllHandle);

        return true;
    }


    bool PluginManager::Initialize(const string& pluginPath)
    {
        ALIMER_LOGTRACE("Initializing Plugin System...");

        ALIMER_LOGDEBUG("Scanning for plugins in directory '{}' ...", pluginPath);

        vector<string> files;
        ScanDirectory(files, pluginPath, PLUGIN_EXT, ScanDirMask::Files, false);

        for (const string& pluginFile : files)
        {
            PluginDesc desc;
            if (ValidatePlugin(pluginFile, &desc))
            {
                _plugins[pluginFile] = desc;
            }
            else
            {
                ALIMER_LOGWARN("Failed loading native plugin \"{}\".", GetFileNameAndExtension(pluginFile));
            }
        }

        // List enumerated plugins
        for (auto it : _plugins)
        {
            const PluginDesc& desc = it.second;
            ALIMER_LOGINFO("Loaded plugin => Name: '{}', Version: '{}.{}'",
                desc.name,
                ALIMER_VERSION_MAJOR(desc.version),
                ALIMER_VERSION_MAJOR(desc.version));
        }

        return true;
    }

    void PluginManager::Update()
    {
    }
}
