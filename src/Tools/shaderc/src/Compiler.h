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

#include <functional>
#include <string>
#include <vector>

#define COMPILER_VERSION_MAJOR 0
#define COMPILER_VERSION_MINOR 9
#define COMPILER_VERSION_PATCH 0

namespace ShaderCompiler
{
    enum class ShaderStage : uint32_t
    {
        Vertex = 0,
        Hull = 1,
        Domain = 2,
        Geometry = 3,
        Pixel = 4,
        Compute = 5,
        Count
    };

    enum class CompilerShaderFormat : uint32_t
    {
        DEFAULT,
        BLOB,
        C_HEADER
    };

    enum class ShadingLanguage : uint32_t
    {
        DXIL = 0,
        SPIRV,
        DXC,

        HLSL,
        GLSL,
        ESSL,
        MSL,

        COUNT,
    };

    struct MacroDefine
    {
        std::string name;
        std::string value;
    };

    struct Shader
    {
        ShaderStage stage;
        std::string entry;
    };

    /**
    * Defines class for compiling shaders.
    */
    class Compiler final
    {
    public:
        struct Options
        {
            std::string source;
            std::string fileName;
            std::vector<Shader> shaders;
            std::vector<MacroDefine> defines;
            std::function<std::string(const std::string& includeName)> loadIncludeCallback;
            ShadingLanguage targetLanguage;
        };

        struct CompileResult
        {
            std::vector<uint8_t> bytecode;
            bool isText;

            std::string errorWarningMsg;
            bool hasError;
        };

        static CompileResult Compile(Options options);
    };
}
