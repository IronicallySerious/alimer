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
#include "../IO/FileSystem.h"
#include "../IO/Path.h"
#include "../Util/Util.h"
#include "../Core/Log.h"
using namespace std;

namespace Alimer
{
	ResourceManager::ResourceManager()
	{
#ifdef ALIMER_ASSET_PIPELINE
		_dataDirectory = FileSystem::Get().GetProtocol("assets")->GetPath();
		_dataDirectory += '_';
		_dataDirectory += "windows";
#else
		_dataDirectory = FileSystem::Get().GetProtocol("assets")->GetPath();
#endif
	}

	ResourceManager::~ResourceManager()
	{
	}

	ResourcePtr ResourceManager::LoadResource(const std::string& assetName)
	{
		auto paths = Path::ProtocolSplit(assetName);
		string fullPath = Path::Join(_dataDirectory, paths.second);
		string compiledAssetName = fullPath + ".alb";

		if (!FileSystem::Get().FileExists(compiledAssetName))
		{
			if (!FileSystem::Get().FileExists(assetName))
			{
				return nullptr;
			}
		}

		auto b = FileSystem::Get().FileExists(assetName);
		return nullptr;
	}
}
