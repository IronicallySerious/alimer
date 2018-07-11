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

#pragma once

#include "../IO/FileSystem.h"
#include "../Resource/ResourceLoader.h"
#include <mutex>
#include <atomic>
#include <string>
#include <vector>
#include <map>

namespace Alimer
{
    /// Sets to priority so that a package or file is pushed to the end of the vector.
    static constexpr uint32_t PRIORITY_LAST = 0xffffffff;

	/// Resource cache subsystem. Loads resources on demand and stores them for later access.
	class ALIMER_API ResourceManager final
	{
	public:
		/// Constructor.
		ResourceManager();

		/// Destructor.
		~ResourceManager();

        /// Add a resource load directory. Optional priority parameter which will control search order.
        bool AddResourceDir(const std::string& assetName, uint32_t priority = PRIORITY_LAST);

        std::unique_ptr<Stream> Open(const std::string &path, StreamMode mode = StreamMode::ReadOnly);

        SharedPtr<Resource> LoadResource(const std::string& assetName);

		template <class T> SharedPtr<T> Load(const std::string& assetName)
		{
			return StaticCast<T>(LoadResource(assetName));
		}

        /// Remove unsupported constructs from the resource name to prevent ambiguity, and normalize absolute filename to resource path relative if possible.
        std::string SanitateResourceName(const std::string& name) const;

        /// Remove unnecessary constructs from a resource directory name and ensure it to be an absolute path.
        std::string SanitateResourceDirName(const std::string& name) const;

	private:
        /// Search FileSystem for file.
        std::unique_ptr<Stream> SearchResourceDirs(const std::string& name);
        /// Search resource packages for file.
        std::unique_ptr<Stream> SearchPackages(const std::string& name);

        /// Mutex for thread-safe access to the resource directories, resource packages and resource dependencies.
        mutable std::mutex _resourceMutex;

        /// Resource load directories.
        std::vector<std::string> _resourceDirs;

		std::map<std::string, SharedPtr<Resource>> _resources;

        /// Search priority flag.
        bool _searchPackagesFirst{ true };

    private:
		DISALLOW_COPY_MOVE_AND_ASSIGN(ResourceManager);
	};

    /// Access to current resource manager instance.
    ALIMER_API ResourceManager* gResources();
}
