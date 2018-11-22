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
#include <stdint.h>
#include <string>
#include <vector>
#include <unordered_map>

#define COMPILER_VERSION_MAJOR 0
#define COMPILER_VERSION_MINOR 9
#define COMPILER_VERSION_PATCH 0

namespace Alimer
{
    enum class CompilerShaderLang : uint32_t
    {
        DEFAULT,
        GLSL,
        GLES,
        HLSL,
        METAL,
        SPIRV
    };

    enum class CompilerShaderFormat : uint32_t
    {
        DEFAULT,
        BLOB,
        C_HEADER
    };

    enum class CompilerShaderStage : uint32_t
    {
        Vertex = 0,
        TessControl = 1,
        TessEvaluation = 2,
        Geometry = 3,
        Fragment = 4,
        Compute = 5,
        Count
    };

    class CompilerOptions
    {
    public:
        std::string inputFile;
        std::string entryPoint = "main";
        bool invertY = false;
        bool preprocess = false;
        std::unordered_map<std::string, std::string> defines;
        std::vector<std::string> includeDirs;
        std::string outputFile;
        CompilerShaderLang language = CompilerShaderLang::DEFAULT;
        CompilerShaderFormat format = CompilerShaderFormat::DEFAULT;
    };

    /**
    * Defines class for compiling shaders.
    */
    class Compiler final
    {
    public:
        Compiler();
        ~Compiler();
        Compiler(const Compiler&) = delete;
        Compiler& operator=(const Compiler&) = delete;
        Compiler(const Compiler&&) = delete;
        Compiler& operator=(const Compiler&&) = delete;

        bool Compile(const CompilerOptions& options);

        const std::string& GetErrorMessage() const
        {
            return _errorMessage;
        }

    private:
        std::string _errorMessage;
    };
}
