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

#include "../Base/Cache.h"
#include "../Graphics/Shader.h"
#include <mutex>
#include <vector>

namespace Alimer
{
    class GraphicsDevice;

    
    class ALIMER_API ShaderManager final
    {
    public:
        ShaderManager(GraphicsDevice* device)
            : _device(device)
        {
        }

        ~ShaderManager();

        ShaderProgram* RegisterGraphics(const std::string &vertex, const std::string &fragment);

    private:
        ShaderTemplate* GetTemplate(const std::string &source);

    private:
        GraphicsDevice* _device;

        Cache<ShaderTemplate> _templates;
        Cache<ShaderProgram> _programs;

#ifdef ALIMER_THREADING
        std::mutex _lock;
#endif
    };

	/// Class for shader compilation support.
	namespace ShaderCompiler
	{
		ALIMER_API bool Compile(const String& filePath, const String& entryPoint, std::vector<uint32_t>& spirv, String& infoLog);
        ALIMER_API bool Compile(ShaderStage stage, const String& source, const String& entryPoint, std::vector<uint32_t>& spirv, const String& filePath, String& infoLog);
	}
}
