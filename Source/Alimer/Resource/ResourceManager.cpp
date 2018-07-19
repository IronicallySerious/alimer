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
#include "../Util/Util.h"
#include "../Core/Log.h"
using namespace std;

namespace Alimer
{
    ResourceManager::ResourceManager()
    {
    }

    ResourceManager::~ResourceManager()
    {
    }

    bool ResourceManager::AddResourceDir(const string& path, uint32_t priority)
    {
        lock_guard<mutex> guard(_resourceMutex);

        if (!DirectoryExists(path))
        {
            ALIMER_LOGERROR("Directory '%s' does not exists", path.c_str());
            return false;
        }

        // Convert path to absolute
        string fixedPath = SanitateResourceDirName(path);

        // Check that the same path does not already exist
        for (size_t i = 0; i < _resourceDirs.size(); ++i)
        {
            if (!_resourceDirs[i].compare(fixedPath))
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

        ALIMER_LOGINFO("Added resource path '{}'", fixedPath);
        return true;
    }

    unique_ptr<Stream> ResourceManager::Open(const string &assetName, StreamMode mode)
    {
        lock_guard<mutex> guard(_resourceMutex);

        string sanitatedName = SanitateResourceName(assetName);

        if (sanitatedName.length())
        {
            unique_ptr<Stream> stream = {};

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

    bool ResourceManager::Exists(const std::string &assetName)
    {
        lock_guard<mutex> guard(_resourceMutex);

        string sanitatedName = SanitateResourceName(assetName);

        if (sanitatedName.length())
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

    SharedPtr<Resource> ResourceManager::LoadResource(const string& assetName)
    {
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

    string ResourceManager::SanitateResourceName(const string& name) const
    {
        // Sanitate unsupported constructs from the resource name
        string sanitatedName = str::Replace(name, "../", "");
        sanitatedName = str::Replace(sanitatedName, "./", "");

        // If the path refers to one of the resource directories, normalize the resource name
        if (_resourceDirs.size())
        {
            string namePath = GetPath(sanitatedName);
            string exePath = str::Replace(GetExecutableFolder(), "/./", "/");
            for (size_t i = 0; i < _resourceDirs.size(); ++i)
            {
                string relativeResourcePath = _resourceDirs[i];
                if (str::StartsWith(relativeResourcePath, exePath))
                    relativeResourcePath = relativeResourcePath.substr(exePath.length());

                if (str::StartsWith(namePath, _resourceDirs[i], false))
                    namePath = namePath.substr(_resourceDirs[i].length());
                else if (str::StartsWith(namePath, relativeResourcePath, false))
                    namePath = namePath.substr(relativeResourcePath.length());
            }

            sanitatedName = namePath + GetFileNameAndExtension(sanitatedName);
        }

        str::Trim(sanitatedName);
        return sanitatedName;
    }

    string ResourceManager::SanitateResourceDirName(const string& name) const
    {
        string cleanName = AddTrailingSlash(name);
        if (!IsAbsolutePath(name))
            cleanName = Path::Join(GetCurrentDir(), name);

        // Sanitate away /./ construct
        str::Replace(cleanName, "/./", "/");
        str::Trim(cleanName);
        return cleanName;
    }

    unique_ptr<Stream> ResourceManager::SearchResourceDirs(const string& name)
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

    unique_ptr<Stream> ResourceManager::SearchPackages(const string& name)
    {
        // TODO:
        return {};
    }

    bool ResourceManager::ExistsInResourceDirs(const string& name)
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

    bool ResourceManager::ExistsInPackages(const string& name)
    {
        // TODO:
        return false;
    }
}
