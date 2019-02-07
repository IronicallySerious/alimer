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

#include "foundation/StringUtils.h"
#include "Resource/ResourceManager.h"
#include "Resource/Image.h"
#include "IO/FileSystem.h"
#include "IO/Path.h"
#include "Core/Log.h"

using namespace std;

namespace alimer
{
    ResourceManager::ResourceManager()
    {
        Register();

        // Lookup assets folder at executable path first.
        if (FileSystem::DirectoryExists(Path::Join(FileSystem::GetExecutableFolder(), "assets")))
        {
            AddResourceDir("assets");
        }
#ifdef ALIMER_DEFAULT_ASSETS_DIRECTORY
        else
        {

            const char *assetsDir = ALIMER_DEFAULT_ASSETS_DIRECTORY;
            if (assetsDir
                && FileSystem::DirectoryExists(assetsDir))
            {
                AddResourceDir(assetsDir);
            }
        }
#endif // ALIMER_DEFAULT_ASSETS_DIRECTORY

        AddSubsystem(this);
    }

    ResourceManager::~ResourceManager()
    {
        RemoveSubsystem(this);
    }

    bool ResourceManager::AddResourceDir(const string& path, uint32_t priority)
    {
        std::lock_guard<std::mutex> guard(_resourceMutex);

        if (!FileSystem::DirectoryExists(path))
        {
            ALIMER_LOGERROR("Directory '{}' does not exists", path);
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

        if (priority < _resourceDirs.size()) {
            _resourceDirs.insert(_resourceDirs.begin() + priority, fixedPath);
        }
        else {
            _resourceDirs.push_back(fixedPath);
        }

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

    void ResourceManager::AddLoader(ResourceLoader* loader)
    {
        ALIMER_ASSERT(loader);
        _loaders[loader->GetLoadingType()].reset(loader);
    }

    ResourceLoader* ResourceManager::GetLoader(StringHash type) const
    {
        auto it = _loaders.find(type);
        return it != end(_loaders) ? it->second.get() : nullptr;
    }

    unique_ptr<Stream> ResourceManager::OpenStream(const string& assetName)
    {
        std::lock_guard<std::mutex> guard(_resourceMutex);

        string sanitatedName = SanitateResourceName(assetName);

        if (sanitatedName.length())
        {
            unique_ptr<Stream> stream = {};

            if (_searchPackagesFirst)
            {
                stream = SearchPackages(sanitatedName);
                if (!stream) {
                    stream = SearchResourceDirs(sanitatedName);
                }
            }
            else
            {
                stream = SearchResourceDirs(sanitatedName);
                if (!stream) {
                    stream = SearchPackages(sanitatedName);
                }
            }

            return stream;
        }

        return {};
    }

    SharedPtr<Object> ResourceManager::Load(StringHash type, const string& assetName)
    {
        if (assetName.empty())
        {
            ALIMER_LOGCRITICAL("Cannot load with empty asset name");
        }

        // Check for existing resource
        auto it = _resources.find(assetName);
        if (it != _resources.end())
        {
            return it->second;
        }

        // Resolve loader first.
        ResourceLoader* loader = GetLoader(type);
        if (!loader)
        {
            //ALIMER_LOGERROR("Could not load unknown resource type {}, no loader found.", string(type).c_str());
            return nullptr;
        }

        string path;
        string fileName;
        string extension;
        SplitPath(assetName, path, fileName, extension, true);
        
        string assetPath = SanitateResourceName(assetName);
        if (!loader->CanLoad(extension))
        {
            // Find asset importer and process it.
            string convertedAssetFullPath = path + fileName + ".alb";
            auto stream = OpenStream(convertedAssetFullPath);
            if (!stream)
            {
                // Asset conversion is needed.
#ifdef ALIMER_TOOLS
                /*AssetImporter* importer = nullptr;
                for (size_t i = 0; i < _assetImporters.size(); ++i)
                {
                    if (_assetImporters[i]->IsExtensionSupported(extension))
                    {
                        importer = _assetImporters[i];
                        break;
                    }
                }

                if (importer)
                {
                    ALIMER_LOGINFO("Importing asset '{}'", assetName);
                    UniquePtr<Stream> source = OpenStream(assetPath);
                    importer->Import(*source, convertedAssetFullPath, nullptr);
                }*/
#endif

                return nullptr;
            }
        }
        
        auto stream = OpenStream(assetPath);
        if (!stream)
        {
            return nullptr;
        }

        ALIMER_LOGDEBUG("Loading resource '{}'", assetPath);
        SharedPtr<Object> object = loader->Load(*stream);

        if (object.IsNull())
        {
            ALIMER_LOGERROR("Failed to load resource '{}'", assetPath);
            return nullptr;
        }

        //newResource->SetName(sanitatedName);
        // Store to cache
        _resources.insert(std::make_pair(assetName, object));
        return object;
    }

    string ResourceManager::SanitateResourceName(const string& name) const
    {
        // Sanitate unsupported constructs from the resource name
        string sanitatedName = StringUtils::Replace(name, "../", "");
        sanitatedName = StringUtils::Replace(sanitatedName, "./", "");

        // If the path refers to one of the resource directories, normalize the resource name
        if (_resourceDirs.size() > 0)
        {
            string namePath = FileSystem::GetPath(sanitatedName);
            string exePath = StringUtils::Replace(FileSystem::GetExecutableFolder(), "/./", "/");
            for (size_t i = 0; i < _resourceDirs.size(); ++i)
            {
                string relativeResourcePath = _resourceDirs[i];
                if (StringUtils::StartsWith(relativeResourcePath, exePath))
                {
                    relativeResourcePath = relativeResourcePath.substr(exePath.length());
                }

                if (StringUtils::StartsWith(namePath, _resourceDirs[i], false))
                {
                    namePath = namePath.substr(_resourceDirs[i].length());
                }
                else if (StringUtils::StartsWith(namePath, relativeResourcePath, false))
                {
                    namePath = namePath.substr(relativeResourcePath.length());
                }
            }

            sanitatedName = namePath + FileSystem::GetFileNameAndExtension(sanitatedName);
        }

        return StringUtils::Trim(sanitatedName);
    }

    string ResourceManager::SanitateResourceDirName(const string& name) const
    {
        string cleanName = AddTrailingSlash(name);
        if (!IsAbsolutePath(name))
            cleanName = Path::Join(FileSystem::GetCurrentDir(), name);

        // Sanitate away /./ construct
        cleanName = StringUtils::Trim(StringUtils::Replace(cleanName, "/./", "/"));
        return cleanName;
    }

    unique_ptr<Stream> ResourceManager::SearchResourceDirs(const string& name)
    {
        for (size_t i = 0; i < _resourceDirs.size(); ++i)
        {
            if (FileSystem::FileExists(_resourceDirs[i] + name))
            {
                return make_unique<FileStream>(_resourceDirs[i] + name);
            }
        }

        // Fallback using absolute path
        if (FileSystem::FileExists(name))
        {
            return make_unique<FileStream>(name);
        }

        return {};
    }

    unique_ptr<Stream> ResourceManager::SearchPackages(const string& name)
    {
        ALIMER_UNUSED(name);
        // TODO:
        return {};
    }

    bool ResourceManager::ExistsInResourceDirs(const string& name)
    {
        for (size_t i = 0; i < _resourceDirs.size(); ++i)
        {
            if (FileSystem::FileExists(_resourceDirs[i] + name))
            {
                return true;
            }
        }

        // Fallback using absolute path
        if (FileSystem::FileExists(name))
            return true;

        return false;
    }

    bool ResourceManager::ExistsInPackages(const string& name)
    {
        ALIMER_UNUSED(name);
        // TODO:
        return false;
    }

    void ResourceManager::Register()
    {
        static bool registered = false;
        if (registered)
            return;
        registered = true;

        Image::RegisterObject();
    }
}
