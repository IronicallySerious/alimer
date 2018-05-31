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
#include <string>
#include <atomic>

namespace Alimer
{
	/// Resource cache subsystem. Loads resources on demand and stores them for later access.
	class ResourceManager final
	{
	public:
		/// Constructor.
		ResourceManager();

		/// Destructor.
		~ResourceManager();

		ResourcePtr LoadResource(const std::string& assetName);

		template <class T> std::shared_ptr<T> Load(const std::string& assetName)
		{
			return std::static_pointer_cast<T>(LoadResource(assetName));
		}

	private:
		std::string _dataDirectory;

		DISALLOW_COPY_MOVE_AND_ASSIGN(ResourceManager);
	};

	// Direct access to ResourceManager module.
	extern ResourceManager* resources;
}
