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
            _rootDirectory = "assets";
        }
#ifdef ALIMER_DEFAULT_ASSETS_DIRECTORY
        else
        {

            const char *assetsDir = ALIMER_DEFAULT_ASSETS_DIRECTORY;
            if (assetsDir
                && FileSystem::DirectoryExists(assetsDir))
            {
                _rootDirectory = assetsDir;
            }
        }
#endif // ALIMER_DEFAULT_ASSETS_DIRECTORY

        AddSubsystem(this);
    }

    ResourceManager::~ResourceManager()
    {
        RemoveSubsystem(this);
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
        string path;
        string fileName;
        string extension;
        SplitPath(assetName, path, fileName, extension, true);

        string cleanPath = Path::Join(_rootDirectory, fileName + ".alb");
        if (FileSystem::FileExists(cleanPath)) {
            return FileSystem::Get().Open(cleanPath, FileAccess::ReadOnly);
        }
    }

    SharedPtr<Resource> ResourceManager::Load(const TypeInfo* type, const string& assetName, bool sendEventOnFailure)
    {
        ALIMER_ASSERT_MSG(type, "Invalid TypeInfo for loading");
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

        SharedPtr<Resource> newResource;
        // Make sure the pointer is non-null and is a Resource subclass
        newResource = DynamicCast<Resource>(CreateObject(type->GetType()));

        auto stream = OpenStream(assetName);
        if (!stream)
        {
            return nullptr;
        }

        if (!newResource)
        {
            // Resolve loader first.
            ResourceLoader* loader = GetLoader(type->GetType());
            if (!loader)
            {
                ALIMER_LOGERROR("Could not load unknown resource type '{}', no loader found.", type->GetTypeName());

                if (sendEventOnFailure)
                {
                }

                return nullptr;
            }


            string extension = Path::GetExtension(assetName);
            if (!loader->CanLoad(extension))
            {
            }

            return nullptr;
        }

#if TODO
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


        SharedPtr<Object> object = loader->Load(*stream);

        if (object.IsNull())
        {
            ALIMER_LOGERROR("Failed to load resource '{}'", assetPath);
            return nullptr;
        }
#endif // TODO

        ALIMER_LOGDEBUG("Loading resource '{}'", assetName);

        if (!newResource->Load(*stream))
        {
            // Error should already been logged by corresponding resource descendant class
            if (sendEventOnFailure)
            {
                // TODO: Fire event.
            }

            if (!_returnFailedResources) {
                return nullptr;
            }
        }

        // Set name and store to cache
        newResource->SetName(stream->GetName());
        _resources.insert(std::make_pair(assetName, newResource));
        return newResource;
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
