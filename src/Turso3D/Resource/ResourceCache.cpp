//
// Alimer is based on the Turso3D codebase.
// Copyright (c) 2018-2019 Amer Koleci and contributors.
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

#include "../Debug/Log.h"
#include "../Debug/Profiler.h"
#include "../IO/File.h"
#include "../IO/FileSystem.h"
#include "Image.h"
#include "JSONFile.h"
#include "ResourceCache.h"

#include "../Debug/DebugNew.h"

namespace Turso3D
{

    ResourceCache::ResourceCache()
    {
        RegisterSubsystem(this);
    }

    ResourceCache::~ResourceCache()
    {
        UnloadAllResources(true);
        RemoveSubsystem(this);
    }

    bool ResourceCache::AddResourceDir(const String& pathName, bool addFirst)
    {
        TURSO3D_PROFILE(AddResourceDir);

        if (!DirExists(pathName))
        {
            TURSO3D_LOGERROR("Could not open directory " + pathName);
            return false;
        }

        String fixedPath = SanitateResourceDirName(pathName);

        // Check that the same path does not already exist
        for (size_t i = 0; i < resourceDirs.Size(); ++i)
        {
            if (!resourceDirs[i].Compare(fixedPath, false))
                return true;
        }

        if (addFirst)
            resourceDirs.Insert(0, fixedPath);
        else
            resourceDirs.Push(fixedPath);

        TURSO3D_LOGINFO("Added resource path " + fixedPath);
        return true;
    }

    bool ResourceCache::AddManualResource(Resource* resource)
    {
        if (!resource)
        {
            TURSO3D_LOGERROR("Null manual resource");
            return false;
        }

        if (resource->Name().IsEmpty())
        {
            TURSO3D_LOGERROR("Manual resource with empty name, can not add");
            return false;
        }

        resources[MakePair(resource->Type(), StringHash(resource->Name()))] = resource;
        return true;
    }

    void ResourceCache::RemoveResourceDir(const String& pathName)
    {
        // Convert path to absolute
        String fixedPath = SanitateResourceDirName(pathName);

        for (size_t i = 0; i < resourceDirs.Size(); ++i)
        {
            if (!resourceDirs[i].Compare(fixedPath, false))
            {
                resourceDirs.Erase(i);
                TURSO3D_LOGINFO("Removed resource path " + fixedPath);
                return;
            }
        }
    }

    void ResourceCache::UnloadResource(StringHash type, const String& name, bool force)
    {
        auto key = MakePair(type, StringHash(name));
        auto it = resources.Find(key);
        if (it == resources.End())
            return;

        Resource* resource = it->second;
        if (resource->Refs() == 1 || force)
            resources.Erase(key);
    }

    void ResourceCache::UnloadResources(StringHash type, bool force)
    {
        // In case resources refer to other resources, repeat until there are no further unloads
        for (;;)
        {
            size_t unloaded = 0;

            for (auto it = resources.Begin(); it != resources.End();)
            {
                auto current = it++;
                if (current->first.first == type)
                {
                    Resource* resource = current->second;
                    if (resource->Refs() == 1 || force)
                    {
                        resources.Erase(current);
                        ++unloaded;
                    }
                }
            }

            if (!unloaded)
                break;
        }
    }

    void ResourceCache::UnloadResources(StringHash type, const String& partialName, bool force)
    {
        // In case resources refer to other resources, repeat until there are no further unloads
        for (;;)
        {
            size_t unloaded = 0;

            for (auto it = resources.Begin(); it != resources.End();)
            {
                auto current = it++;
                if (current->first.first == type)
                {
                    Resource* resource = current->second;
                    if (resource->Name().StartsWith(partialName) && (resource->Refs() == 1 || force))
                    {
                        resources.Erase(current);
                        ++unloaded;
                    }
                }
            }

            if (!unloaded)
                break;
        }
    }

    void ResourceCache::UnloadResources(const String& partialName, bool force)
    {
        // In case resources refer to other resources, repeat until there are no further unloads
        for (;;)
        {
            size_t unloaded = 0;

            for (auto it = resources.Begin(); it != resources.End();)
            {
                auto current = it++;
                Resource* resource = current->second;
                if (resource->Name().StartsWith(partialName) && (!resource->Refs() == 1 || force))
                {
                    resources.Erase(current);
                    ++unloaded;
                }
            }

            if (!unloaded)
                break;
        }
    }

    void ResourceCache::UnloadAllResources(bool force)
    {
        // In case resources refer to other resources, repeat until there are no further unloads
        for (;;)
        {
            size_t unloaded = 0;

            for (auto it = resources.Begin(); it != resources.End();)
            {
                auto current = it++;
                Resource* resource = current->second;
                if (resource->Refs() == 1 || force)
                {
                    resources.Erase(current);
                    ++unloaded;
                }
            }

            if (!unloaded)
                break;
        }
    }

