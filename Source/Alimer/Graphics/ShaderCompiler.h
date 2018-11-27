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

#if TODO_SHADER_COMPILER
#include "../Graphics/Shader.h"
#include <mutex>
#include <vector>

namespace Alimer
{
    enum class ShaderLanguage
    {
        GLSL,
        HLSL
    };

	/// Class for shader compilation support.
	class ALIMER_API ShaderCompiler final
	{
    public:
        ShaderCompiler() = default;

        ShaderBlob Compile(
            const char* filePath,
            const char* entryPoint,
            ShaderLanguage language,
            const std::vector<std::pair<std::string, int>> &defines = {});
        ShaderBlob Compile(
            const char* source,
            const char* entryPoint,
            ShaderLanguage language,
            ShaderStage stage,
            const char* filePath = nullptr,
            const std::vector<std::pair<std::string, int>> &defines = {});

        const std::string &get_error_message() const
        {
            return _errorMessage;
        }

    private:
        std::string _errorMessage;

        DISALLOW_COPY_MOVE_AND_ASSIGN(ShaderCompiler);
    };
}

#endif /* TODO_SHADER_COMPILER */
