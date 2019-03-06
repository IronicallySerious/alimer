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

#include <string>
#include <memory>
#include <mutex>
#include <atomic>
#include <map>
#include <utility>
#include <unordered_map>
#include "../Base/StringHash.h"
#include "../IO/FileSystem.h"
#include "../Resource/Resource.h"
#include "../Resource/ResourceLoader.h"

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

        void AddLoader(ResourceLoader* loader);
        ResourceLoader* GetLoader(StringHash type) const;

        std::unique_ptr<Stream> OpenStream(const std::string& assetName);

        SharedPtr<Resource> Load(const TypeInfo* type, const std::string& assetName, bool sendEventOnFailure = true);

		template <class T> SharedPtr<T> Load(const std::string& assetName, bool sendEventOnFailure = true)
		{
            static_assert(std::is_base_of<Resource, T>(), "T is not a resource thus cannot load");
			return StaticCast<T>(Load(T::GetTypeInfoStatic(), assetName, sendEventOnFailure));
		}

	private:
        /// Register object.
        static void Register();

        std::string _rootDirectory;

        std::unordered_map<StringHash, std::unique_ptr<ResourceLoader>> _loaders;
		std::map<std::string, SharedPtr<Resource>> _resources;

        /// Search priority flag.
        bool _searchPackagesFirst = true ;

        /// Return failed resources flag.
        bool _returnFailedResources = false;
	};
}