    bool ResourceCache::ReloadResource(Resource* resource)
    {
        if (!resource)
            return false;

        AutoPtr<Stream> stream = OpenResource(resource->Name());
        return stream ? resource->Load(*stream) : false;
    }

    AutoPtr<Stream> ResourceCache::OpenResource(const String& nameIn)
    {
        String name = SanitateResourceName(nameIn);
        AutoPtr<Stream> ret;

        for (size_t i = 0; i < resourceDirs.Size(); ++i)
        {
            if (FileExists(resourceDirs[i] + name))
            {
                // Construct the file first with full path, then rename it to not contain the resource path,
                // so that the file's name can be used in further OpenResource() calls (for example over the network)
                ret = new File(resourceDirs[i] + name);
                break;
            }
        }

        // Fallback using absolute path
        if (!ret)
            ret = new File(name);

        if (!ret->IsReadable())
        {
            TURSO3D_LOGERROR("Could not open resource file " + name);
            ret.Reset();
        }

        return ret;
    }

    Resource* ResourceCache::LoadResource(StringHash type, const String& nameIn)
    {
        String name = SanitateResourceName(nameIn);

        // If empty name, return null pointer immediately without logging an error
        if (name.IsEmpty())
            return nullptr;

        // Check for existing resource
        auto key = MakePair(type, StringHash(name));
        auto it = resources.Find(key);
        if (it != resources.End())
            return it->second;

        SharedPtr<Object> newObject = Create(type);
        if (!newObject)
        {
            TURSO3D_LOGERROR("Could not load unknown resource type " + String(type));
            return nullptr;
        }
        Resource* newResource = dynamic_cast<Resource*>(newObject.Get());
        if (!newResource)
        {
            TURSO3D_LOGERROR("Type " + String(type) + " is not a resource");
            return nullptr;
        }

        // Attempt to load the resource
        AutoPtr<Stream> stream = OpenResource(name);
        if (!stream)
            return nullptr;

        TURSO3D_LOGDEBUGF("Loading resource %s", name);
        newResource->SetName(name);
        if (!newResource->Load(*stream))
            return nullptr;

        // Store to cache
        resources[key] = newResource;
        return newResource;
    }

    void ResourceCache::ResourcesByType(Vector<Resource*>& result, StringHash type) const
    {
        result.Clear();

        for (auto it = resources.Begin(); it != resources.End(); ++it)
        {
            if (it->second->Type() == type)
                result.Push(it->second);
        }
    }

    bool ResourceCache::Exists(const String& nameIn) const
    {
        String name = SanitateResourceName(nameIn);

        for (size_t i = 0; i < resourceDirs.Size(); ++i)
        {
            if (FileExists(resourceDirs[i] + name))
                return true;
        }

        // Fallback using absolute path
        return FileExists(name);
    }

    String ResourceCache::ResourceFileName(const String& name) const
    {
        for (unsigned i = 0; i < resourceDirs.Size(); ++i)
        {
            if (FileExists(resourceDirs[i] + name))
                return resourceDirs[i] + name;
        }

        return String();
    }

    String ResourceCache::SanitateResourceName(const String& nameIn) const
    {
        // Sanitate unsupported constructs from the resource name
        String name = NormalizePath(nameIn);
        name.Replace("../", "");
        name.Replace("./", "");

        // If the path refers to one of the resource directories, normalize the resource name
        if (resourceDirs.Size())
        {
            String namePath = Path(name);
            String exePath = ExecutableDir();
            for (unsigned i = 0; i < resourceDirs.Size(); ++i)
            {
                String relativeResourcePath = resourceDirs[i];
                if (relativeResourcePath.StartsWith(exePath))
                    relativeResourcePath = relativeResourcePath.Substring(exePath.Length());

                if (namePath.StartsWith(resourceDirs[i], false))
                    namePath = namePath.Substring(resourceDirs[i].Length());
                else if (namePath.StartsWith(relativeResourcePath, false))
                    namePath = namePath.Substring(relativeResourcePath.Length());
            }

            name = namePath + FileNameAndExtension(name);
        }

        return name.Trimmed();
    }

    String ResourceCache::SanitateResourceDirName(const String& nameIn) const
    {
        // Convert path to absolute
        String fixedPath = AddTrailingSlash(nameIn);
        if (!IsAbsolutePath(fixedPath))
            fixedPath = CurrentDir() + fixedPath;

        // Sanitate away /./ construct
        fixedPath.Replace("/./", "/");

        return fixedPath.Trimmed();
    }

    void RegisterResourceLibrary()
    {
        static bool registered = false;
        if (registered)
            return;
        registered = true;

        Image::RegisterObject();
        JSONFile::RegisterObject();
    }

}
