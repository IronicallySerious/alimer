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

#include "../Base/String.h"
#include "../Base/StringHash.h"
#include "../IO/FileSystem.h"
#include "../Resource/ResourceLoader.h"
#include <mutex>
#include <atomic>
#include <map>
#include <utility>

namespace alimer
{
    /// Sets to priority so that a package or file is pushed to the end of the vector.
    static constexpr uint32_t PRIORITY_LAST = 0xffffffff;

	/// Resource cache subsystem. Loads resources on demand and stores them for later access.
	class ALIMER_API ResourceManager final : public Object
	{
        ALIMER_OBJECT(ResourceManager, Object);

	public:
		/// Constructor.
		ResourceManager();

		/// Destructor.
		~ResourceManager();

        bool AddResourceDir(const String& assetName, uint32_t priority = PRIORITY_LAST);

        void AddLoader(ResourceLoader* loader);
        ResourceLoader* GetLoader(StringHash type) const;

        UniquePtr<Stream> OpenStream(const String& assetName);

        SharedPtr<Object> Load(StringHash type, const String& assetName);

		template <class T> SharedPtr<T> Load(const String& assetName)
		{
            static_assert(std::is_base_of<Object, T>(), "T is not a resource thus cannot load");
			return StaticCast<T>(Load(T::GetTypeStatic(), assetName));
		}

        /// Remove unsupported constructs from the resource name to prevent ambiguity, and normalize absolute filename to resource path relative if possible.
        String SanitateResourceName(const String& name) const;

        /// Remove unnecessary constructs from a resource directory name and ensure it to be an absolute path.
        String SanitateResourceDirName(const String& name) const;

	private:
        /// Register object.
        static void Register();

        /// Search FileSystem for file.
        UniquePtr<Stream> SearchResourceDirs(const String& name);
        /// Search resource packages for file.
        UniquePtr<Stream> SearchPackages(const String& name);

        /// Search FileSystem for file.
        bool ExistsInResourceDirs(const String& name);

        /// Search resource packages for file.
        bool ExistsInPackages(const String& name);

        /// Mutex for thread-safe access to the resource directories, resource packages and resource dependencies.
        mutable std::mutex _resourceMutex;

        /// Resource load directories.
        Vector<String> _resourceDirs;

        std::unordered_map<StringHash, UniquePtr<ResourceLoader>> _loaders;
		std::map<String, SharedPtr<Object>> _resources;

        /// Search priority flag.
        bool _searchPackagesFirst{ true };
	};
}
