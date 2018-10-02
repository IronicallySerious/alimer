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

namespace Alimer
{
    enum class CompilerShaderLang : uint32_t
    {
        GLSL,
        GLES,
        HLSL,
        METAL
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

    struct CompilerStageOptions
    {
        std::string file;
        std::string entryPoint = "main";
    };

    class CompilerOptions
    {
    public:
        CompilerShaderLang language = CompilerShaderLang::GLSL;
        bool invertY = false;
        bool preprocess = false;
        std::unordered_map<std::string, std::string> defines;
        CompilerStageOptions shaders[static_cast<uint32_t>(CompilerShaderStage::Count)] = {};
        std::vector<std::string> includeDirs;
        std::string outputFile;
    };

    class Compiler final 
    {
    public:
        Compiler();
        ~Compiler();

        bool Compile(const CompilerOptions& options);
    };
}
