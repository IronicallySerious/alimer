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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4458) 
#endif

#include "CLI11.hpp"
#include "Compiler.h"
#include <iostream>

#define ALIMER_SHADERC_VERSION_MAJOR 0
#define ALIMER_SHADERC_VERSION_MINOR 9

using namespace Alimer;
using namespace std;

int main(int argc, char* argv[])
{
    CLI::App app{ "shaderc, Alimer shader compiler tool, version 0.9.", "shaderc" };

    CompilerOptions options;
    std::string shaderLanguage;
    std::string shaderFormat;

    app.add_option("-i,--input", options.inputFile, "Shader source file")->check(CLI::ExistingFile);
    app.add_option("-o,--output", options.outputFile, "Compiled output file")->required(true);
    app.add_option("-e,--entryPoint", options.entryPoint, "Shader entry point")->required(false);
    app.add_option("-l,--lang", shaderLanguage, "Shader output language")->required(false);
    app.add_option("-f,--format", shaderFormat, "Shader output format")->required(false);

    try {
        app.parse(argc, argv);
    }
    catch (const CLI::ParseError &e) {
        return app.exit(e);
    }

    if (!shaderLanguage.empty())
    {
        if (shaderLanguage == "hlsl")
        {
            options.language = CompilerShaderLang::HLSL;
        }
        else if (shaderLanguage == "glsl")
        {
            options.language = CompilerShaderLang::GLSL;
        }
        else if (shaderLanguage == "gles" || shaderLanguage == "es")
        {
            options.language = CompilerShaderLang::GLES;
        }
        else if (shaderLanguage == "msl" || shaderLanguage == "metal")
        {
            options.language = CompilerShaderLang::METAL;
        }
        else if (shaderLanguage == "spirv" || shaderLanguage == "vulkan")
        {
            options.language = CompilerShaderLang::SPIRV;
        }
    }

    if (!shaderFormat.empty())
    {
        if (shaderFormat == "blob")
        {
            options.format = CompilerShaderFormat::BLOB;
        }
        else if (shaderFormat == "header")
        {
            options.format = CompilerShaderFormat::C_HEADER;
        }
    }

    Compiler compiler;
    if (!compiler.Compile(options))
    {
        auto errorMessage = compiler.GetErrorMessage();
        fprintf(stderr, "%s\n", errorMessage.c_str());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
