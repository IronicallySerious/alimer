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

#include "../Resource/ResourceManager.h"
#include "../Application/Application.h"
#include "../IO/FileSystem.h"
#include "../IO/Path.h"
#include "../Core/Log.h"

namespace Alimer
{
    ResourceManager::ResourceManager()
    {
    }

    ResourceManager::~ResourceManager()
    {
    }

    bool ResourceManager::AddResourceDir(const String& path, uint32_t priority)
    {
        std::lock_guard<std::mutex> guard(_resourceMutex);

        if (!DirectoryExists(path))
        {
            ALIMER_LOGERRORF("Directory '%s' does not exists", path.CString());
            return false;
        }

        // Convert path to absolute
        String fixedPath = SanitateResourceDirName(path);

        // Check that the same path does not already exist
        for (size_t i = 0; i < _resourceDirs.size(); ++i)
        {
            if (!_resourceDirs[i].Compare(fixedPath))
                return true;
        }

        if (priority < _resourceDirs.size())
            _resourceDirs.insert(_resourceDirs.begin() + priority, fixedPath);
        else
            _resourceDirs.push_back(fixedPath);

        // If resource auto-reloading active, create a file watcher for the directory
       /* if (_autoReloadResources)
        {
            SharedPtr<FileWatcher> watcher(new FileWatcher(context_));
            watcher->StartWatching(fixedPath, true);
            fileWatchers_.Push(watcher);
        }*/

        ALIMER_LOGINFOF("Added resource path '%s'", fixedPath.CString());
        return true;
    }

    UniquePtr<Stream> ResourceManager::Open(const String &assetName, StreamMode mode)
    {
        ALIMER_UNUSED(mode);
        std::lock_guard<std::mutex> guard(_resourceMutex);

        String sanitatedName = SanitateResourceName(assetName);

        if (sanitatedName.Length())
        {
            UniquePtr<Stream> stream = {};

            if (_searchPackagesFirst)
            {
                stream = SearchPackages(sanitatedName);
                if (!stream)
                    stream = SearchResourceDirs(sanitatedName);
            }
            else
            {
                stream = SearchResourceDirs(sanitatedName);
                if (!stream)
                    stream = SearchPackages(sanitatedName);
            }

            return stream;
        }

        return {};
    }

    bool ResourceManager::Exists(const String &assetName)
    {
        std::lock_guard<std::mutex> guard(_resourceMutex);

        String sanitatedName = SanitateResourceName(assetName);

        if (sanitatedName.Length())
        {
            bool exists = false;
            if (_searchPackagesFirst)
            {
                exists = ExistsInPackages(sanitatedName);
                if (!exists)
                    exists = ExistsInResourceDirs(sanitatedName);
            }
            else
            {
                exists = ExistsInResourceDirs(sanitatedName);
                if (!exists)
                    exists = ExistsInPackages(sanitatedName);
            }

            return exists;
        }

        return false;
    }

    SharedPtr<Resource> ResourceManager::LoadResource(const String& assetName)
    {
        ALIMER_UNUSED(assetName);
        /*auto paths = Path::ProtocolSplit(assetName);
        string fullPath = Path::Join(_dataDirectory, paths.second);
        string compiledAssetName = fullPath + ".alb";

        if (!FileSystem::Get().FileExists(compiledAssetName))
        {
            if (!FileSystem::Get().FileExists(assetName))
            {
                return nullptr;
            }
        }

        auto b = FileSystem::Get().FileExists(assetName);*/
        return nullptr;
    }

    String ResourceManager::SanitateResourceName(const String& name) const
    {
        // Sanitate unsupported constructs from the resource name
        String sanitatedName = name.Replaced("../", "");
        sanitatedName.Replace("./", "");

        // If the path refers to one of the resource directories, normalize the resource name
        if (_resourceDirs.size())
        {
            String namePath = GetPath(sanitatedName);
            String exePath = GetExecutableFolder().Replaced("/./", "/");
            for (size_t i = 0; i < _resourceDirs.size(); ++i)
            {
                String relativeResourcePath = _resourceDirs[i];
                if (relativeResourcePath.StartsWith(exePath))
                    relativeResourcePath = relativeResourcePath.Substring(exePath.Length());

                if (namePath.StartsWith(_resourceDirs[i], false))
                    namePath = namePath.Substring(_resourceDirs[i].Length());
                else if (namePath.StartsWith(relativeResourcePath, false))
                    namePath = namePath.Substring(relativeResourcePath.Length());
            }

            sanitatedName = namePath + GetFileNameAndExtension(sanitatedName);
        }

        return sanitatedName.Trimmed();
    }

    String ResourceManager::SanitateResourceDirName(const String& name) const
    {
        String cleanName = AddTrailingSlash(name);
        if (!IsAbsolutePath(name))
            cleanName = Path::Join(GetCurrentDir(), name);

        // Sanitate away /./ construct
        cleanName = cleanName.Replaced("/./", "/").Trimmed();
        return cleanName;
    }

    UniquePtr<Stream> ResourceManager::SearchResourceDirs(const String& name)
    {
        for (size_t i = 0; i < _resourceDirs.size(); ++i)
        {
            if (FileExists(_resourceDirs[i] + name))
            {
                return OpenStream(_resourceDirs[i] + name);
            }
        }

        // Fallback using absolute path
        if (FileExists(name))
            return OpenStream(name);

        return {};
    }

    UniquePtr<Stream> ResourceManager::SearchPackages(const String& name)
    {
        ALIMER_UNUSED(name);
        // TODO:
        return {};
    }

    bool ResourceManager::ExistsInResourceDirs(const String& name)
    {
        for (size_t i = 0; i < _resourceDirs.size(); ++i)
        {
            if (FileExists(_resourceDirs[i] + name))
            {
                return true;
            }
        }

        // Fallback using absolute path
        if (FileExists(name))
            return true;

        return false;
    }

    bool ResourceManager::ExistsInPackages(const String& name)
    {
        ALIMER_UNUSED(name);
        // TODO:
        return false;
    }
}
